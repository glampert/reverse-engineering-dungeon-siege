//////////////////////////////////////////////////////////////////////////////
//
// File     :  TankStructure.h
// Author(s):  Scott Bilas
//
// Summary  :  Contains the binary structure and docs for Tank files.
//
// Copyright © 1999 Gas Powered Games, Inc.  All rights reserved.
//----------------------------------------------------------------------------
//  $Revision:: $              $Date:$
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __TANKSTRUCTURE_H
#define __TANKSTRUCTURE_H

//////////////////////////////////////////////////////////////////////////////

#include "FileSys.h"
#include "FileSysDiyMap.h"

//////////////////////////////////////////////////////////////////////////////
// storage rules

/*
    use offsets rather than indexes whenever possible to save the multiply. use
    indexes when memory size more important and a WORD size possible.

    all names will be in lower case for faster compares. convert any incoming
    search names to lower case as well. you'll get assertions otherwise.

    always favor memory usage efficiency over performance. the hard drive or
    CDROM is always going to be the bottleneck.

    always update version numbers when storage layout changes (additions,
    deletions, changes). then go back and update the reader and writer classes
    to make sure they incorporate the changes.
*/

//////////////////////////////////////////////////////////////////////////////
// storage layout

/*
    Two basic formats for a tank - first is the RAW style, which is just the
    tank sections stored with header followed by data followed by index, where
    data is aligned to a page boundary (4K). Second is the PE style, which is an
    ordinary Win32 PE format file with the TANK stored in .ITANK and .DTANK
    sections, plus a .RSRC section containing the VERSIONINFO which gets its
    data from .ITANK.

    General notes:

    *   All headers are 4-byte aligned in size

    *   All data chunks are aligned to DATA_ALIGNMENT-byte boundaries

    *   Magic numbers must always go UP as time goes on (to keep builds in
        order) so use some form of time based algo to get it. Possibly hash into
        the machine name and then an ID (such as 'DDir') or something.

    *   Directories are used to index the tank. After that's all done,
        directories are no longer needed so they can be thrown out. So don't
        bother optimizing memory usage too much. Performance is more important:
        be able to build that master index quickly.

    Codes:

        WW  = packed word + word
        E   = cast to enum type
        EB  = cast to bitfield-style enum type
        ZST = zero-terminated string
        NST = zero-terminated string preceded by a WORD with its length (length does not include NULL)
        HO  = offset from top of Header
        DSO = offset from top of DirSet
        FSO = offset from top of FileSet
        DO  = offset from top of data section
        R4  = reversed fourcc (human readable)
        #   = # byte structure

    Structure of a Tank goes:

        RawIndex (I)
            Header

        RawData
            FILEDATA        <pad to DATA_ALIGNMENT>
            FILEDATA
            FILEDATA
            ...

        RawIndex (II)
            DirSet
                Header
                DIRENTRY
                DIRENTRY
                DIRENTRY
                ...

            FileSet
                Header
                FILEENTRY
                FILEENTRY
                FILEENTRY
                ...

    Decompression of chunked compression formats:

        We break up files into chunks to allow random access decompression (used
        for LNC's and such). Note that chunks must be a multiple of the system
        memory page size. To save memory and CPU time we read the compressed
        data directly into our target decompression buffer, which is the memory
        that has been requested via SEH handling of the FileHandle on a page
        fault. We decompress in-place, which will use up all the space in the
        buffer (due to decompression overhead). Then we go back and overwrite
        the overrun with the remaining 'extra' bytes, which follows the
        compressed data in the stream.

        In this example, assume a 4K system memory page size, and a 16K chunk
        size. We're working on a full 16K chunk, which compresses down to 5189
        bytes, with a 16 byte overhead.

        Legend: C = compressed data, U = uncompressed data, E = extra

        1. Commit and unprotect the target memory pages:

        +-------+-------+-------+-------+
        |       |       |       |       |   <- decompression buffer
        +-------+-------+-------+-------+
        0                               1   <- chunk boundary
        0       4       8       12      16  <- pages (in K)

        2. Find the compressed data:

        +----------+-+
        |CCCCCCCCCC|E|                      <= compressed data + extra
        +----------+-+

        3. Memcpy the compressed data into our target:

        +--------------------+----------+
        |                    |CCCCCCCCCC|
        +--------------------+----------+

        4. Decompress in place:

        +-----------------------------+-+
        |UUUUUUUUUUUUUUUUUUUUUUUUUUUUU|C|
        +-----------------------------+-+

        5. Copy in extra at the end:

                                      +-+
                                      |E|
                                      +-+

        6. Result:

        +-------------------------------+
        |UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU|
        +-------------------------------+

        Note: if the file is stored un-chunked (chunksize of 0) then none of
        this applies. Just alloc the required memory. On lzo, be sure to
        include the overhead of decompression as usual when allocating the
        memory. Do this always when allocating memory regardless.

    Computing overhead:

        We'll use a simple brute-force approach. Start with an overhead of 16,
        then decompress, and see if it works. If not, then double the overhead
        and repeat until it works. If the compressed + extra ever exceeds the
        minimum compression ratio, then just toss the whole chunk and store
        uncompressed.
*/

