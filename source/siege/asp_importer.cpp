
// ================================================================================================
// -*- C++ -*-
// File: asp_importer.cpp
// Author: Guilherme R. Lampert
// Created on: 27/12/14
// Brief: Definition of the AspImporter inner class of AspModel.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/asp_model.hpp"
#include <cmath> // For std::isnan()

namespace siege
{

// ========================================================
// Local helpers:
// ========================================================

// Can be easily disabled to avoid excessively verbose output.
#if SIEGE_ASP_DEBUG
	#define AspLog SiegeLog
#else // !SIEGE_ASP_DEBUG
	#ifndef AspLog
		#define AspLog(x) /* nothing */
	#endif // AspLog
#endif // SIEGE_ASP_DEBUG

namespace
{

// "Raw" version numbers of the ASP sections.
// There are also alternate versions that can be fetched by 'versionOf()'.
struct Version
{
	enum Enum
	{
		null = 0,
		v1_2 = 513,
		v1_3 = 769,
		v2_0 = 2,
		v2_1 = 258,
		v2_2 = 514,
		v2_3 = 770,
		v2_4 = 1026,
		v2_5 = 1282,
		v4_0 = 4,
		v4_1 = 260,
		v5_0 = 5 // DS Legends of Aranna expansion.
	};
};

inline uint32_t versionOf(const uint32_t v)
{
	// Adapted from "ASPImport.ms".
	// The question to ask here is, why didn't
	// they save these numbers in the file, instead
	// of the wacky version constants?
	switch (v)
	{
	case Version::v1_2 : return 12;
	case Version::v1_3 : return 13;
	case Version::v2_0 : return 20;
	case Version::v2_1 : return 21;
	case Version::v2_2 : return 22;
	case Version::v2_3 : return 23;
	case Version::v2_4 : return 24;
	case Version::v2_5 : return 25;
	case Version::v4_0 : return 40;
	case Version::v4_1 : return 41;
	case Version::v5_0 : return 50;
	default : return Version::null;
	} // switch (v)
}

} // namespace {}

// ========================================================
// AspModel::AspImporter:
// ========================================================

AspModel::AspImporter::AspImporter(AspModel & mdl, ByteArray fileData,
                                   const uint32_t impFlags, const std::string & filename)
	: model(mdl)
	, importFlags(impFlags)
	, currentSubMeshIndex(0)
	, readPosition(0)
	, fileContents(std::move(fileData))
	, srcFileName(filename)
{
	assert(!fileContents.empty());
	importAspModel();

	// TODO actually use the importFlags !!!
	(void)importFlags;
}

void AspModel::AspImporter::readBytes(void * buffer, const size_t numBytes)
{
	assert(buffer   != nullptr);
	assert(numBytes != 0);
	assert(!fileContents.empty());

	if (readPosition == fileContents.size() || (readPosition + numBytes) > fileContents.size())
	{
		SiegeThrow(Exception, "Trying to read past the end of ASP file \"" << srcFileName << "\"!");
	}

	const uint8_t * dataPtr = fileContents.data() + readPosition;
	std::memcpy(buffer, dataPtr, numBytes);
	readPosition += numBytes;
}

uint32_t AspModel::AspImporter::readU32()
{
	uint32_t x = 0;
	readBytes(&x, sizeof(x));
	return x;
}

float AspModel::AspImporter::readF32()
{
	static_assert(sizeof(float) == 4, "This assumes a 32bit float type!");

	float x = 0;
	readBytes(&x, sizeof(x));
	return x;
}

AspModel::Vec4b AspModel::AspImporter::readColor()
{
	static_assert(sizeof(Vec4b) == 4, "This assumes RGBA 8bit color size!");

	Vec4b c;
	readBytes(&c, sizeof(c));
	return c;
}

AspModel::Vec2 AspModel::AspImporter::readTexCoord()
{
	static_assert(sizeof(Vec2) == 8, "This assumes 8 bytes for Vec2 (u : float32, v : float32)!");

	Vec2 t;
	readBytes(&t, sizeof(t));

	#if SIEGE_ASP_DEBUG
	if (std::isnan(t.x) || std::isnan(t.y))
	{
		SiegeWarn("readTexCoord(): NAN tex coord data inside ASP model!");
	}
	#endif // SIEGE_ASP_DEBUG

	return t;
}

AspModel::Vec3 AspModel::AspImporter::readVec3()
{
	static_assert(sizeof(Vec3) == 12, "This assumes 12 bytes for Vec3 (x : float32, y : float32, z : float32)!");

	Vec3 v;
	readBytes(&v, sizeof(v));

	#if SIEGE_ASP_DEBUG
	if (std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z))
	{
		SiegeWarn("readVec3(): NAN vector data inside ASP model!");
	}
	#endif // SIEGE_ASP_DEBUG

