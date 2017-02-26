
// ================================================================================================
// -*- C++ -*-
// File: raw2png.cpp
// Author: Guilherme R. Lampert
// Created on: 29/01/15
// Brief: Command line tool that converts a Dungeon Siege RAW image file into a PNG image file.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "tools/raw2x/raw2x_base.hpp"

namespace tools
{

// ========================================================
// Raw2Png:
// ========================================================

class Raw2Png final
	: public Raw2xBase
{
public:

	 Raw2Png(const int argc, const char * argv[]);
	~Raw2Png();

	void writeImageSurf(const siege::RawImage & rawImage, unsigned int surfIndex,
	                    const std::string & filename, bool swizzlePixels) const override;
};

Raw2Png::Raw2Png(const int argc, const char * argv[])
	: Raw2xBase(argc, argv, ".png", "PNG")
{ }

Raw2Png::~Raw2Png()
{ }

void Raw2Png::writeImageSurf(const siege::RawImage & rawImage, unsigned int surfIndex,
                             const std::string & filename, bool swizzlePixels) const
{
	rawImage.writeSurfaceAsPngImage(surfIndex, filename, swizzlePixels);
}

} // namespace tools {}

// ========================================================
// main():
// ========================================================

int main(int argc, const char * argv[])
{
	siege::setDefaultLogStream(std::cout);

	try
	{
		tools::Raw2Png raw2png(argc, argv);
		return raw2png.run();
	}
	catch (std::exception & e)
	{
		std::cerr << "ERROR.: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
