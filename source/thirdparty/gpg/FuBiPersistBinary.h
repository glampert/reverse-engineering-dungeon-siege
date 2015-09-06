//////////////////////////////////////////////////////////////////////////////
//
// File     :  FuBiPersistBinary.h
// Author(s):  Scott Bilas
//
// Summary  :  Contains a generic hierarchical binary data format designed for
//             fast streaming ability (for save game).
//
// Copyright © 2001 Gas Powered Games, Inc.  All rights reserved.
//----------------------------------------------------------------------------
//  $Revision:: $              $Date:$
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __FUBIPERSISTBINARY_H
#define __FUBIPERSISTBINARY_H

//////////////////////////////////////////////////////////////////////////////

#include "FileSysDefs.h"
#include "FuBiPersistImpl.h"

namespace FuBi  {  // begin of namespace FuBi

//////////////////////////////////////////////////////////////////////////////
// namespace TreeBinaryRaw declaration

namespace TreeBinaryRaw  {  // begin of namespace TreeBinaryRaw

    /*

    Structure of the DATA goes:

        It's simple as hell - entries are just strung back to back with no
        padding. All strings are stored directly, and null terminated. If
        storing a block name, it's just the name. If storing a key/value pair,
        then first store the keyname, then the value. Note that values are not
        stored null-terminated if a string. Instead, a DWORD prefix with the
        length is stored first, then the non-null-terminated string. This is
        consistent with BinaryPersistWriter convention for WriteString().

        DataHeader
        String\0                <blockname>
        String\0Value           <entryname><valuedata>
        String\0Value           <entryname><valuedata>
        String\0                <blockname>
        ...etc

    Structure of the INDEX goes:

        This one is simple as hell too - blocks are stored back to back, exactly
        as they are constructed on a write, in the same order. If a lookup fails
        it's probably because something was reset, so then fall back to lsearch.

        IndexHeader
        Block
            Values
            Blocks
        Block
            Values
            Blocks
        Block
            Values
            Blocks
        ...etc

    */

    enum
    {
        DATA_XFER_ID            = REVERSE_FOURCC( 'DXfr' ),     // identifier for header
        DATA_HEADER_VERSION     = MAKEVERSION( 1, 0, 0 ),       // version of DataHeader
        DATA_FLAG_NONE          = 0,                            // no flags currently

        INDEX_XFER_ID           = REVERSE_FOURCC( 'IXfr' ),     // identifier for header
        INDEX_HEADER_VERSION    = MAKEVERSION( 1, 0, 0 ),       // version of IndexHeader
        INDEX_FLAG_NONE         = 0,                            // no flags currently
    };

    struct DataHeader
    {
        FOURCC m_ProductId;
        FOURCC m_XferId;
        DWORD  m_HeaderVersion;
        DWORD  m_Flags;

        void Init( void );
    };

    struct IndexHeader
    {
        FOURCC m_ProductId;
        FOURCC m_XferId;
        DWORD  m_HeaderVersion;
        DWORD  m_Flags;

        void Init( void );
    };

}  // end of namespace TreeBinaryRaw

//////////////////////////////////////////////////////////////////////////////
// class TreeBinaryReader declaration

class TreeBinaryReader : public BinaryPersistReader
{
public:
    SET_INHERITED( TreeBinaryReader, BinaryPersistReader );

    // i use all this memory so don't throw it away until done reading!
    TreeBinaryReader( void );
    virtual ~TreeBinaryReader( void );

    bool Init( const_mem_ptr data, const_mem_ptr index );

#   if !GP_RETAIL
    void Dump( const char* rootName, ReportSys::ContextRef ctx = NULL ) const;
#   endif // !GP_RETAIL

    virtual bool EnterBlock( const char* name );
    virtual bool LeaveBlock( void );
    virtual bool ReadString( eXfer xfer, const char* key, gpstring& value );
    virtual bool ReadString( eXfer xfer, const char* key, gpwstring& value );
    virtual bool ReadBinary( const char* key, mem_ptr ptr );

private:
    struct Block;

#   if !GP_RETAIL
//$$$   void outputBlock( const char* blockName, const Block* block, int*& sizeIter, ReportSys::ContextRef ctx ) const;
#   endif // !GP_RETAIL

    bool readBinary( const char* key, mem_ptr* ptr, gpstring* str, gpwstring* wstr );

