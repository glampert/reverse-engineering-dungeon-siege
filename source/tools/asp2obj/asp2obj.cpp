
// ================================================================================================
// -*- C++ -*-
// File: asp2obj.cpp
// Author: Guilherme R. Lampert
// Created on: 03/09/15
// Brief: Command line tool that converts a Dungeon Siege ASP model into a static Wavefront OBJ.
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
// Asp2Obj:
// ========================================================

class Asp2Obj final
{
public:

	Asp2Obj(int argc, const char * argv[]);
	int run();

private:

	void printHelpText() const;
	void writeObjFile(std::ofstream & outFile) const;
	void writeMtlFile(std::ofstream & outFile, const std::string & texFileNameExt) const;

	siege::AspModel model;
	std::string inputFileName;
	std::string objFileName;
	std::string mtlFileName;

	const std::string programName; // argv[0]
	utils::SimpleCmdLineParser cmdLine;

	// Options:
	const bool verbose;
	const bool timings;
	float modelScale;
};

// ========================================================

#define VPrint(x) if (verbose) { std::cout << x << "\n"; }

Asp2Obj::Asp2Obj(int argc, const char * argv[])
	: programName(argv[0])
	, cmdLine(argc, argv)
	, verbose(cmdLine.hasFlag("v") || cmdLine.hasFlag("verbose"))
	, timings(cmdLine.hasFlag("t") || cmdLine.hasFlag("timings"))
	, modelScale(1.0f)
{
}

int Asp2Obj::run()
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
		std::cerr << "ERROR.: First argument must be the name of an ASP file!" << std::endl;
		return EXIT_FAILURE;
	}

	// Input file + output file
	if (cmdLine.getArgCount() >= 2 && cmdLine.getArg(1)[0] != '-') // If arg[1] is not a flag...
	{
		inputFileName = cmdLine.getArg(0);
		objFileName   = cmdLine.getArg(1);
		mtlFileName   = utils::filesys::removeFilenameExtension(objFileName) + ".mtl";
	}
	else // Just an input file
	{
		inputFileName = cmdLine.getArg(0);
		std::string baseName = utils::filesys::removeFilenameExtension(inputFileName);
		objFileName = baseName + ".obj";
		mtlFileName = baseName + ".mtl";
	}

	// Otherwise it will be the default 1.0f.
	utils::CmdLineFlag modelScaleFlag;
	if (cmdLine.getFlag("scale", modelScaleFlag))
	{
		modelScale = std::stof(modelScaleFlag.value);
	}

	VPrint("Input file.....: " << inputFileName);
	VPrint("OBJ output.....: " << objFileName);
	VPrint("MTL output.....: " << mtlFileName);
	VPrint("Options........: " << cmdLine.getFlagsString());
	VPrint("Model scale....: " << modelScale);

	// We optionally measure execution time.
	using namespace std::chrono;
	system_clock::time_point t0, t1;

	if (timings)
	{
		t0 = system_clock::now();
	}

	model.initFromFile(inputFileName);

	// User might have provided a name starting with a separator
	// Remove the prefix separator before continuing.
	if (objFileName[0] == utils::filesys::getPathSeparator()[0])
	{
		objFileName = objFileName.substr(1, objFileName.length());
	}
	if (mtlFileName[0] == utils::filesys::getPathSeparator()[0])
	{
		mtlFileName = mtlFileName.substr(1, mtlFileName.length());
	}

	// Ensure that the file path is valid.
	// Create the full path if needed.
	if (!utils::filesys::createPath(objFileName))
	{
		SiegeThrow(siege::Exception, "Failed to create path \"" << objFileName << "\": " << utils::filesys::getLastFileError());
	}

	// Open/Write the .OBJ file:
	{
		std::ofstream outFile;
		if (!utils::filesys::tryOpen(outFile, objFileName))
		{
			SiegeThrow(siege::Exception, "Failed to open file \""
					<< objFileName << "\" for writing!");
		}

		writeObjFile(outFile);
	}

	// Open/Write the .MTL (material info) file:
	{
		std::ofstream outFile;
		if (!utils::filesys::tryOpen(outFile, mtlFileName))
		{
			SiegeThrow(siege::Exception, "Failed to open file \""
					<< mtlFileName << "\" for writing!");
		}

		utils::CmdLineFlag texExtFlag;
		if (cmdLine.getFlag("tex_ext", texExtFlag))
		{
			writeMtlFile(outFile, texExtFlag.value);
		}
		else
		{
			writeMtlFile(outFile, "");
		}
	}

	VPrint("Done!");

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