	return v;
}

AspModel::Vec4 AspModel::AspImporter::readVec4()
{
	static_assert(sizeof(Vec4) == 16, "This assumes 16 bytes for Vec4 (x : float32, y : float32, z : float32, w : float32)!");

	Vec4 v;
	readBytes(&v, sizeof(v));

	#if SIEGE_ASP_DEBUG
	if (std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z) || std::isnan(v.w))
	{
		SiegeWarn("readVec4(): NAN vector data inside ASP model!");
	}
	#endif // SIEGE_ASP_DEBUG

	return v;
}

bool AspModel::AspImporter::readFourCC(FourCC & fcc)
{
	if (readPosition == fileContents.size() || (readPosition + sizeof(FourCC)) > fileContents.size())
	{
		return false; // End of file reached.
	}

	const uint8_t * dataPtr = fileContents.data() + readPosition;
	std::memcpy(&fcc, dataPtr, sizeof(FourCC));
	readPosition += sizeof(FourCC);
	return true;
}

void AspModel::AspImporter::readBMSH()
{
	AspLog("====== Reading BMSH section ======");

	const auto version = readU32();
	validateVersion("BMSH", version);

	// Common mesh fields:
	const auto sizeTextField = readU32();
	const auto boneCount     = readU32();
	const auto textureCount  = readU32();
	const auto vertexCount   = readU32();
	const auto subMeshCount  = readU32();
	const auto renderFlags   = readU32();

	// A length this big can only mean a broken file...
	if (sizeTextField >= (1024 * 1024))
	{
		SiegeThrow(Exception, "Bogus text length in BMSH section for ASP file \"" << srcFileName << "\"!");
	}

	// Read the text payload that follows BMSH:
	ByteArray rawText(sizeTextField);
	readBytes(rawText.data(), rawText.size());

	// Split textures from bone names.
	// Each string is separated by one or more null bytes.
	size_t index = 0;

	model.textureNames.resize(textureCount);
	for (uint32_t t = 0; t < textureCount; ++t)
	{
		for (; index < rawText.size(); ++index)
		{
			const char c = static_cast<char>(rawText[index]);
			if (c == '\0')
			{
				// Skip null padding for the next name:
				while (index < rawText.size() && rawText[index] == 0)
				{
					++index;
				}
				break;
			}
			model.textureNames[t].push_back(c);
		}
	}

	model.boneInfos.resize(boneCount);
	for (uint32_t b = 0; b < boneCount; ++b)
	{
		for (; index < rawText.size(); ++index)
		{
			const char c = static_cast<char>(rawText[index]);
			if (c == '\0')
			{
				// Skip null padding for the next name:
				while (index < rawText.size() && rawText[index] == 0)
				{
					++index;
				}
				break;
			}
			model.boneInfos[b].name.push_back(c);
		}
	}

	// Allocate memory for other fields:
	model.subMeshes.resize(subMeshCount);

#if SIEGE_ASP_DEBUG

	// Print the struct:
	AspLog("sizeTextField...: " << sizeTextField);
	AspLog("boneCount.......: " << boneCount);
	AspLog("textureCount....: " << textureCount);
	AspLog("vertexCount.....: " << vertexCount);
	AspLog("subMeshCount....: " << subMeshCount);
	AspLog("renderFlags.....: " << renderFlags);

	// Put a ` in the null bytes so we can easily visualize-it:
	std::transform(std::begin(rawText), std::end(rawText), std::begin(rawText),
			[](uint8_t b) { return (b != 0) ? b : '`'; });

	// Print it as a null-terminated string:
	rawText.push_back(0);
	AspLog("rawText.........: " << reinterpret_cast<const char *>(rawText.data()));

	// Print texture names and bone names we've parsed:
	for (const auto & texName : model.textureNames)
	{
		AspLog("textureName.....: " << texName);
	}
	for (const auto & bone : model.boneInfos)
	{
		AspLog("boneName........: " << bone.name);
	}

#endif // SIEGE_ASP_DEBUG
}

