
/* From here: http://scottbilas.com/games/dungeon-siege/

This is the format of our .raw files for textures. It is a custom format specific to
Dungeon Siege and has no relation to the .raw files that Photoshop or other image-processing
tools may output. If you’re messing with texture formats, you will probably also want to pay
attention to the following note on the layout of the mipmap images for all imported (non-raw)
formats from the source:

Be sure to store images in this bitmap upside down! This reader is
expecting mips stored like so (after flipping):

   +--------+.........
   |        |        .
   |   1    +---+    .
   |        | 2 +-+  .
   |        |   |3+-+.
   +--------+---+-+-++
   |                 |
   |                 |
   |                 |
   |                 |
   |        0        |
   |                 |
   |                 |
   |                 |
   +-----------------+

If there are no hand-made mipmaps then it will construct them on top of
the 0 surface. Each successive request for a new surface will box filter
the surface down over itself.

*/

//////////////////////////////////////////////////////////////////////////////
// class RapiRawReader declaration

/*
    ideas:  $$

    palettizing - look for unique colors and do up-to-256 index palette
    compression. also save 16-bit palette in addition to 32-bit palette, but
    the same compressed bits.
*/

class RapiRawReader : public RapiImageReader
{
public:
    SET_INHERITED( RapiRawReader, RapiImageReader );

// Header type.

    enum  {  HEADER_MAGIC = 'Rapi' /*FOURCC*/  };

    enum /*FOURCC*/ eFormat
    {
        FORMAT_UNKNOWN   = 0,
        FORMAT_ARGB_8888 = '8888',

        // future: possible dxt, etc...
    };

    enum eFlags
    {
        FLAG_NONE,
    };

#   pragma pack ( push, 1 )

    struct RawHeader
    {
        FOURCC m_HeaderMagic;   // special magic number
        FOURCC m_Format;        // format of bits
        WORD   m_Flags;         // any special flags (for future expansion)
        WORD   m_SurfaceCount;  // total surfaces stored (for mip maps), always >= 1
        WORD   m_Width;         // width of surface 0
        WORD   m_Height;        // height of surface 0
    //  BYTE   m_Bits[];        // raw image data (format-dependent)

        RawHeader( void )
        {
            ::ZeroObject( *this );
        }
    };

#   pragma pack ( pop )

// Setup.

    static RapiImageReader* CreateReader( const char* name, const_mem_ptr mem, bool wantsMips )
    {
        RapiRawReader* reader = NULL;

        if ( mem.size >= sizeof( RawHeader ) )
        {
            const RawHeader* rawHeader = (const RawHeader*)mem.mem;
            if ( rawHeader->m_HeaderMagic == HEADER_MAGIC )
            {
                reader = new RapiRawReader( mem );
                if ( !reader->Init( name, wantsMips ) )
                {
                    Delete ( reader );
                }
            }
        }

        return ( reader );
    }

    RapiRawReader( const_mem_ptr mem )
        : m_Mem( mem )
    {
        m_Header  = (const RawHeader*)m_Mem.mem;
        m_Width   = m_Header->m_Width;
        m_Height  = m_Header->m_Height;
        m_Surface = 0;

        m_Mem.advance( sizeof( RawHeader ) );
        m_Iter = (const DWORD*)m_Mem.mem;
    }

    virtual ~RapiRawReader( void )
    {
        // this space intentionally left blank...
    }

    bool Init( const char* name, bool wantsMips )
    {
        // check vitals
        if (   (m_Header->m_Format != FORMAT_ARGB_8888)     // this is all we support at the moment
            || (m_Header->m_Flags  != FLAG_NONE) )          // ditto
        {
            gperrorf(( "Unsupported RAW format, file = '%s'\n", name ));
            return ( false );
        }

        // warn about mips
        bool ok = true;
        if ( wantsMips )
        {
            if ( m_Header->m_SurfaceCount <= 1 )
            {
                gperrorf(( "Texture wants mipmaps but RAW file not built with any, file = '%s'\n", name ));
                ok = false;
            }

            if ( !::IsPower2( m_Width ) || !::IsPower2( m_Height ) )
            {
                gperrorf(( "Bitmap wants to be a texture but it is not a power of 2, file = '%s'\n", name ));
                ok = false;
            }
        }

        // done
        return ( ok );
    }

// Query.

    virtual int GetWidth( void ) const
    {
        return ( m_Width );
    }

    virtual int GetHeight( void ) const
    {
        return ( m_Height );
    }

    virtual bool GetNextSurface( void* out, ePixelFormat format, int stride, bool flip, bool allowDither, int dstWidth = -1, int dstHeight = -1 )
    {
        // copy
        if ( out != NULL )
        {
            CopyPixels( out, format, dstWidth > 0 ? dstWidth : m_Width, dstHeight > 0 ? dstHeight : m_Height, stride,
                        m_Iter, PIXEL_ARGB_8888, m_Width, m_Height, m_Width * 4,
                        flip, allowDither );
        }

        // advance surface iter
        m_Iter += m_Width * m_Height;

        // adjust for next
        m_Width /= 2;
        m_Height /= 2;
        ++m_Surface;

        // more?
        return ( m_Surface < m_Header->m_SurfaceCount );
    }

private:
    const RawHeader* m_Header;
    int              m_Width;
    int              m_Height;
    const_mem_iter   m_Mem;
    const DWORD*     m_Iter;
    int              m_Surface;

    SET_NO_COPYING( RapiRawReader );
};