//////////////////////////////////////////////////////////////////////////////
// useful macros

#define GET_PAD_TO_DWORD( x ) (((((x) & 3) + 3) & 4) - ((x) & 3))

//////////////////////////////////////////////////////////////////////////////
// namespace TankConstants declaration

namespace TankConstants  {  // begin of namespace TankConstants

// Enums.

    enum ePriority  // $ max width = 16 bits
    {
        // this enum is used to determine the priority of one tank vs. another
        //  regarding which is added to the master index first.

        // $ note on how priority works: it's like a version in that it is built
        //   from two pieces - a major priority and a minor priority.

        PRIORITY_FACTORY   = 0x0000,                    // GPG-issued "factory configured" tank (original CD release)
        PRIORITY_LANGUAGE  = 0x1000,                    // GPG-issued language pack, filled with localized resource overrides
        PRIORITY_EXPANSION = 0x2000,                    // GPG- or affiliate-issued expansion pack tank
        PRIORITY_PATCH     = 0x3000,                    // some kind of patch tank
        PRIORITY_USER      = 0x4000,                    // user-constructed tank
    };

    gpstring ToString  ( ePriority priority );
    bool     FromString( const char* str, ePriority& priority );

    enum eTankFlags  // $ max width = 32 bits
    {
        // flags used for an entire tank file

        TANKFLAG_NONE                   =      0,
        TANKFLAG_NON_RETAIL             = 1 << 0,       // this is a development-only tank (not for retail usage)
        TANKFLAG_ALLOW_MULTIPLAYER_XFER = 1 << 1,       // allow transfer of this tank during multiplayer
        TANKFLAG_PROTECTED_CONTENT      = 1 << 2,       // this is protected content (for optional use by extractors)
    };

    MAKE_ENUM_BIT_OPERATORS( eTankFlags );

    gpstring ToString  ( eTankFlags flags );
    bool     FromString( const char* str, eTankFlags& flags );

    enum eDataFormat  // $ max width = 16 bits
    {
        // this enum is for the different formats the data can be stored in

        DATAFORMAT_RAW,                                 // this resource is in raw format
        DATAFORMAT_ZLIB,                                // this resource is zlib-compressed
        DATAFORMAT_LZO,                                 // this resource is lzo-compressed
    };

    inline bool IsCompressed( eDataFormat format )  {  return ( format != DATAFORMAT_RAW );  }

    enum eFileFlags  // $ max width = 16 bits
    {
        // flags used per file resource

        FILEFLAG_NONE    =       0,
        FILEFLAG_INVALID = 1 << 15,                     // this resource had a problem during construction and is invalid
    };

// Constants.

    const DWORD DATA_SECTION_ALIGNMENT = 4 << 10;       // alignment for data section (RAW format)
    const DWORD DATA_ALIGNMENT         = 8;             // alignment for data files
    const DWORD INVALID_OFFSET         = 0xFFFFFFFF;    // can't use null, so use this instead
    const DWORD INVALID_CHECKSUM       = 0x00000000;    // use a zero checksum to say that it's not important or wasn't computed
    const DWORD LARGE_FILE             = 4 * 1024;      // this is what we consider a "large" file minimum size for optimization purposes (put small at front)

    const FOURCC TANK_ID               = REVERSE_FOURCC( 'Tank');

    const FOURCC CREATOR_GPG           = 'GPG!';
    const FOURCC CREATOR_USER          = 'USER';

}  // end of namespace TankConstants

// add these to global namespace for fubi traits
using TankConstants::ToString;

// define traits for NSTRING type
namespace Tank  {  struct NSTRING;  }
template <> struct StringTraits <Tank::NSTRING> : StringManagedTag <char>  {  };