void AspModel::AspImporter::readBONH()
{
	AspLog("====== Reading BONH section ======");

	const auto version = readU32();
	validateVersion("BONH", version);

	// A tuple of [bone_index, parent_index, bone_flags]
	// for every bone of the mesh. Indexes are zero based.
	for (size_t b = 0; b < model.boneInfos.size(); ++b)
	{
		const auto boneIndex   = readU32();
		const auto parentIndex = readU32();
		const auto boneFlags   = readU32();

		model.boneInfos[boneIndex].parentIndex = parentIndex;
		model.boneInfos[boneIndex].flags       = boneFlags;

		AspLog("bone[" << boneIndex << "].name.........: " << model.boneInfos[boneIndex].name);
		AspLog("bone[" << boneIndex << "].parentIndex..: " << model.boneInfos[boneIndex].parentIndex);
		AspLog("bone[" << boneIndex << "].flags........: " << model.boneInfos[boneIndex].flags);
	}
}

void AspModel::AspImporter::readBSUB()
{
	AspLog("====== Reading BSUB section ======");

	const auto version = readU32();
	validateVersion("BSUB", version);

	// Zero based index if v > 40
	currentSubMeshIndex = readU32();
	if (versionOf(version) <= 40)
	{
		currentSubMeshIndex += 1; // Convert -1 based index to 0 based index.
	}

	// Why is this stored twice? I have no idea...
	const auto textureCount = readU32();
	if (textureCount != model.textureNames.size())
	{
		SiegeThrow(Exception, "Texture count mismatch in BSUB section for ASP file \"" << srcFileName << "\"!");
	}

	auto & mesh = model.subMeshes[currentSubMeshIndex];
	mesh.vertexCount = readU32();
	mesh.cornerCount = readU32();
	mesh.faceCount   = readU32();

	AspLog("subMeshIndex....: " << currentSubMeshIndex);
	AspLog("textureCount....: " << textureCount);
	AspLog("vertexCount.....: " << mesh.vertexCount);
	AspLog("cornerCount.....: " << mesh.cornerCount);
	AspLog("faceCount.......: " << mesh.faceCount);
}

void AspModel::AspImporter::readBSMM()
{
	AspLog("====== Reading BSMM section ======");

	const auto version = readU32();
	validateVersion("BSMM", version);

	auto & mesh = model.subMeshes[currentSubMeshIndex];
	mesh.textureCount = readU32();

	mesh.matInfo.resize(mesh.textureCount);
	for (uint32_t t = 0; t < mesh.textureCount; ++t)
	{
		mesh.matInfo[t].textureIndex = readU32();
		mesh.matInfo[t].faceSpan     = readU32();

		AspLog("mat[" << t << "].textureIndex.: " << mesh.matInfo[t].textureIndex);
		AspLog("mat[" << t << "].faceSpan.....: " << mesh.matInfo[t].faceSpan);
	}
}

void AspModel::AspImporter::readBVTX()
{
	AspLog("====== Reading BVTX section ======");

	const auto version = readU32();
	validateVersion("BVTX", version);

	auto & mesh = model.subMeshes[currentSubMeshIndex];
	if (mesh.vertexCount != readU32())
	{
		SiegeThrow(Exception, "Vertex count mismatch in BVTX section for ASP file \"" << srcFileName << "\"!");
	}

	mesh.positions.resize(mesh.vertexCount);
	for (uint32_t v = 0; v < mesh.vertexCount; ++v)
	{
		mesh.positions[v] = readVec3();
	}

	AspLog("vertexCount.....: " << mesh.vertexCount);
}

