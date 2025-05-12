#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(VFS)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include<fstream>
#include<filesystem>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"


#endif // PARTING_MODULE_BUILD

using Path = std::filesystem::path;

enum class FileState :Uint8 {
	OK,
	Failed,
	PathNotFound,
	NotImplemented
	//NOTE do not tounderlaying
};

using EnumerateCallback_T = Function<void(StringView)>;

inline Function<void(StringView)> EnumerateToVector(Vector<String>& v) { return [&v](StringView s) { v.push_back(String{ s }); }; }

// A blob is a package for untyped data, typically read from a file.
//I do not want to use CRTP, becase ratner tan render file ,it just need call a virtual func 
class IBlob {
public:
	virtual ~IBlob(void) = default;
	STDNODISCARD virtual const void* Get_Data(void) const = 0;
	STDNODISCARD virtual Uint64 Get_Size(void) const = 0;

	static bool Is_Empty(const IBlob* blob) { return blob == nullptr || blob->Get_Data() == nullptr || blob->Get_Size() == 0; }
};

// Specific blob implementation that owns the data and frees it when deleted.
class Blob : public IBlob {
public:
	Blob(void* data, Uint64 size) :
		m_Data{ data },
		m_Size{ size } {

		ASSERT(nullptr != this->m_Data);
		ASSERT(0 != this->m_Size);
	}
	~Blob(void) override {
		if (nullptr == this->m_Data) {
			free(this->m_Data);
			this->m_Data = nullptr;
		}
	}

public:
	STDNODISCARD const void* Get_Data(void) const override { return this->m_Data; }
	STDNODISCARD Uint64 Get_Size(void) const override { return this->m_Size; }

private:
	void* m_Data{ nullptr };
	Uint64 m_Size{ 0 };

};

// Basic interface for the virtual file system.
class IFileSystem {
public:
	IFileSystem(void) = default;
	virtual ~IFileSystem(void) = default;

public:
	// Test if a folder exists.
	virtual bool FolderExists(const Path& name) = 0;

	// Test if a file exists.
	virtual bool FileExists(const Path& name) = 0;

	// Read the entire file.
	// Returns nullptr if the file cannot be read.
	virtual SharedPtr<IBlob> ReadFile(const Path& name) = 0;

	// Write the entire file.
	// Returns false if the file cannot be written.
	virtual bool WriteFile(const Path& name, const void* data, Uint64 size) = 0;

	// Search for files with any of the provided 'extensions' in 'path'.
	// Extensions should not include any wildcard char.
	// The file names, relative to the 'path', are passed to 'callback' in no particular order.
	virtual Expected<Uint32, FileState> EnumerateFiles(const Path& path, const Vector<String>& extensions, EnumerateCallback_T callback, bool allowDuplicates = false) = 0;

	// Search for directories in 'path'.
	// The directory names, relative to the 'path', are passed to 'callback' in no particular order.
	virtual Expected<Uint32, FileState> EnumerateDirectories(const Path& path, EnumerateCallback_T callback, bool allowDuplicates = false) = 0;
};

//some default filesystem

// An imp of VFS that directly maps to the OS files.
class NativeFileSystem  final : public IFileSystem {
public:
	bool FolderExists(const Path& name) override { return std::filesystem::exists(name) && std::filesystem::is_directory(name); }

	bool FileExists(const Path& name) override { return std::filesystem::exists(name) && std::filesystem::is_regular_file(name); }

	SharedPtr<IBlob> ReadFile(const Path& name) override {
		std::ifstream file(name, std::ios::binary);

		if (!file.is_open())
			return nullptr;

		file.seekg(0, std::ios::end);
		auto size{ file.tellg() };
		file.seekg(0, std::ios::beg);

		char* data{ static_cast<char*>(malloc(size)) };

		if (data == nullptr) {
			// out of memory
			ASSERT(false);
			return nullptr;
		}

		file.read(data, size);

		if (!file.good()) {
			// reading error
			ASSERT(false);
			free(data);
			return nullptr;
		}

		return std::make_shared<Blob>(data, size);
	}

