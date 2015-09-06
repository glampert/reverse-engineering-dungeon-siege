
// ================================================================================================
// -*- C++ -*-
// File: compression.cpp
// Author: Guilherme R. Lampert
// Created on: 02/09/15
// Brief: Compression/decompression helpers (low-level wrapper over mini-Z).
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "utils/compression.hpp"

// ========================================================
// The header-only mini-Z library (only included here).

// Things we don't need right now...
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES 1
#define MINIZ_NO_ARCHIVE_APIS 1
#define MINIZ_NO_STDIO 1

#include "thirdparty/miniz/miniz.h"

// ========================================================

namespace utils
{
namespace compression
{

int decompress(uint8_t * dest, unsigned long * destSizeBytes,
               const uint8_t * source, const unsigned long sourceSizeBytes)
{
	assert(dest != nullptr);
	assert(destSizeBytes != nullptr && *destSizeBytes != 0);

	assert(source != nullptr);
	assert(sourceSizeBytes != 0);

	return mz_uncompress(dest, destSizeBytes, source, sourceSizeBytes);
}

int compress(uint8_t * dest, unsigned long * destSizeBytes, const uint8_t * source,
             const unsigned long sourceSizeBytes, const unsigned long compressionLevel)
{
	assert(dest != nullptr);
	assert(destSizeBytes != nullptr && *destSizeBytes != 0);

	assert(source != nullptr);
	assert(sourceSizeBytes != 0);

	return mz_compress2(dest, destSizeBytes, source,
			sourceSizeBytes, static_cast<mz_uint>(compressionLevel));
}

uint8_t * writeImageToPngInMemory(const uint8_t * image, const int w, const int h, const int numChans,
                                  size_t * lenOut, const unsigned long compressionLevel, const bool flip)
{
	assert(image  != nullptr);
	assert(lenOut != nullptr);
	assert(w > 0 && h > 0);

	return reinterpret_cast<uint8_t *>(tdefl_write_image_to_png_file_in_memory_ex(
			image, w, h, numChans, lenOut, static_cast<mz_uint>(compressionLevel), flip));
}

std::string getErrorString(const int errorCode)
{
	return mz_error(errorCode);
}

} // namespace compression {}
} // namespace utils {}
