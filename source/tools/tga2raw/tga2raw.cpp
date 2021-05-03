
// ================================================================================================
// -*- C++ -*-
// File: tga2raw.cpp
// Author: Guilherme R. Lampert
// Created on: 03/05/21
// Brief: Command line tool that converts a TGA image back into Dungeon Siege RAW image format.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/siege.hpp"
#include "utils/utils.hpp"
#include "utils/simple_cmdline_parser.hpp"

#include <iostream>
#include <chrono>

namespace tools
{

// ========================================================
// Tga2Raw:
// ========================================================

class Tga2Raw final
{
public:

	Tga2Raw(int argc, const char * argv[]);
	~Tga2Raw() = default;

	int run();

private:

	// Prints some help text to STDOUT.
	void printHelpText() const;

	const std::string programName; // argv[0]
	utils::SimpleCmdLineParser cmdLine;

	// Options:
	const bool verbose;
	const bool timings;
	const bool swizzle;
};

Tga2Raw::Tga2Raw(const int argc, const char * argv[])
	: programName(argv[0])
	, cmdLine(argc, argv)
	, verbose(cmdLine.hasFlag("v") || cmdLine.hasFlag("verbose"))
	, timings(cmdLine.hasFlag("t") || cmdLine.hasFlag("timings"))
	, swizzle(cmdLine.hasFlag("s") || cmdLine.hasFlag("swizzle"))
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

int Tga2Raw::run()
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

	// Replace '.tga' extension of source file with the proper extension
	// and use it for the output if no explicit filename was provided.
	if (outFileName.empty())
	{
		outFileName = utils::filesys::removeFilenameExtension(inFileName) + ".raw";
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

	// Load TGA
	int width = 0, height = 0;
	auto pixels = siege::loadTgaImageFromFile(inFileName, &width, &height);

	// Convert to Raw
	siege::RawImage rawImage;
	const bool swizzlePixels = false;
	rawImage.initFromPixelBuffer(pixels.get(), width, height, swizzlePixels, outFileName);

	// Write as .raw file
	rawImage.writeToFile();

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

		std::cout << "Finished execution on " << timeStr << "Elapsed time: " << elapsedSeconds.count() << "s\n";
	}

	return 0;
}

void Tga2Raw::printHelpText() const
{
	std::cout << "Usage:\n";
	std::cout << "$ " << programName << " <input_file> [output_file] [options]\n";
	std::cout << " Converts a TGA Image back to a Dungeon Siege RAW image file.\n";
	std::cout << " If the output filename is not provided the input name is used but its extension is replaced with `.raw`.\n";
	std::cout << " Options are:\n";
	std::cout << "  -h, --help    Prints this help text and exits.\n";
	std::cout << "  -v, --verbose If present enables verbose output about the program execution.\n";
	std::cout << "  -t, --timings If present prints the time taken to process the files.\n";
	std::cout << "  -s, --swizzle If present swizzle the RGBA color of each image pixel to BGRA, or vice-versa.\n";
	std::cout << "\n";
	std::cout << "Created by Guilherme R. Lampert, " << __DATE__ << ".\n";
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
		tools::Tga2Raw tga2raw(argc, argv);
		return tga2raw.run();
	}
	catch (std::exception & e)
	{
		std::cerr << "ERROR.: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