    struct BlockChild
    {
        DWORD m_DataOffset;                     // offset into data where name is stored
        DWORD m_BlockIndex;                     // index of child block within the array
    };

    struct Block
    {
        // value info
        DWORD        m_ValueCount;              // number of values in this block
        const DWORD* m_ValueDataOffsets;        // pointer into index containing offsets into data where key/value pairs stored

        // subblock info
        DWORD             m_BlockCount;         // number of subblocks in this block
        const BlockChild* m_BlockInfos;         // pointer into index containing subblock infos
        bool              m_BlocksOwned;        // true if we own the blocks (they will be sorted btw)

        Block( void )
        {
            ::ZeroObject( *this );
        }

       ~Block( void )
        {
            if ( m_BlocksOwned )
            {
                delete [] ( (BlockChild*)m_BlockInfos );
            }
        }
    };

    struct BlockIter
    {
        Block*            m_Block;              // block we're working with
        const DWORD*      m_NextValue;          // next value to use (cache)
        const BlockChild* m_NextBlock;          // next subblock to use (cache)

        BlockIter( Block* block )
        {
            m_Block     = block;
            m_NextValue = m_Block->m_ValueDataOffsets;
            m_NextBlock = m_Block->m_BlockInfos;
        }

        void AdvanceBlockIter( void )
        {
            // advance
            ++m_NextBlock;

            // wrap if go too far
            if ( m_NextBlock == (m_Block->m_BlockInfos + m_Block->m_BlockCount) )
            {
                m_NextBlock = m_Block->m_BlockInfos;
            }
        }

        void AdvanceValueIter( void )
        {
            // advance
            ++m_NextValue;

            // wrap if go too far
            if ( m_NextValue == (m_Block->m_ValueDataOffsets + m_Block->m_ValueCount) )
            {
                m_NextValue = m_Block->m_ValueDataOffsets;
            }
        }
    };

    struct LessByIndirectName
    {
        const_mem_ptr m_Data;

        // $ note: strings are all lower case already, so can use strcmp

        LessByIndirectName( const_mem_ptr data )
            : m_Data( data )  {  }

        bool operator () ( const BlockChild& l, const BlockChild& r ) const
        {
            const char* lstr = (const char*)m_Data.mem + l.m_DataOffset;
            const char* rstr = (const char*)m_Data.mem + r.m_DataOffset;
            return ( compare_with_case( lstr, rstr ) < 0 );
        }

        template <typename T>
        bool operator () ( const T& l, const char* r ) const
        {
            const char* lstr = (const char*)m_Data.mem + l.m_DataOffset;
            return ( compare_with_case( lstr, r ) < 0 );
        }

        template <typename T>
        bool operator () ( const char* l, const T& r ) const
        {
            const char* rstr = (const char*)m_Data.mem + r.m_DataOffset;
            return ( compare_with_case( l, rstr ) < 0 );
        }
    };

    typedef stdx::fast_vector <BlockIter> BlockStack;
    typedef stdx::fast_vector <Block> BlockColl;

    BlockColl     m_BlockColl;              // collection of objects containing pointers into index data
    BlockStack    m_BlockStack;             // stack of blocks we've entered, always at least contains root
    const_mem_ptr m_Data;                   // base of data file

    SET_NO_COPYING( TreeBinaryReader );
};

//////////////////////////////////////////////////////////////////////////////
// class TreeBinaryWriter declaration

class TreeBinaryWriter : public BinaryPersistWriter
{
public:
    SET_INHERITED( TreeBinaryWriter, BinaryPersistWriter );

    TreeBinaryWriter( FileSys::StreamWriter& dataWriter );
    virtual ~TreeBinaryWriter( void );

    bool WriteIndex( FileSys::StreamWriter& indexWriter );

    virtual bool EnterBlock ( const char* name );
    virtual bool LeaveBlock ( void );
    virtual bool WriteString( eXfer xfer, const char* key, const gpstring& value, eVarType type );
    virtual bool WriteString( eXfer xfer, const char* key, const gpwstring& value );
    virtual bool WriteBinary( const char* key, const_mem_ptr ptr );

private:

    DWORD writeData( const void* data, int size );

    template <typename T>
    DWORD writeData( const T& data )
    {
        return ( writeData( &data, sizeof( T ) ) );
    }