void AspModel::AspImporter::readBCRN()
{
	AspLog("====== Reading BCRN section ======");

	const auto version = readU32();
	validateVersion("BCRN", version);

	auto & mesh = model.subMeshes[currentSubMeshIndex];
	if (mesh.cornerCount != readU32())
	{
		SiegeThrow(Exception, "Corner/edge count mismatch in BCRN section for ASP file \"" << srcFileName << "\"!");
	}

	mesh.corners.resize(mesh.cornerCount);
	for (uint32_t c = 0; c < mesh.cornerCount; ++c)
	{
		auto & corner = mesh.corners[c];

		// Vertex position:
		corner.vtxIndex = readU32();
		if (corner.vtxIndex > mesh.positions.size())
		{
			SiegeWarn("Out-of-bounds vertex index in BCRN section! Clamping it...");
			corner.vtxIndex = static_cast<uint32_t>(mesh.positions.size() - 1);
		}

		// Vertex normal, color:
		corner.normal = readVec3();
		corner.color  = readColor();

		// Why did they leave this unused field here in the middle?
		/* auto unused = */ readU32();

		// Float UVs:
		corner.texCoord = readTexCoord();
	}

	AspLog("cornerCount.....: " << mesh.cornerCount);
}

void AspModel::AspImporter::readWCRN()
{
	AspLog("====== Reading WCRN section ======");

	const auto version = readU32();
	validateVersion("WCRN", version);

	auto & mesh = model.subMeshes[currentSubMeshIndex];
	if (mesh.cornerCount != readU32())
	{
		SiegeThrow(Exception, "Corner/edge count mismatch in WCRN section for ASP file \"" << srcFileName << "\"!");
	}

	mesh.wCorners.resize(mesh.cornerCount);
	for (uint32_t c = 0; c < mesh.cornerCount; ++c)
	{
		auto & wCorner = mesh.wCorners[c];

		wCorner.pos      = readVec3();
		wCorner.weight   = readVec4();
		wCorner.bone     = readColor();
		// TODO
		//if (versionOf(version) > 40)
		//	bone = ReadFourBB2
		//else
		//	bone = ReadFourBB
		wCorner.normal   = readVec3();
		wCorner.color    = readColor();
		wCorner.texCoord = readTexCoord();
		/* TODO
		// remove null bone/weights
		// This is a reverse iteration, I guess, from 4 to 1 (or 0)
		for i = 4 to 1 by -1 do
		if (w[i] == 0) do
		(
			deleteItem w i
			deleteItem b i
		)
		*/
	}

	AspLog("cornerCount.....: " << mesh.cornerCount);
}

void AspModel::AspImporter::readBVMP()
{
	AspLog("====== Reading BVMP section ======");

	const auto version = readU32();
	validateVersion("BVMP", version);

	// TODO
}

void AspModel::AspImporter::readBTRI()
{
	AspLog("====== Reading BTRI section ======");

	const auto version = readU32();
	validateVersion("BTRI", version);

	auto & mesh = model.subMeshes[currentSubMeshIndex];
	if (mesh.faceCount != readU32())
	{
		SiegeThrow(Exception, "Face count mismatch in BTRI section for ASP file \"" << srcFileName << "\"!");
	}

	if (versionOf(version) == 22)
	{
		AspLog("BTRI version == 2.2");

		mesh.faceInfo.cornerSpan.resize(mesh.textureCount);
		for (uint32_t i = 0; i < mesh.textureCount; ++i)
		{
			mesh.faceInfo.cornerSpan[i] = readU32();
		}

		mesh.faceInfo.cornerStart.resize(mesh.textureCount);
		mesh.faceInfo.cornerStart[0] = 0;
		for (uint32_t i = 0; i < mesh.textureCount - 1; ++i)
		{
			mesh.faceInfo.cornerStart[i + 1] =
				mesh.faceInfo.cornerStart[i] + mesh.faceInfo.cornerSpan[i];
		}
	}
	else if (versionOf(version) > 22)
	{
		AspLog("BTRI version > 2.2");

		mesh.faceInfo.cornerStart.resize(mesh.textureCount);
		mesh.faceInfo.cornerSpan.resize(mesh.textureCount);
		for (uint32_t i = 0; i < mesh.textureCount; ++i)
		{
			mesh.faceInfo.cornerStart[i] = readU32();
			mesh.faceInfo.cornerSpan[i]  = readU32();
		}
	}
	else
	{
		AspLog("BTRI version < 2.2");

		mesh.faceInfo.cornerStart.resize(mesh.textureCount);
		mesh.faceInfo.cornerSpan.resize(mesh.textureCount);
		for (uint32_t i = 0; i < mesh.textureCount; ++i)
		{
			mesh.faceInfo.cornerStart[i] = 0;
			mesh.faceInfo.cornerSpan[i]  = mesh.cornerCount;
		}
	}

	mesh.faceInfo.cornerIndex.resize(mesh.faceCount);
	for (uint32_t f = 0; f < mesh.faceCount; ++f)
	{
		mesh.faceInfo.cornerIndex[f].index[0] = readU32();
		mesh.faceInfo.cornerIndex[f].index[1] = readU32();
		mesh.faceInfo.cornerIndex[f].index[2] = readU32();
	}

	AspLog("faceCount.......: " << mesh.faceCount);
}