//////////////////////////////////////////////////////////////////////////////
// begin namespace Tank

namespace Tank  {  // begin of namespace Tank

using namespace TankConstants;

//////////////////////////////////////////////////////////////////////////////
// struct NSTRING declaration

// this is a zero-terminated string where we know its length in advance for
//  fast rejection on searching and quick iteration. it's also dword-aligned
//  in size.

#pragma pack ( push, 1 )

struct NSTRING
{
    WORD m_Length;
    CHAR m_Text   [ 1 /*m_Length + 1*/ ];                           // always zero-terminated
//  BYTE m_Padding[ GET_PAD_TO_DWORD( m_Length + 3 ) ];             // pad entire structure to dword size

    // utility
    static size_t   GetSize( int len )      {  return ( GetDwordAlignUp( sizeof( WORD ) + len + 1 ) );  }
    size_t          GetSize( void ) const   {  return ( GetSize( m_Length ) );  }

    const char* c_str( void ) const         {  return ( m_Text );  }
    size_t length( void ) const             {  return ( m_Length );  }
    bool empty( void ) const                {  return ( m_Length == 0 );  }

    // operators
    operator const char* ( void ) const     {  return ( m_Text );  }

    // call delete to free what these create
    static NSTRING* Create( int len );
    static NSTRING* Create( const char* str, int len = -1 );
};

//////////////////////////////////////////////////////////////////////////////
// struct WNSTRING declaration

// this is a zero-terminated wide string where we know its length in advance for
//  fast rejection on searching and quick iteration. it's also dword-aligned
//  in size.

struct WNSTRING
{
    WORD  m_Length;
    WCHAR m_Text   [ 1 /*m_Length + 1*/ ];                          // always zero-terminated
//  BYTE  m_Padding[ GET_PAD_TO_DWORD( m_Length + 2 ) ];            // pad entire structure to dword size

    // utility
    static size_t   GetSize( int len )      {  return ( GetDwordAlignUp( sizeof( WORD ) + ((len + 1) * 2) ) );  }
    size_t          GetSize( void ) const   {  return ( GetSize( m_Length ) );  }

    const wchar_t* c_str( void ) const      {  return ( m_Text );  }
    size_t length( void ) const             {  return ( m_Length );  }
    bool empty( void ) const                {  return ( m_Length == 0 );  }

    // operators
    operator const wchar_t* ( void ) const  {  return ( m_Text );  }

    // call delete to free what these create
    static WNSTRING* Create( int len );
    static WNSTRING* Create( const wchar_t* str, int len = -1 );
};

//////////////////////////////////////////////////////////////////////////////
// struct RawIndex declaration

struct RawIndex
{

// Header.

    enum
    {
        HEADER_VERSION        = MAKEVERSION( 1, 0, 2 ),                 // $ change when modifying the static data format

        COPYRIGHT_TEXT_LENGTH = 100,
        BUILD_TEXT_LENGTH     = 100,
        TITLE_TEXT_LENGTH     = 100,
        AUTHOR_TEXT_LENGTH    = 40,
        RAW_HEADER_PAD        = 16,                                     // used for padding between end of header and start of raw data
    };

    struct Header
    {
        // $ all offsets are to the base of Header (this)

        //             ------ Base ------
/*R4*/  FOURCC         m_ProductId;                                     // ID of product (human readable) - always PRODUCT_ID
/*R4*/  FOURCC         m_TankId;                                        // ID of tank (human readable) - always TANK_ID
/*WW */ DWORD          m_HeaderVersion;                                 // version of this particular header
/*HO */ DWORD          m_DirSetOffset;                                  // (DirSet*) offset
/*HO */ DWORD          m_FileSetOffset;                                 // (FileSet*) offset
        DWORD          m_IndexSize;                                     // total size of index (header plus all dir data - used for RAW format)
/*HO */ DWORD          m_DataOffset;                                    // offset to start of data (used for RAW format)

