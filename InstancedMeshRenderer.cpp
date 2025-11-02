#include "include/fxlib/FXLib.h"
#include "include/fxlib/InstancedMeshRenderer.h"

using namespace FX;
using namespace fdm;

InstancedMeshRenderer::InstancedMeshRenderer(const Mesh* mesh) : InstancedMeshRenderer()
{
	init(mesh);
}

InstancedMeshRenderer::~InstancedMeshRenderer()
{
	cleanup();
}

void InstancedMeshRenderer::setMesh(const Mesh* mesh)
{
	init(mesh);
}

void InstancedMeshRenderer::updateMesh(const Mesh* mesh)
{
	if (VAO && vertexCount > 0)
	{
		assert(mesh->buffCount() == bufferCount);

		glBindVertexArray(VAO);
		initAttrs(mesh);

		vertexCount = mesh->vertCount();

		const void* data = mesh->indexBuffData();

		if (data)
		{
			assert(indexVBO != 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffSize(), data, GL_STATIC_DRAW);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	else
	{
		init(mesh);
	}
}

void InstancedMeshRenderer::render(const std::vector<void*>& instanceData)
{
	updateData(instanceData);

	render();
}

void InstancedMeshRenderer::render() const
{
	if (!instanceCount) return;

	glBindVertexArray(VAO);

	SSBO.use(0);

	if (indexVBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
		glDrawElementsInstanced(mode, vertexCount, GL_UNSIGNED_INT, 0, instanceCount);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else
	{
		glDrawArraysInstanced(mode, 0, vertexCount, instanceCount);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void InstancedMeshRenderer::updateData(const std::vector<void*>& instanceData)
{
	SSBO.uploadData(dataSize, instanceData);
	this->instanceCount = instanceData.size();
}

void InstancedMeshRenderer::updateData(const void* instanceData, int instanceCount)
{
	SSBO.uploadData(dataSize * instanceCount, instanceData);
	this->instanceCount = instanceCount;
}

void InstancedMeshRenderer::setDataSize(int dataSize)
{
	if (this->dataSize == dataSize) return;

	this->dataSize = dataSize;

	setCount(instanceCount);
}

void InstancedMeshRenderer::setCount(int instanceCount)
{
	if (this->instanceCount == instanceCount) return;

	this->instanceCount = instanceCount;

	SSBO.resize(dataSize * instanceCount);
}

void InstancedMeshRenderer::init(const Mesh* mesh)
{
	cleanup();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	bufferCount = mesh->buffCount();

	VBOs = new uint32_t[bufferCount];
	glGenBuffers(bufferCount, VBOs);
	initAttrs(mesh);

	vertexCount = mesh->vertCount();
	const void* data = mesh->indexBuffData();
	if (data)
	{
		glGenBuffers(1, &indexVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffSize(), data, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void InstancedMeshRenderer::cleanup()
{
	if (VAO)
	{
		glBindVertexArray(VAO);

		glDeleteBuffers(bufferCount, VBOs);
		delete VBOs;
		VBOs = NULL;

		glDeleteBuffers(1, &indexVBO);
		indexVBO = NULL;

		glDeleteVertexArrays(1, &VAO);
		VAO = NULL;


		vertexCount = 0;
		bufferCount = 0;
		instanceCount = 0;
		dataSize = 0;
	}

	SSBO.cleanup();
}

int InstancedMeshRenderer::getAttrSize(uint32_t type, int size)
{
	switch (type)
	{
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		return sizeof(GLbyte) * size;
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		return sizeof(GLshort) * size;
	case GL_INT_2_10_10_10_REV:
	case GL_INT:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT:
		return sizeof(GLint) * size;
	case GL_FLOAT:
		return sizeof(GLfloat) * size;
	case GL_DOUBLE:
		return sizeof(GLdouble) * size;
	case GL_FIXED:
		return sizeof(GLfixed) * size;
	case GL_HALF_FLOAT:
		return sizeof(GLhalf) * size;
	}

	return 0;
}

void InstancedMeshRenderer::initAttrs(const Mesh* mesh)
{
	if (bufferCount <= 0) return;

	this->attrCount = 0;
	for (int buff = 0; buff < bufferCount; ++buff)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[buff]);
		glBufferData(GL_ARRAY_BUFFER, mesh->buffSize(buff), mesh->buffData(buff), GL_STATIC_DRAW);

		int attrCount = mesh->attrCount(buff);

		int bufferOffset = 0;
		for (int attr = 0; attr < attrCount; ++attr)
		{
			int attrSize = mesh->attrSize(buff, attr);
			uint32_t attrType = mesh->attrType(buff, attr);
			int attrStride = mesh->attrStride(buff, attr);

			//if (isInteger)
			//{
			//	glVertexAttribIPointer(this->attrCount, attrSize, attrType, attrStride, (void*)bufferOffset);
			//}
			//else
			//{
				glVertexAttribPointer(this->attrCount, attrSize, attrType, GL_FALSE, attrStride, (void*)bufferOffset);
			//}

			glEnableVertexAttribArray(this->attrCount);
			++this->attrCount;
			bufferOffset += getAttrSize(attrType, attrSize);
		}
	}
}

InstancedMeshRenderer& InstancedMeshRenderer::operator=(InstancedMeshRenderer&& other) noexcept
{
	this->VAO = other.VAO;
	this->VBOs = other.VBOs;
	this->indexVBO = other.indexVBO;
	this->SSBO = other.SSBO;
	this->vertexCount = other.vertexCount;
	this->bufferCount = other.bufferCount;
	this->attrCount = other.attrCount;
	this->mode = other.mode;
	this->instanceCount = other.instanceCount;
	this->dataSize = other.dataSize;

	other.VAO = 0;
	other.VBOs = nullptr;
	other.indexVBO = 0;
	other.SSBO = 0;
	other.vertexCount = 0;
	other.bufferCount = 0;
	other.attrCount = 0;
	other.mode = GL_LINES_ADJACENCY;
	other.instanceCount = 0;
	other.dataSize = 0;

	return *this;
}
