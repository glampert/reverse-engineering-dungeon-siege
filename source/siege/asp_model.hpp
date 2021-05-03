#pragma once
// ================================================================================================
// -*- C++ -*-
// File: asp_model.hpp
// Author: Guilherme R. Lampert
// Created on: 20/12/14
// Brief: Handler for the Dungeon Siege "Aspects" (ASP) 3D model format.
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
// AspModel:
// ========================================================

//
// Importer class for a Dungeon Siege ASPECT (.ASP) 3D model.
// ASP models are used for everything but the terrain.
//
// This importer is largely based on the 3D Max import/export scripts
// from the SiegeTheDay.org forums. Refer to ASPImport.ms for
// the original script code.
//
// You can import an ASP from a file or from a memory buffer.
// The asp2obj command line tool can convert an ASP into a
// static Wavefront OBJ model, which can then be opened in
// a variety of 3D modeling tools and viewers.
//
// Skeleton data is contained in the ASP, but animation frames
// are stored separately in another file with the .PRS extension.
//
// The term "Corner" is used here to mean an interleaved model
// vertex (position, tex-coords, normal, etc). This term was used
// by the Max scripts, so I decided to use it here as well to keep
// the C++ code as close to the original scripts as possible.
//
class AspModel final
	: public utils::NonCopyable
{
public:

	enum ImportFlags
	{
		// Default import mode. Loads most of the stuff but ignores some irrelevant data.
		Default     = 0,

		// Load minimal data to import quickly for preview.
		QuickImport = 1 << 1,

		// Load and validate everything, even the unused stuff of the ASP format.
		FullImport  = 1 << 2
	};

	// Shorthand names for internal class usage:
	using Vec2  = utils::Vec2;
	using Vec3  = utils::Vec3;
	using Vec4  = utils::Vec4;
	using Vec4b = utils::Vec4b;

	// Indexes into the shared corner (vertex) array for a face triangle.
	struct TriIndex
	{
		uint32_t index[3];
	};

	// A model vertex, which can be thought of as a "corner"...
	// "Corner" is the term used in the 3DMax export scripts.
	struct WCornerInfo
	{
		Vec3  pos;
		Vec3  normal;
		Vec4  weight;
		Vec2  texCoord;
		Vec4b color;
		Vec4b bone;
	};

	// A simpler model vertex (corner), without animation data.
	// This was probably used for static geometry.
	struct CornerInfo
	{
		uint32_t vtxIndex;
		Vec3  normal;
		Vec2  texCoord;
		Vec4b color;
	};

	struct MatInfo
	{
		uint32_t textureIndex;
		uint32_t faceSpan;
	};

	struct FaceInfo
	{
		std::vector<uint32_t> cornerStart;
		std::vector<uint32_t> cornerSpan;
		std::vector<TriIndex> cornerIndex;
	};

	struct BoneInfo
	{
		uint32_t parentIndex;
		uint32_t flags;
		std::string name;
	};

	struct SubMesh
	{
		uint32_t textureCount = 0;
		uint32_t vertexCount  = 0;
		uint32_t cornerCount  = 0;
		uint32_t faceCount    = 0;
		uint32_t stitchCount  = 0;

		std::vector<MatInfo>     matInfo;   // BSMM
		std::vector<Vec3>        positions; // BVTX
		std::vector<CornerInfo>  corners;   // BCRN
		std::vector<WCornerInfo> wCorners;  // WCRN
		FaceInfo                 faceInfo;  // BTRI
	};

public:

	// Construct an empty model.
	AspModel() = default;

	// Construct from ASP model file.
	AspModel(std::string filename, uint32_t importFlags = Default);

	// Construct from an ASP model file loaded into memory.
	AspModel(ByteArray fileContents, uint32_t importFlags = Default, std::string filename = "");

	// Load ASP model from file. Discards current, if any.
	void initFromFile(std::string filename, uint32_t importFlags = Default);

	// Load ASP model from memory. Discards current, if any.
	void initFromMemory(ByteArray fileContents, uint32_t importFlags = Default, std::string filename = "");

	// Disposes model data, making this class an empty/invalid model.
	void dispose();

	// Test if this object has valid model data.
	bool isValid() const;

	// Source file that originated this model. May be empty if the model was loaded from memory.
	const std::string & getSourceFileName() const { return srcFileName; }

	// Used by the OBJ exporter tool.
	const std::vector<SubMesh>     & getSubMeshes()    const { return subMeshes;    }
	const std::vector<BoneInfo>    & getBoneInfos()    const { return boneInfos;    }
	const std::vector<std::string> & getTextureNames() const { return textureNames; }

private:

	// Reading and handling the ASP data is fairly complex
	// and requires some temporary state. This class facilitates that.
	class AspImporter final
		: public utils::NonCopyable
	{
	public:

		// Imports a model from file data, writing to 'mdl'.
		// Might throw an exception on error.
		AspImporter(AspModel & mdl, ByteArray fileData,
		            uint32_t impFlags, const std::string & filename);

	private:

		// Reader helpers:
		void     readBytes(void * buffer, size_t numBytes);
		uint32_t readU32();
		float    readF32();
		Vec4b    readColor();
		Vec2     readTexCoord();
		Vec3     readVec3();
		Vec4     readVec4();
		bool     readFourCC(FourCC & fcc);

		// Readers for each sections of the ASP file format:
		void readBMSH();
		void readBONH();
		void readBSUB();
		void readBSMM();
		void readBVTX();
		void readBCRN();
		void readWCRN();
		void readBVMP();
		void readBTRI();
		void readBVWL();
		void readSTCH();
		void readRPOS();
		void readBBOX();
		void readBEND();

		// The main reading loop. This will branch on each
		// ASP section and call one of the above handlers.
		void importAspModel();
		void validateVersion(const char * sectName, uint32_t version) const;

		// Importer state:
		AspModel &          model;
		uint32_t            importFlags;
		uint32_t            currentSubMeshIndex;
		size_t              readPosition;
		const ByteArray     fileContents;
		const std::string & srcFileName;
	};

	// Importer can access the internal data structures of AspModel.
	friend class AspImporter;

	// Model data:
	std::vector<SubMesh>     subMeshes;
	std::vector<BoneInfo>    boneInfos;
	std::vector<std::string> textureNames;

	// Source filename for debug printing.
	// May be empty if the model was loaded from memory.
	std::string srcFileName;
};

} // namespace siege {}
