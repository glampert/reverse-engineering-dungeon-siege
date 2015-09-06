//////////////////////////////////////////////////////////////////////////////
// type Goid declaration

// Handle definition.

    COMPILER_ASSERT( sizeof( Goid ) == sizeof( DWORD ) );

    FUBI_REPLACE_NAME( "Goid_", Goid );
    FUBI_EXPORT_POINTER_CLASS( Goid_ );

    // to int
    FUBI_EXPORT inline DWORD MakeInt( Goid g )  {  return ( (DWORD)g );  }

    // to goid
    FUBI_EXPORT Goid MakeGoid( DWORD intGoid );
    FUBI_DOC       ( MakeGoid,      "intGoid",
                    "Returns a Goid based on its integer version." );

// Helpers.

    // $ these are the Goid classes used to select the database. note that the
    //   top two bits of every class of Goid must always be reserved for the
    //   selector.

    const int GO_CLASS_GLOBAL    = 0;           // global go (normal) - must be replicated across all machines (marshalled by the server)
    const int GO_CLASS_LOCAL     = 1;           // local-only go for waypoints and other client-only gizmos
    const int GO_CLASS_CLONE_SRC = 2;           // clone source go (ammo)
    const int GO_CLASS_CONSTANT  = 3;           // special constant

    // $ note that the funky union code in the following GoidBits structs is
    //   meant for fast indexing - getting the m_Index as a WORD (via ax) is
    //   much faster than as a bitfield (which involves shifts and ors in eax).
    //   so long as MAX_BITS_INDEX == 16 this will work.

#   pragma pack ( push, 1 )
    struct GlobalGoidBits
    {
        enum
        {
            // sizes to use for bit fields
            MAX_BITS_MAJOR_INDEX = 16,          // LSB (  0 - 15 )
            MAX_BITS_MINOR_INDEX =  8,          //     ( 16 - 23 )
            MAX_BITS_MAGIC       =  6,          //     ( 24 - 29 )
            MAX_BITS_CLASS       =  2,          // MSB ( 30 - 31 )

            // sizes to compare against for asserting dereferences
            MAX_MAJOR_INDEX  = (1 << MAX_BITS_MAJOR_INDEX) - 1,
            MAX_MINOR_INDEX  = (1 << MAX_BITS_MINOR_INDEX) - 1,
            MAX_MAGIC        = (1 << MAX_BITS_MAGIC      ) - 1,
            MAX_CLASS        = (1 << MAX_BITS_CLASS      ) - 1,
        };

        union
        {
            COMPILER_ASSERT(   (MAX_BITS_MAJOR_INDEX == sizeof( WORD ))
                            && ((  MAX_BITS_MAJOR_INDEX
                                 + MAX_BITS_MINOR_INDEX
                                 + MAX_BITS_MAGIC
                                 + MAX_BITS_CLASS) == 32) );

            union
            {
                struct
                {
                    WORD m_MajorIndex;                              // major index into resource bucket set
                    WORD m_Dummy1;
                };

                struct
                {
                    unsigned m_Dummy2     : MAX_BITS_MAJOR_INDEX;
                    unsigned m_MinorIndex : MAX_BITS_MINOR_INDEX;   // minor index into resource bucket
                    unsigned m_Magic      : MAX_BITS_MAGIC;         // magic number to check against structure
                    unsigned m_Class      : MAX_BITS_CLASS;         // the class of this Go
                };
            };
            DWORD m_Handle;
        };

        GlobalGoidBits( void )                      {  m_Handle = 0;  }
        GlobalGoidBits( Goid goid )                 {  m_Handle = MakeInt( goid );  }
        explicit GlobalGoidBits( DWORD handle )     {  m_Handle = handle;  }

        GlobalGoidBits( UINT majorIndex, UINT minorIndex, UINT magic )
        {
            gpassert( majorIndex <= MAX_MAJOR_INDEX );
            gpassert( minorIndex <= MAX_MINOR_INDEX );
            gpassert( (magic != 0) && (magic <= MAX_MAGIC) );

            m_MajorIndex = (WORD)majorIndex;
            m_MinorIndex = minorIndex;
            m_Magic      = magic;
            m_Class      = GO_CLASS_GLOBAL;
        }

        operator Goid ( void ) const                {  return ( (Goid)m_Handle );  }
    };
#   pragma pack ( pop )

