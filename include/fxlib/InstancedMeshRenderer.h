#pragma once

#include "FXLib.h"

#include "ShaderStorageBuffer.h"

namespace FX
{
	class FXLIB_API InstancedMeshRenderer
	{
	public:
		uint32_t VAO = 0;
		uint32_t* VBOs = nullptr;
		uint32_t indexVBO = 0;
		ShaderStorageBuffer SSBO{};
		int vertexCount = 0;
		int bufferCount = 0;
		int attrCount = 0;
		uint32_t mode = GL_LINES_ADJACENCY;

		InstancedMeshRenderer() {}
		InstancedMeshRenderer(const fdm::Mesh* mesh);
		~InstancedMeshRenderer();
		void setMesh(const fdm::Mesh* mesh);
		void updateMesh(const fdm::Mesh* mesh);
		void render(const std::vector<void*>& instanceData);
		void render() const;
		void updateData(const std::vector<void*>& instanceData);
		void updateData(const void* instanceData, int instanceCount);
		void setCount(int instanceCount = 1);
		void setDataSize(int dataSize = 1);
		void cleanup();

		InstancedMeshRenderer& operator=(InstancedMeshRenderer&& other) noexcept;

	private:
		int instanceCount = 0;
		int dataSize = 0;
		void init(const fdm::Mesh* mesh);
		void initAttrs(const fdm::Mesh* mesh);
		int getAttrSize(uint32_t type, int size);
	};
}
