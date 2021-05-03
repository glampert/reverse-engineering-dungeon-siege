
// ================================================================================================
// -*- C++ -*-
// File: raw_image.cpp
// Author: Guilherme R. Lampert
// Created on: 11/12/14
// Brief: Handler for the Dungeon Siege RAW image/texture format.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/raw_image.hpp"
#include <fstream>
#include <cstdio>

namespace siege
{

// ========================================================
// RawImage:
// ========================================================

RawImage::RawImage()
	: width(0)
	, height(0)
	, surfaceCount(0)
{
}

RawImage::RawImage(std::string filename)
	: RawImage()
{
	initFromFile(std::move(filename));
}

RawImage::RawImage(ByteArray fileContents, std::string filename)
	: RawImage()
{
	initFromMemory(std::move(fileContents), std::move(filename));
}

bool RawImage::isValid() const
{
	return (width != 0) && (height != 0) &&
	       (surfaceCount >= 1) && !rawData.empty();
}

RawImage::Pixel RawImage::getPixelAt(const unsigned int x, const unsigned int y,
                                     const unsigned int surfaceIndex) const
{
	assert(isValid());

	const auto surfPixels     = getSurfacePixels(surfaceIndex);
	const auto surfWidth      = getSurfaceWidth(surfaceIndex);
	const auto surfPixelCount = getSurfacePixelCount(surfaceIndex);

	const auto index = x + y * surfWidth;
	assert(index < surfPixelCount);

	return surfPixels[index];
}

const RawImage::Pixel * RawImage::getSurfacePixels(const unsigned int surfaceIndex) const
{
	assert(isValid());

	const Pixel * pixelsStart = getPixels();
	for (unsigned int s = 0; s < surfaceIndex; ++s)
	{
		pixelsStart += getSurfacePixelCount(s);
	}

	return pixelsStart;
}

const RawImage::Pixel * RawImage::getPixels() const
{
	assert(isValid());

	// Pixel data follows the header block.
	const Pixel * pixelsStart = reinterpret_cast<const Pixel *>(rawData.data() + sizeof(Header));
	return pixelsStart;
}

unsigned int RawImage::getSurfaceWidth(const unsigned int surfaceIndex) const
{
	assert(surfaceIndex < surfaceCount);

	unsigned int w = width;
	for (int i = surfaceIndex; i > 0; --i)
	{
		w = w / 2;
		if (w == 0) { w = 1; break; }
	}
	return w;
}

unsigned int RawImage::getSurfaceHeight(const unsigned int surfaceIndex) const
{
	assert(surfaceIndex < surfaceCount);

	unsigned int h = height;
	for (int i = surfaceIndex; i > 0; --i)
	{
		h = h / 2;
		if (h == 0) { h = 1; break; }
	}
	return h;
}

unsigned int RawImage::getSurfacePixelCount(const unsigned int surfaceIndex) const
{
	return getSurfaceWidth(surfaceIndex) * getSurfaceHeight(surfaceIndex);
}

void RawImage::dispose()
{
	width = height = 0;
	surfaceCount = 0;

	rawData.clear();
	srcFileName.clear();
}

void RawImage::initFromFile(std::string filename)
{
	if (filename.empty())
	{
		SiegeThrow(Exception, "No filename provided for RawImage::initFromFile()!");
	}

	std::ifstream file;
	if (!utils::filesys::tryOpen(file, filename, std::ifstream::binary))
	{
		SiegeThrow(Exception, "Failed to open RAW image file \""
				<< filename << "\": '" << utils::filesys::getLastFileError() << "'.");
	}

	size_t fileSizeBytes = 0;
	utils::filesys::queryFileSize(filename, fileSizeBytes);
	if (fileSizeBytes == 0)
	{
		SiegeWarn("RAW image file \"" << filename << "\" appears to be empty! Making an empty image...");

		// Make this an empty image. NOTE: Should this be changed?
		dispose();
		srcFileName = std::move(filename);
		return;
	}

	ByteArray fileContents(fileSizeBytes);
	if (!file.read(reinterpret_cast<char *>(fileContents.data()), fileContents.size()))
	{
		SiegeThrow(Exception, "Failed to read " << utils::formatMemoryUnit(fileContents.size())
				<< " from RAW image file \"" << filename << "\"!");
	}

	initFromMemory(std::move(fileContents), std::move(filename));
}

void RawImage::initFromMemory(ByteArray fileContents, std::string filename)
{
	if (fileContents.size() < sizeof(Header))
	{
		SiegeThrow(Exception, "Size of input data on RawImage::initFromMemory() is too small!");
	}

	// RAW header validation:
	const Header * header = reinterpret_cast<const Header *>(fileContents.data());
	if (header->magic != "ipaR") // 'Rapi'
	{
		SiegeThrow(Exception, "Bad header magic on RawImage::initFromMemory(): " << header->magic);
	}
	if (header->format != "8888")
	{
		SiegeThrow(Exception, "Bad pixel format on RawImage::initFromMemory(): " << header->format);
	}
	if (header->flags != 0)
	{
		SiegeThrow(Exception, "Bad header flags on RawImage::initFromMemory()!");
	}
	if (header->width == 0 || header->height == 0)
	{
		SiegeThrow(Exception, "Bad image dimensions on RawImage::initFromMemory()!");
	}

	// Non-fatal warnings:
	if (header->surfaceCount == 0)
	{
		SiegeWarn("RAW image \"" << filename << "\" header surface count is zero! Defaulting to 1 ...");
	}
	if (!utils::isPowerOfTwo(header->width) || !utils::isPowerOfTwo(header->height))
	{
		SiegeWarn("RAW image \"" << filename << "\" dimensions are not powers-of-two!");
	}

	// Store the input data:
	width        = header->width;
	height       = header->height;
	surfaceCount = (header->surfaceCount != 0) ? header->surfaceCount : 1;
	rawData      = std::move(fileContents);
	srcFileName  = std::move(filename);

	SiegeLog("RawImage \"" << srcFileName << "\" initialized. "
			<< width << "x" << height << " px, " << surfaceCount << " surfaces.");
}

void RawImage::initFromPixelBuffer(const Pixel * const buffer, unsigned int width, unsigned int height, bool swizzlePixels, std::string filename)
{
	assert(buffer != nullptr);
	assert(width  <= UINT16_MAX);
	assert(height <= UINT16_MAX);

	dispose();

	const size_t pixelCount  = (width * height);
	const size_t storageSize = sizeof(Header) + (pixelCount * sizeof(Pixel));
	rawData.resize(storageSize);

	this->srcFileName    = std::move(filename);
	this->width          = width;
	this->height         = height;
	this->surfaceCount   = 1; // No mipmaps

	auto * header = reinterpret_cast<Header *>(rawData.data());
	auto * pixels = reinterpret_cast<Pixel  *>(header + 1);

	header->magic        = FourCC{ 'i','p','a','R' };
	header->format       = FourCC{ '8','8','8','8' };
	header->flags        = 0; // No flags
	header->surfaceCount = 1; // No mipmaps
	header->width        = static_cast<uint16_t>(width);
	header->height       = static_cast<uint16_t>(height);

	if (swizzlePixels) // RGBA <=> BGRA swizzle
	{
		for (size_t i = 0; i < pixelCount; ++i)
		{
			pixels[i].b = buffer[i].r;
			pixels[i].g = buffer[i].g;
			pixels[i].r = buffer[i].b;
			pixels[i].a = buffer[i].a;
		}
	}
	else // Assume input is BRGA
	{
		std::memcpy(pixels, buffer, pixelCount * sizeof(Pixel));
	}
}

void RawImage::writeSurfaceAsTgaImage(const unsigned int surfaceIndex,
                                      const std::string & filename,
                                      const bool swizzlePixels) const
{
	assert(!filename.empty());
	assert(surfaceIndex < surfaceCount);
	assert(isValid());

	std::ofstream outFile;
	if (!utils::filesys::tryOpen(outFile, filename, std::ofstream::binary))
	{
		SiegeThrow(Exception, "Unable to open file \"" << filename
				<< "\" for writing! " << utils::filesys::getLastFileError());
	}

	const auto surfWidth  = getSurfaceWidth(surfaceIndex);
	const auto surfHeight = getSurfaceHeight(surfaceIndex);
	auto imageData = reinterpret_cast<const uint8_t *>(getSurfacePixels(surfaceIndex));

	struct TGAImageDescriptor
	{
		uint8_t alpha    : 4;
		uint8_t right    : 1;
		uint8_t top      : 1;
		uint8_t reserved : 2;
	};
	static_assert(sizeof(TGAImageDescriptor) == 1, "Bad TGAImageDescriptor size!");

	// Always BGRA/RGBA.
	const TGAImageDescriptor imageDesc = {8,0,0,0};
	const uint8_t tgaType = 2, bpp = 32;

	// Other unused fields of the TGA header:
	const uint8_t  idLenght     = 0; // unused
	const uint8_t  hasColormap  = 0; // unused
	const uint16_t cmFirstEntry = 0; // unused
	const uint16_t cmLength     = 0; // unused
	const uint8_t  cmSize       = 0; // unused
	const uint16_t xOrigin      = 0; // start at x:0
	const uint16_t yOrigin      = 0; // start at y:0

	// Image x,y dimensions:
	const uint16_t dimX = static_cast<uint16_t>(surfWidth);
	const uint16_t dimY = static_cast<uint16_t>(surfHeight);

	// Write the image header fields:
	outFile.write(reinterpret_cast<const char *>(&idLenght),     sizeof(idLenght));
	outFile.write(reinterpret_cast<const char *>(&hasColormap),  sizeof(hasColormap));
	outFile.write(reinterpret_cast<const char *>(&tgaType),      sizeof(tgaType));
	outFile.write(reinterpret_cast<const char *>(&cmFirstEntry), sizeof(cmFirstEntry));
	outFile.write(reinterpret_cast<const char *>(&cmLength),     sizeof(cmLength));
	outFile.write(reinterpret_cast<const char *>(&cmSize),       sizeof(cmSize));
	outFile.write(reinterpret_cast<const char *>(&xOrigin),      sizeof(xOrigin));
	outFile.write(reinterpret_cast<const char *>(&yOrigin),      sizeof(yOrigin));
	outFile.write(reinterpret_cast<const char *>(&dimX),         sizeof(dimX));
	outFile.write(reinterpret_cast<const char *>(&dimY),         sizeof(dimY));
	outFile.write(reinterpret_cast<const char *>(&bpp),          sizeof(bpp));
	outFile.write(reinterpret_cast<const char *>(&imageDesc),    sizeof(imageDesc));

	// Check for IO problems:
	if (outFile.fail())
	{
		SiegeThrow(Exception, "Error while writing TGA header for file \"" << filename << "\"!");
	}

	// Now write the pixels:
	if (swizzlePixels)
	{
		uint8_t pixel[4];
		for (unsigned int i = 0; i < surfWidth * surfHeight; ++i)
		{
			// RGBA <=> BGRA
			pixel[0] = imageData[2];
			pixel[1] = imageData[1];
			pixel[2] = imageData[0];
			pixel[3] = imageData[3];
			imageData += 4;

			if (!outFile.write(reinterpret_cast<const char *>(pixel), sizeof(pixel)))
			{
				SiegeThrow(Exception, "Failed to write image pixels to TGA file \"" << filename << "\"!");
			}
		}
	}
	else
	{
		// No need to swizzle pixels, write data as it is:
		if (!outFile.write(reinterpret_cast<const char *>(imageData), surfWidth * surfHeight * 4))
		{
			SiegeThrow(Exception, "Failed to write image pixels to TGA file \"" << filename << "\"!");
		}
	}

	SiegeLog("Successfully written TGA image to file \"" + filename + "\".");
}

void RawImage::writeSurfaceAsPngImage(const unsigned int surfaceIndex,
                                      const std::string & filename,
                                      const bool swizzlePixels) const
{
	assert(!filename.empty());
	assert(surfaceIndex < surfaceCount);
	assert(isValid());

	std::ofstream outFile;
	if (!utils::filesys::tryOpen(outFile, filename, std::ofstream::binary))
	{
		SiegeThrow(Exception, "Unable to open file \"" << filename
				<< "\" for writing! " << utils::filesys::getLastFileError());
	}

	const auto surfWidth  = getSurfaceWidth(surfaceIndex);
	const auto surfHeight = getSurfaceHeight(surfaceIndex);
	auto imageData = reinterpret_cast<const uint8_t *>(getSurfacePixels(surfaceIndex));

	const uint8_t * imageDataPtr;
	ByteArray tempImage; // Only allocated if we need to swizzle the color.

	if (swizzlePixels)
	{
		tempImage.resize(surfWidth * surfHeight * 4);
		for (unsigned int i = 0, j = 0; i < surfWidth * surfHeight; ++i)
		{
			// RGBA <=> BGRA
			tempImage[j++] = imageData[2];
			tempImage[j++] = imageData[1];
			tempImage[j++] = imageData[0];
			tempImage[j++] = imageData[3];
			imageData += 4;
		}
		imageDataPtr = tempImage.data();
	}
	else
	{
		imageDataPtr = imageData;
	}

	// Create the PNG image with the help of Mini-Z.
	// (note: must use free() to release the returned memory!)
	size_t    pngSize = 0;
	uint8_t * pngData = utils::compression::writeImageToPngInMemory(imageDataPtr,
						surfWidth, surfHeight, /* numChans = */ 4, &pngSize,
						utils::compression::Level::BestCompression, /* flip = */ true);

	if (pngData == nullptr)
	{
		SiegeThrow(Exception, "Failed to compress PNG image \"" << filename << "\"! Null data.");
	}

	if (pngSize == 0)
	{
		std::free(pngData);
		SiegeThrow(Exception, "Failed to compress PNG image \"" << filename << "\"! Zero size.");
	}

	// Dump the data to a file:
	if (!outFile.write(reinterpret_cast<const char *>(pngData), pngSize))
	{
		std::free(pngData);
		SiegeThrow(Exception, "Failed to write image pixels to PNG file \"" << filename << "\"!");
	}

	std::free(pngData);
	SiegeLog("Successfully written PNG image to file \"" + filename + "\".");
}

void RawImage::writeToFile() const
{
	const char * const fname = (!srcFileName.empty() ? srcFileName.c_str() : "image.raw");

	std::ofstream outFile;
	if (!utils::filesys::tryOpen(outFile, fname, std::ofstream::binary))
	{
		SiegeThrow(Exception, "Unable to open file \"" << fname
			<< "\" for writing! " << utils::filesys::getLastFileError());
	}

	outFile.write(reinterpret_cast<const char*>(rawData.data()), rawData.size());
}

// ========================================================
// Output operator for RawImage debug printing:
// ========================================================

std::ostream & operator << (std::ostream & s, const RawImage & img)
{
	s << "========== RawImage =========\n";

	s << "file........: \"" << img.getSourceFileName() << "\"\n";
	s << "is valid....: " << (img.isValid() ? "yes" : "no") << '\n';
	s << "width.......: " << img.getWidth()  << '\n';
	s << "height......: " << img.getHeight() << '\n';
	s << "surfaces....: " << img.getSurfaceCount() << '\n';

	for (unsigned int i = 0; i < img.getSurfaceCount(); ++i)
	{
		s << "surf[" << i << "] => " << img.getSurfaceWidth(i) << "x" << img.getSurfaceHeight(i) << " px\n";
	}

	s << "=============================";

	return s;
}

// ========================================================
// TGA image loader:
// - Output image is always BGRA 32bits
//   (matching the Raw pixel format).
// ========================================================

std::unique_ptr<RawImage::Pixel[]> loadTgaImageFromFile(const std::string& filename, int * width, int * height)
{
	size_t fileSize = 0;
	if (!utils::filesys::queryFileSize(filename, fileSize))
	{
		SiegeThrow(Exception, "Unable to query image file size: \"" << filename
			<< "\" - " << utils::filesys::getLastFileError());
	}

	std::ifstream inFile;
	if (!utils::filesys::tryOpen(inFile, filename, std::ifstream::binary))
	{
		SiegeThrow(Exception, "Unable to open image file \"" << filename
			<< "\" for reading! " << utils::filesys::getLastFileError());
	}

	ByteArray fileData;
	fileData.resize(fileSize);
	inFile.read(reinterpret_cast<char*>(fileData.data()), fileData.size());

	struct TGAFileHeader
	{
		uint8_t  idLength;
		uint8_t  colormapType;
		uint8_t  imageType;
		uint16_t colormapIndex;
		uint16_t colormapLength;
		uint8_t  colormapSize;
		uint16_t xOrigin;
		uint16_t yOrigin;
		uint16_t width;
		uint16_t height;
		uint8_t  pixelSize;
		uint8_t  attributes;
	};

	TGAFileHeader header = {};
	alignas(uint32_t) uint8_t tmp[2] = {};

	const uint8_t * buf_p = fileData.data();
	header.idLength       = *buf_p++;
	header.colormapType   = *buf_p++;
	header.imageType      = *buf_p++;

	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	header.colormapIndex = *reinterpret_cast<const uint16_t *>(tmp);
	buf_p += 2;

	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	header.colormapLength = *reinterpret_cast<const uint16_t *>(tmp);
	buf_p += 2;

	header.colormapSize = *buf_p++;

	header.xOrigin = *reinterpret_cast<const uint16_t *>(buf_p); buf_p += 2;
	header.yOrigin = *reinterpret_cast<const uint16_t *>(buf_p); buf_p += 2;
	header.width   = *reinterpret_cast<const uint16_t *>(buf_p); buf_p += 2;
	header.height  = *reinterpret_cast<const uint16_t *>(buf_p); buf_p += 2;

	header.pixelSize  = *buf_p++;
	header.attributes = *buf_p++;

	if (header.imageType != 2 && header.imageType != 10)
	{
		SiegeThrow(Exception, "Only type 2 and 10 TARGA RGB images supported! " << filename);
	}
	if (header.colormapType != 0 || (header.pixelSize != 32 && header.pixelSize != 24))
	{
		SiegeThrow(Exception, "Only 32 or 24 bit TGA images supported (no colormaps)! " << filename);
	}

	const int columns    = header.width;
	const int rows       = header.height;
	const int pixelCount = columns * rows;

	if (width != nullptr)
	{
		*width = columns;
	}
	if (height != nullptr)
	{
		*height = rows;
	}

	std::unique_ptr<RawImage::Pixel[]> result{ new RawImage::Pixel[pixelCount] };
	auto * decodedImage = reinterpret_cast<uint8_t *>(result.get());

	if (header.idLength != 0)
	{
		buf_p += header.idLength; // skip TARGA image comment
	}

	if (header.imageType == 2) // Uncompressed, RGB images
	{
		for (int row = rows - 1; row >= 0; --row)
		{
			uint8_t * pixbuf = decodedImage + row * columns * 4;
			for (int column = 0; column < columns; ++column)
			{
				uint8_t red, green, blue, alpha;
				switch (header.pixelSize)
				{
				case 24:
					blue      = *buf_p++;
					green     = *buf_p++;
					red       = *buf_p++;
					*pixbuf++ = blue;
					*pixbuf++ = green;
					*pixbuf++ = red;
					*pixbuf++ = 255;
					break;

				case 32:
					blue      = *buf_p++;
					green     = *buf_p++;
					red       = *buf_p++;
					alpha     = *buf_p++;
					*pixbuf++ = blue;
					*pixbuf++ = green;
					*pixbuf++ = red;
					*pixbuf++ = alpha;
					break;

				default:
					assert(false);
				} // switch
			}
		}
	}
	else if (header.imageType == 10) // Run-length encoded RGB images
	{
		uint8_t red, green, blue, alpha;
		uint8_t packetHeader, packetSize, j;

		for (int row = rows - 1; row >= 0; --row)
		{
			uint8_t * pixbuf = decodedImage + row * columns * 4;
			for (int column = 0; column < columns;)
			{
				packetHeader = *buf_p++;
				packetSize = 1 + (packetHeader & 0x7F);

				if (packetHeader & 0x80) // Run-length packet
				{
					switch (header.pixelSize)
					{
					case 24:
						blue  = *buf_p++;
						green = *buf_p++;
						red   = *buf_p++;
						alpha = 255;
						break;

					case 32:
						blue  = *buf_p++;
						green = *buf_p++;
						red   = *buf_p++;
						alpha = *buf_p++;
						break;

					default:
						assert(false);
					} // switch

					for (j = 0; j < packetSize; ++j)
					{
						*pixbuf++ = blue;
						*pixbuf++ = green;
						*pixbuf++ = red;
						*pixbuf++ = alpha;

						++column;
						if (column == columns) // run spans across rows
						{
							column = 0;
							if (row > 0)
							{
								--row;
							}
							else
							{
								goto BREAKOUT;
							}
							pixbuf = decodedImage + row * columns * 4;
						}
					}
				}
				else // Non run-length packet
				{
					for (j = 0; j < packetSize; ++j)
					{
						switch (header.pixelSize)
						{
						case 24:
							blue      = *buf_p++;
							green     = *buf_p++;
							red       = *buf_p++;
							*pixbuf++ = blue;
							*pixbuf++ = green;
							*pixbuf++ = red;
							*pixbuf++ = 255;
							break;

						case 32:
							blue      = *buf_p++;
							green     = *buf_p++;
							red       = *buf_p++;
							alpha     = *buf_p++;
							*pixbuf++ = blue;
							*pixbuf++ = green;
							*pixbuf++ = red;
							*pixbuf++ = alpha;
							break;

						default:
							assert(false);
						} // switch

						++column;
						if (column == columns) // pixel packet run spans across rows
						{
							column = 0;
							if (row > 0)
							{
								--row;
							}
							else
							{
								goto BREAKOUT;
							}
							pixbuf = decodedImage + row * columns * 4;
						}
					}
				}
			}
		BREAKOUT:
			;
		}
	}

	// Flip vertically
	{
		const int maxY = rows - 1;
		const int halfHeight = rows / 2;

		for (int y = 0; y < halfHeight; ++y)
		{
			for (int x = 0; x < columns; ++x)
			{
				std::swap(result[x + y * columns], result[x + (maxY - y) * columns]);
			}
		}
	}

	return result;
}

} // namespace siege {}
