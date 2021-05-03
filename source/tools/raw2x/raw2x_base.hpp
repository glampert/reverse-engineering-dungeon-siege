#pragma once
// ================================================================================================
// -*- C++ -*-
// File: raw2x_base.hpp
// Author: Guilherme R. Lampert
// Created on: 30/01/15
// Brief: Base class for the RAW to X image format converters.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "utils/utils.hpp"
#include "siege/siege.hpp"
#include <iostream>

namespace tools
{

// ========================================================
// Raw2xBase:
// ========================================================

class Raw2xBase
{
public:

	Raw2xBase(int argc, const char * argv[], const char * outputExt, const char * outputType);

	// Runs the tool. The return value of this method is
	// passed on as the return of `main()`.
	int run();

protected:

	// Prints some help text to STDOUT.
	void printHelpText() const;

	// Writes a given surface of the raw image to a file.
	// Each class implementation will output in a different format, e.g.: TGA, PNG.
	virtual void writeImageSurf(const siege::RawImage & rawImage, unsigned int surfIndex,
	                            const std::string & filename, bool swizzlePixels) const = 0;

	virtual ~Raw2xBase();

	// Common data:
	const std::string outputFileExt;  // ".tga" / ".png"
	const std::string outputFileType; // "TGA"  / "PNG"
	const std::string programName;    // argv[0]
	utils::SimpleCmdLineParser cmdLine;

	// Options:
	const bool verbose;
	const bool timings;
	const bool swizzle;
	const bool mipmaps;
};

} // namespace tools {}
