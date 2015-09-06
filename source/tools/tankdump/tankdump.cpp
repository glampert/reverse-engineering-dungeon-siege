
// ================================================================================================
// -*- C++ -*-
// File: tankdump.cpp
// Author: Guilherme R. Lampert
// Created on: 30/01/15
// Brief: Command line tool to display info about a DS Tank file and also
//        the possibility to decompress it entirely into a normal directory tree.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "utils/utils.hpp"
#include "siege/siege.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

namespace tools
{

// ========================================================
// TankDump:
// ========================================================

class TankDump final
{
public:

	TankDump(int argc, const char * argv[]);
	int run();

private:

	void writeFile(std::string destFileName, const siege::ByteArray & fileContents) const;
	void extractSingleFile();
	void extractAllFiles();

	void printTankHeader() const;
	void printTankFiles()  const;
	void printTankDirs()   const;
	void printHelpText()   const;

	// Inputs/outputs:
	const std::string programName;
	utils::SimpleCmdLineParser cmdLine;
	std::string inputTankFile;
	std::string outputFileDir;

	// Tank file handlers:
	siege::TankFile tankFile;
	siege::TankFile::Reader tankReader;

	// Options:
	const bool verbose;
	const bool timings;
	const bool raw2png; // Convert RAW images to PNG
	const bool raw2tga; // Convert RAW images to TGA
};

// ========================================================

#define VPrint(x) if (verbose) { std::cout << x << "\n"; }

TankDump::TankDump(const int argc, const char * argv[])
	: programName(argv[0])
	, cmdLine(argc, argv)
	, verbose(cmdLine.hasFlag("v") || cmdLine.hasFlag("verbose"))
	, timings(cmdLine.hasFlag("t") || cmdLine.hasFlag("timings"))
	, raw2png(cmdLine.hasFlag("P") || cmdLine.hasFlag("raw2png"))
	, raw2tga(cmdLine.hasFlag("T") || cmdLine.hasFlag("raw2tga"))
{
}

int TankDump::run()
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

	if (cmdLine.getArg(0)[0] == '-')
	{
		std::cerr << "ERROR.: First argument must be the name of a Tank file!" << std::endl;
		return EXIT_FAILURE;
	}

	// Input file + output file/dir
	if (cmdLine.getArgCount() >= 2 && cmdLine.getArg(1)[0] != '-') // If arg[1] is not a flag...
	{
		inputTankFile = cmdLine.getArg(0);
		outputFileDir = cmdLine.getArg(1);
	}
	else // Just an input file
	{
		inputTankFile = cmdLine.getArg(0);
		outputFileDir.clear();
	}

	VPrint("In file......: " << inputTankFile);
	VPrint("Out file/dir.: " << outputFileDir);
	VPrint("Options......: " << cmdLine.getFlagsString());

	// We optionally measure execution time.
	using namespace std::chrono;
	system_clock::time_point t0, t1;

	if (timings)
	{
		t0 = system_clock::now();
	}

	VPrint("Opening Tank \"" << inputTankFile << "\"...");
	tankFile.openForReading(inputTankFile);
	VPrint("Ok.");

	VPrint("Indexing Tank file...");
	tankReader.indexFile(tankFile);
	VPrint("Ok.");

	if (cmdLine.hasFlag("H") || cmdLine.hasFlag("tank_header"))
	{
		printTankHeader();
	}

	if (cmdLine.hasFlag("f") || cmdLine.hasFlag("list_files"))
	{
		printTankFiles();
	}

	if (cmdLine.hasFlag("d") || cmdLine.hasFlag("list_dirs"))
	{
		printTankDirs();
	}

	// You can either extract a single file or the whole
	// archive, but not both at the same time!
	//
	if (cmdLine.hasFlag("e") || cmdLine.hasFlag("extract"))
	{
		extractSingleFile();
	}
	else if (cmdLine.hasFlag("D") || cmdLine.hasFlag("dump_all"))
	{
		extractAllFiles();
	}

	VPrint("Done!");

	if (timings)
	{
		t1 = system_clock::now();

		const duration<double> elapsedSeconds(t1 - t0);
		const auto endTime = system_clock::to_time_t(t1);

		std::cout << "Finished execution on " << std::ctime(&endTime)
		          << "Elapsed time: " << elapsedSeconds.count() << "s\n";
	}

	return 0;
}

void TankDump::writeFile(std::string destFileName, const siege::ByteArray & fileContents) const
{
	assert(!destFileName.empty());

	// User might have provided a name starting with a separator
	// E.g.: "/notes.gas". Remove the prefix separator before continuing.
	if (destFileName[0] == utils::filesys::getPathSeparator()[0])
	{
		destFileName = destFileName.substr(1, destFileName.length());
	}

	// Ensure that the file path is valid.
	// Create the full path if needed.
	utils::filesys::createPath(destFileName);

	// Open/Write the file:
	std::ofstream outFile;
	if (!utils::filesys::tryOpen(outFile, destFileName, std::ofstream::binary))
	{
		SiegeThrow(siege::Exception, "Failed to open file \""
				<< destFileName << "\" for writing!");
	}

	if (fileContents.empty())
	{
		VPrint("Written an empty resource file...");
		return;
	}

	if (!outFile.write(reinterpret_cast<const char *>(fileContents.data()), fileContents.size()))
	{
		SiegeThrow(siege::Exception, "Failed to write " << fileContents.size()
				<< " bytes to file \"" << destFileName << "\"!");
	}
}

