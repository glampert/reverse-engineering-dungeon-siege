
// ================================================================================================
// -*- C++ -*-
// File: sno_importer.cpp
// Author: Guilherme R. Lampert
// Created on: 04/09/15
// Brief: Definition of the SnoImporter inner class of SnoModel.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/sno_model.hpp"
#include <cmath> // For std::isnan()

namespace siege
{

// ========================================================
// Local helpers:
// ========================================================

// Can be easily disabled to avoid excessively verbose output.
#if SIEGE_SNO_DEBUG
	#define SnoLog SiegeLog
#else // !SIEGE_SNO_DEBUG
	#ifndef SnoLog
		#define SnoLog(x) /* nothing */
	#endif // SnoLog
#endif // SIEGE_SNO_DEBUG

// ========================================================
// SnoModel::SnoImporter:
// ========================================================

SnoModel::SnoImporter::SnoImporter(SnoModel & mdl, ByteArray fileData,
                                   const uint32_t impFlags, const std::string & filename)
	: model(mdl)
	, importFlags(impFlags)
	, readPosition(0)
	, fileContents(std::move(fileData))
	, srcFileName(filename)
{
	assert(!fileContents.empty());
	importSnoModel();

	// TODO actually use the importFlags !!!
	(void)importFlags;
}

void SnoModel::SnoImporter::readBytes(void * buffer, const size_t numBytes)
{
	assert(buffer   != nullptr);
	assert(numBytes != 0);
	assert(!fileContents.empty());

	if (readPosition == fileContents.size() || (readPosition + numBytes) > fileContents.size())
	{
		SiegeThrow(Exception, "Trying to read past the end of SNO file \"" << srcFileName << "\"!");
	}

	const uint8_t * dataPtr = fileContents.data() + readPosition;
	std::memcpy(buffer, dataPtr, numBytes);
	readPosition += numBytes;
}

void SnoModel::SnoImporter::readFloat4x3(float values[][3])
{
	//
	// NOTE: This might need some tweaking.
	// I'm not 100% sure if the matrices where stored
	// row or column major... This assumes *row-major*.
	//

	// Rotation 3x3 matrix:
	values[0][0] = readF32();
	values[0][1] = readF32();
	values[0][2] = readF32();

	values[1][0] = readF32();
	values[1][1] = readF32();
	values[1][2] = readF32();

	values[2][0] = readF32();
	values[2][1] = readF32();
	values[2][2] = readF32();

	// Position/translation vector:
	values[3][0] = readF32();
	values[3][1] = readF32();
	values[3][2] = readF32();
}

uint8_t SnoModel::SnoImporter::readU8()
{
	uint8_t x = 0;
	readBytes(&x, sizeof(x));
	return x;
}

uint16_t SnoModel::SnoImporter::readU16()
{
	uint16_t x = 0;
	readBytes(&x, sizeof(x));
	return x;
}

uint32_t SnoModel::SnoImporter::readU32()
{
	uint32_t x = 0;
	readBytes(&x, sizeof(x));
	return x;
}

float SnoModel::SnoImporter::readF32()
{
	static_assert(sizeof(float) == 4, "This assumes a 32bit float type!");

	float x = 0;
	readBytes(&x, sizeof(x));

	#if SIEGE_SNO_DEBUG
	if (std::isnan(x))
	{
		SiegeWarn("readF32(): NAN float data inside SNO file!");
	}
	#endif // SIEGE_SNO_DEBUG

	return x;
}

std::string SnoModel::SnoImporter::readString()
{
	std::string tmp;
	for (;;)
	{
		// Strings are terminated by a null byte.
		char c = '\0';
		readBytes(&c, 1);
		if (c == '\0')
		{
			break;
		}
		tmp.push_back(c);
	}
	return tmp;
}

void SnoModel::SnoImporter::readHeader()
{
	readBytes(&model.header, sizeof(SnoModel::Header));

	auto toHexa = [](uint32_t val) { return utils::format("0x%08X", val); };
	SnoLog("header.magic.........: " << model.header.magic);
	SnoLog("header.version.......: " << model.header.version)
	SnoLog("header.unused0.......: " << model.header.unused0);
	SnoLog("header.doorCount.....: " << model.header.doorCount);
	SnoLog("header.spotCount.....: " << model.header.spotCount);
	SnoLog("header.cornerCount...: " << model.header.cornerCount);
	SnoLog("header.faceCount.....: " << model.header.faceCount);
	SnoLog("header.textureCount..: " << model.header.textureCount);
	SnoLog("header.minBBox.......: " << utils::toString(model.header.minBBox));
	SnoLog("header.maxBBox.......: " << utils::toString(model.header.maxBBox));
	SnoLog("header.unused1.......: " << toHexa(model.header.unused1));
	SnoLog("header.unused2.......: " << toHexa(model.header.unused2));
	SnoLog("header.unused3.......: " << toHexa(model.header.unused3));
	SnoLog("header.unused4.......: " << toHexa(model.header.unused4));
	SnoLog("header.unused5.......: " << toHexa(model.header.unused5));
	SnoLog("header.unused6.......: " << toHexa(model.header.unused6));
	SnoLog("header.unused7.......: " << toHexa(model.header.unused7));
	SnoLog("header.dataCrc32.....: " << toHexa(model.header.dataCrc32));

	if (model.header.magic != "SNOD")
	{
		SiegeThrow(Exception, "SNO file is not a valid Siege Node! Bad header magic! "
				<< model.header.magic);
	}

	if (model.header.version < VersionExpected)
	{
		SiegeThrow(Exception, "SNO file is not a version " << VersionExpected
				<< " Siege Node! " << model.header.version);
	}
}

void SnoModel::SnoImporter::readSpots()
{
	SnoLog("====== readSpots() ======");

	if (model.header.spotCount == 0)
	{
		SnoLog("No spots.");
		return;
	}

	model.spots.resize(model.header.spotCount);
	for (uint32_t s = 0; s < model.header.spotCount; ++s)
	{
		readFloat4x3(model.spots[s].xform);
		model.spots[s].name = readString();
		SnoLog("Spot name: " << model.spots[s].name);
	}

	SnoLog("Read " << model.header.spotCount << " spots.");
}

void SnoModel::SnoImporter::readDoors()
{
	SnoLog("====== readDoors() ======");

	if (model.header.doorCount == 0)
	{
		SnoLog("No doors.");
		return;
	}

	model.doors.resize(model.header.doorCount);
	for (uint32_t d = 0; d < model.header.doorCount; ++d)
	{
		model.doors[d].index = readU32();
		readFloat4x3(model.doors[d].xform);
		const uint32_t hotSpotCount = readU32();

		SnoLog("Door index " << model.doors[d].index << " has " << hotSpotCount << " hot-spots.");

		if (hotSpotCount == 0)
		{
			continue;
		}

		model.doors[d].hotSpots.resize(hotSpotCount);
		for (uint32_t h = 0; h < hotSpotCount; ++h)
		{
			model.doors[d].hotSpots[h] = readU32();
			SnoLog("Door[" << d << "].hotSpot[" << h << "] = " << model.doors[d].hotSpots[h]);
		}
	}

	SnoLog("Read " << model.header.doorCount << " doors.");
}

void SnoModel::SnoImporter::readCorners()
{
	SnoLog("====== readCorners() ====");

	if (model.header.cornerCount == 0)
	{
		SnoLog("No corners.");
		return;
	}

	auto & mdlCorners = model.corners;
	mdlCorners.resize(model.header.cornerCount);

	for (uint32_t c = 0; c < model.header.cornerCount; ++c)
	{
		mdlCorners[c].pos.x = readF32();
		mdlCorners[c].pos.y = readF32();
		mdlCorners[c].pos.z = readF32();

		mdlCorners[c].normal.x = readF32();
		mdlCorners[c].normal.y = readF32();
		mdlCorners[c].normal.z = readF32();

		// Swizzle back to RGBA (why the heck this insane layout??)
		const auto rrr = readU8();
		const auto bbb = readU8();
		const auto ggg = readU8();
		const auto aaa = readU8();

		mdlCorners[c].color.x = rrr;
		mdlCorners[c].color.y = ggg;
		mdlCorners[c].color.z = bbb;
		mdlCorners[c].color.w = aaa;

		mdlCorners[c].texCoord.x = readF32();
		mdlCorners[c].texCoord.y = readF32();
	}

	SnoLog("Read " << model.header.cornerCount << " corners.");
}

void SnoModel::SnoImporter::readSurfaces()
{
	SnoLog("====== readSurfaces() ===");

	if (model.header.textureCount == 0) // One texture per surface, it seems.
	{
		SnoLog("No surfaces.");
		return;
	}

	auto & mdlSurfaces = model.surfaces;
	mdlSurfaces.resize(model.header.textureCount);

	for (uint32_t s = 0; s < model.header.textureCount; ++s)
	{
		mdlSurfaces[s].textureName = readString();
		mdlSurfaces[s].startCorner = readU32();
		mdlSurfaces[s].spanCorner  = readU32();
		mdlSurfaces[s].cornerCount = readU32();

		if (mdlSurfaces[s].cornerCount == 0)
		{
			continue;
		}

		const auto faceCount = (mdlSurfaces[s].cornerCount / 3); // Triangles (corner == vertex)
		mdlSurfaces[s].faces.resize(faceCount);

		for (uint32_t f = 0; f < faceCount; ++f)
		{
			mdlSurfaces[s].faces[f].index[0] = readU16();
			mdlSurfaces[s].faces[f].index[1] = readU16();
			mdlSurfaces[s].faces[f].index[2] = readU16();
		}
	}

	SnoLog("Read " << model.header.textureCount << " surfaces.");
}

void SnoModel::SnoImporter::importSnoModel()
{
	SnoLog("====== Beginning SNO import... ======");

	readHeader();
	readSpots();
	readDoors();
	readCorners();
	readSurfaces();

	SnoLog("==== Finished importing SNO file ====");
}

#undef SnoLog

} // namespace siege {}