    struct Block;

    // note that the m_NameIndex below has three purposes. first, it allows
    //  storing the name in a big string buffer rather than lots of little
    //  individual allocations. second it allows us to reference the strings
    //  even after they have been written to the data (which is then lost and
    //  can't be used for comparisons). and third it is a convenient way to
    //  re-sort the entries in the order that they were originally written to,
    //  which is an optimization for reading them back. of course m_DataOffset
    //  could be used for the same purpose, but whatever...

    struct ValueChild
    {
        int   m_NameIndex;              // index into m_StringBuffer where my name is
        DWORD m_DataOffset;             // offset into saved data where my name begins - value is followed by this
    };

    struct BlockChild
    {
        int   m_NameIndex;              // index into m_StringBuffer where my name is
        DWORD m_BlockIndex;             // index into m_BlockBuffer where my block is
        DWORD m_DataOffset;             // offset into saved data where my name begins
    };

    typedef stdx::fast_vector <char, stdx::st_pool_allocator> StringBuffer;

    struct LessByIndirectName
    {
        const StringBuffer& m_StringBuffer;

        // $ note: strings are all lower case already, so can use strcmp

        LessByIndirectName( const StringBuffer& stringBuffer )
            : m_StringBuffer( stringBuffer )  {  }

        bool operator () ( const ValueChild& l, const ValueChild& r ) const
        {
            const char* lstr = &m_StringBuffer[ l.m_NameIndex ];
            const char* rstr = &m_StringBuffer[ r.m_NameIndex ];
            return ( compare_with_case( lstr, rstr ) < 0 );
        }

        bool operator () ( const BlockChild& l, const BlockChild& r ) const
        {
            const char* lstr = &m_StringBuffer[ l.m_NameIndex ];
            const char* rstr = &m_StringBuffer[ r.m_NameIndex ];
            return ( compare_with_case( lstr, rstr ) < 0 );
        }

        template <typename T>
        bool operator () ( const T& l, const char* r ) const
        {
            const char* lstr = &m_StringBuffer[ l.m_NameIndex ];
            return ( compare_with_case( lstr, r ) < 0 );
        }

        template <typename T>
        bool operator () ( const char* l, const T& r ) const
        {
            const char* rstr = &m_StringBuffer[ r.m_NameIndex ];
            return ( compare_with_case( l, rstr ) < 0 );
        }
    };

    struct SameByIndirectName
    {
        const StringBuffer& m_StringBuffer;
        const char*         m_Compare;

        SameByIndirectName( const StringBuffer& stringBuffer, const char* compare )
            : m_StringBuffer( stringBuffer ), m_Compare( compare )  {  }

        template <typename T>
        bool operator () ( const T& obj ) const
        {
            const char* str = &m_StringBuffer[ obj.m_NameIndex ];
            return ( same_with_case( m_Compare, str ) );
        }
    };

    struct LessByIndex
    {
        template <typename T, typename U>
        bool operator () ( const T& l, const U& r ) const
        {
            return ( l.m_NameIndex < r.m_NameIndex );
        }
    };

    typedef stdx::fast_vector <ValueChild, stdx::st_pool_allocator> ValueColl;
    typedef stdx::linear_set <BlockChild, LessByIndirectName, stdx::fast_vector <BlockChild, stdx::st_pool_allocator> > BlockColl;
    typedef stdx::fast_vector <Block, stdx::st_pool_allocator> BlockBuffer;
    typedef stdx::fast_vector <int, stdx::st_pool_allocator> IntColl;

    struct Block
    {
        ValueColl m_Values;
        BlockColl m_Blocks;

        Block( const StringBuffer& stringBuffer )
            : m_Blocks( stringBuffer )  {  }
    };

    FileSys::StreamWriter& m_DataWriter;            // data file goes here
    DWORD                  m_DataOffset;            // current offset into data
    StringBuffer           m_StringBuffer;          // cheapo string allocator
    BlockBuffer            m_BlockBuffer;           // linear array of built blocks
    IntColl                m_BlockStack;            // stack of indexes of current blocks

    SET_NO_COPYING( TreeBinaryWriter );
};

//////////////////////////////////////////////////////////////////////////////

}  // end of namespace FuBi

#endif  // __FUBIPERSISTBINARY_H

//////////////////////////////////////////////////////////////////////////////
