#pragma once

#include "FXLib.h"

namespace FX
{
	class FXLIB_API TextureBuffer
	{
	public:
		enum DataType
		{
			R8i,
			RG8i,
			RGB8i,
			RGBA8i,
			R8u,
			RG8u,
			RGB8u,
			RGBA8u,
			R16i,
			RG16i,
			RGB16i,
			RGBA16i,
			R16u,
			RG16u,
			RGB16u,
			RGBA16u,
			R32i,
			RG32i,
			RGB32i,
			RGBA32i,
			R32u,
			RG32u,
			RGB32u,
			RGBA32u,
			R32f,
			RG32f,
			RGB32f,
			RGBA32f,
		};

	private:
		uint32_t ID = NULL;
		size_t x = 0, y = 0, z = 0;
		int dimensions = 1;
		DataType type = R8i;
		uint64_t handle = NULL;

		void init(size_t x, DataType type, const void* data = nullptr);
		void init(size_t x, size_t y, DataType type, const void* data = nullptr);
		void init(size_t x, size_t y, size_t z, DataType type, const void* data = nullptr);

		static GLenum glType(DataType type);
		static GLenum glCompType(DataType type);
		static GLenum glFormat(DataType type);

	public:
		TextureBuffer() {}
		TextureBuffer(TextureBuffer&& other) noexcept;
		TextureBuffer& operator=(TextureBuffer&& other) noexcept;
		TextureBuffer(size_t x, DataType type, const void* data = nullptr);
		TextureBuffer(size_t x, size_t y, DataType type, const void* data = nullptr);
		TextureBuffer(size_t x, size_t y, size_t z, DataType type, const void* data = nullptr);

		void uploadData(size_t x, const void* data = nullptr);
		void uploadData(size_t x, size_t y, const void* data = nullptr);
		void uploadData(size_t x, size_t y, size_t z, const void* data = nullptr);

		// might clear the data! use with caution!
		void fit(size_t x);
		void fit(size_t x, size_t y);
		void fit(size_t x, size_t y, size_t z);

		// might clear the data! use with caution!
		void resize(size_t x);
		void resize(size_t x, size_t y);
		void resize(size_t x, size_t y, size_t z);

		void cleanup();

		uint32_t id() const
		{
			return ID;
		}

		void bind(uint32_t unit) const
		{
			if (ID)
			{
				glBindTextureUnit(unit, ID);
			}
		}
		
		uint64_t getHandle() const
		{
			return handle;
		}

		DataType getType() const
		{
			return type;
		}
		int getDimensions() const
		{
			return dimensions;
		}
		// x
		size_t width() const
		{
			return x;
		}
		// y
		size_t height() const
		{
			return y;
		}
		// z
		size_t depth() const
		{
			return z;
		}
		size_t getX() const
		{
			return x;
		}
		size_t getY() const
		{
			return y;
		}
		size_t getZ() const
		{
			return z;
		}

		~TextureBuffer();
	};
}