        //             ------ V1.0 Extra - Basic ------
/*12 */ ProductVersion m_ProductVersion;                                // product version this tank was built with
/*12 */ ProductVersion m_MinimumVersion;                                // minimum product version required to use this tank
/*WW */ DWORD          m_Priority;                                      // priority that this tank is entered into the master index
/*EB */ DWORD          m_Flags;                                         // flags regarding this tank (eTankFlags)
        FOURCC         m_CreatorId;                                     // who created this tank (creation tool will choose, not user)
        GUID           m_GUID;                                          // true GUID assigned at creation time
        DWORD          m_IndexCRC32;                                    // CRC-32 of just the index (not including the header)
        DWORD          m_DataCRC32;                                     // CRC-32 of just the data
        SYSTEMTIME     m_UtcBuildTime;                                  // when this tank was constructed (stored in UTC)
/*ZST*/ WCHAR          m_CopyrightText  [ COPYRIGHT_TEXT_LENGTH ];      // copyright text
/*ZST*/ WCHAR          m_BuildText      [ BUILD_TEXT_LENGTH ];          // text about how this was built

        //             ------ V1.0 Extra - User Info ------
/*ZST*/ WCHAR          m_TitleText      [ TITLE_TEXT_LENGTH ];          // title of this tank
/*ZST*/ WCHAR          m_AuthorText     [ AUTHOR_TEXT_LENGTH ];         // who made this tank
        WNSTRING       m_DescriptionText;                               // anything the user wants can go here

        //             ------ Other Version Extra ------
    //                 ... extra header data for future versions ... add these
    //                 as struct Header_X_YY below, where X is the major version
    //                 and YY is the minor version.

        void Init( void );
    };

#   if 0
    struct Header_1_10 : public Header // Header v1.10 (this is an example)
    {
        // ...
    };
#   endif // 0

// Directories.

    struct DirSet
    {
        DWORD    m_Count;                                   // total number of directories
/*DSO*/ DWORD    m_Offsets   [ 1 /* m_Count */ ];           // for easiest iteration
    //  DirEntry m_DirEntries[ m_Count ];                   // sorted alphabetically within each node
    };

    struct DirEntry
    {
/*DSO*/ DWORD    m_ParentOffset;                            // where's the base of our parent DirEntry? (null for root)
        DWORD    m_ChildCount;                              // how many children in this DirEntry?
        FILETIME m_FileTime;                                // last modified timestamp of dir
        NSTRING  m_Name;                                    // what's my name?
//DSO   DWORD    m_ChildOffsets[ m_ChildCount ];            // offsets to each child - use range to determine if file or dir (these are sorted)

        const DWORD* GetOffsets( void ) const
        {
            const BYTE* base = rcast <const BYTE*> ( &m_Name );
            return ( rcast <const DWORD*> ( base + m_Name.GetSize() ) );
        }
    };

// Files.

    struct FileSet
    {
        // $ all offsets here are to the base of FileSet (this)

        DWORD     m_Count;                                  // total number of files
/*FSO*/ DWORD     m_Offsets    [ 1 /* m_Count */ ];         // for easiest iteration
    //  FileEntry m_FileEntries[ m_Count ];                 // sorted alphabetically overall
    };

    struct CompressedHeader
    {
        DWORD       m_CompressedSize;                       // size of compressed data
        DWORD       m_ChunkSize;                            // size of chunks, 0 for not chunked
    //  ChunkHeader m_ChunkHeaders[ ceil( m_Size / m_ChunkSize ) ];
    };

    struct ChunkHeader
    {
        DWORD m_UncompressedSize;                           // note: sizes are the same if this chunk not compressed
        DWORD m_CompressedSize;
        DWORD m_ExtraBytes;                                 // extra bytes to read into the end to allow for decompression overhead
        DWORD m_Offset;                                     // offset from start of data to this chunk

        bool IsCompressed( void ) const                     {  return ( m_UncompressedSize != m_CompressedSize );  }
    };

    struct FileEntry
    {
/*DSO*/ DWORD    m_ParentOffset;                            // where's the base of our parent DirEntry?
        DWORD    m_Size;                                    // size of resource
/*DO*/  DWORD    m_Offset;                                  // offset to data from top of data section
        DWORD    m_CRC32;                                   // CRC-32 of just this resource
        FILETIME m_FileTime;                                // last modified timestamp of file when it was added
/*E*/   WORD     m_Format;                                  // data format (eDataFormat)
/*EB*/  WORD     m_Flags;                                   // flags (eFileFlags)
        NSTRING  m_Name;                                    // what's my name?
    //  CompressedHeader m_CompressedHeader;                // (optional compression header follows)

        const CompressedHeader* GetCompressedHeader( void ) const
        {
            gpassert( IsCompressed() );
            return ( rcast <const CompressedHeader*> ( rcast <const BYTE*> ( &m_Name ) + m_Name.GetSize() ) );
        }

