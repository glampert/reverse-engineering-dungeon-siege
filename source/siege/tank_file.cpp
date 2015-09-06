
// ================================================================================================
// -*- C++ -*-
// File: tank_file.cpp
// Author: Guilherme R. Lampert
// Created on: 22/07/14
// Brief: Contains the binary structure and reader for Dungeon Siege / GPG "Tank" files.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/tank_file.hpp"
#include <cmath> // For std::ceil()

namespace siege
{

// ========================================================
// Local helpers:
// ========================================================

namespace
{

#if SIEGE_TANK_DEBUG

inline std::string toString(const WideString & wStr)
{
	return wStr.empty() ? "<EMPTY>" : ("\"" + wideStringToStdString(wStr) + "\"");
}

inline std::string toHexa(const uint32_t val)
{
	return utils::format("0x%08X", val);
}

#endif // SIEGE_TANK_DEBUG

inline uint16_t alignToDword(const uint16_t size) noexcept
{
	const uint16_t offset = 4 - (size % 4);
	return size + offset;
}

} // namespace {}

// ========================================================
// TankFile::Header:
// ========================================================

TankFile::Header::Header()
{
	setDefaults();
}

void TankFile::Header::setDefaults()
{
	productId      = {};
	tankId         = {};
	headerVersion  = 0;
	dirsetOffset   = 0;
	filesetOffset  = 0;
	indexSize      = 0;
	dataOffset     = 0;
	productVersion = {};
	minimumVersion = {};
	priority       = TankFile::Priority::Factory;
	flags          = 0;
	creatorId      = {};
	guid           = {};
	indexCrc32     = 0;
	dataCrc32      = 0;
	utcBuildTime   = {};

	utils::clearArray(copyrightText);
	utils::clearArray(buildText);
	utils::clearArray(titleText);
	utils::clearArray(authorText);

	descriptionText.clear();
}

// ========================================================
// TankFile::FileEntryChunkHeader:
// ========================================================

TankFile::FileEntryChunkHeader::FileEntryChunkHeader(const uint32_t nUncompressedSize,
                                                     const uint32_t nCompressedSize,
                                                     const uint32_t nExtraBytes,
                                                     const uint32_t nOffset)
	: uncompressedSize(nUncompressedSize)
	, compressedSize(nCompressedSize)
	, extraBytes(nExtraBytes)
	, offset(nOffset)
{
}

bool TankFile::FileEntryChunkHeader::isCompressed() const noexcept
{
	return uncompressedSize != compressedSize;
}

// ========================================================
// TankFile::CompressedFileEntryHeader:
// ========================================================

TankFile::CompressedFileEntryHeader::CompressedFileEntryHeader(const uint32_t nCompressedSize,
                                                               const uint32_t nChunkSize,
                                                               const uint32_t nFileSize)
	: compressedSize(nCompressedSize)
	, chunkSize(nChunkSize)
	, numChunks((chunkSize != 0 && nFileSize != 0) ?
	            uint32_t(std::ceil(double(nFileSize) / double(chunkSize))) : 0)
{
	if (numChunks != 0)
	{
		chunkHeaders.reserve(numChunks);
	}

	#if SIEGE_TANK_DEBUG
	constexpr uint32_t Win32PageSize = 4;
	if ((chunkSize % Win32PageSize) != 0)
	{
		SiegeWarn("Compressed chunk size is not rounded to the size of a 4KB page!");
	}
	#endif // SIEGE_TANK_DEBUG
}

// ========================================================
// TankFile::FileEntry:
// ========================================================

TankFile::FileEntry::FileEntry(const uint32_t nParentOffs, const uint32_t nSize,
                               const uint32_t nOffset, const uint32_t crc, const FileTime ft,
                               const DataFormat dataFormat, const uint16_t fileFlags, std::string filename)
	: parentOffset(nParentOffs)
	, size(nSize)
	, offset(nOffset)
	, crc32(crc)
	, fileTime(ft)
	, format(dataFormat)
	, flags(fileFlags)
	, name(std::move(filename))
{
	#if SIEGE_TANK_DEBUG
	if (name.empty()) { SiegeWarn("Empty FileEntry name!"); }
	#endif // SIEGE_TANK_DEBUG
}

void TankFile::FileEntry::setCompressedHeader(std::unique_ptr<TankFile::CompressedFileEntryHeader> header)
{
	assert(header != nullptr);
	compressedHeader = std::move(header);
}

TankFile::CompressedFileEntryHeader & TankFile::FileEntry::getCompressedHeader()
{
	assert(compressedHeader != nullptr);
	return *compressedHeader.get();
}

const TankFile::CompressedFileEntryHeader & TankFile::FileEntry::getCompressedHeader() const
{
	assert(compressedHeader != nullptr);
	return *compressedHeader.get();
}

const TankFile::FileEntryChunkHeader & TankFile::FileEntry::getChunkHeader(const uint32_t index) const
{
	assert(compressedHeader != nullptr);
	assert(compressedHeader->chunkSize != 0);
	assert(index <= (size / compressedHeader->chunkSize));
	return compressedHeader->chunkHeaders[index];
}

bool TankFile::FileEntry::isInvalidFile() const noexcept
{
	return !!(flags & FileFlagInvalid);
}

bool TankFile::FileEntry::isCompressed() const noexcept
{
	return isDataFormatCompressed(format);
}

uint32_t TankFile::FileEntry::getUncompressedSize() const
{
	return size;
}

uint32_t TankFile::FileEntry::getCompressedSize() const
{
	return isCompressed() ? getCompressedHeader().compressedSize : size;
}

uint32_t TankFile::FileEntry::getChunkSize() const
{
	return isCompressed() ? getCompressedHeader().chunkSize : 0;
}

uint32_t TankFile::FileEntry::getChunkIndex(const uint32_t offs) const
{
	assert(getChunkSize() != 0);
	return offs / getChunkSize();
}

// ========================================================
// TankFile::FileSet:
// ========================================================

TankFile::FileSet::FileSet(const uint32_t numEntries)
	: numFiles(numEntries)
{
	if (numFiles != 0)
	{
		fileOffsets.reserve(numFiles);
		fileEntries.reserve(numFiles);
	}
}

// ========================================================
// TankFile::DirEntry:
// ========================================================

TankFile::DirEntry::DirEntry(const uint32_t nParentOffs, const uint32_t nChildCount,
                             const FileTime ft, std::string dirName)
	: parentOffset(nParentOffs)
	, childCount(nChildCount)
	, fileTime(ft)
	, name(std::move(dirName))
{
	if (childCount != 0)
	{
		childOffsets.reserve(childCount);
	}

	#if SIEGE_TANK_DEBUG
	if (name.empty()) { SiegeWarn("Empty DirEntry name!"); }
	#endif // SIEGE_TANK_DEBUG
}

TankFile::DirEntry::DirEntry(const uint32_t nParentOffs, const uint32_t nChildCount, const FileTime ft,
                             std::string dirName, std::vector<uint32_t> && childOffs)
	: parentOffset(nParentOffs)
	, childCount(nChildCount)
	, fileTime(ft)
	, name(std::move(dirName))
	, childOffsets(std::forward<std::vector<uint32_t>>(childOffs))
{
	#if SIEGE_TANK_DEBUG
	if (name.empty()) { SiegeWarn("Empty DirEntry name!"); }
	#endif // SIEGE_TANK_DEBUG
}

// ========================================================
// TankFile::DirSet:
// ========================================================

TankFile::DirSet::DirSet(const uint32_t numEntries)
	: numDirs(numEntries)
{
	if (numDirs != 0)
	{
		dirOffsets.reserve(numDirs);
		dirEntries.reserve(numDirs);
	}
}

// ========================================================
// TankFile::Error:
// ========================================================

TankFile::Error::Error() noexcept
	: Exception("Undefined TankFile error!")
{
}

TankFile::Error::Error(const char * error) noexcept
	: Exception(error)
{
}

TankFile::Error::Error(const std::string & error) noexcept
	: Exception(error.c_str())
{
}

TankFile::Error::~Error()
{
	// We are compiling with the `-Wweak-vtables` flag.
	// If we don't provide this dummy implementation here
	// the compiler would emit a warning for this class.
}

// ========================================================
// Static properties of TankFile:
// ========================================================

const FourCC TankFile::ProductId     = { 'D', 'S', 'i', 'g' }; // 'DSig'
const FourCC TankFile::TankId        = { 'T', 'a', 'n', 'k' }; // 'Tank'
const FourCC TankFile::CreatorIdGPG  = { '!', 'G', 'P', 'G' }; // '!GPG'
const FourCC TankFile::CreatorIdUser = { 'U', 'S', 'E', 'R' }; // 'USER'

// ========================================================
// Static methods of TankFile:
// ========================================================

std::string TankFile::priorityToString(const Priority priority)
{
	switch (priority)
	{
	case Priority::Factory   : return "Factory";
	case Priority::Language  : return "Language";
	case Priority::Expansion : return "Expansion";
	case Priority::Patch     : return "Patch";
	case Priority::User      : return "User";
	default : SiegeThrow(TankFile::Error, "Invalid TankFile::Priority flag!");
	} // switch (priority)
}

TankFile::Priority TankFile::priorityFromString(const std::string & str)
{
	if (str == "Factory")   return Priority::Factory;
	if (str == "Language")  return Priority::Language;
	if (str == "Expansion") return Priority::Expansion;
	if (str == "Patch")     return Priority::Patch;
	if (str == "User")      return Priority::User;
	SiegeThrow(TankFile::Error, "Invalid TankFile::Priority string: '" << str << "'.");
}

std::string TankFile::dataFormatToString(const DataFormat format)
{
	switch (format)
	{
	case DataFormat::Raw  : return "Raw";
	case DataFormat::Zlib : return "Zlib";
	case DataFormat::Lzo  : return "Lzo";
	default : SiegeThrow(TankFile::Error, "Invalid TankFile::DataFormat flag!");
	} // switch (format)
}

TankFile::DataFormat TankFile::dataFormatFromString(const std::string & str)
{
	if (str == "Raw")  return DataFormat::Raw;
	if (str == "Zlib") return DataFormat::Zlib;
	if (str == "Lzo")  return DataFormat::Lzo;
	SiegeThrow(TankFile::Error, "Invalid TankFile::DataFormat string: '" << str << "'.");
}

// ========================================================
// TankFile instance methods:
// ========================================================

void TankFile::openForReading(std::string filename)
{
	if (isOpen())
	{
		SiegeThrow(TankFile::Error, "File already open!");
	}

	if (filename.empty())
	{
		SiegeThrow(TankFile::Error, "No filename provided!");
	}

	if (!utils::filesys::tryOpen(file, filename, std::ios::binary))
	{
		SiegeThrow(TankFile::Error, "Failed to open Tank file \"" << filename
				<< "\": '" << utils::filesys::getLastFileError() << "'.");
	}

	fileName     = std::move(filename);
	fileOpenMode = std::ios::in | std::ios::binary;

	queryFileSize();
	readAndValidateHeader();

	SiegeLog("Successfully opened Tank file \"" << fileName
			<< "\" for reading. File size: " << utils::formatMemoryUnit(fileSizeBytes));
}

void TankFile::close()
{
	if (file.is_open())
	{
		file.close();
	}

	fileSizeBytes = 0;
	fileOpenMode  = 0;

	fileName.clear();
	fileHeader.setDefaults();
}

bool TankFile::isOpen() const noexcept
{
	return file.is_open();
}

bool TankFile::isReadOnly() const noexcept
{
	return (fileOpenMode & std::ios::in) &&
	      !(fileOpenMode & std::ios::out);
}

bool TankFile::isWriteOnly() const noexcept
{
	return (fileOpenMode & std::ios::out) &&
	      !(fileOpenMode & std::ios::in);
}

bool TankFile::isReadWrite() const noexcept
{
	return (fileOpenMode & std::ios::in) ||
	       (fileOpenMode & std::ios::out);
}

size_t TankFile::getFileSizeBytes() const noexcept
{
	return fileSizeBytes;
}

const TankFile::Header & TankFile::getFileHeader() const noexcept
{
	return fileHeader;
}

const std::string & TankFile::getFileName() const noexcept
{
	return fileName;
}

void TankFile::queryFileSize()
{
	assert(isOpen());
	utils::filesys::queryFileSize(fileName, fileSizeBytes);
	if (fileSizeBytes == 0)
	{
		SiegeWarn("Tank file \"" << fileName << "\" appears to be empty...");
	}
}

void TankFile::readAndValidateHeader()
{
	// Read in the header:
	fileHeader.productId      = readFourCC();
	fileHeader.tankId         = readFourCC();
	fileHeader.headerVersion  = readU32();
	fileHeader.dirsetOffset   = readU32();
	fileHeader.filesetOffset  = readU32();
	fileHeader.indexSize      = readU32();
	fileHeader.dataOffset     = readU32();
	fileHeader.productVersion = readProductVersion();
	fileHeader.minimumVersion = readProductVersion();
	fileHeader.priority       = static_cast<TankFile::Priority>(readU32());
	fileHeader.flags          = readU32();
	fileHeader.creatorId      = readFourCC();
	fileHeader.guid           = readGuid();
	fileHeader.indexCrc32     = readU32();
	fileHeader.dataCrc32      = readU32();
	fileHeader.utcBuildTime   = readSystemTime();
	readBytes(fileHeader.copyrightText, sizeof(fileHeader.copyrightText));
	readBytes(fileHeader.buildText,     sizeof(fileHeader.buildText));
	readBytes(fileHeader.titleText,     sizeof(fileHeader.titleText));
	readBytes(fileHeader.authorText,    sizeof(fileHeader.authorText));
	fileHeader.descriptionText = readWNString();

	// Optional debug printing:
	#if SIEGE_TANK_DEBUG
	SiegeLog("====== TANK HEADER FOR FILE: \"" << fileName << "\" ======");
	SiegeLog("productId.........: " << fileHeader.productId);
	SiegeLog("tankId............: " << fileHeader.tankId);
	SiegeLog("headerVersion.....: " << versionWordToStr(fileHeader.headerVersion));
	SiegeLog("dirsetOffset......: " << toHexa(fileHeader.dirsetOffset)  << " (" << utils::formatMemoryUnit(fileHeader.dirsetOffset)  << ")");
	SiegeLog("filesetOffset.....: " << toHexa(fileHeader.filesetOffset) << " (" << utils::formatMemoryUnit(fileHeader.filesetOffset) << ")");
	SiegeLog("indexSize.........: " << utils::formatMemoryUnit(fileHeader.indexSize));
	SiegeLog("dataOffset........: " << toHexa(fileHeader.dataOffset) << " (" << utils::formatMemoryUnit(fileHeader.dataOffset) << ")");
	SiegeLog("productVersion....: " << fileHeader.productVersion);
	SiegeLog("minimumVersion....: " << fileHeader.minimumVersion);
	SiegeLog("priority..........: " << priorityToString(fileHeader.priority));
	SiegeLog("flags.............: " << fileHeader.flags);
	SiegeLog("creatorId.........: " << fileHeader.creatorId);
	SiegeLog("Guid..............: " << fileHeader.guid);
	SiegeLog("indexCrc32........: " << toHexa(fileHeader.indexCrc32));
	SiegeLog("dataCrc32.........: " << toHexa(fileHeader.dataCrc32));
	SiegeLog("utcBuildTime......: " << fileHeader.utcBuildTime);
	SiegeLog("copyrightText.....: " << toString(fileHeader.copyrightText));
	SiegeLog("buildText.........: " << toString(fileHeader.buildText));
	SiegeLog("titleText.........: " << toString(fileHeader.titleText));
	SiegeLog("authorText........: " << toString(fileHeader.authorText));
	SiegeLog("descriptionText...: " << toString(fileHeader.descriptionText));
	SiegeLog("====== END TANK HEADER ======");
	#endif // SIEGE_TANK_DEBUG

	// Fatal errors:
	if (fileHeader.productId != TankFile::ProductId)
	{
		SiegeThrow(TankFile::Error, "\"" << fileName
				<< "\": Header product id doesn't match the expected value!");
	}
	if (fileHeader.tankId != TankFile::TankId)
	{
		SiegeThrow(TankFile::Error, "\"" << fileName
				<< "\": Header Tank id doesn't match the expected value!");
	}

	// Warnings:
	if (fileHeader.creatorId != TankFile::CreatorIdGPG &&
	    fileHeader.creatorId != TankFile::CreatorIdUser)
	{
		SiegeWarn("Tank creator id is unknown: " << fileHeader.creatorId);
	}
	if (fileHeader.headerVersion != Header::ExpectedVersion)
	{
		SiegeWarn("Unknown Tank header version: " << fileHeader.headerVersion);
	}
}

void TankFile::seekAbsoluteOffset(const size_t offsetInBytes)
{
	assert(isOpen());
	// Seek absolute offset relative to the beginning of the file.
	if (!file.seekg(offsetInBytes, std::ifstream::beg))
	{
		SiegeThrow(TankFile::Error, "Failed to seek file offset on TankFile::seekAbsoluteOffset()!");
	}
}

void TankFile::readBytes(void * buffer, const size_t numBytes)
{
	assert(buffer   != nullptr);
	assert(numBytes != 0);
	assert(isOpen());

	if (!file.read(reinterpret_cast<char *>(buffer), numBytes))
	{
		SiegeError("Only " << file.gcount() << " bytes of " << numBytes
				<< " could be read from \"" << fileName << "\"!");

		SiegeThrow(TankFile::Error, "Failed to read " << utils::formatMemoryUnit(numBytes)
				<< " from Tank file \"" << fileName << "\"!");
	}
}

uint16_t TankFile::readU16()
{
	uint16_t x = 0;
	readBytes(&x, sizeof(x));
	return x;
}

uint32_t TankFile::readU32()
{
	uint32_t x = 0;
	readBytes(&x, sizeof(x));
	return x;
}

std::string TankFile::readNString()
{
	auto lenInChars = readU16();
	if (lenInChars == 0)
	{
		readU16(); // Waste another word to make this a dword
		return std::string();
	}

	// NSTRINGs are stored aligned at dword boundary:
	lenInChars = alignToDword(lenInChars + 2) - 2; // 2 for the word we've just read
	assert(((lenInChars + 2) % sizeof(uint32_t)) == 0);

	if (lenInChars >= utils::MaxTempStringLen)
	{
		SiegeThrow(TankFile::Error, "String overflow in TankFile::readNString()! "
				<< lenInChars << " >= " << utils::MaxTempStringLen);
	}

	char buffer[utils::MaxTempStringLen];
	readBytes(buffer, lenInChars);
	buffer[lenInChars] = 0;

	return buffer;
}

WideString TankFile::readWNString()
{
	auto lenInChars = readU16();
	if (lenInChars == 0)
	{
		readU16(); // Waste another word to make this a dword
		return WideString();
	}

	// NSTRINGs are stored aligned at dword boundary:
	lenInChars = alignToDword(lenInChars + 2) - 2; // 2 for the word we've just read
	assert(((lenInChars + 2) % sizeof(uint32_t)) == 0);

	if (lenInChars >= utils::MaxTempStringLen)
	{
		SiegeThrow(TankFile::Error, "String overflow in TankFile::readWNString()! "
				<< lenInChars << " >= " << utils::MaxTempStringLen);
	}

	WideChar buffer[utils::MaxTempStringLen];
	readBytes(buffer, lenInChars * sizeof(WideChar));
	buffer[lenInChars] = 0;

	return buffer;
}

FileTime TankFile::readFileTime()
{
	FileTime ft;
	readBytes(&ft, sizeof(ft));
	return ft;
}

SystemTime TankFile::readSystemTime()
{
	SystemTime st;
	readBytes(&st, sizeof(st));
	return st;
}

ProductVersion TankFile::readProductVersion()
{
	ProductVersion pv;
	readBytes(&pv, sizeof(pv));
	return pv;
}

FourCC TankFile::readFourCC()
{
	FourCC fcc;
	readBytes(&fcc, sizeof(fcc));
	return fcc;
}

Guid TankFile::readGuid()
{
	Guid guid;
	readBytes(&guid, sizeof(guid));
	return guid;
}

} // namespace siege {}