	bool WriteFile(const Path& name, const void* data, Uint64 size) override {
		std::ofstream file{ name, std::ios::binary };

		if (!file.is_open())
			return false;

		if (size > 0)
			file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));

		return file.good();
	}
	Expected<Uint32, FileState> EnumerateFiles(const Path& path, const Vector<String>& extensions, EnumerateCallback_T callback, bool allowDuplicates = false) override {
		(void)allowDuplicates;//do not use

		if (extensions.empty()) {
			String pattern{ (path / "*").generic_string() };
			return NativeFileSystem::EnumerateNativeFiles(pattern.c_str(), false, callback);
		}

		Uint32 numEntries = 0;
		for (const auto& ext : extensions) {
			String pattern{ (path / ("*" + ext)).generic_string() };
			auto Re{ NativeFileSystem::EnumerateNativeFiles(pattern.c_str(), false, callback) };

			if (!Re.HasValue())
				return Re;

			numEntries += Re.Value();
		}

		return Expected<Uint32, FileState>{numEntries };
	}

	Expected<Uint32, FileState> EnumerateDirectories(const Path& path, EnumerateCallback_T callback, bool allowDuplicates = false) override {
		(void)allowDuplicates;//do not use
		String pattern{ (path / "*").generic_string() };
		return NativeFileSystem::EnumerateNativeFiles(pattern.c_str(), true, callback);
	}


private:
	static Expected<Uint32, FileState> EnumerateNativeFiles(const char* pattern, bool directories, EnumerateCallback_T callback) {
		WIN32_FIND_DATAA findData;
		HANDLE hFind{ FindFirstFileA(pattern, &findData) };

		if (INVALID_HANDLE_VALUE == hFind) {
			if (ERROR_FILE_NOT_FOUND == GetLastError())
				return  Expected<Uint32, FileState>{ 0u };

			return  Expected<Uint32, FileState>{ FileState::Failed };
		}

		Uint32 numEntries = 0;

		do {
			bool isDirectory{ (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 };
			bool isDot{ strcmp(findData.cFileName, ".") == 0 };
			bool isDotDot{ strcmp(findData.cFileName, "..") == 0 };

			if ((isDirectory == directories) && !isDot && !isDotDot) {
				callback(findData.cFileName);
				++numEntries;
			}
		} while (FindNextFileA(hFind, &findData) != 0);

		ASSERT(FindClose(hFind));

		return Expected<Uint32, FileState>{ numEntries };
	}


};


// A layer that represents some path in the underlying file system as an entire FS.
// Effectively, just prepends the provided base path to every file name and passes the requests to the underlying FS.
class RelativeFileSystem final : public IFileSystem {
public:
	RelativeFileSystem(SharedPtr<IFileSystem> fs, const Path& basePath) :
		IFileSystem{},
		m_UnderlyingFS{ fs },
		m_BasePath{ basePath } {

		ASSERT(nullptr != this->m_UnderlyingFS);
		ASSERT(!this->m_BasePath.empty());
	}

public:
	STDNODISCARD Path const& Get_BasePath(void) const { return this->m_BasePath; }

public:

	bool FolderExists(const Path& name) override { return this->m_UnderlyingFS->FolderExists(this->m_BasePath / name.relative_path()); }

	bool FileExists(const Path& name) override { return this->m_UnderlyingFS->FileExists(this->m_BasePath / name.relative_path()); }

	SharedPtr<IBlob> ReadFile(const Path& name) override { return this->m_UnderlyingFS->ReadFile(this->m_BasePath / name.relative_path()); }

	bool WriteFile(const Path& name, const void* data, Uint64 size) override { return this->m_UnderlyingFS->WriteFile(this->m_BasePath / name.relative_path(), data, size); }

	Expected<Uint32, FileState> EnumerateFiles(const Path& path, const Vector<String>& extensions, EnumerateCallback_T callback, bool allowDuplicates = false) override { return this->m_UnderlyingFS->EnumerateFiles(this->m_BasePath / path.relative_path(), extensions, callback, allowDuplicates); }

