#include "include/fxlib/FXLib.h"
#include "include/fxlib/TextureBuffer.h"

using namespace FX;

TextureBuffer::TextureBuffer(size_t x, DataType type, const void* data)
{
	init(x, type, data);
}

TextureBuffer::TextureBuffer(size_t x, size_t y, DataType type, const void* data)
{
	init(x, y, type, data);
}

TextureBuffer::TextureBuffer(size_t x, size_t y, size_t z, DataType type, const void* data)
{
	init(x, y, z, type, data);
}

TextureBuffer::~TextureBuffer()
{
	cleanup();
}

void TextureBuffer::uploadData(size_t x, const void* data)
{
	if (dimensions != 1)
	{
		throw std::runtime_error(std::format("Tried uploading 1D data into a {}D texture!", dimensions));
	}

	if (data == nullptr) return;

	fit(x);

	glTextureSubImage1D(ID, 0, 0, x, glFormat(type), glCompType(type), data);
}

void TextureBuffer::uploadData(size_t x, size_t y, const void* data)
{
	if (dimensions != 2)
	{
		throw std::runtime_error(std::format("Tried uploading 2D data into a {}D texture!", dimensions));
	}

	if (data == nullptr) return;

	fit(x, y);

	glTextureSubImage2D(ID, 0, 0, 0, x, y, glFormat(type), glCompType(type), data);
}

void TextureBuffer::uploadData(size_t x, size_t y, size_t z, const void* data)
{
	if (dimensions != 3)
	{
		throw std::runtime_error(std::format("Tried uploading 3D data into a {}D texture!", dimensions));
	}

	if (data == nullptr) return;

	fit(x, y, z);

	glTextureSubImage3D(ID, 0, 0, 0, 0, x, y, z, glFormat(type), glCompType(type), data);
}


void TextureBuffer::fit(size_t x)
{
	if (x > this->x)
	{
		init(x, type, nullptr);
	}
}

void TextureBuffer::fit(size_t x, size_t y)
{
	if (x > this->x || y > this->y)
	{
		init(x, y, type, nullptr);
	}
}

void TextureBuffer::fit(size_t x, size_t y, size_t z)
{
	if (x > this->x || y > this->y || z > this->z)
	{
		init(x, y, z, type, nullptr);
	}
}

void TextureBuffer::resize(size_t x)
{
	if (x != this->x)
	{
		init(x, type, nullptr);
	}
}

void TextureBuffer::resize(size_t x, size_t y)
{
	if (x != this->x || y != this->y)
	{
		init(x, y, type, nullptr);
	}
}

void TextureBuffer::resize(size_t x, size_t y, size_t z)
{
	if (x != this->x || y != this->y || z != this->z)
	{
		init(x, y, z, type, nullptr);
	}
}

void TextureBuffer::cleanup()
{
	if (ID)
	{
		glMakeTextureHandleNonResidentARB(handle);
		handle = NULL;
		glDeleteTextures(1, &ID);
		ID = NULL;
		dimensions = 1;
		type = R8i;
		x = 0;
		y = 0;
		z = 0;
	}
}

void TextureBuffer::init(size_t x, DataType type, const void* data)
{
	cleanup();

	this->dimensions = 1;
	this->x = x;
	this->type = type;
	
	glCreateTextures(GL_TEXTURE_1D, 1, &ID);

	glTextureStorage1D(ID, 1, glType(type), x);

	glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureStorage1D(ID, 1, glType(type), x);

	if (data != nullptr)
	{
		uploadData(x, data);
	}
}

void TextureBuffer::init(size_t x, size_t y, DataType type, const void* data)
{
	cleanup();

	this->dimensions = 2;
	this->x = x;
	this->y = y;
	this->type = type;

	glCreateTextures(GL_TEXTURE_2D, 1, &ID);

	glTextureStorage2D(ID, 1, glType(type), x, y);

	glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureStorage2D(ID, 1, glType(type), x, y);

	handle = glGetTextureHandleARB(ID);
	glMakeTextureHandleResidentARB(handle);

	if (data != nullptr)
	{
		uploadData(x, y, data);
	}
}

void TextureBuffer::init(size_t x, size_t y, size_t z, DataType type, const void* data)
{
	cleanup();

	this->dimensions = 3;
	this->x = x;
	this->y = y;
	this->z = z;
	this->type = type;

	glCreateTextures(GL_TEXTURE_3D, 1, &ID);

	glTextureStorage3D(ID, 1, glType(type), x, y, z);

	glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureStorage3D(ID, 1, glType(type), x, y, z);

	handle = glGetTextureHandleARB(ID);
	glMakeTextureHandleResidentARB(handle);

	if (data != nullptr)
	{
		uploadData(x, y, z, data);
	}
}

TextureBuffer::TextureBuffer(TextureBuffer&& other) noexcept
{
	this->ID = other.ID;
	this->dimensions = other.dimensions;
	this->type = other.type;
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;
	this->handle = other.handle;

	other.ID = NULL;
	other.dimensions = 1;
	other.type = R8i;
	other.x = 0;
	other.y = 0;
	other.z = 0;
	other.handle = NULL;
}
TextureBuffer& TextureBuffer::operator=(TextureBuffer&& other) noexcept
{
	if (this != &other)
	{
		this->ID = other.ID;
		this->dimensions = other.dimensions;
		this->type = other.type;
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
		this->handle = other.handle;

		other.ID = NULL;
		other.dimensions = 1;
		other.type = R8i;
		other.x = 0;
		other.y = 0;
		other.z = 0;
		other.handle = NULL;
	}

	return *this;
}