#   pragma pack ( push, 1 )
    struct LocalGoidBits
    {
        enum
        {
            // sizes to use for bit fields
            MAX_BITS_INDEX = 16,                // LSB (  0 - 15 )
            MAX_BITS_MAGIC = 14,                //     ( 16 - 29 )
            MAX_BITS_CLASS =  2,                // MSB ( 30 - 31 )

            // sizes to compare against for asserting dereferences
            MAX_INDEX  = (1 << MAX_BITS_INDEX) - 1,
            MAX_MAGIC  = (1 << MAX_BITS_MAGIC) - 1,
            MAX_CLASS  = (1 << MAX_BITS_CLASS) - 1,
        };

        union
        {
            COMPILER_ASSERT(   (MAX_BITS_INDEX == sizeof( WORD ))
                            && ((  MAX_BITS_INDEX
                                 + MAX_BITS_MAGIC
                                 + MAX_BITS_CLASS) == 32) );

            union
            {
                struct
                {
                    WORD m_Index;                               // index into resource vector
                    WORD m_Dummy1;
                };

                struct
                {
                    unsigned m_Dummy2 : MAX_BITS_INDEX;
                    unsigned m_Magic  : MAX_BITS_MAGIC;         // magic number to check against structure
                    unsigned m_Class  : MAX_BITS_CLASS;         // the class of this Go
                };
            };
            DWORD m_Handle;
        };

        LocalGoidBits( void )                       {  m_Handle = 0;  }
        LocalGoidBits( Goid goid )                  {  m_Handle = MakeInt( goid );  }
        explicit LocalGoidBits( DWORD handle )      {  m_Handle = handle;  }

        LocalGoidBits( UINT index, UINT magic, bool local )
        {
            gpassert( index <= MAX_INDEX );
            gpassert( (magic != 0) && (magic <= MAX_MAGIC) );

            m_Index = (WORD)index;
            m_Magic = magic;
            m_Class = local ? GO_CLASS_LOCAL : GO_CLASS_CLONE_SRC;
        }

        operator Goid ( void ) const                {  return ( (Goid)m_Handle );  }
    };
#   pragma pack ( pop )

#   pragma pack ( push, 1 )
    struct ConstantGoidBits
    {
        enum
        {
            // sizes to use for bit fields
            MAX_BITS_CONSTANT = 30,             // LSB (  0 - 29 )
            MAX_BITS_CLASS    =  2,             // MSB ( 30 - 31 )

            // sizes to compare against for asserting dereferences
            MAX_CONSTANT = (1 << MAX_BITS_CONSTANT) - 1,
            MAX_CLASS    = (1 << MAX_BITS_CLASS   ) - 1,
        };

        union
        {
            COMPILER_ASSERT( (MAX_BITS_CONSTANT + MAX_BITS_CLASS) == 32 );

            struct
            {
                unsigned m_Constant : MAX_BITS_CONSTANT;        // arbitrary constant value, whatever you like
                unsigned m_Class    : MAX_BITS_CLASS;           // the class of this Go
            };
            DWORD m_Handle;
        };

        ConstantGoidBits( void )                    {  m_Handle = 0;  }
        ConstantGoidBits( Goid goid )               {  m_Handle = MakeInt( goid );  }
        explicit ConstantGoidBits( DWORD handle )   {  m_Handle = handle;  }

        ConstantGoidBits( UINT constant )
        {
            gpassert( constant <= MAX_CONSTANT );

            m_Constant = constant;
            m_Class    = GO_CLASS_CONSTANT;
        }

        operator Goid ( void ) const                {  return ( (Goid)m_Handle );  }
    };
#   pragma pack ( pop )

// Constructors.

    // to global
    inline Goid MakeGlobalGoid( DWORD d )                                           {  return ( GlobalGoidBits( d ) );  }
    inline Goid MakeGlobalGoid( UINT majorIndex, UINT minorIndex, UINT magic )      {  return ( GlobalGoidBits( majorIndex, minorIndex, magic ) );  }

    // to local
    inline Goid MakeLocalGoid( DWORD d )                                            {  return ( LocalGoidBits( d ) );  }
    inline Goid MakeLocalGoid( UINT index, UINT magic )                             {  return ( LocalGoidBits( index, magic, true ) );  }

    // to clone source
    inline Goid MakeCloneSrcGoid( DWORD d )                                         {  return ( LocalGoidBits( d ) );  }
    inline Goid MakeCloneSrcGoid( UINT index, UINT magic )                          {  return ( LocalGoidBits( index, magic, false ) );  }

    // to constant
    inline Goid MakeConstantGoid( UINT constant )                                   {  return ( ConstantGoidBits( constant ) );  }

    // to general
    inline Goid MakeGoid( DWORD d )                                                 {  return ( (Goid)d );  }

