#pragma once
// ================================================================================================
// -*- C++ -*-
// File: sno_model.hpp
// Author: Guilherme R. Lampert
// Created on: 04/09/15
// Brief: Handler for the Dungeon Siege "Siege Nodes" (SNO) 3D geometry files.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/common.hpp"
#include "siege/helper_types.hpp"

namespace siege
{

// ========================================================
// SnoModel:
// ========================================================

//
// The SNO format (4CC "SNOD") are the so called "Siege Nodes"
// or just "Nodes" that compose the static level geometry of
// Dungeon Siege. A Siege Node is much like a 3D tile of arbitrary
// dimensions. The world of Dungeon Siege is built by attaching
// these nodes side by side to construct the larger scenes.
//
// The article by Scott Bilas "The Continuous World of Dungeon Siege"
// explains the node system is great detail. You can find an online copy at:
//  http://scottbilas.com/files/2003/gdc_san_jose/continuous_world_paper.pdf
//
// More information is available on his website:
//  http://scottbilas.com/games/dungeon-siege/
//
// This class allows importing a SNO mesh either from file of from memory.
//
// The sno2obj tool can be used to convert a SNO to a Wavefront
// OBJ model file, which can then be opened on pretty much any
// 3D modeling tool out there.
//
class SnoModel final
	: public utils::NonCopyable
{
public:

	enum ImportFlags
	{
		// Default import mode. Loads most of the stuff but ignores some irrelevant data.
		Default     = 0,

		// Load minimal data to import quickly for preview.
		QuickImport = 1 << 1,

		// Load and validate everything, even the unused stuff of the SNO format.
		FullImport  = 1 << 2
	};

	// Shorthand names for internal class usage:
	using Vec2  = utils::Vec2;
	using Vec3  = utils::Vec3;
	using Vec4  = utils::Vec4;
	using Vec4b = utils::Vec4b;

	// Accepted value for `Header::version`.
	static constexpr uint32_t VersionExpected = 7;

	#pragma pack(push, 1)

	struct Header
	{
		// File id, always 'SNOD'.
		FourCC magic;

		// Misc info and sizes:
		uint32_t version;
		uint32_t unused0;
		uint32_t doorCount;
		uint32_t spotCount;
		uint32_t cornerCount;
		uint32_t faceCount;
		uint32_t textureCount;

		// AABBs for the whole mesh.
		Vec3 minBBox;
		Vec3 maxBBox;

		// Junk/not used. Why was this left here? I dunno...
		uint32_t unused1;
		uint32_t unused2;
		uint32_t unused3;
		uint32_t unused4;
		uint32_t unused5;
		uint32_t unused6;
		uint32_t unused7;

		// CRC32 of the data, excluding this header.
		uint32_t dataCrc32;
	};

	struct TriIndex
	{
		// Unlike the ASP models, Siege Nodes use 16bits indexes.
		uint16_t index[3];
	};

	#pragma pack(pop)

	struct SpotInfo
	{
		float xform[4][3]; // 3x3 rotation matrix + translation vector.
		std::string name;
	};

	struct DoorInfo
	{
		uint32_t index;
		float xform[4][3]; // 3x3 rotation matrix + translation vector.
		std::vector<uint32_t> hotSpots;
	};

	// Corners are the interleaved models vertexes.
	// The Max scripts used the term corners to mean a vertex.
	struct CornerInfo
	{
		Vec3  pos;
		Vec3  normal;
		Vec4b color; // NOTE: Stored as RBGA in the file! We swizzle back to RGBA on load.
		Vec2  texCoord;
	};

	struct SurfaceInfo
	{
		uint32_t startCorner;
		uint32_t spanCorner;
		uint32_t cornerCount;
		std::vector<TriIndex> faces;
		std::string textureName;
	};

public:

	// Construct an empty model.
	SnoModel() = default;

	// Construct from SNO model file.
	SnoModel(std::string filename, uint32_t importFlags = Default);

	// Construct from a SNO model file loaded into memory.
	SnoModel(ByteArray fileContents, uint32_t importFlags = Default, std::string filename = "");

	// Load SNO model from file. Discards current, if any.
	void initFromFile(std::string filename, uint32_t importFlags = Default);

	// Load SNO model from memory. Discards current, if any.
	void initFromMemory(ByteArray fileContents, uint32_t importFlags = Default, std::string filename = "");

	// Disposes model data, making this class an empty/invalid model.
	void dispose();

	// Test if this object has valid model data.
	bool isValid() const;

	// Source file that originated this model. May be empty if the model was loaded from memory.
	const std::string & getSourceFileName() const { return srcFileName; }

	// Data accessors for the conversion tools:
	const Header                   & getHeader()   const { return header;   }
	const std::vector<SpotInfo>    & getSpots()    const { return spots;    }
	const std::vector<DoorInfo>    & getDoors()    const { return doors;    }
	const std::vector<CornerInfo>  & getCorners()  const { return corners;  }
	const std::vector<SurfaceInfo> & getSurfaces() const { return surfaces; }

private:

	// Helper to store the intermediate import data.
	class SnoImporter final
		: public utils::NonCopyable
	{
	public:

		// Imports a model from file data, writing to 'mdl'.
		// Might throw an exception on error.
		SnoImporter(SnoModel & mdl, ByteArray fileData,
		            uint32_t impFlags, const std::string & filename);

	private:

		// Reader helpers:
		void        readBytes(void * buffer, size_t numBytes);
		void        readFloat4x3(float values[][3]);
		uint8_t     readU8();
		uint16_t    readU16();
		uint32_t    readU32();
		float       readF32();
		std::string readString();
		void        readHeader();
		void        readSpots();
		void        readDoors();
		void        readCorners();
		void        readSurfaces();

		// Perform the full import, filling the model reference on successful completion.
		void importSnoModel();

		// Importer state:
		SnoModel &          model;
		uint32_t            importFlags;
		size_t              readPosition;
		const ByteArray     fileContents;
		const std::string & srcFileName;
	};

	// Importer can access the internal data structures of SnoModel.
	friend class SnoImporter;

	Header header;
	std::vector<SpotInfo>    spots;
	std::vector<DoorInfo>    doors;
	std::vector<CornerInfo>  corners;
	std::vector<SurfaceInfo> surfaces;

	// Source filename for debug printing.
	// May be empty if the model was loaded from memory.
	std::string srcFileName;
};

} // namespace siege {}
