
// ================================================================================================
// -*- C++ -*-
// File: asp_model.cpp
// Author: Guilherme R. Lampert
// Created on: 20/12/14
// Brief: Handler for the Dungeon Siege "Aspects" (ASP) 3D model format.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/asp_model.hpp"
#include <fstream>

namespace siege
{

// ========================================================
// AspModel:
// ========================================================

AspModel::AspModel(std::string filename, const uint32_t importFlags)
{
	initFromFile(std::move(filename), importFlags);
}

AspModel::AspModel(ByteArray fileContents, const uint32_t importFlags, std::string filename)
{
	initFromMemory(std::move(fileContents), importFlags, std::move(filename));
}

void AspModel::initFromFile(std::string filename, const uint32_t importFlags)
{
	if (filename.empty())
	{
		SiegeThrow(Exception, "No filename provided for AspModel::initFromFile()!");
	}

	std::ifstream file;
	if (!utils::filesys::tryOpen(file, filename, std::ifstream::binary))
	{
		SiegeThrow(Exception, "Failed to open ASP file \""
				<< filename << "\": '" << utils::filesys::getLastFileError() << "'.");
	}

	size_t fileSizeBytes = 0;
	utils::filesys::queryFileSize(filename, fileSizeBytes);
	if (fileSizeBytes == 0)
	{
		SiegeWarn("ASP file \"" << filename << "\" appears to be empty...");

		// Make this an empty model.
		// NOTE: Should this be changed to an error instead?
		dispose();
		srcFileName = std::move(filename);
		return;
	}

	ByteArray fileContents(fileSizeBytes);
	if (!file.read(reinterpret_cast<char *>(fileContents.data()), fileContents.size()))
	{
		SiegeThrow(Exception, "Failed to read " << utils::formatMemoryUnit(fileContents.size())
				<< " from ASP mode file \"" << filename << "\"!");
	}

	initFromMemory(std::move(fileContents), importFlags, std::move(filename));
}

void AspModel::initFromMemory(ByteArray fileContents, const uint32_t importFlags, std::string filename)
{
	dispose(); // Get rid of any existing import.

	AspImporter importer(*this, std::move(fileContents), importFlags, filename);
	srcFileName = std::move(filename);

	SiegeLog("AspModel \"" << srcFileName << "\" initialized. "
			<< subMeshes.size() << " sub-mesh(es), " << boneInfos.size()
			<< " bone(s), " << textureNames.size() << " texture(s).");
}

void AspModel::dispose()
{
	subMeshes.clear();
	boneInfos.clear();
	textureNames.clear();
	srcFileName.clear();
}

bool AspModel::isValid() const
{
	// Must have at least one mesh to be a valid model.
	return !subMeshes.empty();
}

} // namespace siege {}