GLenum TextureBuffer::glType(DataType type)
{
	switch (type)
	{
	case FX::TextureBuffer::R8i:
		return GL_R8I;
	case FX::TextureBuffer::RG8i:
		return GL_RG8I;
	case FX::TextureBuffer::RGB8i:
		return GL_RGB8I;
	case FX::TextureBuffer::RGBA8i:
		return GL_RGBA8I;
	case FX::TextureBuffer::R8u:
		return GL_R8UI;
	case FX::TextureBuffer::RG8u:
		return GL_RG8UI;
	case FX::TextureBuffer::RGB8u:
		return GL_RGB8UI;
	case FX::TextureBuffer::RGBA8u:
		return GL_RGBA8UI;
	case FX::TextureBuffer::R16i:
		return GL_R16I;
	case FX::TextureBuffer::RG16i:
		return GL_RG16I;
	case FX::TextureBuffer::RGB16i:
		return GL_RGB16I;
	case FX::TextureBuffer::RGBA16i:
		return GL_RGBA16I;
	case FX::TextureBuffer::R16u:
		return GL_R16UI;
	case FX::TextureBuffer::RG16u:
		return GL_RG16UI;
	case FX::TextureBuffer::RGB16u:
		return GL_RGB16UI;
	case FX::TextureBuffer::RGBA16u:
		return GL_RGBA16UI;
	case FX::TextureBuffer::R32i:
		return GL_R32I;
	case FX::TextureBuffer::RG32i:
		return GL_RG32I;
	case FX::TextureBuffer::RGB32i:
		return GL_RGB32I;
	case FX::TextureBuffer::RGBA32i:
		return GL_RGBA32I;
	case FX::TextureBuffer::R32u:
		return GL_R32UI;
	case FX::TextureBuffer::RG32u:
		return GL_RG32UI;
	case FX::TextureBuffer::RGB32u:
		return GL_RGB32UI;
	case FX::TextureBuffer::RGBA32u:
		return GL_RGBA32UI;
	case FX::TextureBuffer::R32f:
		return GL_R32F;
	case FX::TextureBuffer::RG32f:
		return GL_RG32F;
	case FX::TextureBuffer::RGB32f:
		return GL_RGB32F;
	case FX::TextureBuffer::RGBA32f:
		return GL_RGBA32F;
	}
	return GL_R8I;
}

GLenum TextureBuffer::glFormat(DataType type)
{
	switch (type)
	{
	case FX::TextureBuffer::R8i:
	case FX::TextureBuffer::R8u:
	case FX::TextureBuffer::R16i:
	case FX::TextureBuffer::R16u:
	case FX::TextureBuffer::R32i:
	case FX::TextureBuffer::R32u:
		return GL_RED_INTEGER;
	case FX::TextureBuffer::R32f:
		return GL_RED;
	case FX::TextureBuffer::RG8i:
	case FX::TextureBuffer::RG8u:
	case FX::TextureBuffer::RG16i:
	case FX::TextureBuffer::RG16u:
	case FX::TextureBuffer::RG32i:
	case FX::TextureBuffer::RG32u:
		return GL_RG_INTEGER;
	case FX::TextureBuffer::RG32f:
		return GL_RG;
	case FX::TextureBuffer::RGB8i:
	case FX::TextureBuffer::RGB8u:
	case FX::TextureBuffer::RGB16i:
	case FX::TextureBuffer::RGB16u:
	case FX::TextureBuffer::RGB32i:
	case FX::TextureBuffer::RGB32u:
		return GL_RGB_INTEGER;
	case FX::TextureBuffer::RGB32f:
		return GL_RGB;
	case FX::TextureBuffer::RGBA8i:
	case FX::TextureBuffer::RGBA8u:
	case FX::TextureBuffer::RGBA16i:
	case FX::TextureBuffer::RGBA16u:
	case FX::TextureBuffer::RGBA32i:
	case FX::TextureBuffer::RGBA32u:
		return GL_RGBA_INTEGER;
	case FX::TextureBuffer::RGBA32f:
		return GL_RGBA;
	}
	return GL_RED;
}

GLenum TextureBuffer::glCompType(DataType type)
{
	switch (type)
	{
	case FX::TextureBuffer::R8i:
	case FX::TextureBuffer::RG8i:
	case FX::TextureBuffer::RGB8i:
	case FX::TextureBuffer::RGBA8i:
		return GL_BYTE;
	case FX::TextureBuffer::R8u:
	case FX::TextureBuffer::RG8u:
	case FX::TextureBuffer::RGB8u:
	case FX::TextureBuffer::RGBA8u:
		return GL_UNSIGNED_BYTE;
	case FX::TextureBuffer::R16i:
	case FX::TextureBuffer::RG16i:
	case FX::TextureBuffer::RGB16i:
	case FX::TextureBuffer::RGBA16i:
		return GL_SHORT;
	case FX::TextureBuffer::R16u:
	case FX::TextureBuffer::RG16u:
	case FX::TextureBuffer::RGB16u:
	case FX::TextureBuffer::RGBA16u:
		return GL_UNSIGNED_SHORT;
	case FX::TextureBuffer::R32i:
	case FX::TextureBuffer::RG32i:
	case FX::TextureBuffer::RGB32i:
	case FX::TextureBuffer::RGBA32i:
		return GL_INT;
	case FX::TextureBuffer::R32u:
	case FX::TextureBuffer::RG32u:
	case FX::TextureBuffer::RGB32u:
	case FX::TextureBuffer::RGBA32u:
		return GL_UNSIGNED_INT;
	case FX::TextureBuffer::R32f:
	case FX::TextureBuffer::RG32f:
	case FX::TextureBuffer::RGB32f:
	case FX::TextureBuffer::RGBA32f:
		return GL_FLOAT;
	}
	return GL_BYTE;
}
