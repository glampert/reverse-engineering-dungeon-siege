
// ================================================================================================
// -*- C++ -*-
// File: raw2tga.cpp
// Author: Guilherme R. Lampert
// Created on: 29/01/15
// Brief: Command line tool that converts a Dungeon Siege RAW image file into a TGA image file.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "tools/raw2x/raw2x_base.hpp"

namespace tools
{

// ========================================================
// Raw2Tga:
// ========================================================

class Raw2Tga final
	: public Raw2xBase
{
public:

	 Raw2Tga(const int argc, const char * argv[]);
	~Raw2Tga();

	void writeImageSurf(const siege::RawImage & rawImage, const unsigned int surfIndex,
	                    const std::string & filename, const bool swizzlePixels) const override;
};

Raw2Tga::Raw2Tga(const int argc, const char * argv[])
	: Raw2xBase(argc, argv, ".tga", "TGA")
{ }

Raw2Tga::~Raw2Tga()
{ }

void Raw2Tga::writeImageSurf(const siege::RawImage & rawImage, const unsigned int surfIndex,
                             const std::string & filename, const bool swizzlePixels) const
{
	rawImage.writeSurfaceAsTgaImage(surfIndex, filename, swizzlePixels);
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
		tools::Raw2Tga raw2tga(argc, argv);
		return raw2tga.run();
	}
	catch (std::exception & e)
	{
		std::cerr << "ERROR.: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
