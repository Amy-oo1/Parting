#pragma once

#include "Core/VectorMath/Module/VectorMath.h"

static const Math::VecF3 g_Positions[] = {
	{-0.5f,  0.5f, -0.5f}, // front face
	{ 0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f, -0.5f},
	{ 0.5f,  0.5f, -0.5f},

	{ 0.5f, -0.5f, -0.5f}, // right side face
	{ 0.5f,  0.5f,  0.5f},
	{ 0.5f, -0.5f,  0.5f},
	{ 0.5f,  0.5f, -0.5f},

	{-0.5f,  0.5f,  0.5f}, // left side face
	{-0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f,  0.5f},
	{-0.5f,  0.5f, -0.5f},

	{ 0.5f,  0.5f,  0.5f}, // back face
	{-0.5f, -0.5f,  0.5f},
	{ 0.5f, -0.5f,  0.5f},
	{-0.5f,  0.5f,  0.5f},

	{-0.5f,  0.5f, -0.5f}, // top face
	{ 0.5f,  0.5f,  0.5f},
	{ 0.5f,  0.5f, -0.5f},
	{-0.5f,  0.5f,  0.5f},

	{ 0.5f, -0.5f,  0.5f}, // bottom face
	{-0.5f, -0.5f, -0.5f},
	{ 0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f,  0.5f},
};

static const Math::VecF2 g_TexCoords[] = {
	{0.0f, 0.0f}, // front face
	{1.0f, 1.0f},
	{0.0f, 1.0f},
	{1.0f, 0.0f},

	{0.0f, 1.0f}, // right side face
	{1.0f, 0.0f},
	{1.0f, 1.0f},
	{0.0f, 0.0f},

	{0.0f, 0.0f}, // left side face
	{1.0f, 1.0f},
	{0.0f, 1.0f},
	{1.0f, 0.0f},

	{0.0f, 0.0f}, // back face
	{1.0f, 1.0f},
	{0.0f, 1.0f},
	{1.0f, 0.0f},

	{0.0f, 1.0f}, // top face
	{1.0f, 0.0f},
	{1.0f, 1.0f},
	{0.0f, 0.0f},

	{1.0f, 1.0f}, // bottom face
	{0.0f, 0.0f},
	{1.0f, 0.0f},
	{0.0f, 1.0f},
};

static const Uint32 g_Normals[] = {
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, -1.0f, 0.0f)), // front face
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, -1.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, -1.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, -1.0f, 0.0f)),

	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 0.0f)), // right side face
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 0.0f)),

	Math::VecFToSnorm8(Math::VecF4(-1.0f, 0.0f, 0.0f, 0.0f)), // left side face
	Math::VecFToSnorm8(Math::VecF4(-1.0f, 0.0f, 0.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(-1.0f, 0.0f, 0.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(-1.0f, 0.0f, 0.0f, 0.0f)),

	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, 1.0f, 0.0f)), // back face
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, 1.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, 1.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, 1.0f, 0.0f)),

	Math::VecFToSnorm8(Math::VecF4(0.0f, 1.0f, 0.0f, 0.0f)), // top face
	Math::VecFToSnorm8(Math::VecF4(0.0f, 1.0f, 0.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 1.0f, 0.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 1.0f, 0.0f, 0.0f)),

	Math::VecFToSnorm8(Math::VecF4(0.0f, -1.0f, 0.0f, 0.0f)), // bottom face
	Math::VecFToSnorm8(Math::VecF4(0.0f, -1.0f, 0.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, -1.0f, 0.0f, 0.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, -1.0f, 0.0f, 0.0f)),
};

static const Uint32 g_Tangents[] = {
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)), // front face
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),

	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, 1.0f, 1.0f)), // right side face
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, 1.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, 1.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, 1.0f, 1.0f)),

	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, -1.0f, 1.0f)), // left side face
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, -1.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, -1.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(0.0f, 0.0f, -1.0f, 1.0f)),

	Math::VecFToSnorm8(Math::VecF4(-1.0f, 0.0f, 0.0f, 1.0f)), // back face
	Math::VecFToSnorm8(Math::VecF4(-1.0f, 0.0f, 0.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(-1.0f, 0.0f, 0.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(-1.0f, 0.0f, 0.0f, 1.0f)),

	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)), // top face
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),

	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)), // bottom face
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),
	Math::VecFToSnorm8(Math::VecF4(1.0f, 0.0f, 0.0f, 1.0f)),
};

static const Uint32 g_Indices[] = {
	 0,  1,  2,   0,  3,  1, // front face
	 4,  5,  6,   4,  7,  5, // left face
	 8,  9, 10,   8, 11,  9, // right face
	12, 13, 14,  12, 15, 13, // back face
	16, 17, 18,  16, 19, 17, // top face
	20, 21, 22,  20, 23, 21, // bottom face
};