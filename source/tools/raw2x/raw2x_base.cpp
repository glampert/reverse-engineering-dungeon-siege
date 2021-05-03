
// ================================================================================================
// -*- C++ -*-
// File: raw2x_base.cpp
// Author: Guilherme R. Lampert
// Created on: 30/01/15
// Brief: Base class for the RAW to X image format converters.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "tools/raw2x/raw2x_base.hpp"
#include <chrono>

namespace tools
{

// ========================================================
// Raw2xBase:
// ========================================================

Raw2xBase::Raw2xBase(const int argc, const char * argv[],
                     const char * outputExt, const char * outputType)
	: outputFileExt(outputExt)
	, outputFileType(outputType)
	, programName(argv[0])
	, cmdLine(argc, argv)
	, verbose(cmdLine.hasFlag("v") || cmdLine.hasFlag("verbose"))
	, timings(cmdLine.hasFlag("t") || cmdLine.hasFlag("timings"))
	, swizzle(cmdLine.hasFlag("s") || cmdLine.hasFlag("swizzle"))
	, mipmaps(cmdLine.hasFlag("m") || cmdLine.hasFlag("mipmaps"))
{
	if (verbose)
	{
		siege::defaultLogVerbosity = siege::LogVerbosity::All;
	}
	else
	{
		siege::defaultLogVerbosity = siege::LogVerbosity::Silent;
	}
}

Raw2xBase::~Raw2xBase()
{
	// Implemented here to avoid the `-Wweak-vtables` warning.
}

int Raw2xBase::run()
{
	if (cmdLine.getArgCount() == 0)
	{
		std::cout << "Not enough arguments!\n";
		printHelpText();
		return 0;
	}

	if (cmdLine.hasFlag("h") || cmdLine.hasFlag("help"))
	{
		printHelpText();
		return 0;
	}

	std::string inFileName, outFileName;

	// input_file + output_file
	if (cmdLine.getArgCount() >= 2 && cmdLine.getArg(1)[0] != '-') // If arg[1] is not a flag...
	{
		inFileName  = cmdLine.getArg(0);
		outFileName = cmdLine.getArg(1);
	}
	else // Just input_file
	{
		inFileName = cmdLine.getArg(0);
		outFileName.clear();
	}

	// Replace '.raw' extension of source file with the proper extension
	// and use it for the output if no explicit filename was provided.
	if (outFileName.empty())
	{
		outFileName = utils::filesys::removeFilenameExtension(inFileName) + outputFileExt;
	}

	if (verbose)
	{
		std::cout << "In file..: " << inFileName  << "\n";
		std::cout << "Out file.: " << outFileName << "\n";
		std::cout << "Options..: " << cmdLine.getFlagsString() << "\n";
	}

	// We optionally measure execution time.
	using namespace std::chrono;
	system_clock::time_point t0, t1;

	if (timings)
	{
		t0 = system_clock::now();
	}

	// Try to open the input file. This might result in an exception.
	siege::RawImage rawImage(inFileName);

	if (rawImage.getSurfaceCount() > 1 && mipmaps)
	{
		std::string surfName;
		const int surfCount = rawImage.getSurfaceCount();

		for (int s = 0; s < surfCount; ++s)
		{
			surfName = utils::filesys::removeFilenameExtension(outFileName) + "_" + std::to_string(s) + outputFileExt;
			writeImageSurf(rawImage, s, surfName, swizzle);
		}
	}
	else // Single image (mipmap 0):
	{
		writeImageSurf(rawImage, 0, outFileName, swizzle);
	}

	if (timings)
	{
		t1 = system_clock::now();

		const duration<double> elapsedSeconds(t1 - t0);
		const auto endTime = system_clock::to_time_t(t1);

#ifdef _MSC_VER
		char timeStr[256];
		ctime_s(timeStr, sizeof(timeStr), &endTime);
#else // _MSC_VER
		const char * const timeStr = std::ctime(&endTime);
#endif // _MSC_VER

		std::cout << "Finished execution on " << timeStr
		          << "Elapsed time: " << elapsedSeconds.count() << "s\n";
	}

	return 0;
}

void Raw2xBase::printHelpText() const
{
	std::cout << "Usage:\n";
	std::cout << "$ " << programName << " <input_file> [output_file] [options]\n";
	std::cout << " Converts a Dungeon Siege RAW image to a " << outputFileType << " image.\n";
	std::cout << " If the output filename is not provided the input name is used but its extension is replaced with `" << outputFileExt << "`.\n";
	std::cout << " Options are:\n";
	std::cout << "  -h, --help    Prints this help text and exits.\n";
	std::cout << "  -v, --verbose If present enables verbose output about the program execution.\n";
	std::cout << "  -t, --timings If present prints the time taken to process the files.\n";
	std::cout << "  -s, --swizzle If present swizzle the RGBA color of each image pixel to BGRA, or vice-versa.\n";
	std::cout << "  -m, --mipmaps If present also dumps each mipmap of the original RAW image as a " << outputFileType << " file.\n";
	std::cout << "                Each mipmap level will be named as \"output_file_<mip_num>" << outputFileExt << "\".\n";
	std::cout << "\n";
	std::cout << "Created by Guilherme R. Lampert, " << __DATE__ << ".\n";
}

} // namespace tools {}