void TankDump::extractSingleFile()
{
	assert(tankFile.isOpen());
	if (outputFileDir.empty())
	{
		SiegeThrow(siege::Exception, "`--extract | -e` flag requires a filename as the second parameter!");
	}

	if (outputFileDir[0] != utils::filesys::getPathSeparator()[0])
	{
		outputFileDir = utils::filesys::getPathSeparator() + outputFileDir;
	}

	siege::ByteArray fileContents = tankReader.extractResourceToMemory(tankFile, outputFileDir, /* validateCRCs = */ true);
	writeFile(outputFileDir, fileContents);
	VPrint("Successfully extracted resource file \"" << outputFileDir << "\".");
}

void TankDump::extractAllFiles()
{
	assert(tankFile.isOpen());
	if (outputFileDir.empty())
	{
		SiegeThrow(siege::Exception, "`--dump_all | -D` flag requires a path as the second parameter!");
	}

	VPrint("Extracting whole Tank to path \"" << outputFileDir << "\"...");
	VPrint("------------------------------");

	utils::filesys::createPath(outputFileDir);

	std::vector<siege::TankFile::Task> taskList;
	taskList.reserve(tankReader.getFileCount());

	std::string destFilename, extension;
	std::vector<std::string> fileList = tankReader.getFileList();
	std::sort(std::begin(fileList), std::end(fileList));

	// Walk the file table and decompress each resource asynchronously:
	for (const auto & resourceName : fileList)
	{
		destFilename = outputFileDir + resourceName;
		utils::filesys::createPath(destFilename);

		VPrint("Extracting resource file \"" << resourceName << "\"");

		siege::TankFile::Task task;
		extension = utils::filesys::getFilenameExtension(resourceName);

		// User might want to convert textures to PNG or TGA...
		if (extension == ".raw" && (raw2png || raw2tga))
		{
			auto resourceData = tankReader.extractResourceToMemory(tankFile,
					resourceName, /* validateCRCs = */ true);

			task = std::async(std::launch::async,
				[] (siege::ByteArray imageData, std::string imageName,
				    std::string destFileName, const bool writePng) -> bool
				{
					try
					{
						siege::RawImage rawImage(std::move(imageData), std::move(imageName));
						if (writePng)
						{
							rawImage.writeSurfaceAsPngImage(0,
								utils::filesys::removeFilenameExtension(destFileName) + ".png", true);
						}
						else // Assume TGA
						{
							rawImage.writeSurfaceAsTgaImage(0,
								utils::filesys::removeFilenameExtension(destFileName) + ".tga", false);
						}
						return true;
					}
					catch (std::exception & e)
					{
						std::cerr << "ERROR.: " << e.what() << std::endl;
						return false;
					}
				},
			std::move(resourceData), resourceName, destFilename, raw2png);
		}
		else
		{
			task = tankReader.extractResourceToFileAsync(tankFile,
				resourceName, destFilename, /* validateCRCs = */ true);
		}

		taskList.emplace_back(std::move(task));
	}

	VPrint("------------------------------");

	// Once all files are done, we synchronize.
	int filesExtracted = 0;
	int filesFailed    = 0;
	for (auto & task : taskList)
	{
		if (task.get())
		{
			++filesExtracted;
		}
		else
		{
			++filesFailed;
		}
	}

	if (filesFailed != 0)
	{
		std::cerr << "ERROR.: Failed to extract " << filesFailed << " resource files!" << std::endl;
	}

	VPrint(filesExtracted << " resource files extracted from Tank \"" <<
		tankFile.getFileName() << "\" to path \"" << outputFileDir << "\".");
}

