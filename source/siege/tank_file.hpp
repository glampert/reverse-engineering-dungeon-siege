
// ================================================================================================
// -*- C++ -*-
// File: tank_file.hpp
// Author: Guilherme R. Lampert
// Created on: 22/07/14
// Brief: Contains the binary structure and reader for Dungeon Siege / GPG "Tank" files.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#ifndef SIEGE_TANK_FILE_HPP
#define SIEGE_TANK_FILE_HPP

#include "siege/common.hpp"
#include "siege/helper_types.hpp"

#include <fstream>
#include <future>
#include <memory>
#include <unordered_map>

namespace siege
{

// ========================================================
// TankFile:
// ========================================================

//
// Gas Powered Games Tank file format.
// This file format is used to store most of the Dungeon Siege game assets and resources.
// It uses a virtual file system (much like a ZIP archive) to store compressed files
// and directory references. See "gpg/TankStructure.h" for more details.
//
class TankFile final
	: public utils::NonCopyable
{
public:

	//
	// Original comment from "TankStructure.h":
	//
	//  This enum is used to determine the priority of one tank vs another
	//  regarding which is added to the master index first.
	//
	//  Note on how priority works: it's like a version in that it is built
	//  from two pieces - a major priority and a minor priority.
	//
	enum class Priority : uint32_t
	{
		Factory   = 0x0000, // GPG-issued "factory configured" tank (original CD release).
		Language  = 0x1000, // GPG-issued language pack, filled with localized resource overrides.
		Expansion = 0x2000, // GPG- or affiliate-issued expansion pack tank.
		Patch     = 0x3000, // Some kind of patch tank.
		User      = 0x4000  // User-constructed tank.
	};

	static std::string priorityToString(Priority priority);
	static Priority priorityFromString(const std::string & str);

	//
	// Original comment from "TankStructure.h":
	//
	//  This enum is for the different formats the data can be stored in.
	//  Max width = 16 bits.
	//
	enum class DataFormat : uint16_t
	{
		Raw,  // This resource is in raw format.
		Zlib, // This resource is zlib-compressed.
		Lzo   // This resource is lzo-compressed.
	};

	static std::string dataFormatToString(DataFormat format);
	static DataFormat dataFormatFromString(const std::string & str);
	static bool isDataFormatCompressed(DataFormat format) noexcept { return format != DataFormat::Raw; }

	//
	// Original comment from "TankStructure.h":
	//
	//  Flags used for an entire tank file.
	//  Max width = 32 bits.
	//
	static constexpr uint32_t TankFlagNone                 = 0;
	static constexpr uint32_t TankFlagNonRetail            = 1 << 0; // This is a development-only tank (not for retail usage).
	static constexpr uint32_t TankFlagAllowMultiplayerXfer = 1 << 1; // Allow transfer of this tank during multiplayer.
	static constexpr uint32_t TankFlagProtectedContent     = 1 << 2; // This is protected content (for optional use by extractors).

	//
	// Original comment from "TankStructure.h":
	//
	//  Flags used per file resource.
	//  Max width = 16 bits.
	//
	static constexpr uint16_t FileFlagNone    = 0;
	static constexpr uint16_t FileFlagInvalid = 1 << 15; // This resource had a problem during construction and is invalid.

	//
	// Other miscellaneous constants:
	//
	static constexpr uint32_t DataSectionAlignment = 4 << 10;    // Alignment for data section (RAW format).
	static constexpr uint32_t DataAlignment        = 8;          // Alignment for data files.
	static constexpr uint32_t InvalidOffset        = 0xFFFFFFFF; // Can't use null, so use this instead.
	static constexpr uint32_t InvalidChecksum      = 0x00000000; // Use a zero checksum to say that it's not important or wasn't computed.
	static constexpr uint32_t LargeFileSize        = 4 * 1024;   // This is what we consider a "large" file minimum size for optimization
	                                                             // purposes (put small at front).

	//
	// Four Character Codes:
	//
	static const FourCC ProductId;     // 'DSig'
	static const FourCC TankId;        // 'Tank'
	static const FourCC CreatorIdGPG;  // '!GPG'
	static const FourCC CreatorIdUser; // 'USER'

	//
	// Tank header:
	//
	// WW  = packed word + word
	// E   = cast to enum type
	// EB  = cast to bitfield-style enum type
	// ZST = zero-terminated string
	// NST = zero-terminated string preceded by a WORD with its length (length does not include the NUL)
	// HO  = offset from top of Header
	// DSO = offset from top of DirSet
	// FSO = offset from top of FileSet
	// DO  = offset from top of data section
	// R4  = reversed fourCC (human readable)
	// #   = # byte structure
	//
	// All offsets are to the base of Header (this)
	//
	struct Header final
	{
		static constexpr uint32_t CopyrightTextMaxLength = 100;
		static constexpr uint32_t BuildTextMaxLength     = 100;
		static constexpr uint32_t TitleTextMaxLength     = 100;
		static constexpr uint32_t AuthorTextMaxLength    = 40;
		static constexpr uint32_t RawHeaderPad           = 16; // Used for padding between end of header and start of raw data
		static constexpr uint32_t ExpectedVersion        = makeVersionWord(1, 0, 2); // Version used on Dungeon Siege 1

		// ------ Base ------
		FourCC         productId;                             // (R4) ID of product (human readable) - always ProductId
		FourCC         tankId;                                // (R4) ID of tank (human readable) - always TankId
		uint32_t       headerVersion;                         // (WW) Version of this particular header
		uint32_t       dirsetOffset;                          // (HO) DirSet offset
		uint32_t       filesetOffset;                         // (HO) FileSet offset
		uint32_t       indexSize;                             // Total size of index (header plus all dir data - used for RAW format)
		uint32_t       dataOffset;                            // (HO) Offset to start of data (used for RAW format)

		// ------ V1.0 Extra - Basic ------
		ProductVersion productVersion;                        // (#12) Product version this tank was built with
		ProductVersion minimumVersion;                        // (#12) Minimum product version required to use this tank
		Priority       priority;                              // (WW) Priority that this tank is entered into the master index (Priority enum)
		uint32_t       flags;                                 // (EB) Flags regarding this tank
		FourCC         creatorId;                             // Who created this tank (creation tool will choose, not user)
		Guid           guid;                                  // True GUID assigned at creation time
		uint32_t       indexCrc32;                            // CRC-32 of just the index (not including the header)
		uint32_t       dataCrc32;                             // CRC-32 of just the data
		SystemTime     utcBuildTime;                          // When this tank was constructed (stored in UTC)
		WideChar       copyrightText[CopyrightTextMaxLength]; // (ZST) Copyright text
		WideChar       buildText[BuildTextMaxLength];         // (ZST) Text about how this was built

		// ------ V1.0 Extra - User Info ------
		WideChar       titleText[TitleTextMaxLength];         // (ZST) Title of this tank
		WideChar       authorText[AuthorTextMaxLength];       // (ZST) Who made this tank
		WideString     descriptionText;                       // Anything the user wants can go here

		// Default constructor initializes all fields
		// to default/invalid values.
		Header();

		// Clears the header to its defaults.
		void setDefaults();
	};

	//
	// FileEntryChunkHeader:
	//
	struct FileEntryChunkHeader final
	{
		const uint32_t uncompressedSize; // Note: sizes are the same if this chunk not compressed
		const uint32_t compressedSize;   // Size in bytes while compressed
		const uint32_t extraBytes;       // Extra bytes to read into the end to allow for decompression overhead
		const uint32_t offset;           // Offset from start of file data to this chunk (FileEntry::offset)

		FileEntryChunkHeader(uint32_t nUncompressedSize, uint32_t nCompressedSize,
		                     uint32_t nExtraBytes, uint32_t nOffset);

		// Test if this chunk is compressed by comparing
		// its uncompressed and compressed sizes.
		bool isCompressed() const noexcept;
	};

	//
	// CompressedFileEntryHeader:
	//
	struct CompressedFileEntryHeader final
	{
		const uint32_t compressedSize; // Size of compressed data (in bytes)
		const uint32_t chunkSize;      // Size of chunks in bytes, 0 for not chunked (rounded to 4KB page size)
		const uint32_t numChunks;      // ceil( FileEntry::size / chunkSize )
		std::vector<FileEntryChunkHeader> chunkHeaders; // chunkHeaders[ ceil( FileEntry::size / chunkSize ) ]

		CompressedFileEntryHeader(uint32_t nCompressedSize, uint32_t nChunkSize, uint32_t nFileSize);
	};

	//
	// FileEntry:
	//
	class FileEntry final
	{
		// Optional compression header if this file is compressed. May be null.
		std::unique_ptr<CompressedFileEntryHeader> compressedHeader;

	public:

		const uint32_t    parentOffset; // (DSO) Where's the base of our parent DirEntry?
		const uint32_t    size;         // Size of resource
		const uint32_t    offset;       // (DO) Offset to data from top of data section
		const uint32_t    crc32;        // CRC-32 of just this resource
		const FileTime    fileTime;     // last Modified timestamp of file when it was added
		const DataFormat  format;       // (E) Data format (DataFormat)
		const uint16_t    flags;        // (EB) Tank file flags (FileFlag*)
		const std::string name;         // What's my name?

		FileEntry(uint32_t nParentOffs, uint32_t nSize, uint32_t nOffset, uint32_t crc,
		          FileTime ft, DataFormat dataFormat, uint16_t fileFlags, std::string filename);

		void setCompressedHeader(std::unique_ptr<CompressedFileEntryHeader> header);

		CompressedFileEntryHeader & getCompressedHeader();
		const CompressedFileEntryHeader & getCompressedHeader() const;
		const FileEntryChunkHeader & getChunkHeader(uint32_t index) const;

		bool isInvalidFile() const noexcept;
		bool isCompressed()  const noexcept;

		uint32_t getUncompressedSize()        const;
		uint32_t getCompressedSize()          const;
		uint32_t getChunkSize()               const;
		uint32_t getChunkIndex(uint32_t offs) const;
	};

	//
	// FileSet:
	//
	struct FileSet final
	{
		// All offsets here are to the base of FileSet (this)
		const uint32_t         numFiles;    // Total number of files
		std::vector<uint32_t>  fileOffsets; // (FSO)
		std::vector<FileEntry> fileEntries; // Sorted alphabetically

		FileSet(uint32_t numEntries = 0);
	};

	//
	// DirEntry:
	//
	struct DirEntry final
	{
		const uint32_t        parentOffset; // (DSO) Where's the base of our parent DirEntry? (zero for root)
		const uint32_t        childCount;   // How many children in this DirEntry?
		const FileTime        fileTime;     // Last modified timestamp of dir
		const std::string     name;         // What's my name?
		std::vector<uint32_t> childOffsets; // (DSO) childCount offsets to each child (these are sorted)

		DirEntry(uint32_t nParentOffs, uint32_t nChildCount,
		         FileTime ft, std::string dirName);

		DirEntry(uint32_t nParentOffs, uint32_t nChildCount, FileTime ft,
		         std::string dirName, std::vector<uint32_t> && childOffs);

		bool isRoot() const noexcept { return parentOffset == 0; }
	};

	//
	// DirSet:
	//
	struct DirSet final
	{
		const uint32_t        numDirs;    // Total number of directories
		std::vector<uint32_t> dirOffsets; // (DSO) numDirs offsets for easier iteration
		std::vector<DirEntry> dirEntries; // numDirs entries sorted alphabetically within each node

		DirSet(uint32_t numEntries = 0);
	};

	//
	// Exception type thrown by errors generated by
	// TankFile and other Tank helpers like TankFile::Reader.
	//
	class Error final
		: public Exception
	{
	public:

		Error() noexcept;
		Error(const char * error) noexcept;
		Error(const std::string & error) noexcept;
		~Error();
	};

	// Asynchronous file operations with a TankFile.
	using Task = std::future<bool>;

	//
	// Reads the Tank file using a stream opened by a TankFile instance.
	// This class is a friend of the TankFile class.
	//
	class Reader final
		: public utils::NonCopyable
	{
	public:

		Reader() = default;

		// Calls indexFile().
		Reader(TankFile & tank);

		// Build indexing tables for the given Tank file. Must be called before extracting any
		// resources from a Tank. Any indexing data from a previous call is discarded.
		// The file may be discarded after this method returns. Throws TankFile::Error on failure.
		void indexFile(TankFile & tank);

		// Attempts to extract a resource and write an uncompressed binary file with its contents.
		// Might throw TankFile::Error if any step of the process should fail. Might also throw std::bad_alloc if out-of-memory.
		// If 'validateCRCs' is true and the CRC32 of the file doesn't match the computed one, also fails with an exception.
		// CRC32 of the extracted file is not computed if 'validateCRCs' is false.
		void extractResourceToFile(TankFile & tank, const std::string & resourcePath,
		                           const std::string & destFile, bool validateCRCs) const;

		// Same as extractResourceToFile() but writes the dest file as an async background task.
		// Unlike the previous method however, this one DOES NOT throw and exception if the file can't be written.
		// If the async IO fails, when the returned std::future is dereferenced, it will return a boolean
		// with the result of the file write operation. Data is fetched from the Tank before spawning the thread,
		// so an exception might still be thrown if data can't be read/decompressed from the source Tank.
		Task extractResourceToFileAsync(TankFile & tank, const std::string & resourcePath,
		                                const std::string & destFile, bool validateCRCs) const;

		// Attempts to extract a resource to a memory buffer.
		// Might throw TankFile::Error if the file cannot be extracted. Might also throw std::bad_alloc if out-of-memory.
		// If 'validateCRCs' is true and the CRC32 of the file doesn't match the computed one, also fails with an exception.
		// CRC32 of the extracted file is not computed if 'validateCRCs' is false.
		ByteArray extractResourceToMemory(TankFile & tank, const std::string & resourcePath, bool validateCRCs) const;

		// Extracts all files present in the Tank to the given path. Tank must have been previously indexed with indexFile().
		// The name of the Tank minus its extension will be the first directory in the path hierarchy.
		void extractWholeTank(TankFile & tank, const std::string & destPath, bool validateCRCs) const;

		// Directory and file lists for printing.
		// NOTE: Lists are not sorted!
		std::vector<std::string> getFileList() const;
		std::vector<std::string> getDirectoryList() const;

		// Misc queries:
		unsigned int getDirectoryCount() const noexcept { return (dirSet  != nullptr) ? dirSet->numDirs   : 0; }
		unsigned int getFileCount()      const noexcept { return (fileSet != nullptr) ? fileSet->numFiles : 0; }

	private:

		void readDirSet(TankFile & tank);
		void readFileSet(TankFile & tank);

		void buildDirPaths();
		void buildFilePaths();

		struct TankEntry
		{
			// Pointer into dirSet.dirEntries[] or fileSet.fileEntries[].
			union Entry
			{
				const DirEntry  * dir;
				const FileEntry * file;
			} ptr;

			// Type tag to select from the pointers above.
			enum Type
			{
				TypeDir,
				TypeFile
			} type;

			TankEntry(const DirEntry * d)
				: type(TypeDir) { ptr.dir = d; }

			TankEntry(const FileEntry * f)
				: type(TypeFile) { ptr.file = f; }

			TankEntry() = default;
		};

		using FileTable  = std::unordered_map<std::string, TankEntry>;
		using DirSetPtr  = std::unique_ptr<TankFile::DirSet>;
		using FileSetPtr = std::unique_ptr<TankFile::FileSet>;

		DirSetPtr  dirSet;
		FileSetPtr fileSet;
		FileTable  fileTable;
	};

	// TankFile::Reader will have access to private data
	// and methods of TankFile so that it can read the file.
	friend Reader;

public:

	// Opens a file for reading. File must exist. Throws TankFile::Error.
	void openForReading(std::string filename);

	// Manually close the file (closed automatically by the destructor).
	void close();

	// Queries:
	bool isOpen()      const noexcept;
	bool isReadOnly()  const noexcept;
	bool isWriteOnly() const noexcept;
	bool isReadWrite() const noexcept;

	// Accessors:
	size_t getFileSizeBytes()         const noexcept;
	const Header & getFileHeader()    const noexcept;
	const std::string & getFileName() const noexcept;

private:

	void queryFileSize();
	void readAndValidateHeader();
	void seekAbsoluteOffset(size_t offsetInBytes);

	void           readBytes(void * buffer, size_t numBytes);
	uint16_t       readU16();
	uint32_t       readU32();
	std::string    readNString();
	WideString     readWNString();
	FileTime       readFileTime();
	SystemTime     readSystemTime();
	ProductVersion readProductVersion();
	FourCC         readFourCC();
	Guid           readGuid();

	using OpenMode = std::ios::open_mode;
	std::ifstream  file;
	std::string    fileName;
	Header         fileHeader;
	OpenMode       fileOpenMode  = 0;
	size_t         fileSizeBytes = 0;
};

} // namespace siege {}

#endif // SIEGE_TANK_FILE_HPP
