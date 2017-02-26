
// ================================================================================================
// -*- C++ -*-
// File: tank_file_reader.cpp
// Author: Guilherme R. Lampert
// Created on: 23/07/14
// Brief: TankFile::Reader inner class implementation.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/tank_file.hpp"
#include <algorithm>

namespace siege
{

// Can be easily disabled to avoid excessively verbose output.
#if SIEGE_TANK_DEBUG
	#define TankReaderLog SiegeLog
#else // !SIEGE_TANK_DEBUG
	#ifndef TankReaderLog
		#define TankReaderLog(x) /* nothing */
	#endif // TankReaderLog
#endif // SIEGE_TANK_DEBUG

// ========================================================
// TankFile::Reader:
// ========================================================

TankFile::Reader::Reader(TankFile & tank)
{
	indexFile(tank);
}

void TankFile::Reader::indexFile(TankFile & tank)
{
	if (!tank.isOpen())
	{
		SiegeThrow(TankFile::Error, "Tank file \"" << tank.getFileName() << "\" is not open!");
	}

	if (!tank.isReadWrite() && !tank.isReadOnly())
	{
		SiegeThrow(TankFile::Error, "Tank file \"" << tank.getFileName()
				<< "\" must be opened for reading before TankFile::Reader can index it!");
	}

	TankReaderLog("Preparing to index Tank file...");

	// Discard current metadata, if any, before loading new.
	dirSet  = nullptr;
	fileSet = nullptr;
	fileTable.clear();

	readDirSet(tank);
	readFileSet(tank);
}

void TankFile::Reader::readDirSet(TankFile & tank)
{
	const auto fileSize = tank.getFileSizeBytes();
	const auto & header = tank.getFileHeader();

	tank.seekAbsoluteOffset(header.dirsetOffset); // Seek DirSet position.

	const auto numDirectories = tank.readU32();
	dirSet.reset(new DirSet(numDirectories));

	TankReaderLog("====== readDirSet() ======");
	TankReaderLog("numDirectories = " << numDirectories);
	TankReaderLog("-");

	// Scan dir offset list:
	for (uint32_t d = 0; d < numDirectories; ++d)
	{
		const auto dirOffs = tank.readU32();
		if (dirOffs == TankFile::InvalidOffset || (header.dirsetOffset + dirOffs) > fileSize)
		{
			SiegeThrow(TankFile::Error, "Invalid directory offset: " << dirOffs);
		}
		dirSet->dirOffsets.push_back(dirOffs);
	}

	// Scan list of DirEntries:
	std::vector<uint32_t> childOffsets;
	std::string dirEntryName;
	for (uint32_t d = 0; d < numDirectories; ++d)
	{
		const auto dirOffs = dirSet->dirOffsets[d];
		tank.seekAbsoluteOffset(header.dirsetOffset + dirOffs);

		const auto dirParentOffset = tank.readU32();
		const auto dirChildCount   = tank.readU32();
		const auto dirFileTime     = tank.readFileTime();
		dirEntryName               = tank.readNString();

		// Validate parent offset:
		if (dirParentOffset == TankFile::InvalidOffset || (header.dirsetOffset + dirParentOffset) > fileSize)
		{
			SiegeThrow(TankFile::Error, "Invalid directory parent offset: " << dirParentOffset);
		}

		// Check for directory root and give it the proper root name if so:
		if (dirParentOffset == 0 && dirEntryName.empty())
		{
			// This must be the root directory, which is a dummy entry.
			dirEntryName = utils::filesys::getPathSeparator();
		}

		// Scan list of offsets to the children of this directory (files and other dirs):
		childOffsets.reserve(dirChildCount);
		for (uint32_t c = 0; c < dirChildCount; ++c)
		{
			const auto childOffs = tank.readU32();
			if (childOffs == TankFile::InvalidOffset || (header.dirsetOffset + childOffs) > fileSize)
			{
				SiegeThrow(TankFile::Error, "Invalid directory child offset: " << childOffs);
			}
			childOffsets.push_back(childOffs);
		}

		TankReaderLog("dirEntry.parentOffset..: " << dirParentOffset);
		TankReaderLog("dirEntry.myOffset......: " << dirOffs);
		TankReaderLog("dirEntry.childCount....: " << dirChildCount);
		TankReaderLog("dirEntry.fileTime......: " << dirFileTime);
		TankReaderLog("dirEntry.childOffsets..: " << childOffsets.size());
		TankReaderLog("dirEntry.name..........: " << dirEntryName);
		TankReaderLog("-");

		dirSet->dirEntries.emplace_back(dirParentOffset, dirChildCount, dirFileTime, dirEntryName, std::move(childOffsets));
		assert(childOffsets.empty());
	}

	// Fill 'fileTable' with the full directory paths:
	buildDirPaths();
}

void TankFile::Reader::readFileSet(TankFile & tank)
{
	const auto fileSize = tank.getFileSizeBytes();
	const auto & header = tank.getFileHeader();

	tank.seekAbsoluteOffset(header.filesetOffset); // Seek FileSet position.

	const auto numFiles = tank.readU32();
	fileSet.reset(new FileSet(numFiles));

	TankReaderLog("====== readFileSet() ======");
	TankReaderLog("numFiles = " << numFiles);
	TankReaderLog("-");

	// Scan file offset list:
	for (uint32_t f = 0; f < numFiles; ++f)
	{
		const auto fileOffs = tank.readU32();
		if (fileOffs == TankFile::InvalidOffset || (header.filesetOffset + fileOffs) > fileSize)
		{
			SiegeThrow(TankFile::Error, "Invalid file offset: " << fileOffs);
		}
		fileSet->fileOffsets.push_back(fileOffs);
	}

	// Scan list of FileEntries:
	std::string fileEntryName;
	for (uint32_t f = 0; f < numFiles; ++f)
	{
		const auto fileOffs = fileSet->fileOffsets[f];
		tank.seekAbsoluteOffset(header.filesetOffset + fileOffs);

		const auto fileParentOffset = tank.readU32();
		const auto fileEntrySize    = tank.readU32();
		const auto fileDataOffset   = tank.readU32();
		const auto fileCrc32        = tank.readU32();
		const auto fileTime         = tank.readFileTime();
		const auto fileFormat       = tank.readU16();
		const auto fileFlags        = tank.readU16();
		fileEntryName               = tank.readNString();

		// Validate parent offset:
		if (fileParentOffset == TankFile::InvalidOffset ||
		   (header.filesetOffset + fileParentOffset) > fileSize)
		{
			SiegeThrow(TankFile::Error, "Invalid file parent offset: " << fileParentOffset);
		}

		const auto fileDataFormat = static_cast<TankFile::DataFormat>(fileFormat);

		TankReaderLog("fileEntry.parentOffset..: " << fileParentOffset);
		TankReaderLog("fileEntry.myOffset......: " << fileOffs);
		TankReaderLog("fileEntry.size..........: " << utils::formatMemoryUnit(fileEntrySize));
		TankReaderLog("fileEntry.offset........: " << fileDataOffset);
		TankReaderLog("fileEntry.crc32.........: " << utils::format("0x%08X", fileCrc32));
		TankReaderLog("fileEntry.fileTime......: " << fileTime);
		TankReaderLog("fileEntry.format........: " << TankFile::dataFormatToString(fileDataFormat));
		TankReaderLog("fileEntry.flags.........: " << fileFlags);
		TankReaderLog("fileEntry.name..........: " << fileEntryName);
		TankReaderLog("fileEntry.isCompressed..: " << (TankFile::isDataFormatCompressed(fileDataFormat) ? "yes" : "no"));
		TankReaderLog("-");

		fileSet->fileEntries.emplace_back(fileParentOffset, fileEntrySize, fileDataOffset,
				fileCrc32, fileTime, fileDataFormat, fileFlags, fileEntryName);

		// We need to grab the compressed header for the compressed file entries.
		if (TankFile::isDataFormatCompressed(fileDataFormat) && fileEntrySize != 0)
		{
			const auto compressedSize = tank.readU32();
			const auto chunkSize      = tank.readU32();

			assert(compressedSize < fileSize);
			FileEntry & fileEntry = fileSet->fileEntries.back();

			fileEntry.setCompressedHeader(std::unique_ptr<CompressedFileEntryHeader>(
					new CompressedFileEntryHeader(compressedSize, chunkSize, fileEntrySize)));

			// Might have to read in some chunk headers:
			const auto numChunks = fileEntry.getCompressedHeader().numChunks;
			for (uint32_t c = 0; c < numChunks; ++c)
			{
				const auto uncompressedBytes = tank.readU32();
				const auto compressedBytes   = tank.readU32();
				const auto extraBytes        = tank.readU32();
				const auto offset            = tank.readU32();

				// Add to compressedHeader:
				fileEntry.getCompressedHeader().chunkHeaders.emplace_back(
						uncompressedBytes, compressedBytes, extraBytes, offset);
			}
		}
	}

	// Add full file paths to 'fileTable':
	buildFilePaths();
}

// ========================================================
// Local helpers:
// ========================================================

namespace
{

void buildPathRecursive(const size_t entryIndex, const TankFile::DirSet & dirSet, std::string & path)
{
	if (dirSet.dirEntries[entryIndex].parentOffset == 0)
	{
		return; // Is the root directory.
	}

	const auto parentIter = std::find(std::begin(dirSet.dirOffsets),
			std::end(dirSet.dirOffsets), dirSet.dirEntries[entryIndex].parentOffset);

	if (parentIter == std::end(dirSet.dirOffsets))
	{
		SiegeThrow(TankFile::Error, "Found an orphan directory entry! '"
				<< dirSet.dirEntries[entryIndex].name << "'.");
	}

	const auto parentIndex = std::distance(std::begin(dirSet.dirOffsets), parentIter);

	buildPathRecursive(parentIndex, dirSet, path);
	path += utils::filesys::getPathSeparator() + dirSet.dirEntries[entryIndex].name;
}

bool writeResourceFile(const std::string & destFileName, ByteArray fileContents)
{
	std::ofstream outFile;
	if (!utils::filesys::tryOpen(outFile, destFileName, std::ofstream::binary))
	{
		SiegeError("Failed to open file \"" << destFileName << "\" for writing!");
		return false;
	}

	if (fileContents.empty())
	{
		SiegeWarn("Written an empty resource file...");
		return true;
	}

	if (!outFile.write(reinterpret_cast<const char *>(fileContents.data()), fileContents.size()))
	{
		SiegeError("Failed to write " << fileContents.size() << " bytes to file \"" << destFileName << "\"!");
		return false;
	}

	return true;
}

} // namespace {}

// ========================================================

void TankFile::Reader::buildDirPaths()
{
	TankReaderLog("Building master directory table...");

	std::string fullPath;
	for (uint32_t d = 0; d < dirSet->numDirs; ++d)
	{
		fullPath.clear();
		buildPathRecursive(d, *dirSet, fullPath);
		fullPath += utils::filesys::getPathSeparator();

		fileTable.emplace(fullPath, TankEntry(&dirSet->dirEntries[d]));

		TankReaderLog("Dir: " << fullPath.c_str());
	}
}

void TankFile::Reader::buildFilePaths()
{
	TankReaderLog("Building master file table...");

	std::string fullPath;
	for (uint32_t f = 0; f < fileSet->numFiles; ++f)
	{
		fullPath.clear();

		// We don't need to do all that work if the file is at the root.
		if (fileSet->fileEntries[f].parentOffset != 0)
		{
			const auto parentIter = std::find(std::begin(dirSet->dirOffsets),
					std::end(dirSet->dirOffsets), fileSet->fileEntries[f].parentOffset);

			if (parentIter == std::end(dirSet->dirOffsets))
			{
				SiegeThrow(TankFile::Error, "Found an orphan file entry! '"
						<< fileSet->fileEntries[f].name << "' (parentOffset = "
						<< fileSet->fileEntries[f].parentOffset << ")");
			}

			const auto parentIndex = std::distance(std::begin(dirSet->dirOffsets), parentIter);
			buildPathRecursive(parentIndex, *dirSet, fullPath);
		}

		fullPath += utils::filesys::getPathSeparator() + fileSet->fileEntries[f].name;
		fileTable.emplace(fullPath, TankEntry(&fileSet->fileEntries[f]));

		TankReaderLog("File: " << fullPath.c_str());
	}
}

void TankFile::Reader::extractResourceToFile(TankFile & tank, const std::string & resourcePath,
                                             const std::string & destFile, const bool validateCRCs) const
{
	if (destFile.empty())
	{
		SiegeThrow(TankFile::Error, "No dest filename provided!");
	}

	auto fileContents = extractResourceToMemory(tank, resourcePath, validateCRCs);
	if (!writeResourceFile(destFile, std::move(fileContents)))
	{
		SiegeThrow(TankFile::Error, "Failed write resource file \""
				<< destFile << "\": '" << utils::filesys::getLastFileError() << "'.");
	}
}

TankFile::Task TankFile::Reader::extractResourceToFileAsync(TankFile & tank, const std::string & resourcePath,
                                                            const std::string & destFile, const bool validateCRCs) const
{
	if (destFile.empty())
	{
		SiegeThrow(TankFile::Error, "No dest filename provided!");
	}

	// Attempt the data extraction.
	// This must be done from the caller thread since we don't lock the Tank file.
	auto fileContents = extractResourceToMemory(tank, resourcePath, validateCRCs);

	// Fire the asynchronous task of writing the output and return the std::future handle.
	return std::async(std::launch::async, writeResourceFile, destFile, std::move(fileContents));
}

ByteArray TankFile::Reader::extractResourceToMemory(TankFile & tank, const std::string & resourcePath, const bool validateCRCs) const
{
	if (!tank.isOpen())
	{
		SiegeThrow(TankFile::Error, "Tank file \"" << tank.getFileName() << "\" is not open!");
	}

	if (!tank.isReadWrite() && !tank.isReadOnly())
	{
		SiegeThrow(TankFile::Error, "Tank file \"" << tank.getFileName()
				<< "\" must be opened for reading before you can extract data from it!");
	}

	const auto it = fileTable.find(resourcePath);
	if (it == std::end(fileTable))
	{
		SiegeThrow(TankFile::Error, "Resource \"" << resourcePath
				<< "\" not found in Tank file \"" << tank.getFileName() << "\"!");
	}

	assert(it->first == resourcePath);
	const Reader::TankEntry & entry = it->second;

	if (entry.type != TankEntry::TypeFile)
	{
		SiegeThrow(TankFile::Error, "Resource \"" << resourcePath << "\" in Tank file \""
				<< tank.getFileName() << "\" is a directory and cannot be decompressed to file!");
	}

	assert(entry.ptr.file != nullptr);
	const TankFile::FileEntry & resFile = *(entry.ptr.file);

	if (resFile.isInvalidFile())
	{
		// NOTE: Not sure if this should be a hard error...
		SiegeWarn("Resource file entry \"" << resFile.name << "\" is flagged as invalid!");
	}

	const auto fileOffset  = resFile.offset;
	const auto fileSize    = resFile.size;
	const auto dataOffset  = tank.getFileHeader().dataOffset;
	ByteArray fileContents;

	if (!resFile.isCompressed()) // Simple raw resource file:
	{
		TankReaderLog("Extracting UNCOMPRESSED Tank resource \"" << resourcePath << "\"...");

		// 'devlogic.dsres' (one of our test Tanks from the game) has
		// a few empty uncompressed dummy files. This check handles those.
		if (fileSize != 0)
		{
			tank.seekAbsoluteOffset(dataOffset + fileOffset);
			fileContents.resize(fileSize);
			tank.readBytes(fileContents.data(), fileContents.size());
		}
	}
	else // LZO/Zlib compressed:
	{
		TankReaderLog("Extracting COMPRESSED Tank resource \"" << resourcePath << "\". "
				<< "Uncompressed size: " << utils::formatMemoryUnit(fileSize, true)
				<< ", compression fmt: " << dataFormatToString(resFile.format));

		const auto & compressedHeader = resFile.getCompressedHeader();

		ByteArray compressedData;
		ByteArray uncompressedData;
		unsigned long uncompressedLen;

		fileContents.reserve(fileSize);

		for (uint32_t c = 0; c < compressedHeader.numChunks; ++c)
		{
			const TankFile::FileEntryChunkHeader & chunk = compressedHeader.chunkHeaders[c];

			// Individual chunks of data inside a compressed file might
			// be stored without compression. So this check is necessary.
			if (chunk.isCompressed())
			{
				tank.seekAbsoluteOffset(dataOffset + fileOffset + chunk.offset);
				compressedData.resize(chunk.compressedSize + chunk.extraBytes);
				tank.readBytes(compressedData.data(), compressedData.size());

				uncompressedData.resize(chunk.uncompressedSize + chunk.extraBytes);
				uncompressedLen = static_cast<unsigned long>(uncompressedData.size());

				TankReaderLog("Attempting to decompress resource chunk #" << (c + 1)
						<< " of " << compressedHeader.numChunks << "...");

				// Let Mini-Z do the decompression:
				const int errorCode = utils::compression::decompress(uncompressedData.data(), &uncompressedLen,
							compressedData.data(), static_cast<unsigned long>(compressedData.size() - chunk.extraBytes));

				assert(uncompressedLen != 0 && "Nothing was decompressed!");
				assert(uncompressedLen <= uncompressedData.size() && "Buffer overrun!");

				if (errorCode != 0)
				{
					auto errorInfo = utils::compression::getErrorString(errorCode);
					SiegeThrow(TankFile::Error, "Failed to decompress resource \"" << resourcePath
							<< "\"! Mini-Z error: '" << errorInfo << "'");
				}
			}
			else
			{
				TankReaderLog("Chunk #" << (c + 1) << " of " << compressedHeader.numChunks << " is stored without compression...");

				compressedData.clear();
				assert(chunk.uncompressedSize == chunk.compressedSize);

				tank.seekAbsoluteOffset(dataOffset + fileOffset + chunk.offset);
				uncompressedData.resize(chunk.uncompressedSize);
				tank.readBytes(uncompressedData.data(), uncompressedData.size());

				uncompressedLen = static_cast<unsigned long>(uncompressedData.size());
			}

			// Append decompressed chunk to the whole resource:
			//
			// extraBytes are not decompressed, they should be copied unchanged to the
			// end of the decompressed chunk. Refer to "gpg/TankStructure.h" for a nice
			// ASCII drawing of the process.
			//
			fileContents.insert(std::end(fileContents), std::begin(uncompressedData),
					std::begin(uncompressedData) + uncompressedLen);

			// Append extraBytes at the end of this chunk:
			// (compressedData is only empty for an uncompressed chunk)
			if (chunk.extraBytes != 0 && !compressedData.empty())
			{
				fileContents.insert(std::end(fileContents), std::begin(compressedData) + chunk.compressedSize,
						std::begin(compressedData) + chunk.compressedSize + chunk.extraBytes);
			}
		}
	}

	if (validateCRCs && !fileContents.empty())
	{
		const auto expectedCrc = resFile.crc32;
		const auto contentsCrc = utils::computeCrc32(fileContents.data(), fileContents.size());

		if (contentsCrc != expectedCrc)
		{
			auto errorInfo = utils::format("Tank resource \"%s\" CRC (0x%08X) does not match the expected (0x%08X)!",
					resourcePath.c_str(), contentsCrc, expectedCrc);

			SiegeThrow(TankFile::Error, errorInfo);
		}
	}

	TankReaderLog("Tank resource \"" << resourcePath << "\" extracted without errors.");
	return fileContents;
}

void TankFile::Reader::extractWholeTank(TankFile & tank, const std::string & destPath, const bool validateCRCs) const
{
	// destPath + '/' + tankName:
	std::string basePath = destPath;
	if (basePath.back() != utils::filesys::getPathSeparator()[0])
	{
		basePath += utils::filesys::getPathSeparator();
	}
	basePath += utils::filesys::removeFilenameExtension(tank.getFileName());

	TankReaderLog("Extracting whole Tank to \"" << basePath << "\"...");

	// Create the base path if not there yet:
	utils::filesys::createPath(basePath);

	// Preallocate for the table size. All entries are potentially files.
	std::vector<Task> tasks;
	tasks.reserve(fileTable.size());

	// Walk the file table and decompress each resource file:
	std::string destFile;
	for (const auto & entry : fileTable)
	{
		if (entry.second.type == TankEntry::TypeDir)
		{
			continue;
		}

		destFile = basePath + entry.first;

		utils::filesys::createPath(destFile);
		auto task = extractResourceToFileAsync(tank, entry.first, destFile, validateCRCs);
		tasks.emplace_back(std::move(task));
	}

	// Once all files are done, we synchronize.
	// NOTE: Should we allow for the caller to synchronize instead?
	//
	int filesSuccessfullyWritten = 0;
	for (auto & task : tasks)
	{
		filesSuccessfullyWritten += int(task.get());
	}

	TankReaderLog("extractWholeTank() successfully written " <<
			filesSuccessfullyWritten << " files to path: \"" << basePath << "\"");
}

std::vector<std::string> TankFile::Reader::getFileList() const
{
	std::vector<std::string> fileList;
	fileList.reserve(getFileCount());

	for (const auto & entry : fileTable)
	{
		if (entry.second.type == TankEntry::TypeDir)
		{
			continue;
		}
		fileList.push_back(entry.first);
	}

	return fileList;
}

std::vector<std::string> TankFile::Reader::getDirectoryList() const
{
	std::vector<std::string> dirList;
	dirList.reserve(getDirectoryCount());

	for (const auto & entry : fileTable)
	{
		if (entry.second.type == TankEntry::TypeFile)
		{
			continue;
		}
		dirList.push_back(entry.first);
	}

	return dirList;
}

#undef TankReaderLog

} // namespace siege {}
