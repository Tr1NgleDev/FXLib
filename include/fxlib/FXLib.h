#pragma once

#include <4dm.h>

#ifdef FXLIB_DLL
#define FXLIB_API __declspec(dllexport)
#else
#define FXLIB_API __declspec(dllimport)
#endif

namespace FX
{
	FXLIB_API bool hasInitializedContext();
}

#include "utils.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "TextureBuffer.h"
#include "InstancedMeshRenderer.h"
#include "ThreadPool.h"
#include "TrailRenderer.h"
#include "ParticleSystem.h"
#include "ShaderPatcher.h"
#include "PostPass.h"