void TankDump::printTankHeader() const
{
	assert(tankFile.isOpen());

	// Local helper lambdas:
	//
	auto toString = [](const siege::WideString & wStr)
	{
		return wStr.empty() ? "<EMPTY>" : ("\"" + siege::wideStringToStdString(wStr) + "\"");
	};
	auto toHexa = [](uint32_t val)
	{
		return utils::format("0x%08X", val);
	};

	const auto header = tankFile.getFileHeader();
	std::cout << "\n";
	std::cout << "-------- TANK HEADER --------\n";
	std::cout << "Product id.........: " << header.productId << "\n";
	std::cout << "Tank id............: " << header.tankId << "\n";
	std::cout << "Header version.....: " << siege::versionWordToStr(header.headerVersion) << "\n";
	std::cout << "DirSet offset......: " << toHexa(header.dirsetOffset)  << " (" << utils::formatMemoryUnit(header.dirsetOffset)  << ")" << "\n";
	std::cout << "FileSet offset.....: " << toHexa(header.filesetOffset) << " (" << utils::formatMemoryUnit(header.filesetOffset) << ")" << "\n";
	std::cout << "Index size.........: " << utils::formatMemoryUnit(header.indexSize) << "\n";
	std::cout << "Data offset........: " << toHexa(header.dataOffset) << " (" << utils::formatMemoryUnit(header.dataOffset) << ")" << "\n";
	std::cout << "Product version....: " << header.productVersion << "\n";
	std::cout << "Minimum version....: " << header.minimumVersion << "\n";
	std::cout << "Priority...........: " << siege::TankFile::priorityToString(header.priority) << "\n";
	std::cout << "Flags..............: " << header.flags << "\n";
	std::cout << "Creator id.........: " << header.creatorId << "\n";
	std::cout << "GUID...............: " << header.guid << "\n";
	std::cout << "Index CRC-32.......: " << toHexa(header.indexCrc32) << "\n";
	std::cout << "Data CRC-32........: " << toHexa(header.dataCrc32) << "\n";
	std::cout << "UTC build time.....: " << header.utcBuildTime << "\n";
	std::cout << "Copyright text.....: " << toString(header.copyrightText) << "\n";
	std::cout << "Build text.........: " << toString(header.buildText) << "\n";
	std::cout << "Title text.........: " << toString(header.titleText) << "\n";
	std::cout << "Author text........: " << toString(header.authorText) << "\n";
	std::cout << "Description text...: " << toString(header.descriptionText) << "\n";
	std::cout << "\n";
	std::cout << "Tank file has a total of " << tankReader.getDirectoryCount() << " directories";
	std::cout << " and " << tankReader.getFileCount() << " files.\n";
	std::cout << "Tank file size: " << utils::formatMemoryUnit(tankFile.getFileSizeBytes()) << "\n";
	std::cout << "\n";
}

void TankDump::printTankFiles() const
{
	assert(tankFile.isOpen());

	std::vector<std::string> fileList = tankReader.getFileList();
	std::sort(std::begin(fileList), std::end(fileList));

	std::cout << "\n";
	std::cout << "-------- TANK FILES --------\n";

	for (size_t f = 0; f < fileList.size(); ++f)
	{
		std::cout << "[" << std::setw(4) << std::setfill('0') << f << "] " << fileList[f] << "\n";
	}

	std::cout << "Listed " << fileList.size() << " files.\n";
	std::cout << "\n";
}

void TankDump::printTankDirs() const
{
	assert(tankFile.isOpen());

	std::vector<std::string> dirList = tankReader.getDirectoryList();
	std::sort(std::begin(dirList), std::end(dirList));

	std::cout << "\n";
	std::cout << "-------- TANK DIRECTORIES --------\n";

	for (size_t d = 0; d < dirList.size(); ++d)
	{
		std::cout << "[" << std::setw(4) << std::setfill('0') << d << "] " << dirList[d] << "\n";
	}

	std::cout << "Listed " << dirList.size() << " directories.\n";
	std::cout << "\n";
}

void TankDump::printHelpText() const
{
	std::cout << "Usage:\n";
	std::cout << "$ " << programName << " <tank_file> [decompressed_directory | decompressed_file] [options]\n";
	std::cout << " Display information about a Dungeon Siege Tank file.\n";
	std::cout << " Tank files usually end with the extensions `.dsres` or `.dsm`\n";
	std::cout << " This tool can optionally decompress a Tank into a directory tree in the local File System.\n";
	std::cout << " Options are:\n";
	std::cout << "  -h, --help        Prints this help text and exits.\n";
	std::cout << "  -v, --verbose     If present enables verbose output about the program execution.\n";
	std::cout << "  -t, --timings     If present prints the time taken to process the file(s).\n";
	std::cout << "  -H, --tank_header Displays the Tank file header and exits.\n";
	std::cout << "  -f, --list_files  Displays a list of all FILES in the Tank.\n";
	std::cout << "  -d, --list_dirs   Displays a list of all DIRECTORIES in the Tank.\n";
	std::cout << "  -P, --raw2png     Converts all RAW images to PNG before writing to file (only the 1st surface).\n";
	std::cout << "  -T, --raw2tga     Converts all RAW images to TGA before writing to file (only the 1st surface).\n";
	std::cout << "  -e, --extract     The second parameter is the name of a file that is to be extracted from the Tank.\n";
	std::cout << "  -D, --dump_all    The second parameter is the name of a directory where the whole Tank is to be decompressed into.\n";
	std::cout << "                    The output directory will be created if it does not exists.\n";
	std::cout << "\n";
	std::cout << "Created by Guilherme R. Lampert, " << __DATE__ << ".\n";
}

#undef VPrint

} // namespace tool {}

// ========================================================
// main():
// ========================================================

int main(int argc, const char * argv[])
{
	siege::setDefaultLogStream(std::cout);

	// Set the log to always silent for this program.
	// Our `--verbose` flag does not rely on the Siege Log system.
	siege::defaultLogVerbosity = siege::LogVerbosity::Silent;

	try
	{
		tools::TankDump tankdump(argc, argv);
		return tankdump.run();
	}
	catch (std::exception & e)
	{
		std::cerr << "ERROR.: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