void AspModel::AspImporter::readBVWL()
{
	AspLog("====== Reading BVWL section ======");

	const auto version = readU32();
	validateVersion("BVWL", version);

	// TODO
}

void AspModel::AspImporter::readSTCH()
{
	AspLog("====== Reading STCH section ======");

	const auto version = readU32();
	validateVersion("STCH", version);

	// TODO
}

void AspModel::AspImporter::readRPOS()
{
	AspLog("====== Reading RPOS section ======");

	const auto version = readU32();
	validateVersion("RPOS", version);

	// TODO
}

void AspModel::AspImporter::readBBOX()
{
	AspLog("====== Reading BBOX section ======");

	const auto version = readU32();
	validateVersion("BBOX", version);

	// TODO
}

void AspModel::AspImporter::readBEND()
{
	AspLog("====== Reading BEND section ======");

	// 'INFO' section follows immediately.
	FourCC infoCC;
	if (!readFourCC(infoCC) || infoCC != "INFO")
	{
		SiegeWarn("Missing INFO section after BEND!");
		return;
	}

	// We only read and print these. This data has no other use.
	std::string info;
	const auto infoEntryCount = readU32();
	for (uint32_t i = 0; i < infoEntryCount; ++i)
	{
		for (;;)
		{
			// Strings are separated by null bytes.
			char c = '\0';
			readBytes(&c, 1);
			if (c == '\0')
			{
				break;
			}
			info.push_back(c);
		}
		AspLog(info);
		info.clear();
	}
}

void AspModel::AspImporter::importAspModel()
{
	AspLog("====== Beginning ASP import... ======");

	//
	// This is the entry point of the importer.
	// Each 4CC will run a specific chunk handler.
	// Chunks are not required to be in any specific
	// order, so we need to test each 4CC and figure
	// out how to handle the data that follows.
	//
	FourCC chunkId;
	while (readFourCC(chunkId))
	{
		if      (chunkId == "BMSH") { readBMSH(); } // Model header.
		else if (chunkId == "BONH") { readBONH(); } // Bone Hierarchy.
		else if (chunkId == "BSUB") { readBSUB(); } // Sub-mesh info.
		else if (chunkId == "BSMM") { readBSMM(); } // More data related to sub-meshes or materials.
		else if (chunkId == "BVTX") { readBVTX(); } // Model vertex positions.
		else if (chunkId == "BCRN") { readBCRN(); } // Corners (what I would call a model vertex).
		else if (chunkId == "WCRN") { readWCRN(); } // Weighted corners (same as BCRN but with vertex weights).
		else if (chunkId == "BVMP") { readBVMP(); } // Bunch of corner indexes. Not quite sure what for...
		else if (chunkId == "BTRI") { readBTRI(); } // Triangle indexes.
		else if (chunkId == "BVWL") { readBVWL(); } // Stuff related to bone weights.
		else if (chunkId == "STCH") { readSTCH(); } // Stitches (what the heck are they?).
		else if (chunkId == "RPOS") { readRPOS(); } // Rotations and positions for bones.
		else if (chunkId == "BBOX") { readBBOX(); } // Bounding boxes? Seems like it was never fully implemented...
		else if (chunkId == "BEND") { readBEND(); } // Some misc info strings for displaying.
		else { } // Unhandled chunk; Ignore it.
	}

	AspLog("====== Reached end of ASP data ======");
}

void AspModel::AspImporter::validateVersion(const char * sectName, const uint32_t version) const
{
	if (versionOf(version) == Version::null)
	{
		SiegeThrow(Exception, "Got unexpected version " << version << " for "
				<< sectName << " section of ASP file \"" << srcFileName << "\"!");
	}
}

#undef AspLog

} // namespace siege {}
