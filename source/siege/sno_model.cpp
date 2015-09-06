
// ================================================================================================
// -*- C++ -*-
// File: sno_model.cpp
// Author: Guilherme R. Lampert
// Created on: 04/09/15
// Brief: Handler for the Dungeon Siege "Siege Nodes" (SNO) 3D geometry files.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/sno_model.hpp"
#include <fstream>

namespace siege
{

// ========================================================
// SnoModel:
// ========================================================

SnoModel::SnoModel(std::string filename, const uint32_t importFlags)
{
	initFromFile(std::move(filename), importFlags);
}

SnoModel::SnoModel(ByteArray fileContents, const uint32_t importFlags, std::string filename)
{
	initFromMemory(std::move(fileContents), importFlags, std::move(filename));
}

void SnoModel::initFromFile(std::string filename, const uint32_t importFlags)
{
	if (filename.empty())
	{
		SiegeThrow(Exception, "No filename provided for SnoModel::initFromFile()!");
	}

	std::ifstream file;
	if (!utils::filesys::tryOpen(file, filename, std::ifstream::binary))
	{
		SiegeThrow(Exception, "Failed to open SNO file \""
				<< filename << "\": '" << utils::filesys::getLastFileError() << "'.");
	}

	size_t fileSizeBytes = 0;
	utils::filesys::queryFileSize(filename, fileSizeBytes);
	if (fileSizeBytes == 0)
	{
		SiegeWarn("SNO file \"" << filename << "\" appears to be empty...");

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
				<< " from SNO mode file \"" << filename << "\"!");
	}

	initFromMemory(std::move(fileContents), importFlags, std::move(filename));
}

void SnoModel::initFromMemory(ByteArray fileContents, const uint32_t importFlags, std::string filename)
{
	dispose(); // Get rid of any existing import.

	SnoImporter importer(*this, std::move(fileContents), importFlags, filename);
	srcFileName = std::move(filename);

	SiegeLog("SnoModel \"" << srcFileName << "\" initialized. "
			<< spots.size() << " spot(s), " << doors.size() << " door(s), "
			<< corners.size() << " corner(s), " << surfaces.size() << " surface(s).");
}

void SnoModel::dispose()
{
	std::memset(&header, 0, sizeof(header));

	spots.clear();
	doors.clear();
	corners.clear();
	surfaces.clear();
	srcFileName.clear();
}

bool SnoModel::isValid() const
{
	return (!surfaces.empty() &&
	         header.magic   == "SNOD" &&
	         header.version >= VersionExpected);
}

} // namespace siege {}