        const ChunkHeader* GetChunkHeader( int index ) const
        {
            const CompressedHeader* header = GetCompressedHeader();
            gpassert( header->m_ChunkSize != 0 );
            gpassert( index <= (int)(m_Size / header->m_ChunkSize) );
            return ( rcast <const ChunkHeader*> ( header + 1 ) + index );
        }

        bool  IsCompressed       ( void ) const             {  return ( TankConstants::IsCompressed( (eDataFormat)m_Format ) );  }
        DWORD GetUncompressedSize( void ) const             {  return ( m_Size );  }
        DWORD GetCompressedSize  ( void ) const             {  return ( IsCompressed() ? GetCompressedHeader()->m_CompressedSize : m_Size );  }
        DWORD GetChunkSize       ( void ) const             {  return ( IsCompressed() ? GetCompressedHeader()->m_ChunkSize      : 0      );  }
        int   GetChunkIndex      ( int offset ) const       {  gpassert( GetChunkSize() != 0 );  return ( offset / GetChunkSize() );  }
        bool  IsInvalidFile      ( void ) const             {  return ( !!(m_Flags & FILEFLAG_INVALID) );  }
    };

// Verification.

    // ensure we're all aligned ok
    COMPILER_ASSERT( ((sizeof( Header           ) - sizeof( WNSTRING )) % sizeof( DWORD )) == 0 );
    COMPILER_ASSERT(  (sizeof( DirSet           )                       % sizeof( DWORD )) == 0 );
    COMPILER_ASSERT( ((sizeof( DirEntry         ) - sizeof( NSTRING ))  % sizeof( DWORD )) == 0 );
    COMPILER_ASSERT(  (sizeof( FileSet          )                       % sizeof( DWORD )) == 0 );
    COMPILER_ASSERT(  (sizeof( CompressedHeader )                       % sizeof( DWORD )) == 0 );
    COMPILER_ASSERT(  (sizeof( ChunkHeader      )                       % sizeof( DWORD )) == 0 );
    COMPILER_ASSERT( ((sizeof( FileEntry        ) - sizeof( NSTRING ))  % sizeof( DWORD )) == 0 );
};

#pragma pack ( pop )

//////////////////////////////////////////////////////////////////////////////
// class TankFile declaration

class TankFile
{
public:
    SET_NO_INHERITED( TankFile );

// Types.

    typedef RawIndex::Header     Header;
    typedef RawIndex::DirSet     DirSet;
    typedef RawIndex::DirEntry   DirEntry;
    typedef RawIndex::FileSet    FileSet;
    typedef RawIndex::FileEntry  FileEntry;
    typedef FileSys ::File       File;
    typedef FileSys ::DiyFileMap DiyFileMap;
    typedef FileSys ::eLocation  eLocation;
    typedef FileSys ::FindData   FindData;

// Methods.

    // ctor/dtor
         TankFile( void );
        ~TankFile( void );

    // resource API
    bool Open    ( const char* name, bool verifyIndex = true );
    bool Close   ( void );
    bool IsOpen  ( void ) const  {  return ( m_TankFile.IsOpen() );  }

    // access API
    File&           GetFile         ( void )                {  return ( m_TankFile );  }
    HANDLE          CreateFile      ( void ) const;
    const gpstring& GetFileName     ( void ) const          {  gpassert( IsOpen() );  return ( m_FileName    );  }
    DWORD           GetFileSize     ( void ) const          {  gpassert( IsOpen() );  return ( m_FileSize    );  }
    eLocation       GetLocation     ( void ) const          {  gpassert( IsOpen() );  return ( m_Location    );  }
    const FindData& GetFindData     ( void ) const          {  gpassert( IsOpen() );  return ( m_FindData    );  }
    const Header&   GetHeader       ( void ) const          {  gpassert( IsOpen() );  return ( m_Header      );  }
    const DirSet*   GetDirSet       ( void ) const          {  gpassert( IsOpen() );  return ( m_DirSet      );  }
    const DirEntry* GetRootDir      ( void ) const          {  gpassert( IsOpen() );  return ( m_DirFirst    );  }
    const FileSet*  GetFileSet      ( void ) const          {  gpassert( IsOpen() );  return ( m_FileSet     );  }
    DWORD           GetDataOffset   ( void ) const          {  gpassert( IsOpen() );  return ( m_DataOffset  );  }
    const void*     GetIndexOffset  ( DWORD offset ) const  {  return ( rcast <const BYTE*> ( GetDirSet() ) + offset );  }
    const void*     GetFileSetOffset( DWORD offset ) const  {  return ( rcast <const BYTE*> ( GetFileSet() ) + offset );  }