	Expected<Uint32, FileState> EnumerateDirectories(const Path& path, EnumerateCallback_T callback, bool allowDuplicates = false) override { return this->m_UnderlyingFS->EnumerateDirectories(this->m_BasePath / path.relative_path(), callback, allowDuplicates); }

private:
	SharedPtr<IFileSystem> m_UnderlyingFS;
	Path m_BasePath;

};


// A virtual file system that allows mounting, or attaching, other VFS objects to paths.
// Does not have any file systems by default, all of them must be mounted first.
class RootFileSystem : public IFileSystem {
public:
	void Mount(const Path& path, std::shared_ptr<IFileSystem> fs) {
		if (this->FindMountPoint(path, nullptr, nullptr)) {
			LOG_ERROR("Cannot mount a filesystem at : there is another FS that includes this path"/*path.c_str()*/);
			return;
		}

		this->m_MountPoints.push_back(MakePair(path.lexically_normal().generic_string(), fs));
	}

	void Mount(const Path& path, const Path& nativePath) { this->Mount(path, MakeShared<RelativeFileSystem>(MakeShared<NativeFileSystem>(), nativePath)); }

	bool Unmount(const Path& path) {
		const String spath{ path.lexically_normal().generic_string() };

		for (Uint64 index = 0; index < this->m_MountPoints.size(); ++index)
			if (this->m_MountPoints[index].first == spath) {
				this->m_MountPoints.erase(this->m_MountPoints.begin() + index);
				return true;
			}

		return false;
	}

public:
	bool FolderExists(const Path& name) override {
		Path relativePath;
		IFileSystem* fs{ nullptr };

		if (this->FindMountPoint(name, &relativePath, &fs))
			return fs->FolderExists(relativePath);

		return false;
	}

	bool FileExists(const Path& name) override {
		Path relativePath;
		IFileSystem* fs{ nullptr };

		if (this->FindMountPoint(name, &relativePath, &fs))
			return fs->FileExists(relativePath);

		return false;
	}

	SharedPtr<IBlob> ReadFile(const Path& name) override {
		Path relativePath;
		IFileSystem* fs{ nullptr };

		if (this->FindMountPoint(name, &relativePath, &fs))
			return fs->ReadFile(relativePath);

		return nullptr;
	}

	bool WriteFile(const Path& name, const void* data, Uint64 size)  override {
		Path relativePath;
		IFileSystem* fs{ nullptr };

		if (this->FindMountPoint(name, &relativePath, &fs))
			return fs->WriteFile(relativePath, data, size);

		return false;
	}

	Expected<Uint32, FileState> EnumerateFiles(const Path& path, const Vector<String>& extensions, EnumerateCallback_T callback, bool allowDuplicates = false) override {
		Path relativePath;
		IFileSystem* fs{ nullptr };

		if (this->FindMountPoint(path, &relativePath, &fs))
			return fs->EnumerateFiles(relativePath, extensions, callback, allowDuplicates);

		return Expected<Uint32, FileState>{ FileState::PathNotFound };
	}

	Expected<Uint32, FileState> EnumerateDirectories(const Path& path, EnumerateCallback_T callback, bool allowDuplicates = false) override {
		Path relativePath;
		IFileSystem* fs{ nullptr };

		if (this->FindMountPoint(path, &relativePath, &fs))
			return fs->EnumerateDirectories(relativePath, callback, allowDuplicates);

		return Expected<Uint32, FileState>{ FileState::PathNotFound };
	}

private:
	bool FindMountPoint(const Path& path, Path* pRelativePath, IFileSystem** ppFS) {
		const String spath{ path.lexically_normal().generic_string() };

		for (const auto& [mountpath, mountFS] : this->m_MountPoints) {
			if (spath.find_first_of(mountpath, 0) == 0 && ((spath.length() == mountpath.length()) || (spath[mountpath.length()] == '/'))) {
				if (pRelativePath)
					*pRelativePath = (spath.length() == mountpath.length()) ? "" : spath.substr(mountpath.size() + 1);

				if (ppFS)
					*ppFS = mountFS.get();

				return true;
			}
		}

		return false;
	}

private:
	Vector<Pair<String, SharedPtr<IFileSystem>>> m_MountPoints;
};