// Constants.

    extern const Goid GOID_INVALID;
    extern const Goid GOID_ANY;
    extern const Goid GOID_NONE;

// Support.

    bool IsValid  ( Goid g, bool testExists = true );
    bool IsValidMp( Goid g );
    Go*  GetGo    ( Goid g );
    Scid GetScid  ( Goid g );

    inline DWORD GetGoidClass     ( Goid g )    {  return ( GlobalGoidBits( g ).m_Class );  }

    inline bool  IsGlobalGoid     ( Goid g )    {  return ( (g != GOID_INVALID) && (GetGoidClass( g ) == GO_CLASS_GLOBAL) );  }
    inline bool  IsLocalGoid      ( Goid g )    {  return ( GetGoidClass( g ) == GO_CLASS_LOCAL     );  }
    inline bool  IsCloneSourceGoid( Goid g )    {  return ( GetGoidClass( g ) == GO_CLASS_CLONE_SRC );  }

    const char* GoidClassToString( DWORD gc );
    inline const char* GoidClassToString( Goid g )      {  return ( GoidClassToString( GetGoidClass( g ) ) );  }

#   if !GP_RETAIL
    const char* GoidToDebugString( Goid g );
#   endif // !GP_RETAIL

// Structure.

    struct Goid_
    {
    private:

    // Methods.

        FUBI_EXPORT bool IsValid( void ) const  {  return ( ::IsValid( this ) );  }
        FUBI_MEMBER_DOC( IsValid, "", "Returns true if this refers to a valid game object." );

        FUBI_EXPORT bool IsValidMp( void ) const  {  return ( ::IsValidMp( this ) );  }
        FUBI_MEMBER_DOC( IsValidMp, "", "Returns true if this refers to a valid game object that is safe for MP transfer." );

        FUBI_EXPORT Go*  GetGo( void ) const  {  return ( ::GetGo( this ) );  }
        FUBI_MEMBER_DOC( GetGo, "", "Returns a pointer to the Go that this Goid refers to, or NULL if invalid." );

        FUBI_EXPORT Scid GetScid( void ) const  {  return ( ::GetScid( this ) );  }
        FUBI_MEMBER_DOC( GetScid, "", "Returns the corresponding Scid, or Scid.InvalidScid if invalid." );

    // Constants.

        FUBI_EXPORT static Goid GetInvalidGoid( void )  {  return ( GOID_INVALID );  }
        FUBI_MEMBER_DOC(        GetInvalidGoid, "", "Returns the invalid Goid constant." );

        FUBI_EXPORT static Goid GetAnyGoid( void )  {  return ( GOID_ANY );  }
        FUBI_MEMBER_DOC(        GetAnyGoid, "", "Returns the 'any' Goid constant." );

        FUBI_EXPORT static Goid GetNoneGoid( void )  {  return ( GOID_NONE );  }
        FUBI_MEMBER_DOC(        GetNoneGoid, "", "Returns the 'none' Goid constant." );

    // cannot actually instantiate one of these... it's for use as a handle only

        Goid_( void );
        Goid_( const Goid_& );
       ~Goid_( void );
    };

