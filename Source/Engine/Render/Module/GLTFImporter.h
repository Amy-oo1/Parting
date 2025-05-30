#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(BindingCache)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "ThirdParty/ShaderMake/include/ShaderMake/ShaderBlob.h"
#include "ThirdParty/taskflow/taskflow.hpp"


#if defined (_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996) // Silence warning from tinyEXR
#endif
#define CGLTF_IMPLEMENTATION
#include "ThirdParty/cgltf/cgltf.h"
#if defined (_MSC_VER)
#pragma warning(pop)
#endif

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/VFS/Module/VFS.h"
#include "Core/Logger/Module/Logger.h"

#include "Engine/Render/Module/SceneGraph-Ini.h"
#include "Engine/Render/Module/Camera.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	class BufferRegionBlob final : public IBlob {
	public:
		BufferRegionBlob(const std::shared_ptr<IBlob>& parent, Uint64 offset, Uint64 size) :
			IBlob{},
			m_Parent{ parent },
			m_Data{ static_cast<const Uint8*>(parent->Get_Data()) + offset },
			m_Size{ size } {
		}
		~BufferRegionBlob(void) = default;

	public:

		STDNODISCARD const void* Get_Data(void) const override { return this->m_Data; }
		STDNODISCARD Uint64 Get_Size(void) const override { return this->m_Size; }

	private:
		SharedPtr<IBlob> m_Parent;
		const void* m_Data;
		Uint64 m_Size;

	};



	template<RHI::APITagConcept APITag>
	class GLTFImporter final {
	public:
		explicit GLTFImporter(SharedPtr<IFileSystem> fs) :
			m_FS{ MoveTemp(fs) } {

		}
		~GLTFImporter(void) = default;
	public:

		bool Load(const Path& fileName, TextureCache<APITag>& textureCache, SceneLoadingStats& stats, tf::Executor* executor, SceneImportResult<APITag>& result);



	private:
		SharedPtr<IFileSystem> m_FS;
	};


	struct cgltf_vfs_context final {
		SharedPtr<IFileSystem> Fs;
		Vector<SharedPtr<IBlob>> Blobs;
	};


	cgltf_result cgltf_read_file_vfs(const cgltf_memory_options* memory_options, const cgltf_file_options* file_options, const char* path, cgltf_size* size, void** data) {
		cgltf_vfs_context* context{ static_cast<cgltf_vfs_context*>(file_options->user_data) };

		auto blob{ context->Fs->ReadFile(path) };

		if (nullptr == blob)
			return cgltf_result_file_not_found;

		context->Blobs.push_back(blob);

		if (nullptr != size)
			*size = blob->Get_Size();
		if (nullptr != data)
			*data = const_cast<void*>(blob->Get_Data());

		return cgltf_result_success;
	}

	void cgltf_release_file_vfs(const cgltf_memory_options*, const cgltf_file_options*, void*) {/*do nothing*/ }

	// glTF only support DDS images through the MSFT_texture_dds extension.
	// Since cgltf does not support this extension, we parse the custom extension string as json here.
	// See https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Vendor/MSFT_texture_dds 
	const cgltf_image* ParseDDSImage(const cgltf_texture* texture, const cgltf_data* objects) {
		for (Uint64 ExtensionIndex = 0; ExtensionIndex < texture->extensions_count; ++ExtensionIndex) {
			const cgltf_extension& ext{ texture->extensions[ExtensionIndex] };

			if (nullptr != ext.name || nullptr != ext.data)
				continue;

			if (strcmp(ext.name, "MSFT_texture_dds") != 0)
				continue;

			Uint64 extensionLength{ strlen(ext.data) };
			if (extensionLength > 1024)
				return nullptr; // safeguard against weird inputs

			jsmn_parser parser;
			jsmn_init(&parser);

			// count the tokens, normally there are 3
			Int32 numTokens{ jsmn_parse(&parser, ext.data, extensionLength, nullptr, 0) };

			// allocate the tokens on the stack
			jsmntok_t* tokens{ static_cast<jsmntok_t*>(_alloca(numTokens * sizeof(jsmntok_t))) };

			// reset the parser and prse
			jsmn_init(&parser);
			Int32 numParsed{ jsmn_parse(&parser, ext.data, extensionLength, tokens, numTokens) };
			if (numParsed != numTokens)
				goto fail;

			if (tokens[0].type != JSMN_OBJECT)
				goto fail; // expecting that the extension is an object

			for (Int32 TokenIndex = 1; TokenIndex < numTokens; ++TokenIndex) {
				if (tokens[TokenIndex].type != JSMN_STRING)
					goto fail; // expecting a string key

				if (cgltf_json_strcmp(tokens + TokenIndex, reinterpret_cast<Uint8*>(ext.data), "source") == 0) {
					++TokenIndex;
					Int32 index{ cgltf_json_to_int(tokens + TokenIndex, reinterpret_cast<Uint8*>(ext.data)) };
					if (index < 0)
						goto fail; // expecting a non-negative integer; non-value results in CGLTF_ERROR_JSON which is negative

					if (static_cast<Uint64>(index) >= objects->images_count) {
						LOG_ERROR("Invalid image index  specified in glTF texture definition"/*, index*/);

						return nullptr;
					}

					return objects->images + index;
				}

				// this was something else - skip it
				TokenIndex = cgltf_skip_json(tokens, TokenIndex);
			}

		fail:
			LOG_ERROR("Failed to parse the DDS glTF extension: {1}"/*, ext.data*/);
			return nullptr;
		}

		return nullptr;
	}


	struct cgltf_subsurface final {
		cgltf_texture_view transmission_color_texture;
		cgltf_float transmission_color[3];
		cgltf_float scattering_color[3];
		cgltf_float scale;
		cgltf_float anisotropy;
	};

	struct cgltf_hair final {
		cgltf_float base_color[3];
		cgltf_float melanin;
		cgltf_float melaninRedness;
		cgltf_float longitudinalRoughness;
		cgltf_float azimuthalRoughness;
		cgltf_float ior;
		cgltf_float cuticleAngle;
		cgltf_float diffuseReflectionWeight;
		cgltf_float diffuse_reflection_tint[3];
	};

	// Parse subsurface scattering extension for glTF material:
	//
	// For setup the SSS properties for material, adding "NV_materials_subsurface" as extension name and setup the following properties:
	// - TransmissionColor: Determines the base color of the SSS surface, it's similar to the diffuse albedo color for diffuse materials. This parameter can also be set with a texture map.
	// - ScatteringColor: Determines the distance (mean free path) that light will be transported inside the SSS object for each color channel. Larger value will allow the corresponding color scattered further on the surface, it will look like a tail extends from the diffuse model.
	// - Scale: A scale that controls the SSS intensity of the whole object.
	// - Anisotropy: Determines the overall scattering direction of the volume phase function, the range is (-1, 1). When this value less then 0, it models backwards scattering. Vice versa, it models forward scattering when the value larger than 0. The volume is isotropic when this value is 0.
	//
	// Example:
	// "extensions": {
	//     "NV_materials_subsurface":{
	//         "TransmissionColor": [1.0, 1.0, 1.0] ,
	//         "ScatteringColor" : [0.856, 0.34, 0.3] ,
	//         "Scale" : 20.0,
	//         "Anisotropy" : -0.5
	//     },
	// }
	Int32 cgltf_parse_json_subsurface(cgltf_options* options, jsmntok_t const* tokens, Int32 i, const Uint8* json_chunk, cgltf_subsurface* out_subsurface) {
		CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
		Int32 size{ tokens[i].size };
		++i;

		for (Int32 j = 0; j < size; ++j) {
			CGLTF_CHECK_KEY(tokens[i]);

			if (cgltf_json_strcmp(tokens + i, json_chunk, "transmissionColorTexture") == 0)
				i = cgltf_parse_json_texture_view(options, tokens, i + 1, json_chunk, &out_subsurface->transmission_color_texture);
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "transmissionColor") == 0)
				i = cgltf_parse_json_float_array(tokens, i + 1, json_chunk, out_subsurface->transmission_color, 3);
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "scatteringColor") == 0)
				i = cgltf_parse_json_float_array(tokens, i + 1, json_chunk, out_subsurface->scattering_color, 3);
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "scale") == 0) {
				++i;
				out_subsurface->scale = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "anisotropy") == 0) {
				++i;
				out_subsurface->anisotropy = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else
				i = cgltf_skip_json(tokens, i + 1);

			if (i < 0)
				return i;
		}

		return i;
	}

	// Parse hair extension for glTF material:
	//
	// For setup the hair properties for material, adding "NV_materials_hair" as extension name and setup the following properties:
	// - baseColor: The color of the hair, only will be used when the absorption model is color based.
	// - melanin: The melanin is a natural substance that gives color to the hair, the range is [0, 1], 0 means no melanin, which makes the hair white; while 1 means maximum melanin, which makes hair black. This only will be used when the hair absorption model is physics based.
	// - melaninRedness: Melanin redness is a parameter that controls the redness of hair by adjusting the ratio of red pheomelanin to brown eumelanin, the range is [0, 1]. This only will be used when the hair absorption model is physics based.
	// - longitudinalRoughness: Roughness on hair longitudinal direction.
	// - azimuthalRoughness: Roughness on hair azimuthal direction.
	// - ior: The index of refraction of the hair volume.
	// - cuticleAngle: The cuticle angle on top of the hair, the larger angle we have, the 2 hair highlight (R and TRT highlights) will be further away from each other. 0 means completely smooth hair on the cuticle.
	// - diffuseReflectionWeight: The weight of diffuse lobe of hair.
	// - diffuseReflectionTint: The tint color of hair.
	//
	// Example:
	// "extensions": {
	//     "NV_materials_hair" : {
	//         "baseColor": [0.227, 0.130, 0.035] ,
	//         "melanin" : 0.6,
	//         "melaninRedness" : 0.0,
	//         "longitudinalRoughness" : 0.354,
	//         "azimuthalRoughness" : 0.6,
	//         "diffuseReflectionWeight" : 0.0,
	//         "ior" : 1.55,
	//         "cuticleAngle" : 3.0,
	//         "diffuseReflectionTint" : [0.02, 0.008, 0.008]
	//     }
	// }
	Int32 cgltf_parse_json_hair(cgltf_options* options, jsmntok_t const* tokens, Int32 i, const uint8_t* json_chunk, cgltf_hair* out_hair) {
		CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
		Int32 size{ tokens[i].size };
		++i;

		for (Int32 j = 0; j < size; ++j) {
			CGLTF_CHECK_KEY(tokens[i]);

			if (cgltf_json_strcmp(tokens + i, json_chunk, "baseColor") == 0)
				i = cgltf_parse_json_float_array(tokens, i + 1, json_chunk, out_hair->base_color, 3);
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "melanin") == 0) {
				++i;
				out_hair->melanin = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "melaninRedness") == 0) {
				++i;
				out_hair->melaninRedness = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "longitudinalRoughness") == 0) {
				++i;
				out_hair->longitudinalRoughness = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "azimuthalRoughness") == 0) {
				++i;
				out_hair->azimuthalRoughness = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "ior") == 0) {
				++i;
				out_hair->ior = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "cuticleAngle") == 0) {
				++i;
				out_hair->cuticleAngle = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "diffuseReflectionWeight") == 0) {
				++i;
				out_hair->diffuseReflectionWeight = cgltf_json_to_float(tokens + i, json_chunk);
				++i;
			}
			else if (cgltf_json_strcmp(tokens + i, json_chunk, "diffuseReflectionTint") == 0)
				i = cgltf_parse_json_float_array(tokens, i + 1, json_chunk, out_hair->diffuse_reflection_tint, 3);
			else
				i = cgltf_skip_json(tokens, i + 1);

			if (i < 0)
				return i;
		}

		return i;
	}

	// Add support for subsurface scattering and hair in glTF
	// Note: SSS and Hair can't be set at the same time on the same material
	template<RHI::APITagConcept APITag>
	const void ParseMaterialExtensions(cgltf_options* options, const cgltf_material* material, Material<APITag>* matInfo) {
		for (Uint64 Index = 0; Index < material->extensions_count; ++Index) {
			const cgltf_extension& ext{ material->extensions[Index] };

			if (nullptr != ext.name || nullptr != ext.data)
				continue;

			if (strcmp(ext.name, "NV_materials_subsurface") != 0 && strcmp(ext.name, "NV_materials_hair") != 0)//TODO msvc hash a err
				continue;

			Uint64 extensionLength{ strlen(ext.data) };//TODO msvc hash a err
			if (extensionLength > 1024)
				return; // safeguard against weird inputs

			jsmn_parser parser;
			jsmn_init(&parser);

			// count the tokens, normally there are 3
			Int32 numTokens{ jsmn_parse(&parser, ext.data, extensionLength, nullptr, 0) };

			// allocate the tokens on the stack
			jsmntok_t* tokens{ static_cast<jsmntok_t*>(_alloca(numTokens * sizeof(jsmntok_t))) };//TODO msvc hash a err

			// reset the parser and parse
			jsmn_init(&parser);
			Int32 numParsed{ jsmn_parse(&parser, ext.data, extensionLength, tokens, numTokens) };
			if (numParsed != numTokens) {
				LOG_ERROR("Failed to parse the glTF material extension: %s"/*, ext.data*/);
				break;
			}

			if (tokens[0].type != JSMN_OBJECT) {
				// expecting that the extension is an object
				LOG_ERROR("Failed to parse the glTF material extension: %s"/*, ext.data*/);
				break;
			}

			const Uint8* json_chunk{ reinterpret_cast<Uint8*>(ext.data) };
			Int32 k{ 0 };
			if (strcmp(ext.name, "NV_materials_subsurface") == 0) {
				matInfo->EnableSubsurfaceScattering = true;
				cgltf_subsurface gltf_subsurface;
				cgltf_parse_json_subsurface(options, tokens, k, json_chunk, &gltf_subsurface);

				matInfo->Subsurface.TransmissionColor = Math::VecF3{ gltf_subsurface.transmission_color[0], gltf_subsurface.transmission_color[1], gltf_subsurface.transmission_color[2] };
				matInfo->Subsurface.ScatteringColor = Math::VecF3{ gltf_subsurface.scattering_color[0], gltf_subsurface.scattering_color[1], gltf_subsurface.scattering_color[2] };
				matInfo->Subsurface.Scale = gltf_subsurface.scale;
				matInfo->Subsurface.Anisotropy = gltf_subsurface.anisotropy;
			}
			else if (strcmp(ext.name, "NV_materials_hair") == 0) {
				matInfo->EnableHair = true;
				cgltf_hair gltf_hair;
				cgltf_parse_json_hair(options, tokens, k, json_chunk, &gltf_hair);

				matInfo->Hair.BaseColor = Math::VecF3{ gltf_hair.base_color[0], gltf_hair.base_color[1], gltf_hair.base_color[2] };
				matInfo->Hair.Melanin = gltf_hair.melanin;
				matInfo->Hair.MelaninRedness = gltf_hair.melaninRedness;
				matInfo->Hair.LongitudinalRoughness = gltf_hair.longitudinalRoughness;
				matInfo->Hair.AzimuthalRoughness = gltf_hair.azimuthalRoughness;
				matInfo->Hair.IOR = gltf_hair.ior;
				matInfo->Hair.CuticleAngle = gltf_hair.cuticleAngle;
				matInfo->Hair.DiffuseReflectionWeight = gltf_hair.diffuseReflectionWeight;
				matInfo->Hair.DiffuseReflectionTint = Math::VecF3{ gltf_hair.diffuse_reflection_tint[0], gltf_hair.diffuse_reflection_tint[1], gltf_hair.diffuse_reflection_tint[2] };
			}
			else
				LOG_ERROR("Failed to parse the glTF material extension: %s"/*, ext.data*/);
		}
	}

	const char* cgltf_error_to_string(cgltf_result res) {
		switch (res) {
		case cgltf_result_success:				return "Success";
		case cgltf_result_data_too_short:		return "Data is too short";
		case cgltf_result_unknown_format:		return "Unknown format";
		case cgltf_result_invalid_json:			return "Invalid JSON";
		case cgltf_result_invalid_gltf:			return "Invalid glTF";
		case cgltf_result_invalid_options:		return "Invalid options";
		case cgltf_result_file_not_found:		return "File not found";
		case cgltf_result_io_error:				return "I/O error";
		case cgltf_result_out_of_memory:		return "Out of memory";
		case cgltf_result_legacy_gltf:			return "Legacy glTF";
		default:								return "Unknown error";
		}
	}

	Pair<const Uint8*, Uint64> cgltf_buffer_iterator(const cgltf_accessor* accessor, Uint64 defaultStride) {
		// TODO: sparse accessor support
		const cgltf_buffer_view* view{ accessor->buffer_view };
		const Uint8* data{ reinterpret_cast<Uint8*>(view->buffer->data) + view->offset + accessor->offset };
		const Uint64 stride{ 0 != view->stride ? view->stride : defaultStride };
		return MakePair(data, stride);
	}


	template<RHI::APITagConcept APITag>
	inline bool GLTFImporter<APITag>::Load(const Path& fileName, TextureCache<APITag>& textureCache, SceneLoadingStats& stats, tf::Executor* executor, SceneImportResult<APITag>& result) {
		// Set this to 'true' if you need to fix broken tangents in a model.
		// Patched buffers will be saved alongside the gltf file, named like "<scene-name>.buffer<N>.bin"
		constexpr bool c_ForceRebuildTangents{ false };

		// Search for a matching .dds file first if loading an uncompressed texture like .png,
		// even if the DDS is not specified in the glTF file.
		constexpr bool c_SearchForDDS{ true };

		result.RootNode.reset();

		cgltf_vfs_context vfsContext{ .Fs{ this->m_FS } };

		cgltf_options options{
			.file {
				.read { &cgltf_read_file_vfs },
				.release{ &cgltf_release_file_vfs },
				.user_data{ &vfsContext }
			}
		};

		String normalizedFileName{ fileName.lexically_normal().generic_string() };

		cgltf_data* objects{ nullptr };
		cgltf_result res{ cgltf_parse_file(&options, normalizedFileName.c_str(), &objects) };
		if (res != cgltf_result_success) {
			LOG_ERROR("Couldn't load glTF file {1}:{2}"/*, normalizedFileName.c_str(), cgltf_error_to_string(res)*/);

			return false;
		}

		res = cgltf_load_buffers(&options, objects, normalizedFileName.c_str());//NOTE : glft or bin this func will parse werr
		if (res != cgltf_result_success) {
			LOG_ERROR("Failed to load buffers for glTF file : "/*, normalizedFileName.c_str(), cgltf_error_to_string(res)*/);
			return false;
		}

		UnorderedMap<const cgltf_image*, SharedPtr<LoadedTexture<APITag>>> textures;

		auto load_texture = [this, &textures, &textureCache, executor, &fileName, objects, &vfsContext, c_SearchForDDS](const cgltf_texture* texture, bool sRGB)->SharedPtr<LoadedTexture<APITag>> {
			if (!texture)
				return nullptr;

			// See if the extensions include a DDS image
			const cgltf_image* ddsImage{ ParseDDSImage(texture, objects) };

			if ((nullptr == texture->image || (nullptr == texture->image->uri && nullptr == texture->image->buffer_view)) &&
				(nullptr == ddsImage || (nullptr == ddsImage->uri && nullptr == ddsImage->buffer_view)))
				return nullptr;

			// Pick either DDS or standard image, prefer DDS
			const cgltf_image* activeImage{
				(nullptr != ddsImage && (nullptr != ddsImage->uri || nullptr != ddsImage->buffer_view)) ?
				ddsImage :
				texture->image
			};

			if (auto it{ textures.find(activeImage) }; it != textures.end())
				return it->second;

			SharedPtr<LoadedTexture<APITag>> loadedTexture;

			if (activeImage->buffer_view) {
				// If the image has inline data, like coming from a GLB container, use that.

				const Uint8* dataPtr{ static_cast<const Uint8*>(activeImage->buffer_view->buffer->data) + activeImage->buffer_view->offset };
				const Uint64 dataSize{ activeImage->buffer_view->size };

				// We need to have a managed pointer to the texture data for async decoding.
				SharedPtr<IBlob> textureData;

				// Try to find an existing file blob that includes our data.
				for (const auto& blob : vfsContext.Blobs) {
					const Uint8* blobData{ static_cast<const Uint8*>(blob->Get_Data()) };
					const Uint64 blobSize{ blob->Get_Size() };

					if (blobData < dataPtr && blobData + blobSize > dataPtr) {
						// Found the file blob - create a range blob out of it and keep a strong reference.
						ASSERT(dataPtr + dataSize <= blobData + blobSize);
						textureData = MakeShared<BufferRegionBlob>(blob, dataPtr - blobData, dataSize);
						break;
					}
				}

				// Didn't find a file blob - copy the data into a new container.
				if (!textureData) {
					void* dataCopy{ malloc(dataSize) };
					ASSERT(dataCopy);
					memcpy(dataCopy, dataPtr, dataSize);
					textureData = MakeShared<Blob>(dataCopy, dataSize);
				}

				Int64 imageIndex{ activeImage - objects->images };
				String name{ activeImage->name ? activeImage->name : fileName.filename().generic_string() + "[" + std::to_string(imageIndex) + "]" };
				String mimeType{ activeImage->mime_type ? activeImage->mime_type : "" };

				if (executor)
					loadedTexture = textureCache.LoadTextureFromMemoryAsync(textureData, name, mimeType, sRGB, *executor);
				else
					loadedTexture = textureCache.LoadTextureFromMemoryDeferred(textureData, name, mimeType, sRGB);
			}
			else {
				// Decode %-encoded characters in the URI, because cgltf doesn't do that for some reason.
				String uri{ activeImage->uri };
				cgltf_decode_uri(uri.data());

				// No inline data - read a file.
				Path filePath{ fileName.parent_path() / uri };

				// Try to replace the texture with DDS, if enabled.
				if (c_SearchForDDS && !ddsImage) {
					Path filePathDDS{ filePath };

					filePathDDS.replace_extension(".dds");

					if (m_FS->FileExists(filePathDDS))
						filePath = filePathDDS;
				}

				if (executor)
					loadedTexture = textureCache.LoadTextureFromFileAsync(filePath, sRGB, *executor);
				else
					loadedTexture = textureCache.LoadTextureFromFileDeferred(filePath, sRGB);
			}

			return textures[activeImage] = loadedTexture;
			};

		UnorderedMap<const cgltf_material*, SharedPtr<Material<APITag>>> materials;

		for (Uint64 mat_idx = 0; mat_idx < objects->materials_count; ++mat_idx) {
			const cgltf_material& material{ objects->materials[mat_idx] };

			auto matinfo{ MakeShared<Material<APITag>>() };
			if (nullptr != material.name)
				matinfo->Name = material.name;
			matinfo->ModelFileName = normalizedFileName;//TDOD :better Name and MimType to do
			matinfo->MaterialIndexInModel = static_cast<Int32>(mat_idx);

			bool useTransmission = false;

			if (material.has_pbr_specular_glossiness) {
				matinfo->UseSpecularGlossModel = true;
				matinfo->BaseOrDiffuseTexture = load_texture(material.pbr_specular_glossiness.diffuse_texture.texture, true);
				matinfo->MetalRoughOrSpecularTexture = load_texture(material.pbr_specular_glossiness.specular_glossiness_texture.texture, true);
				matinfo->BaseOrDiffuseColor = material.pbr_specular_glossiness.diffuse_factor;
				matinfo->SpecularColor = material.pbr_specular_glossiness.specular_factor;
				matinfo->Roughness = 1.f - material.pbr_specular_glossiness.glossiness_factor;
				matinfo->Opacity = material.pbr_specular_glossiness.diffuse_factor[3];
			}
			else if (material.has_pbr_metallic_roughness) {
				matinfo->UseSpecularGlossModel = false;
				matinfo->BaseOrDiffuseTexture = load_texture(material.pbr_metallic_roughness.base_color_texture.texture, true);
				matinfo->MetalRoughOrSpecularTexture = load_texture(material.pbr_metallic_roughness.metallic_roughness_texture.texture, false);
				matinfo->BaseOrDiffuseColor = material.pbr_metallic_roughness.base_color_factor;
				matinfo->Metalness = material.pbr_metallic_roughness.metallic_factor;
				matinfo->Roughness = material.pbr_metallic_roughness.roughness_factor;
				matinfo->Opacity = material.pbr_metallic_roughness.base_color_factor[3];
			}

			if (material.has_transmission) {
				if (material.has_pbr_specular_glossiness) {
					LOG_ERROR("Material  uses the KHR_materials_transmission extension, which is undefined on materials using the "
						"KHR_materials_pbrSpecularGlossiness extension model."/*, material.name ? material.name : "<Unnamed>"*/);
				}

				matinfo->TransmissionTexture = load_texture(material.transmission.transmission_texture.texture, false);
				matinfo->TransmissionFactor = material.transmission.transmission_factor;
				useTransmission = true;
			}

			matinfo->EmissiveTexture = load_texture(material.emissive_texture.texture, true);
			matinfo->EmissiveColor = material.emissive_factor;
			matinfo->EmissiveIntensity = Math::MaxComponent(matinfo->EmissiveColor);
			if (matinfo->EmissiveIntensity > 0.f)
				matinfo->EmissiveColor /= matinfo->EmissiveIntensity;
			else
				matinfo->EmissiveIntensity = 1.f;
			matinfo->NormalTexture = load_texture(material.normal_texture.texture, false);
			matinfo->NormalTextureScale = material.normal_texture.scale;
			matinfo->OcclusionTexture = load_texture(material.occlusion_texture.texture, false);
			matinfo->OcclusionStrength = material.occlusion_texture.scale;
			matinfo->AlphaCutoff = material.alpha_cutoff;
			matinfo->DoubleSided = material.double_sided;

			// Texture transformation extension:
			// Only scaling transformation for normal map texture coordinate is supported in importer.
			// All other transformations(offset, rotation) and all transformations for other textures is ignored.
			// This is for saving memory of material buffer, and the usage for other textures of this extension is very limited.
			matinfo->NormalTextureTransformScale =
				material.normal_texture.has_transform ?
				Math::VecF2{ material.normal_texture.transform.scale[0], material.normal_texture.transform.scale[1] } :
				Math::VecF2{ 1.0f };
			// Log warnings for all unsupported texture coordinate transformations
			if (material.pbr_metallic_roughness.base_color_texture.has_transform ||
				material.pbr_metallic_roughness.metallic_roughness_texture.has_transform ||
				material.pbr_specular_glossiness.diffuse_texture.has_transform ||
				material.pbr_specular_glossiness.specular_glossiness_texture.has_transform ||
				material.occlusion_texture.has_transform ||
				material.emissive_texture.has_transform ||
				(material.normal_texture.has_transform &&
					(material.normal_texture.transform.rotation != 0.0f ||
						material.normal_texture.transform.offset[0] != 0.0f ||
						material.normal_texture.transform.offset[1] != 0.0f))) {
				LOG_ERROR("Material {} uses the KHR_texture_transform extension, which is not supported on non-scale transformations and all textures except normal"/*, material.name ? material.name : "<Unnamed>"*/);
			}

			switch (material.alpha_mode) {
			case cgltf_alpha_mode_opaque: matinfo->Domain = useTransmission ? MaterialDomain::Transmissive : MaterialDomain::Opaque; break;
			case cgltf_alpha_mode_mask: matinfo->Domain = useTransmission ? MaterialDomain::TransmissiveAlphaTested : MaterialDomain::AlphaTested; break;
			case cgltf_alpha_mode_blend: matinfo->Domain = useTransmission ? MaterialDomain::TransmissiveAlphaBlended : MaterialDomain::AlphaBlended; break;
			default: break;
			}

			// Parse SSS and Hair Extensions
			ParseMaterialExtensions(&options, &material, matinfo.get());

			materials[&material] = matinfo;
		}

		Uint64
			totalIndices{ 0 },
			totalVertices{ 0 },
			morphTargetTotalVertices{ 0 };
		bool hasJoints{ false };

		for (Uint64 mesh_idx = 0; mesh_idx < objects->meshes_count; ++mesh_idx) {
			const cgltf_mesh& mesh{ objects->meshes[mesh_idx] };

			for (Uint64 prim_idx = 0; prim_idx < mesh.primitives_count; ++prim_idx) {
				const cgltf_primitive& prim{ mesh.primitives[prim_idx] };

				if ((prim.type != cgltf_primitive_type_triangles &&
					prim.type != cgltf_primitive_type_line_strip &&
					prim.type != cgltf_primitive_type_lines) ||
					prim.attributes_count == 0)
					continue;// only support

				if (nullptr != prim.indices)
					totalIndices += prim.indices->count;
				else
					totalIndices += prim.attributes->data->count;
				totalVertices += prim.attributes->data->count;

				if (!hasJoints)	// Detect if the primitive has joints or weights attributes.
					for (Uint64 attr_idx = 0; attr_idx < prim.attributes_count; ++attr_idx) {
						const cgltf_attribute& attr{ prim.attributes[attr_idx] };
						if (attr.type == cgltf_attribute_type_joints || attr.type == cgltf_attribute_type_weights) {
							hasJoints = true;
							break;
						}
					}
			}
		}

		SharedPtr<BufferGroup<APITag>> buffers{ MakeShared<BufferGroup<APITag>>() };

		buffers->IndexData.resize(totalIndices);
		buffers->PositionData.resize(totalVertices);
		buffers->NormalData.resize(totalVertices);
		buffers->TangentData.resize(totalVertices);
		buffers->Texcoord1Data.resize(totalVertices);
		buffers->RadiusData.resize(totalVertices);
		if (hasJoints) {
			// Allocate joint/weight arrays for all the vertices in the model.
			// This is wasteful in case the model has both skinned and non-skinned meshes; TODO: improve.
			buffers->JointData.resize(totalVertices);
			buffers->WeightData.resize(totalVertices);
		}

		morphTargetTotalVertices = totalVertices;
		totalIndices = 0;
		totalVertices = 0;

		UnorderedMap<const cgltf_mesh*, SharedPtr<MeshInfo<APITag>>> meshMap;

		Vector<Math::VecF3> computedTangents;
		Vector<Math::VecF3> computedBitangents;
		Vector<SharedPtr<MeshInfo<APITag>>> meshes;
		SharedPtr<Material<APITag>> emptyMaterial;

		for (Uint64 mesh_idx = 0; mesh_idx < objects->meshes_count; ++mesh_idx) {
			const cgltf_mesh& mesh{ objects->meshes[mesh_idx] };

			SharedPtr<MeshInfo<APITag>> minfo{ MakeShared<MeshInfo<APITag>>() };
			if (nullptr != mesh.name)
				minfo->Name = mesh.name;
			minfo->Buffers = buffers;
			minfo->IndexOffset = static_cast<Uint32>(totalIndices);
			minfo->VertexOffset = static_cast<Uint32>(totalVertices);
			meshes.push_back(minfo);

			meshMap[&mesh] = minfo;

			Uint64 morphTargetDataCount{ 0 };
			Vector<Vector<Math::VecF3>> morphTargetData;

			for (Uint64 prim_idx = 0; prim_idx < mesh.primitives_count; ++prim_idx) {
				const cgltf_primitive& prim{ mesh.primitives[prim_idx] };

				if ((prim.type != cgltf_primitive_type_triangles &&
					prim.type != cgltf_primitive_type_line_strip &&
					prim.type != cgltf_primitive_type_lines) ||
					prim.attributes_count == 0)
					continue;

				if (prim.type == cgltf_primitive_type_line_strip ||
					prim.type == cgltf_primitive_type_lines)
					minfo->Type = MeshType::CurvePolytubes;

				if (nullptr != prim.indices) {
					ASSERT(prim.indices->component_type == cgltf_component_type_r_32u ||
						prim.indices->component_type == cgltf_component_type_r_16u ||
						prim.indices->component_type == cgltf_component_type_r_8u);
					ASSERT(prim.indices->type == cgltf_type_scalar);
				}

				const cgltf_accessor* positions{ nullptr };
				const cgltf_accessor* normals{ nullptr };
				const cgltf_accessor* tangents{ nullptr };
				const cgltf_accessor* texcoords{ nullptr };
				const cgltf_accessor* joint_weights{ nullptr };
				const cgltf_accessor* joint_indices{ nullptr };
				const cgltf_accessor* radius{ nullptr };

				for (Uint64 attr_idx = 0; attr_idx < prim.attributes_count; ++attr_idx) {
					const cgltf_attribute& attr{ prim.attributes[attr_idx] };

					switch (attr.type) {
					case cgltf_attribute_type_position:
						ASSERT(attr.data->type == cgltf_type_vec3);
						ASSERT(attr.data->component_type == cgltf_component_type_r_32f);
						positions = attr.data;
						break;

					case cgltf_attribute_type_normal:
						ASSERT(attr.data->type == cgltf_type_vec3);
						ASSERT(attr.data->component_type == cgltf_component_type_r_32f);
						normals = attr.data;
						break;

					case cgltf_attribute_type_tangent:
						ASSERT(attr.data->type == cgltf_type_vec4);
						ASSERT(attr.data->component_type == cgltf_component_type_r_32f);
						tangents = attr.data;
						break;

					case cgltf_attribute_type_texcoord:
						ASSERT(attr.data->type == cgltf_type_vec2);
						ASSERT(attr.data->component_type == cgltf_component_type_r_32f);
						if (attr.index == 0)
							texcoords = attr.data;
						break;

					case cgltf_attribute_type_joints:
						ASSERT(attr.data->type == cgltf_type_vec4);
						ASSERT(attr.data->component_type == cgltf_component_type_r_8u || attr.data->component_type == cgltf_component_type_r_16u);
						joint_indices = attr.data;
						break;

					case cgltf_attribute_type_weights:
						ASSERT(attr.data->type == cgltf_type_vec4);
						ASSERT(attr.data->component_type == cgltf_component_type_r_8u || attr.data->component_type == cgltf_component_type_r_16u || attr.data->component_type == cgltf_component_type_r_32f);
						joint_weights = attr.data;
						break;

					case cgltf_attribute_type_custom:
						if (strncmp(attr.name, "_RADIUS", 7) == 0) {
							ASSERT(attr.data->type == cgltf_type_scalar);
							ASSERT(attr.data->component_type == cgltf_component_type_r_32f);
							radius = attr.data;
						}
						break;

					default:break;
					}
				}

				ASSERT(nullptr != positions);

				Uint64 indexCount{ 0 };

				if (nullptr != prim.indices) {
					indexCount = prim.indices->count;

					// copy the indices
					auto [indexSrc, indexStride] { cgltf_buffer_iterator(prim.indices, 0) };

					Uint32* indexDst{ buffers->IndexData.data() + totalIndices };

					switch (prim.indices->component_type) {
					case cgltf_component_type_r_8u:
						if (0 == indexStride)
							indexStride = sizeof(Uint8);
						for (Uint64 i_idx = 0; i_idx < indexCount; ++i_idx) {
							*indexDst = *reinterpret_cast<const Uint8*>(indexSrc);

							indexSrc += indexStride;
							++indexDst;
						}
						break;

					case cgltf_component_type_r_16u:
						if (0 == indexStride)
							indexStride = sizeof(Uint16);
						for (Uint64 i_idx = 0; i_idx < indexCount; ++i_idx) {
							*indexDst = *reinterpret_cast<const Uint16*>(indexSrc);

							indexSrc += indexStride;
							++indexDst;
						}
						break;

					case cgltf_component_type_r_32u:
						if (0 == indexStride)
							indexStride = sizeof(Uint32);
						for (Uint64 i_idx = 0; i_idx < indexCount; ++i_idx) {
							*indexDst = *reinterpret_cast<const Uint32*>(indexSrc);

							indexSrc += indexStride;
							++indexDst;
						}
						break;

					default:ASSERT(false); break;
					}
				}
				else {
					indexCount = positions->count;

					// generate the indices
					Uint32* indexDst{ buffers->IndexData.data() + totalIndices };
					for (Uint64 i_idx = 0; i_idx < indexCount; ++i_idx) {
						*indexDst = static_cast<Uint32>(i_idx);
						++indexDst;
					}
				}

				Math::BoxF3 bounds{ Math::BoxF3::Empty() };

				if (nullptr != positions) {
					auto [positionSrc, positionStride] { cgltf_buffer_iterator(positions, sizeof(float) * 3) };
					Math::VecF3* positionDst{ buffers->PositionData.data() + totalVertices };

					for (Uint64 v_idx = 0; v_idx < positions->count; ++v_idx) {
						*positionDst = reinterpret_cast<const float*>(positionSrc);

						bounds |= *positionDst;

						positionSrc += positionStride;
						++positionDst;
					}
				}

				if (nullptr != radius) {
					auto [radiusSrc, radiusStride] { cgltf_buffer_iterator(radius, sizeof(float)) };
					float* radiusDst{ buffers->RadiusData.data() + totalVertices };
					for (Uint64 v_idx = 0; v_idx < radius->count; ++v_idx) {
						*radiusDst = *reinterpret_cast<const float*>(radiusSrc);

						bounds |= *radiusDst;//NOTE : radius is a float, but we use snorm8 in our engine

						radiusSrc += radiusStride;
						++radiusDst;
					}
				}
				else
					buffers->RadiusData.clear();

				if (nullptr != normals) {
					ASSERT(normals->count == positions->count);

					auto [normalSrc, normalStride] { cgltf_buffer_iterator(normals, sizeof(float) * 3) };
					Uint32* normalDst{ buffers->NormalData.data() + totalVertices };//NOTEL : glTF normal is vec3, but we use snorm8 in our engine

					for (Uint64 v_idx = 0; v_idx < normals->count; ++v_idx) {
						Math::VecF3 normal{ reinterpret_cast<const float*>(normalSrc) };
						*normalDst = Math::VecFToSnorm8(normal);

						normalSrc += normalStride;
						++normalDst;
					}
				}

				if (nullptr != tangents) {
					ASSERT(tangents->count == positions->count);

					auto [tangentSrc, tangentStride] { cgltf_buffer_iterator(tangents, sizeof(float) * 4) };
					Uint32* tangentDst{ buffers->TangentData.data() + totalVertices };//NOTEL : glTF normal is vec4, but we use snorm8 in our engine

					for (Uint64 v_idx = 0; v_idx < tangents->count; ++v_idx) {
						Math::VecF4 tangent{ reinterpret_cast<const float*>(tangentSrc) };
						*tangentDst = Math::VecFToSnorm8(tangent);

						tangentSrc += tangentStride;
						++tangentDst;
					}
				}

				if (nullptr != texcoords) {
					ASSERT(texcoords->count == positions->count);

					auto [texcoordSrc, texcoordStride] { cgltf_buffer_iterator(texcoords, sizeof(float) * 2) };
					Math::VecF2* texcoordDst{ buffers->Texcoord1Data.data() + totalVertices };

					for (Uint64 v_idx = 0; v_idx < texcoords->count; ++v_idx) {
						*texcoordDst = reinterpret_cast<const float*>(texcoordSrc);

						texcoordSrc += texcoordStride;
						++texcoordDst;
					}
				}
				else {
					Math::VecF2* texcoordDst{ buffers->Texcoord1Data.data() + totalVertices };
					for (Uint64 v_idx = 0; v_idx < positions->count; ++v_idx) {
						*texcoordDst = Math::VecF2::Zero();
						++texcoordDst;
					}
				}

				if (nullptr != normals && nullptr != texcoords && (nullptr == tangents || c_ForceRebuildTangents)) {
					auto [positionSrc, positionStride] { cgltf_buffer_iterator(positions, sizeof(float) * 3) };
					auto [texcoordSrc, texcoordStride] { cgltf_buffer_iterator(texcoords, sizeof(float) * 2) };
					auto [normalSrc, normalStride] { cgltf_buffer_iterator(normals, sizeof(float) * 3) };
					const Uint32* indexSrc{ buffers->IndexData.data() + totalIndices };

					computedTangents.assign(positions->count, Math::VecF3::Zero());
					computedBitangents.assign(positions->count, Math::VecF3::Zero());

					for (Uint64 t_idx = 0; t_idx < indexCount / 3; ++t_idx) {
						Math::VecU3 tri{ indexSrc };//NOTE :use ptr offset
						indexSrc += 3;

						Math::VecF3 p0{ reinterpret_cast<const float*>(positionSrc + positionStride * tri.X) };
						Math::VecF3 p1{ reinterpret_cast<const float*>(positionSrc + positionStride * tri.Y) };
						Math::VecF3 p2{ reinterpret_cast<const float*>(positionSrc + positionStride * tri.Z) };

						Math::VecF2 t0{ reinterpret_cast<const float*>(texcoordSrc + texcoordStride * tri.X) };
						Math::VecF2 t1{ reinterpret_cast<const float*>(texcoordSrc + texcoordStride * tri.Y) };
						Math::VecF2 t2{ reinterpret_cast<const float*>(texcoordSrc + texcoordStride * tri.Z) };

						Math::VecF3 dPds{ p1 - p0 };
						Math::VecF3 dPdt{ p2 - p0 };

						Math::VecF2 dTds{ t1 - t0 };
						Math::VecF2 dTdt{ t2 - t0 };
						float r{ 1.0f / (dTds.X * dTdt.Y - dTds.Y * dTdt.X) };
						Math::VecF3 tangent{ r * (dPds * dTdt.Y - dPdt * dTds.Y) };
						Math::VecF3 bitangent{ r * (dPdt * dTds.X - dPds * dTdt.X) };

						float tangentLength{ Math::Length(tangent) };
						float bitangentLength{ Math::Length(bitangent) };
						if (tangentLength > 0 && bitangentLength > 0) {
							tangent /= tangentLength;
							bitangent /= bitangentLength;

							computedTangents[tri.X] += tangent;
							computedTangents[tri.Y] += tangent;
							computedTangents[tri.Z] += tangent;
							computedBitangents[tri.X] += bitangent;
							computedBitangents[tri.Y] += bitangent;
							computedBitangents[tri.Z] += bitangent;
						}
					}

					Uint8* tangentSrc{ nullptr };
					Uint64 tangentStride{ 0 };
					if (nullptr != tangents) {
						/*Tie(tangentSrc, tangentStride) = cgltf_buffer_iterator(tangents, sizeof(float) * 4);*/

						auto pair{ cgltf_buffer_iterator(tangents, sizeof(float) * 4) };
						tangentSrc = const_cast<Uint8*>(pair.first);
						tangentStride = pair.second;
					}

					Uint32* tangentDst{ buffers->TangentData.data() + totalVertices };

					for (Uint64 v_idx = 0; v_idx < positions->count; ++v_idx) {
						Math::VecF3 normal{ reinterpret_cast<const float*>(normalSrc) };
						Math::VecF3 tangent{ computedTangents[v_idx] };
						Math::VecF3 bitangent{ computedBitangents[v_idx] };

						float sign{ 0 };
						float tangentLength{ Math::Length(tangent) };
						float bitangentLength{ Math::Length(bitangent) };

						if (tangentLength > 0 && bitangentLength > 0) {
							tangent /= tangentLength;
							bitangent /= bitangentLength;
							sign = (Math::Dot(Math::Cross(normal, tangent), bitangent) > 0) ? -1.f : 1.f;
						}

						*tangentDst = Math::VecFToSnorm8(Math::VecF4{ tangent, sign });

						if (c_ForceRebuildTangents && nullptr != tangents) {
							*reinterpret_cast<Math::VecF4*>(tangentSrc) = Math::VecF4{ tangent, sign };
							tangentSrc += tangentStride;
						}

						normalSrc += normalStride;
						++tangentDst;
					}
				}

				if (nullptr != joint_indices) {
					minfo->IsSkinPrototype = true;

					ASSERT(joint_indices->count == positions->count);

					auto [jointSrc, jointStride] { cgltf_buffer_iterator(joint_indices /*sizeof(Uint8) * 4*/, 0) };//NOTE: 8 or 16 ? no know
					Math::Vec<Uint16, 4>* jointDst{ buffers->JointData.data() + totalVertices };

					if (joint_indices->component_type == cgltf_component_type_r_8u) {
						if (0 == jointStride)
							jointStride = sizeof(Uint8) * 4;

						for (Uint64 v_idx = 0; v_idx < joint_indices->count; ++v_idx) {
							*jointDst = Math::Vec<Uint16, 4>{ jointSrc[0], jointSrc[1], jointSrc[2], jointSrc[3] };

							jointSrc += jointStride;
							++jointDst;
						}
					}
					else {
						ASSERT(joint_indices->component_type == cgltf_component_type_r_16u);

						if (0 == jointStride)
							jointStride = sizeof(Uint16) * 4;

						for (Uint64 v_idx = 0; v_idx < joint_indices->count; ++v_idx) {
							const Uint16* jointSrcUshort{ reinterpret_cast<const Uint16*>(jointSrc) };
							*jointDst = Math::Vec<Uint16, 4>{ jointSrcUshort[0], jointSrcUshort[1], jointSrcUshort[2], jointSrcUshort[3] };

							jointSrc += jointStride;
							++jointDst;
						}
					}
				}

				if (nullptr != joint_weights) {
					minfo->IsSkinPrototype = true;

					ASSERT(joint_weights->count == positions->count);

					auto [weightSrc, weightStride] { cgltf_buffer_iterator(joint_weights, 0) };
					Math::VecF4* weightDst{ buffers->WeightData.data() + totalVertices };

					if (joint_weights->component_type == cgltf_component_type_r_8u) {
						if (0 == weightStride)
							weightStride = sizeof(Uint8) * 4;

						for (Uint64 v_idx = 0; v_idx < joint_indices->count; ++v_idx) {
							*weightDst = Math::VecF4{ static_cast<float>(weightSrc[0]) / static_cast<float>(Max_Uint8),static_cast<float>(weightSrc[1]) / static_cast<float>(Max_Uint8),static_cast<float>(weightSrc[2]) / static_cast<float>(Max_Uint8),static_cast<float>(weightSrc[3]) / static_cast<float>(Max_Uint8) };

							weightSrc += weightStride;
							++weightDst;
						}
					}
					else if (joint_weights->component_type == cgltf_component_type_r_16u) {
						if (0 == weightStride)
							weightStride = sizeof(Uint16) * 4;

						for (Uint64 v_idx = 0; v_idx < joint_indices->count; ++v_idx) {
							const Uint16* weightSrcUshort{ reinterpret_cast<const Uint16*>(weightSrc) };
							*weightDst = Math::VecF4{ static_cast<float>(weightSrcUshort[0]) / static_cast<float>(Max_Uint16),static_cast<float>(weightSrcUshort[1]) / static_cast<float>(Max_Uint16),static_cast<float>(weightSrcUshort[2]) / static_cast<float>(Max_Uint16),static_cast<float>(weightSrcUshort[3]) / static_cast<float>(Max_Uint16) };

							weightSrc += weightStride;
							++weightDst;
						}
					}
					else {
						ASSERT(joint_weights->component_type == cgltf_component_type_r_32f);

						if (0 == weightStride)
							weightStride = sizeof(float) * 4;

						for (Uint64 v_idx = 0; v_idx < joint_indices->count; ++v_idx) {
							*weightDst = reinterpret_cast<const float*>(weightSrc);

							weightSrc += weightStride;
							++weightDst;
						}
					}
				}

				SharedPtr<MeshGeometry<APITag>> geometry{ MakeShared<MeshGeometry<APITag>>() };
				if (nullptr != prim.material)
					geometry->Material = materials[prim.material];
				else {
					LOG_ERROR("Geometry  for mesh  doesn't have a material."/*, uint32_t(minfo->geometries.size()), minfo->name.c_str()*/);
					if (nullptr == emptyMaterial) {
						emptyMaterial = MakeShared<Material<APITag>>();
						emptyMaterial->Name = String{ "(empty)" };
					}
					geometry->Material = emptyMaterial;
				}

				if (prim.targets_count > 0) {
					minfo->IsMorphTargetAnimationMesh = true;
					morphTargetData.resize(prim.targets_count);

					for (Uint64 target_idx = 0; target_idx < prim.targets_count; ++target_idx) {
						const cgltf_morph_target& target{ prim.targets[target_idx] };
						const cgltf_accessor* target_positions{ nullptr };
						const cgltf_accessor* target_normals{ nullptr };

						for (Uint64 attr_idx = 0; attr_idx < target.attributes_count; ++attr_idx) {
							const cgltf_attribute& attr{ target.attributes[attr_idx] };
							switch (attr.type) {
							case cgltf_attribute_type_position:
								ASSERT(attr.data->type == cgltf_type_vec3);
								ASSERT(attr.data->component_type == cgltf_component_type_r_32f);
								target_positions = attr.data;
								break;

							case cgltf_attribute_type_normal:
								ASSERT(attr.data->type == cgltf_type_vec3);
								ASSERT(attr.data->component_type == cgltf_component_type_r_32f);
								target_normals = attr.data;
								break;

							default: break;
							}
						}

						if (nullptr != target_positions) {
							auto [positionSrc, positionStride] { cgltf_buffer_iterator(positions, sizeof(float) * 3) };
							auto [morphTargetPositionSrc, morphTargetPositionStride] { cgltf_buffer_iterator(target_positions, sizeof(float) * 3) };

							auto& morphTargetCurrentFrameData = morphTargetData[target_idx];
							morphTargetCurrentFrameData.resize(morphTargetTotalVertices);

							Math::VecF3* morphTargetCurrentData{ morphTargetCurrentFrameData.data() + totalVertices };
							for (Uint64 v_idx = 0; v_idx < positions->count; ++v_idx) {
								*morphTargetCurrentData = *reinterpret_cast<const Math::VecF3*>(morphTargetPositionSrc);

								bounds |= *morphTargetCurrentData;

								morphTargetPositionSrc += morphTargetPositionStride;
								++morphTargetCurrentData;

								positionSrc += positionStride;
							}

							morphTargetDataCount += positions->count;
						}
					}
				}

				geometry->IndexOffsetInMesh = minfo->TotalIndices;
				geometry->VertexOffsetInMesh = minfo->TotalVertices;
				geometry->NumIndices = static_cast<Uint32>(indexCount);
				geometry->NumVertices = static_cast<Uint32>(positions->count);
				geometry->ObjectSpaceBounds = bounds;
				switch (prim.type) {
				case cgltf_primitive_type_triangles:
					geometry->Type = MeshGeometryPrimitiveType::Triangles;
					break;

				case cgltf_primitive_type_lines:
					geometry->Type = MeshGeometryPrimitiveType::Lines;
					break;

				case cgltf_primitive_type_line_strip:
					geometry->Type = MeshGeometryPrimitiveType::LineStrip;
					break;

				default: break;
				}

				minfo->ObjectSpaceBounds |= bounds;
				minfo->TotalIndices += geometry->NumIndices;
				minfo->TotalVertices += geometry->NumVertices;
				minfo->Geometries.push_back(geometry);

				totalIndices += geometry->NumIndices;
				totalVertices += geometry->NumVertices;
			}

			if (morphTargetData.size() > 0) {
				Uint64 morphTargetDataSize{ 0 };

				const Uint64 morphTargetFrameBufferSize{ morphTargetData[0].size() * sizeof(Math::VecF4) };
				buffers->MorphTargetData.reserve(morphTargetDataCount);
				buffers->MorphTargetBufferRange.reserve(morphTargetData.size());

				morphTargetDataCount = 0;
				for (const auto& morphTargetCurrentFrameData : morphTargetData) {
					RHI::RHIBufferRange range{
						.Offset { morphTargetDataCount * sizeof(Math::VecF4) },
						.ByteSize { morphTargetFrameBufferSize }
					};
					buffers->MorphTargetBufferRange.push_back(range);

					for (const auto& morphTargetCurrentData : morphTargetCurrentFrameData) {
						buffers->MorphTargetData.push_back(Math::VecF4{ morphTargetCurrentData, 0.f });
						++morphTargetDataCount;
					}

					morphTargetDataSize += range.ByteSize;
				}
			}
		}

		UnorderedMap<const cgltf_camera*, SharedPtr<SceneCamera<APITag>>> cameraMap;
		for (Uint64 camera_idx = 0; camera_idx < objects->cameras_count; ++camera_idx) {
			const cgltf_camera* src{ &objects->cameras[camera_idx] };
			SharedPtr<SceneCamera<APITag>> dst;

			if (src->type == cgltf_camera_type_perspective) {
				SharedPtr<PerspectiveCamera<APITag>> perspectiveCamera{ MakeShared<PerspectiveCamera<APITag>>() };

				perspectiveCamera->ZNear = src->data.perspective.znear;
				if (src->data.perspective.has_zfar)
					perspectiveCamera->ZFar = src->data.perspective.zfar;
				perspectiveCamera->VerticalFOV = src->data.perspective.yfov;
				if (src->data.perspective.has_aspect_ratio)
					perspectiveCamera->AspectRatio = src->data.perspective.aspect_ratio;

				dst = perspectiveCamera;
			}
			else {
				SharedPtr<OrthographicCamera<APITag>> orthographicCamera{ MakeShared<OrthographicCamera<APITag>>() };

				orthographicCamera->ZNear = src->data.orthographic.znear;
				orthographicCamera->ZFar = src->data.orthographic.zfar;
				orthographicCamera->XMag = src->data.orthographic.xmag;
				orthographicCamera->YMag = src->data.orthographic.ymag;

				dst = orthographicCamera;
			}

			cameraMap[src] = dst;
		}

		UnorderedMap<const cgltf_light*, SharedPtr<Light<APITag>>> lightMap;
		for (Uint64 light_idx = 0; light_idx < objects->lights_count; ++light_idx) {
			const cgltf_light* src{ &objects->lights[light_idx] };
			SharedPtr<Light<APITag>> dst;

			switch (src->type) {
			case cgltf_light_type_directional: {
				auto directional{ MakeShared<DirectionalLight<APITag>>() };
				directional->LightColor = src->color;

				directional->Irradiance = src->intensity;

				dst = directional;
				break;
			}
			case cgltf_light_type_point: {
				auto point{ MakeShared<PointLight<APITag>>() };
				point->LightColor = src->color;

				point->Intensity = src->intensity;
				point->Range = src->range;

				dst = point;
				break;
			}
			case cgltf_light_type_spot: {
				auto spot{ MakeShared<SpotLight<APITag>>() };
				spot->LightColor = src->color;

				spot->Intensity = src->intensity;
				spot->Range = src->range;
				spot->InnerAngle = Math::Degrees(src->spot_inner_cone_angle);
				spot->OuterAngle = Math::Degrees(src->spot_outer_cone_angle);

				dst = spot;
				break;
			}
			default:
				break;
			}

			if (nullptr != dst)
				lightMap[src] = dst;
		}

		//Build
		SharedPtr<SceneGraph<APITag>> graph{ MakeShared<SceneGraph<APITag>>() };
		SharedPtr<SceneGraphNode<APITag>> root{ MakeShared<SceneGraphNode<APITag>>() };
		UnorderedMap<cgltf_node*, SharedPtr<SceneGraphNode<APITag>>> nodeMap;
		Vector<cgltf_node*> skinnedNodes;

		struct StackItem final {
			SharedPtr<SceneGraphNode<APITag>> dstParent;
			cgltf_node** srcNodes{ nullptr };
			Uint64 srcCount = 0;
		};
		Vector<StackItem> stack;

		root->Set_Name(fileName.filename().generic_string());

		Uint32 unnamedCameraCounter{ 1 };

		StackItem context{
			.dstParent{ root },
			.srcNodes{ objects->scene->nodes },
			.srcCount{ objects->nodes_count }
		};

		while (context.srcCount > 0) {
			cgltf_node* src{ *context.srcNodes };
			++context.srcNodes;
			--context.srcCount;

			auto dst{ MakeShared<SceneGraphNode<APITag>>() };

			nodeMap[src] = dst;

			if (src->has_matrix) {
				// decompose the matrix into TRS
				Math::AffineF3 aff{ &src->matrix[0], &src->matrix[4], &src->matrix[8], &src->matrix[12] };

				Math::VecD3 translation;
				Math::VecD3 scaling;
				Math::QuatD rotation;
				Math::DecomposeAffine(Math::AffineD3{ aff }, &translation, &rotation, &scaling);

				dst->Set_Transform(&translation, &rotation, &scaling);
			}
			else {
				if (src->has_scale)
					dst->Set_Scaling(Math::VecD3{ Math::VecF3{ src->scale } });
				if (src->has_rotation)
					dst->Set_Rotation(Math::QuatD{ Math::QuatF::FromXYZW(src->rotation) });
				if (src->has_translation)
					dst->Set_Translation(Math::VecD3{ Math::VecF3{ src->translation } });
			}

			if (nullptr != src->name)
				dst->Set_Name(src->name);

			graph->Attach(context.dstParent, dst);//TODO :Move up


			if (nullptr != src->skin)// process the skinned nodes later, when the graph is constructed
				skinnedNodes.push_back(src);
			else if (nullptr != src->mesh)
				if (auto found = meshMap.find(src->mesh); found != meshMap.end())
					dst->Set_Leaf(MakeShared<MeshInstance<APITag>>(found->second));

			if (nullptr != src->camera) {
				if (auto found{ cameraMap.find(src->camera) }; found != cameraMap.end()) {
					const auto& camera{ found->second };

					if (nullptr != dst->Get_Leaf()) {
						auto node{ MakeShared<SceneGraphNode<APITag>>() };
						node->Set_Leaf(camera);
						graph->Attach(dst, node);
					}
					else
						dst->Set_Leaf(camera);

					if (nullptr != src->camera->name)
						camera->Set_Name(String{ src->camera->name });
					else if (camera->Get_Name().empty())
						camera->Set_Name("Camera" + IntegralToString(unnamedCameraCounter++));
				}
			}

			if (nullptr != src->light) {
				if (auto found{ lightMap.find(src->light) }; found != lightMap.end()) {
					auto light = found->second;

					if (dst->Get_Leaf()) {
						auto node{ MakeShared<SceneGraphNode<APITag>>() };
						node->Set_Leaf(light);
						graph->Attach(dst, node);
					}
					else
						dst->Set_Leaf(light);
				}
			}

			if (src->children_count > 0) {
				stack.push_back(context);
				context.dstParent = dst;
				context.srcNodes = src->children;
				context.srcCount = src->children_count;
			}
			else
				while (context.srcCount == 0 && !stack.empty()) {// go up the stack until we find a node where some nodes are left
					context = stack.back();
					stack.pop_back();
				}

		}

		for (auto* src : skinnedNodes) {
			ASSERT(nullptr != src->skin);
			ASSERT(nullptr != src->mesh);

			SharedPtr<MeshInfo<APITag>> prototypeMesh;
			if (auto found{ meshMap.find(src->mesh) }; found != meshMap.end()) {
				prototypeMesh = found->second;
				ASSERT(prototypeMesh->IsSkinPrototype);

				auto skinnedInstance{ MakeShared<SkinnedMeshInstance<APITag>>(prototypeMesh) };
				skinnedInstance->Joints.resize(src->skin->joints_count);

				for (Uint64 joint_idx = 0; joint_idx < src->skin->joints_count; ++joint_idx) {
					auto& joint{ skinnedInstance->Joints[joint_idx] };
					cgltf_accessor_read_float(src->skin->inverse_bind_matrices, joint_idx, joint.InverseBindMatrix.m_Data, 16);//maybe use [0][0] addressof
					joint.Node = nodeMap[src->skin->joints[joint_idx]];

					auto jointNode{ joint.Node.lock() };
					if (nullptr == jointNode->Get_Leaf())
						jointNode->Set_Leaf(MakeShared<SkinnedMeshReference<APITag>>(skinnedInstance));
				}

				nodeMap[src]->Set_Leaf(skinnedInstance);
			}
		}

		result.RootNode = root;

		auto animationContainer = root;
		if (objects->animations_count > 1) {
			ASSERT(false);//anmiationContainer->Get_Leaf() == nullptr);
			animationContainer = MakeShared<SceneGraphNode<APITag>>();
			animationContainer->Set_Name("Animations");
			graph->Attach(root, animationContainer);
		}

		/*std::unordered_map<const cgltf_animation_sampler*, std::shared_ptr<animation::Sampler>> animationSamplers;

		for (size_t a_idx = 0; a_idx < objects->animations_count; a_idx++)
		{
			const cgltf_animation* srcAnim = &objects->animations[a_idx];
			auto dstAnim = std::make_shared<SceneGraphAnimation>();

			animationSamplers.clear();

			for (size_t s_idx = 0; s_idx < srcAnim->samplers_count; s_idx++)
			{
				const cgltf_animation_sampler* srcSampler = &srcAnim->samplers[s_idx];
				const cgltf_animation_channel* srcChannel = &srcAnim->channels[s_idx];
				auto dstSampler = std::make_shared<animation::Sampler>();

				switch (srcSampler->interpolation)
				{
				case cgltf_interpolation_type_linear:
					if (srcChannel->target_path == cgltf_animation_path_type_rotation)
						dstSampler->SetInterpolationMode(animation::InterpolationMode::Slerp);
					else
						dstSampler->SetInterpolationMode(animation::InterpolationMode::Linear);
					break;
				case cgltf_interpolation_type_step:
					dstSampler->SetInterpolationMode(animation::InterpolationMode::Step);
					break;
				case cgltf_interpolation_type_cubic_spline:
					dstSampler->SetInterpolationMode(animation::InterpolationMode::HermiteSpline);
					break;
				default:
					break;
				}

				const cgltf_accessor* times = srcSampler->input;
				const cgltf_accessor* values = srcSampler->output;
				assert(times->type == cgltf_type_scalar);

				for (size_t sample_idx = 0; sample_idx < times->count; sample_idx++)
				{
					animation::Keyframe keyframe;

					bool timeRead = cgltf_accessor_read_float(times, sample_idx, &keyframe.time, 1);

					bool valueRead;
					if (srcSampler->interpolation == cgltf_interpolation_type_cubic_spline)
					{
						valueRead = cgltf_accessor_read_float(values, sample_idx * 3 + 0, &keyframe.inTangent.x, 4);
						valueRead = cgltf_accessor_read_float(values, sample_idx * 3 + 1, &keyframe.value.x, 4);
						valueRead = cgltf_accessor_read_float(values, sample_idx * 3 + 2, &keyframe.outTangent.x, 4);
					}
					else
					{
						valueRead = cgltf_accessor_read_float(values, sample_idx, &keyframe.value.x, 4);
					}

					if (timeRead && valueRead)
						dstSampler->AddKeyframe(keyframe);
				}

				if (!dstSampler->GetKeyframes().empty())
					animationSamplers[srcSampler] = dstSampler;
				else
					log::warning("Animation channel imported with no keyframes, ignoring.");
			}

			for (size_t channel_idx = 0; channel_idx < srcAnim->channels_count; channel_idx++)
			{
				const cgltf_animation_channel* srcChannel = &srcAnim->channels[channel_idx];

				auto dstNode = nodeMap[srcChannel->target_node];
				if (!dstNode)
					continue;

				AnimationAttribute attribute;
				switch (srcChannel->target_path)
				{
				case cgltf_animation_path_type_translation:
					attribute = AnimationAttribute::Translation;
					break;

				case cgltf_animation_path_type_rotation:
					attribute = AnimationAttribute::Rotation;
					break;

				case cgltf_animation_path_type_scale:
					attribute = AnimationAttribute::Scaling;
					break;

				case cgltf_animation_path_type_weights:
				case cgltf_animation_path_type_invalid:
				default:
					log::warning("Unsupported glTF animation taregt: %d", srcChannel->target_path);
					continue;
				}

				auto dstSampler = animationSamplers[srcChannel->sampler];
				if (!dstSampler)
					continue;

				auto dstTrack = std::make_shared<SceneGraphAnimationChannel>(dstSampler, dstNode, attribute);

				dstAnim->AddChannel(dstTrack);
			}

			if (dstAnim->IsVald())
			{
				auto animationNode = std::make_shared<SceneGraphNode>();
				animationNode->SetName(dstAnim->GetName());
				graph->Attach(animationContainer, animationNode);
				animationNode->SetLeaf(dstAnim);
				if (srcAnim->name)
					animationNode->SetName(srcAnim->name);
			}
		}*/

		if (c_ForceRebuildTangents) {
			for (Uint64 buffer_idx = 0; buffer_idx < objects->buffers_count; ++buffer_idx) {
				Path outputFileName{ fileName.parent_path() / fileName.stem() };
				outputFileName += ".buffer" + IntegralToString(buffer_idx) + ".bin";

				this->m_FS->WriteFile(outputFileName, objects->buffers[buffer_idx].data, objects->buffers[buffer_idx].size);
			}
		}

		cgltf_free(objects);

		return true;
	}

}