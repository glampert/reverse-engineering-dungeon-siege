
// ================================================================================================
// -*- C++ -*-
// File: compression.hpp
// Author: Guilherme R. Lampert
// Created on: 02/09/15
// Brief: Compression/decompression helpers.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#ifndef UTILS_COMPRESSION_HPP
#define UTILS_COMPRESSION_HPP

#include "utils/common.hpp"

namespace utils
{

// Helper functions for compression and decompression of raw data.
// (Mini-Z is the compressor/decompressor back-end).
namespace compression
{

// Data compression levels (a value between 0 and 10).
// Note: Values compatible with ZLib and mini-Z.
struct Level
{
	enum Enum
	{
		NoCompression      = 0,
		BestSpeed          = 1,
		DefaultCompression = 6,
		BestCompression    = 9,
		UberCompression    = 10
	};
};

// 'dest' is the decompression buffer; 'source' is the compressed data.
int decompress(uint8_t * dest, unsigned long * destSizeBytes,
               const uint8_t * source, unsigned long sourceSizeBytes);

// 'dest' is the compressed output; 'source' is the uncompressed input data.
// 'compressionLevel' is one of the Level flags or a value between 0 and 10.
int compress(uint8_t * dest, unsigned long * destSizeBytes,
             const uint8_t * source, unsigned long sourceSizeBytes,
             unsigned long compressionLevel);

// Compresses an image to a compressed PNG file in memory.
// Memory returned should the released with std::free()!
uint8_t * writeImageToPngInMemory(const uint8_t * image, int w, int h,
                                  int numChans, size_t * lenOut,
                                  unsigned long compressionLevel, bool flip);

// Pass the error code returned by the above functions
// to this method to get a printable error string.
std::string getErrorString(int errorCode);

} // namespace compression {}
} // namespace utils {}

#endif // UTILS_COMPRESSION_HPP
