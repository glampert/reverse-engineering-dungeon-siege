
// ================================================================================================
// -*- C++ -*-
// File: raw_image.hpp
// Author: Guilherme R. Lampert
// Created on: 11/12/14
// Brief: Handler for the Dungeon Siege RAW image/texture format.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#ifndef SIEGE_RAW_IMAGE_HPP
#define SIEGE_RAW_IMAGE_HPP

#include "siege/common.hpp"
#include "siege/helper_types.hpp"

namespace siege
{

// ========================================================
// RawImage:
// ========================================================

//
// Gas Powered Games RAW image format.
// This is a very simple format consisting of a small header
// followed by image pixels for each image mipmap level.
// The first block of pixels in the image belongs to mipmap 0
// (the largest one). The other smaller mip-levels follow, if present.
//
// The only known pixel type used by this image format is BGRA 8:8:8:8.
// Data is always uncompressed because the RAW file is already stored
// with compression inside a Tank, so no reason to compress twice.
//
// Refer to "gpg/RapiRawReader.h" for more details.
//
class RawImage final
	: public utils::NonCopyable
{
public:

	// BGRA 8bits pixel:
	struct Pixel
	{
		uint8_t b, g, r, a;
	};

	// Construct an empty image.
	RawImage();

	// Construct from RAW image file.
	RawImage(std::string filename);

	// Construct from a RAW image file loaded into memory.
	RawImage(ByteArray fileContents, std::string filename = "");

	// Load RAW from file. Discards current, if any.
	void initFromFile(std::string filename);

	// Load RAW from memory. Discards current, if any.
	void initFromMemory(ByteArray fileContents, std::string filename = "");

	// Dumps a given surface to disk as an uncompressed TGA image file. No default filename extension provided!
	void writeSurfaceAsTgaImage(unsigned int surfaceIndex, const std::string & filename, bool swizzlePixels) const;

	// Dumps a given surface to disk as a compressed PNG image file. No default filename extension provided!
	void writeSurfaceAsPngImage(unsigned int surfaceIndex, const std::string & filename, bool swizzlePixels) const;

	// Manually Disposes the current image data, if any. Automatically disposed by the destructor.
	void dispose();

	// Test if this object has valid image data.
	bool isValid() const;

	// Access indexed pixel.
	Pixel getPixelAt(unsigned int x, unsigned int y, unsigned int surfaceIndex) const;

	// Access raw pixels of a given surface.
	const Pixel * getSurfacePixels(unsigned int surfaceIndex) const;

	// Access raw pixels of all surfaces. Pointer is to the start of surface 0.
	const Pixel * getPixels() const;

	// Access dimensions of a given surface:
	unsigned int getSurfaceWidth(unsigned int surfaceIndex) const;
	unsigned int getSurfaceHeight(unsigned int surfaceIndex) const;
	unsigned int getSurfacePixelCount(unsigned int surfaceIndex) const;

	// Access image dimensions:
	unsigned int getWidth()  const noexcept { return width;  } // Width  of surface 0
	unsigned int getHeight() const noexcept { return height; } // Height of surface 0
	unsigned int getSurfaceCount() const noexcept { return surfaceCount; }

	// Source file that originated image pixels. May be empty if the image was loaded from memory.
	const std::string & getSourceFileName() const { return srcFileName; }

private:

	#pragma pack(push, 1)
	struct Header
	{
		FourCC   magic;        // Special magic number 'Rapi' (stored reversed: 'ipaR')
		FourCC   format;       // Format of pixels '8888' (BGRA 8:8:8:8 is the only known format)
		uint16_t flags;        // Any special flags (unused)
		uint16_t surfaceCount; // Total surfaces stored (for mip maps), always >= 1
		uint16_t width;        // Width of surface 0 in pixels
		uint16_t height;       // Height of surface 0 in pixels
		/* Pixel pixels[] */   // Raw image data follows (format-dependent)
	};
	#pragma pack(pop)

	static_assert(sizeof(Pixel)  == 4,  "Bad size for RawImage::Pixel!");
	static_assert(sizeof(Header) == 16, "Bad size for RawImage::Header!");

	// Data extracted from file header for quick access:
	unsigned int width;
	unsigned int height;
	unsigned int surfaceCount;

	// Image data read from file.
	// Consists of a Header instance followed by an array of 'RawImage::Pixel'.
	ByteArray rawData;

	// Source filename for debug printing.
	// May be empty if the image was loaded from memory.
	std::string srcFileName;
};

// Output operator for RawImage debug printing:
std::ostream & operator << (std::ostream & s, const RawImage & img);

} // namespace siege {}

#endif // SIEGE_RAW_IMAGE_HPP