    // searching API
                 const void* FindIn    ( const DirEntry* dirEntry, const gpstring& name ) const;        // must be lowercase
    inline const DirEntry*   FindDirIn ( const DirEntry* dirEntry, const gpstring& name ) const;        // must be lowercase
    inline const FileEntry*  FindFileIn( const DirEntry* dirEntry, const gpstring& name ) const;        // must be lowercase

    // verification API
    bool IsDirEntry ( const void* entry ) const  {  gpassert( IsOpen() );  return ( (entry >= m_DirFirst)  && (entry <= m_DirLast ) );  }
    bool IsFileEntry( const void* entry ) const  {  gpassert( IsOpen() );  return ( (entry >= m_FileFirst) && (entry <= m_FileLast) );  }

    // extra API
    const DirEntry* GetParent     ( const void* entry ) const;
    const NSTRING&  GetName       ( const void* entry ) const;
    void            BuildFileName ( gpstring& str, const void* entry, bool prependPath ) const;
    gpstring        BuildFileName ( const void* entry, bool prependPath ) const  {  gpstring str;  BuildFileName( str, entry, prependPath );  return ( str );  }
    bool            ShouldOverride( const TankFile& other ) const;

#   if !GP_RETAIL
    void Dump( ReportSys::ContextRef ctx = NULL );
#   endif // !GP_RETAIL

private:

// Internal utility.

    bool  OpenPrivate  ( DiyFileMap& tempMap, bool verifyIndex );
    bool  OpenDirect   ( bool verifyIndex );
    bool  OpenPE       ( DiyFileMap& tempMap, bool verifyIndex );
    DWORD InternalClose( DWORD error = ERROR_SUCCESS );

// Private data.

    // used for searching
    friend struct CompareEntries;

    // handles
    File           m_TankFile;          // the tank file
    DiyFileMap     m_TankIndex;         // map into the tank file

    // spec
    gpstring       m_FileName;          // filename of tank
    File::eAccess  m_FileAccess;        // access used to open file
    File::eSharing m_FileSharing;       // sharing model used to open file
    DWORD          m_FileFlags;         // flags used to open file
    DWORD          m_FileSize;          // cached size of file (asking for a file's size can be expensive)
    eLocation      m_Location;          // common location data for entire tank
    FindData       m_FindData;          // common find data for entire tank

    // header pointers
    Header         m_Header;            // copy of the header
    gpwstring      m_Description;       // copy of description text (needed 'cause dynamically sized)
    const DirSet*  m_DirSet;            // pointer to collection of dirs
    const FileSet* m_FileSet;           // pointer to collection of files
    DWORD          m_DataOffset;        // offset from base of file to start of data section

    // cache
    const DirEntry*  m_DirFirst;        // first dir in dirset
    const DirEntry*  m_DirLast;         // last dir in dirset
    const FileEntry* m_FileFirst;       // first file in fileset
    const FileEntry* m_FileLast;        // last file in fileset

    SET_NO_COPYING( TankFile );
};

//////////////////////////////////////////////////////////////////////////////
// class TankFile inline implementation

inline const TankFile::DirEntry* TankFile :: FindDirIn ( const DirEntry* dirEntry, const gpstring& name ) const
{
    const void* dir = FindIn( dirEntry, name );
    if ( !IsDirEntry( dir ) )
    {
        dir = NULL;
    }
    return ( rcast <const DirEntry*> ( dir ) );
}

inline const TankFile::FileEntry* TankFile :: FindFileIn( const DirEntry* dirEntry, const gpstring& name ) const
{
    const FileEntry* fileEntry = rcast <const FileEntry*> ( FindIn( dirEntry, name ) );
    if ( !IsFileEntry( fileEntry ) || fileEntry->IsInvalidFile() )
    {
        fileEntry = NULL;
    }
    return ( fileEntry );
}

//////////////////////////////////////////////////////////////////////////////
// end namespace Tank

}  // end of namespace Tank

//////////////////////////////////////////////////////////////////////////////

#endif  // __TANKSTRUCTURE_H

//////////////////////////////////////////////////////////////////////////////