// Collections.

    struct GopColl;
    struct GoidColl;

    // need this everywhere
    typedef stdx::fast_vector <BYTE> Buffer;

    // collection of Go pointers
    struct GopColl : public stdx::fast_vector <Go*>
    {
        void Translate( GoidColl& out ) const           { Translate( out, GOID_INVALID ); }
        void Translate( GoidColl& out, Goid ignore ) const; // translation will not include ignored id

#if !GP_RETAIL
        bool AssertValid()
        {
            bool valid = true;
            for( GopColl::iterator i = this->begin(); i != this->end(); ++i )
            {
                if( (*i) == NULL )
                {
                    gpassertm( 0, "Invalid GopColl" );
                    valid = false;
                }
            }
            return valid;
        }
#endif

    private:
        FUBI_EXPORT void Add  ( Go* go )                {  push_back( go ); }
        FUBI_EXPORT int  Size ( void ) const            {  return ( scast <int> ( size() ) );  }
        FUBI_EXPORT bool Empty( void ) const            {  return ( empty() );  }
        FUBI_EXPORT Go*  Get  ( int index ) const       {  gpassert( (index >= 0) && (index < Size()) );  return ( (*this)[index] );  }
        FUBI_EXPORT void Set  ( int index, Go* g )      {  gpassert( (index >= 0) && (index < Size()) );  (*this)[index] = g;  }
        FUBI_EXPORT void Clear( void )                  {  clear(); }
    };

    // collection of Goids
    struct GoidColl : public stdx::fast_vector <Goid>
    {
        // returns # translated
        int Translate( GopColl& out ) const;

    private:
        FUBI_EXPORT void Add  ( Goid goid )             {  push_back( goid ); }
        FUBI_EXPORT int  Size ( void ) const            {  return ( scast <int> ( size() ) );  }
        FUBI_EXPORT bool Empty( void ) const            {  return ( empty() );  }
        FUBI_EXPORT Goid Get  ( int index ) const       {  gpassert( (index >= 0) && (index < Size()) );  return ( (*this)[index] );  }
        FUBI_EXPORT void Set  ( int index, Goid g )     {  gpassert( (index >= 0) && (index < Size()) );  (*this)[index] = g;  }
        FUBI_EXPORT void Clear( void )                  {  clear(); }
    };

    typedef std::set  <Go*>  GopSet;
    typedef std::list <Go*>  GopList;
    typedef std::set  <Goid> GoidSet;
    typedef std::_Bvector    BitVector;

//////////////////////////////////////////////////////////////////////////////
// type Scid declaration

// Handle definition.

    COMPILER_ASSERT( sizeof( Scid ) == sizeof( DWORD ) );

    FUBI_REPLACE_NAME( "Scid_", Scid );
    FUBI_EXPORT_POINTER_CLASS( Scid_ );

    // to int
    FUBI_EXPORT inline DWORD MakeInt( Scid s )      {  return ( (DWORD)s );  }

    // to scid
    FUBI_EXPORT Scid MakeScid( DWORD intScid );
    FUBI_DOC       ( MakeScid,      "intScid",
                    "Returns a Scid based on its integer version." );

// Constructors.

    // to scid
    inline Scid MakeScid( DWORD d ) {  return ( (Scid)d );  }

// Constants.

    extern const Scid SCID_INVALID;     // bad scid
    extern const Scid SCID_SPAWNED;     // no scid - this go was spawned directly from template

// Inline support.

           bool IsValid   ( Scid s, bool testExists = true );
    inline bool IsInstance( Scid s, bool testExists = true )    {  return ( (s != SCID_SPAWNED) && IsValid( s, testExists ) );  }

    Goid GetGoid( Scid s );
    Go*  GetGo  ( Scid s );

// Structure.

    struct Scid_
    {
    private:

    // Methods.

        FUBI_EXPORT bool IsValid( void ) const  {  return ( ::IsValid( this ) );  }
        FUBI_MEMBER_DOC( IsValid, "", "Returns true if this refers to a valid piece of static content." );

        FUBI_EXPORT bool IsInstance( void ) const  {  return ( ::IsInstance( this ) );  }
        FUBI_MEMBER_DOC( IsInstance, "", "Returns true if this refers to a valid, non-spawned piece of static content." );

        FUBI_EXPORT Goid GetGoid( void ) const  {  return ( ::GetGoid( this ) );  }
        FUBI_MEMBER_DOC( GetGoid, "", "Returns the corresponding Goid, or Goid.InvalidGoid if invalid." );

        FUBI_EXPORT Go*  GetGo( void ) const  {  return ( ::GetGo( this ) );  }
        FUBI_MEMBER_DOC( GetGo, "", "Returns a pointer to the Go that this Scid refers to, or NULL if invalid." );

    // Constants.

        FUBI_EXPORT static Scid GetInvalidScid( void )  {  return ( SCID_INVALID );  }
        FUBI_MEMBER_DOC(        GetInvalidScid, "", "Returns the invalid Scid constant." );

        FUBI_EXPORT static Scid GetSpawnedScid( void )  {  return ( SCID_SPAWNED );  }
        FUBI_MEMBER_DOC(        GetSpawnedScid, "", "Returns the 'spawned' Scid constant." );

    // cannot actually instantiate one of these... it's for use as a handle only

        Scid_( void );
        Scid_( const Scid_& );
       ~Scid_( void );
    };

//////////////////////////////////////////////////////////////////////////////