void Asp2Obj::writeObjFile(std::ofstream & outFile) const
{
	assert(outFile.is_open());
	VPrint("Writing OBJ...");

	int subMeshIndex = 0;
	const auto & subMeshes = model.getSubMeshes();

	outFile << "\n# File generated by asp2obj from Dungeon Siege ASPECT \"" << inputFileName << "\".\n\n";
	outFile << "mtllib " << mtlFileName << "\n\n";

	// Per-vertex info:
	for (const auto & mesh : subMeshes)
	{
		outFile << "g AspMesh_" << subMeshIndex++ << "\n";

		// Vertexes:
		for (const auto & c : mesh.wCorners)
		{
			const utils::Vec3 v = {
				 (c.pos.x * modelScale),
				-(c.pos.z * modelScale),
				 (c.pos.y * modelScale) };

			outFile << "v " << v.x << " " << v.y << " " << v.z << "\n";
		}
		outFile << "\n";

		// Vertex normals:
		for (const auto & c : mesh.wCorners)
		{
			const utils::Vec3 & n = c.normal;
			outFile << "vn " << n.x << " " << n.y << " " << n.z << "\n";
		}
		outFile << "\n";

		// Texture coordinates:
		for (const auto & c : mesh.wCorners)
		{
			const utils::Vec2 & t = c.texCoord;
			outFile << "vt " << t.x << " " << t.y << "\n";
		}
		outFile << "\n";
	}

	// Faces:
	subMeshIndex = 0;
	int cornerOffset = 0;
	const auto & modelTextures = model.getTextureNames();

	for (const auto & mesh : subMeshes)
	{
		outFile << "g AspMesh_" << subMeshIndex++ << "\n";

		int f = 0;
		for (uint32_t i = 0; i < mesh.textureCount; ++i)
		{
			outFile << "usemtl " << modelTextures[mesh.matInfo[i].textureIndex] << "\n";
			outFile << "s 1\n"; // Allow smooth shading.

			for (uint32_t j = 0; j < mesh.matInfo[i].faceSpan; ++j)
			{
				const auto offset = mesh.faceInfo.cornerStart[i] + cornerOffset;
				const auto a = mesh.faceInfo.cornerIndex[f].index[0] + offset + 1;
				const auto b = mesh.faceInfo.cornerIndex[f].index[1] + offset + 1;
				const auto c = mesh.faceInfo.cornerIndex[f].index[2] + offset + 1; // +1 for the OBJ

				// Position + texture + normal
				outFile << "f " << a << "/" << a << "/" << a << " "
				                << b << "/" << b << "/" << b << " "
				                << c << "/" << c << "/" << c << "\n";
				++f;
			}
		}
		cornerOffset += mesh.cornerCount;
	}
	outFile << "\n";

	VPrint("OBJ Finished.");
}

void Asp2Obj::writeMtlFile(std::ofstream & outFile, const std::string & texFileNameExt) const
{
	assert(outFile.is_open());
	VPrint("Writing MTL...");

	const auto & subMeshes     = model.getSubMeshes();
	const auto & modelTextures = model.getTextureNames();

	outFile << "\n";
	for (const auto & mesh : subMeshes)
	{
		for (uint32_t i = 0; i < mesh.textureCount; ++i)
		{
			const auto & textureName = modelTextures[mesh.matInfo[i].textureIndex];

			outFile << "newmtl " << textureName << "\n";
			outFile << "Ka 0.00 0.00 0.00\n"; // Ambient
			outFile << "Kd 1.00 1.00 1.00\n"; // Diffuse
			outFile << "Ks 0.50 0.50 0.50\n"; // Specular
			outFile << "Ns 95.00\n"; // Specular exponent/power
			outFile << "map_Kd " << (textureName + texFileNameExt) << "\n\n";
		}
	}
	outFile << "\n";

	VPrint("MTL Finished.");
}

void Asp2Obj::printHelpText() const
{
	std::cout << "Usage:\n";
	std::cout << "$ " << programName << " <input_file> [output_file] [options]\n";
	std::cout << " Converts a Dungeon Siege ASP model to a static Wavefront OBJ model.\n";
	std::cout << " If the output filename is not provided the input name is used but its extension is replaced with `.obj`.\n";
	std::cout << " Options are:\n";
	std::cout << "  -h, --help      Prints this help text and exits.\n";
	std::cout << "  -v, --verbose   If present enables verbose output about the program execution.\n";
	std::cout << "  -t, --timings   If present prints the time taken to process the files.\n";
	std::cout << "  --scale=<val>   If present the model vertexes are scaled by that amount. Otherwise it defaults to 1.\n";
	std::cout << "  --tex_ext=<val> Filename extension to use on texture filenames in the MTL. No extension by default.\n";
	std::cout << "\n";
	std::cout << "Created by Guilherme R. Lampert, " << __DATE__ << ".\n";
}

#undef VPrint

} // namespace tools {}

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
		tools::Asp2Obj asp2obj(argc, argv);
		return asp2obj.run();
	}
	catch (std::exception & e)
	{
		std::cerr << "ERROR.: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
