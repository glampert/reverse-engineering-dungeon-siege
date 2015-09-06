// string standard header

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _GPSTRING_STD_
#define _GPSTRING_STD_
#include <istream>
#include <xmemory>

#ifdef  _MSC_VER
#pragma pack(push,4)
#endif  /* _MSC_VER */
 #include <xutility>

_STD_BEGIN

#define GPSTRING_TRACKING    0		// $ define to 1 to track gpstrings and allow dumping
#define GPSTRING_LOG_DELETES 0		// $ define to 1 to track deletions of gpstrings (requires GPSTRING_TRACKING)

#if GPSTRING_LOG_DELETES
#include <list>
#endif // GPSTRING_LOG_DELETES

#pragma pack ( push, 1 )
struct StringHeader
{
	size_t         m_Length;
	size_t         m_Reserve;
	unsigned short m_RefCount;
};
#pragma pack ( pop )

#if GPSTRING_TRACKING

struct TrackingStringHeaderBase
{
#	if GPSTRING_LOG_DELETES

	template <typename T>
	struct OwnedList : std::list <T>
	{
	   ~OwnedList( void )
		{
			clear();
		}

		void clear( void )
		{
			for ( iterator i = begin() ; i != end() ; ++i )
			{
				delete [] ( *i );
			}
			std::list <T>::clear();
		}
	};

	typedef OwnedList <char   *> StringAColl;
	typedef OwnedList <wchar_t*> StringWColl;

	static StringAColl& GetRetiredStringsA( void );
	static StringWColl& GetRetiredStringsW( void );

	static void RetireString( const char   * str );
	static void RetireString( const wchar_t* str );

#	endif // GPSTRING_LOG_DELETES

	static void Lock  ( void );
	static void Unlock( void );
};

template <typename T, typename CHAR>
struct TrackingStringHeader : T, TrackingStringHeaderBase
{
	typedef TrackingStringHeader <T, CHAR> ThisType;

	TrackingStringHeader( void )				{  m_Next = m_Prev = this;  }

	const CHAR* c_str( void ) const				{  return ( (CHAR*)(this + 1) );  }

	static ThisType& GetRoot( void )
	{
		static ThisType s_Root;
		return ( s_Root );
	}

	void Link( void )
	{
		Lock();
		m_Next = GetRoot().m_Next;
		m_Prev = &GetRoot();
		m_Next->m_Prev = this;
		m_Prev->m_Next = this;
		Unlock();
	}

	void Unlink( bool ansi )
	{
		Lock();
		m_Next->m_Prev = m_Prev;
		m_Prev->m_Next = m_Next;
		Unlock();

#		if GPSTRING_LOG_DELETES
		if ( m_RefCount == 1 )
		{
			RetireString( c_str() );
		}
#		else // GPSTRING_LOG_DELETES
		UNREFERENCED_PARAMETER( ansi );
#		endif // GPSTRING_LOG_DELETES
	}

	ThisType* m_Next;
	ThisType* m_Prev;
};

#endif // GPSTRING_TRACKING


		// TEMPLATE CLASS gp_basic_string
template<class _E,
	class _Tr = char_traits<_E>,
	class _A = allocator<_E> >
	class gp_basic_string : private _A {
public:

	#if GPSTRING_TRACKING
	typedef TrackingStringHeader <StringHeader, _E> Header;
	#else // GPSTRING_TRACKING
	typedef StringHeader Header;
	#endif // GPSTRING_TRACKING

	typedef gp_basic_string<_E, _Tr, _A> _Myt;
	typedef _A::size_type size_type;
	typedef _A::difference_type difference_type;
	typedef _A::pointer pointer;
	typedef _A::const_pointer const_pointer;
	typedef _A::reference reference;
	typedef _A::const_reference const_reference;
	typedef _A::value_type value_type;
	typedef _A::pointer iterator;
	typedef _A::const_pointer const_iterator;
	typedef reverse_iterator<const_iterator, value_type,
		const_reference, const_pointer, difference_type>
			const_reverse_iterator;
	typedef reverse_iterator<iterator, value_type,
		reference, pointer, difference_type>
			reverse_iterator;
	explicit gp_basic_string(const _A& _Al = _A())
		: _A(_Al) {_Ptr = 0; }
	gp_basic_string(const _Myt& _X)
		: _A(_X)
		{_Ptr = 0; assign(_X, 0, npos); }
	gp_basic_string(const _Myt& _X, size_type _P, size_type _M,
		const _A& _Al = _A())
		: _A(_Al) {_Ptr = 0; assign(_X, _P, _M); }
	gp_basic_string(const _E *_S, size_type _N,
		const _A& _Al = _A())
		: _A(_Al) {_Ptr = 0; assign(_S, _N); }
	gp_basic_string(const _E *_S, const _A& _Al = _A())
		: _A(_Al) {_Ptr = 0; assign(_S); }
	gp_basic_string(size_type _N, _E _C, const _A& _Al = _A())
		: _A(_Al) {_Ptr = 0; assign(_N, _C); }
	typedef const_iterator _It;
	gp_basic_string(_It _F, _It _L, const _A& _Al = _A())
		: _A(_Al) {_Ptr = 0; assign(_F, _L); }
	~gp_basic_string()
		{_Tidy(); }
	typedef _Tr traits_type;
	typedef _A allocator_type;
	enum _Mref {_FROZEN = 65535};
	enum { HEADER_SIZE_ELEMENTS = (sizeof( Header ) / sizeof( _E )) + ((sizeof( Header ) % sizeof( _E )) ? 1 : 0) };
	static const size_type npos;
	_Myt& operator=(const _Myt& _X)
		{return (assign(_X)); }
	_Myt& operator=(const _E *_S)
		{return (assign(_S)); }
	_Myt& operator=(_E _C)
		{return (assign(1, _C)); }
	_Myt& operator+=(const _Myt& _X)
		{return (append(_X)); }
	_Myt& operator+=(const _E *_S)
		{return (append(_S)); }
	_Myt& operator+=(_E _C)
		{return (append(1, _C)); }
	_Myt& append(const _Myt& _X)
		{return (append(_X, 0, npos)); }
	_Myt& append(const _Myt& _X, size_type _P, size_type _M)
		{gpassert(!(_X.size() < _P));
		size_type _N = _X.size() - _P;
		if (_N < _M)
			_M = _N;
		if (0 < _M && _Grow(_N = length() + _M))
			{_Tr::copy(_Ptr + length(), &_X.c_str()[_P], _M);
			_Eos(_N); }
		return (*this); }
	_Myt& append(const _E *_S, size_type _M)
		{size_type _N;
		if (0 < _M && _Grow(_N = length() + _M))
			{_Tr::copy(_Ptr + length(), _S, _M);
			_Eos(_N); }
		return (*this); }
	_Myt& append(const _E *_S)
		{return (append(_S, _Tr::length(_S))); }
	_Myt& append(size_type _M, _E _C)
		{size_type _N;
		if (0 < _M && _Grow(_N = length() + _M))
			{_Tr::assign(_Ptr + length(), _M, _C);
			_Eos(_N); }
		return (*this); }
	_Myt& append(_It _F, _It _L)
		{return (replace(end_split(), end_split(), _F, _L)); }
	_Myt& assign(const _Myt& _X)
		{return (assign(_X, 0, npos)); }

	_Myt& assign( const _Myt& _X, size_type _P, size_type _M )
	{
		gpassert( !(_X.size() < _P) );

		size_type _N = _X.size() - _P;
		if ( _M < _N )
		{
			_N = _M;
		}

		if ( this == &_X )
		{
			erase( _P + _N );
			erase( 0, _P );
		}
		else if (
			   (_N > 0)
			&& (_N == _X.size())
			&& (_Refcnt(_X.c_str()) < _FROZEN - 1)
			&& (get_allocator() == _X.get_allocator()) )
		{
			_Tidy();
			_Ptr = _X._Ptr;
			++_Refcnt(_Ptr);
		}
		else if ( _Grow( _N, true ) )
		{
			_Tr::copy(_Ptr, &_X.c_str()[_P], _N);
			_Eos(_N);
		}

		return ( *this );
	}

	_Myt& assign(const _E *_S, size_type _N)
		{if (_Grow(_N, true))
			{_Tr::copy(_Ptr, _S, _N);
			_Eos(_N); }
		return (*this); }
	_Myt& assign(const _E *_S)
		{return (assign(_S, _Tr::length(_S))); }
	_Myt& assign(size_type _N, _E _C)
		{if (_Grow(_N, true))
			{_Tr::assign(_Ptr, _N, _C);
			_Eos(_N); }
		return (*this); }
	_Myt& assign(_It _F, _It _L)
		{return (replace(begin_split(), end_split(), _F, _L)); }
	_Myt& insert(size_type _P0, const _Myt& _X)
		{return (insert(_P0, _X, 0, npos)); }
	_Myt& insert(size_type _P0, const _Myt& _X, size_type _P,
		size_type _M)
		{gpassert(!(length() < _P0 || _X.size() < _P));
		size_type _N = _X.size() - _P;
		if (_N < _M)
			_M = _N;
		if (0 < _M && _Grow(_N = length() + _M))
			{_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0, length() - _P0);
			_Tr::copy(_Ptr + _P0, &_X.c_str()[_P], _M);
			_Eos(_N); }
		return (*this); }
	_Myt& insert(size_type _P0, const _E *_S, size_type _M)
		{gpassert(!(length() < _P0));
		size_type _N;
		if (0 < _M && _Grow(_N = length() + _M))
			{_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0, length() - _P0);
			_Tr::copy(_Ptr + _P0, _S, _M);
			_Eos(_N); }
		return (*this); }
	_Myt& insert(size_type _P0, const _E *_S)
		{return (insert(_P0, _S, _Tr::length(_S))); }
	_Myt& insert(size_type _P0, size_type _M, _E _C)
		{gpassert(!(length() < _P0));
		size_type _N;
		if (0 < _M && _Grow(_N = length() + _M))
			{_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0, length() - _P0);
			_Tr::assign(_Ptr + _P0, _M, _C);
			_Eos(_N); }
		return (*this); }
	iterator insert(iterator _P, _E _C)
		{size_type _P0 = _Pdif(_P, begin_split());
		insert(_P0, 1, _C);
		return (begin_split() + _P0); }
	void insert(iterator _P, size_type _M, _E _C)
		{size_type _P0 = _Pdif(_P, begin_split());
		insert(_P0, _M, _C); }
	void insert(iterator _P, _It _F, _It _L)
		{replace(_P, _P, _F, _L); }
	_Myt& erase(size_type _P0 = 0, size_type _M = npos)
		{gpassert(!(length() < _P0));
		_Split();
		if (length() - _P0 < _M)
			_M = length() - _P0;
		if (0 < _M)
			{_Tr::move(_Ptr + _P0, _Ptr + _P0 + _M,
				length() - _P0 - _M);
			size_type _N = length() - _M;
			if (_Grow(_N))
				_Eos(_N); }
		return (*this); }
	iterator erase(iterator _P)
		{size_t _M = _Pdif(_P, begin_split());
		erase(_M, 1);
		return (_Psum(_Ptr, _M)); }
	iterator erase(iterator _F, iterator _L)
		{size_t _M = _Pdif(_F, begin_split());
		erase(_M, _Pdif(_L, _F));
		return (_Psum(_Ptr, _M)); }
	_Myt& replace(size_type _P0, size_type _N0, const _Myt& _X)
		{return (replace(_P0, _N0, _X, 0, npos)); }
	_Myt& replace(size_type _P0, size_type _N0, const _Myt& _X,
		size_type _P, size_type _M)
		{gpassert(!(length() < _P0 || _X.size() < _P));
		if (length() - _P0 < _N0)
			_N0 = length() - _P0;
		size_type _N = _X.size() - _P;
		if (_N < _M)
			_M = _N;
		_Split();
		size_type _Nm = length() - _N0 - _P0;
		if (_M < _N0)
			_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0 + _N0, _Nm);
		if ((0 < _M || 0 < _N0) && _Grow(_N = length() + _M - _N0))
			{if (_N0 < _M)
				_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0 + _N0, _Nm);
			_Tr::copy(_Ptr + _P0, &_X.c_str()[_P], _M);
			_Eos(_N); }
		return (*this); }
	_Myt& replace(size_type _P0, size_type _N0, const _E *_S,
		size_type _M)
		{gpassert(!(length() < _P0));
		if (length() - _P0 < _N0)
			_N0 = length() - _P0;
		_Split();
		size_type _Nm = length() - _N0 - _P0;
		if (_M < _N0)
			_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0 + _N0, _Nm);
		size_type _N;
		if ((0 < _M || 0 < _N0) && _Grow(_N = length() + _M - _N0))
			{if (_N0 < _M)
				_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0 + _N0, _Nm);
			_Tr::copy(_Ptr + _P0, _S, _M);
			_Eos(_N); }
		return (*this); }
	_Myt& replace(size_type _P0, size_type _N0, const _E *_S)
		{return (replace(_P0, _N0, _S, _Tr::length(_S))); }
	_Myt& replace(size_type _P0, size_type _N0,
		size_type _M, _E _C)
		{gpassert(!(length() < _P0));
		if (length() - _P0 < _N0)
			_N0 = length() - _P0;
		_Split();
		size_type _Nm = length() - _N0 - _P0;
		if (_M < _N0)
			_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0 + _N0, _Nm);
		size_type _N;
		if ((0 < _M || 0 < _N0) && _Grow(_N = length() + _M - _N0))
			{if (_N0 < _M)
				_Tr::move(_Ptr + _P0 + _M, _Ptr + _P0 + _N0,
					_Nm);
			_Tr::assign(_Ptr + _P0, _M, _C);
			_Eos(_N); }
		return (*this); }
	_Myt& replace(iterator _F, iterator _L, const _Myt& _X)
		{return (replace(
			_Pdif(_F, begin_split()), _Pdif(_L, _F), _X)); }
	_Myt& replace(iterator _F, iterator _L, const _E *_S,
		size_type _M)
		{return (replace(
			_Pdif(_F, begin_split()), _Pdif(_L, _F), _S, _M)); }
	_Myt& replace(iterator _F, iterator _L, const _E *_S)
		{return (replace(
			_Pdif(_F, begin_split()), _Pdif(_L, _F), _S)); }
	_Myt& replace(iterator _F, iterator _L,	size_type _M, _E _C)
		{return (replace(
			_Pdif(_F, begin_split()), _Pdif(_L, _F), _M, _C)); }
	_Myt& replace(iterator _F1, iterator _L1,
		_It _F2, _It _L2)
		{size_type _P0 = _Pdif(_F1, begin_split());
		size_type _M = 0;
		_Distance(_F2, _L2, _M);
		replace(_P0, _Pdif(_L1, _F1), _M, _E(0));
		for (_F1 = begin_split() + _P0; 0 < _M; ++_F1, ++_F2, --_M)
			*_F1 = *_F2;
		return (*this); }
	iterator begin_freeze()
		{_Freeze();
		return (_Ptr); }
	iterator begin_split()
		{_Split();
		return (_Ptr); }
	const_iterator begin() const
		{return (_Ptr); }
	iterator end_freeze()
		{_Freeze();
		return ((iterator)_Psum(_Ptr, length())); }
	iterator end_split()
		{_Split();
		return ((iterator)_Psum(_Ptr, length())); }
	const_iterator end() const
		{return ((const_iterator)_Psum(_Ptr, length())); }
	reverse_iterator rbegin_freeze()
		{return (reverse_iterator(end_freeze())); }
	reverse_iterator rbegin_split()
		{return (reverse_iterator(end_split())); }
	const_reverse_iterator rbegin() const
		{return (const_reverse_iterator(end())); }
	reverse_iterator rend_freeze()
		{return (reverse_iterator(begin_freeze())); }
	reverse_iterator rend_split()
		{return (reverse_iterator(begin_split())); }
	const_reverse_iterator rend() const
		{return (const_reverse_iterator(begin())); }
	reference at(size_type _P0)
		{gpassert(!(length() <= _P0));
		_Freeze();
		return (_Ptr[_P0]); }
	const_reference at(size_type _P0) const
		{gpassert(!(length() <= _P0));
		return (_Ptr[_P0]); }
	reference operator[](size_type _P0)
		{if (length() < _P0 || _Ptr == 0)
			return ((reference)*_Nullstr());
		_Freeze();
		return (_Ptr[_P0]); }
	const_reference operator[](size_type _P0) const
		{if (_Ptr == 0)
			return (*_Nullstr());
		else
			return (_Ptr[_P0]); }
	const _E *c_str() const
		{return (_Ptr == 0 ? _Nullstr() : _Ptr); }
	const _E *data() const
		{return (c_str()); }
	size_type length() const
		{return (_Ptr?(((Header*)_Ptr)[-1].m_Length):0); }
	size_type size() const
		{return (length()); }
	void resize(size_type _N, _E _C)
		{_N <= length() ? erase(_N) : append(_N - length(), _C); }
	void resize(size_type _N)
		{_N <= length() ? erase(_N) : append(_N - length(), _E(0)); }
	size_type capacity() const
		{return (_Ptr?(((Header*)_Ptr)[-1].m_Reserve):0); }
	void reserve(size_type _N = 0)
		{if (capacity() < _N)
			_Grow(_N); }
	bool empty() const
		{return (length() == 0); }
	size_type copy(_E *_S, size_type _N, size_type _P0 = 0) const
		{gpassert(!(length() < _P0));
		if (length() - _P0 < _N)
			_N = length() - _P0;
		if (0 < _N)
			_Tr::copy(_S, _Ptr + _P0, _N);
		return (_N); }
	void swap(_Myt& _X)
		{if (get_allocator() == _X.get_allocator())
			{std::swap(_Ptr, _X._Ptr); }
		else
			{_Myt _Ts = *this; *this = _X, _X = _Ts; }}
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	size_type find(const _Myt& _X, size_type _P = 0) const
		{return (find(_X.c_str(), _P, _X.size())); }
	size_type find(const _E *_S, size_type _P,
		size_type _N) const
		{if (_N == 0 && _P <= length())
			return (_P);
		size_type _Nm;
		if (_P < length() && _N <= (_Nm = length() - _P))
			{const _E *_U, *_V;
			for (_Nm -= _N - 1, _V = _Ptr + _P;
				(_U = _Tr::find(_V, _Nm, *_S)) != 0;
				_Nm -= _U - _V + 1, _V = _U + 1)
				if (_Tr::compare(_U, _S, _N) == 0)
					return (_U - _Ptr); }
		return (npos); }
	size_type find(const _E *_S, size_type _P = 0) const
		{return (find(_S, _P, _Tr::length(_S))); }
	size_type find(_E _C, size_type _P = 0) const
		{return (find((const _E *)&_C, _P, 1)); }
	size_type rfind(const _Myt& _X, size_type _P = npos) const
		{return (rfind(_X.c_str(), _P, _X.size())); }
	size_type rfind(const _E *_S, size_type _P,
		size_type _N) const
		{if (_N == 0)
			return (_P < length() ? _P : length());
		if (_N <= length())
			for (const _E *_U = _Ptr +
				+ (_P < length() - _N ? _P : length() - _N); ; --_U)
				if (_Tr::eq(*_U, *_S)
					&& _Tr::compare(_U, _S, _N) == 0)
					return (_U - _Ptr);
				else if (_U == _Ptr)
					break;
		return (npos); }
	size_type rfind(const _E *_S, size_type _P = npos) const
		{return (rfind(_S, _P, _Tr::length(_S))); }
	size_type rfind(_E _C, size_type _P = npos) const
		{return (rfind((const _E *)&_C, _P, 1)); }
	size_type find_first_of(const _Myt& _X,
		size_type _P = 0) const
		{return (find_first_of(_X.c_str(), _P, _X.size())); }
	size_type find_first_of(const _E *_S, size_type _P,
		size_type _N) const
		{if (0 < _N && _P < length())
			{const _E *const _V = _Ptr + length();
			for (const _E *_U = _Ptr + _P; _U < _V; ++_U)
				if (_Tr::find(_S, _N, *_U) != 0)
					return (_U - _Ptr); }
		return (npos); }
	size_type find_first_of(const _E *_S, size_type _P = 0) const
		{return (find_first_of(_S, _P, _Tr::length(_S))); }
	size_type find_first_of(_E _C, size_type _P = 0) const
		{return (find((const _E *)&_C, _P, 1)); }
	size_type find_last_of(const _Myt& _X,
		size_type _P = npos) const
		{return (find_last_of(_X.c_str(), _P, _X.size())); }
	size_type find_last_of(const _E *_S, size_type _P,
		size_type _N) const
		{if (0 < _N && 0 < length())
			for (const _E *_U = _Ptr
				+ (_P < length() ? _P : length() - 1); ; --_U)
				if (_Tr::find(_S, _N, *_U) != 0)
					return (_U - _Ptr);
				else if (_U == _Ptr)
					break;
		return (npos); }
	size_type find_last_of(const _E *_S,
		size_type _P = npos) const
		{return (find_last_of(_S, _P, _Tr::length(_S))); }
	size_type find_last_of(_E _C, size_type _P = npos) const
		{return (rfind((const _E *)&_C, _P, 1)); }
	size_type find_first_not_of(const _Myt& _X,
		size_type _P = 0) const
		{return (find_first_not_of(_X.c_str(), _P,
			_X.size())); }
	size_type find_first_not_of(const _E *_S, size_type _P,
		size_type _N) const
		{if (_P < length())
			{const _E *const _V = _Ptr + length();
			for (const _E *_U = _Ptr + _P; _U < _V; ++_U)
				if (_Tr::find(_S, _N, *_U) == 0)
					return (_U - _Ptr); }
		return (npos); }
	size_type find_first_not_of(const _E *_S,
		size_type _P = 0) const
		{return (find_first_not_of(_S, _P, _Tr::length(_S))); }
	size_type find_first_not_of(_E _C, size_type _P = 0) const
		{return (find_first_not_of((const _E *)&_C, _P, 1)); }
	size_type find_last_not_of(const _Myt& _X,
		size_type _P = npos) const
		{return (find_last_not_of(_X.c_str(), _P, _X.size())); }
	size_type find_last_not_of(const _E *_S, size_type _P,
		 size_type _N) const
		{if (0 < length())
			for (const _E *_U = _Ptr
				+ (_P < length() ? _P : length() - 1); ; --_U)
				if (_Tr::find(_S, _N, *_U) == 0)
					return (_U - _Ptr);
				else if (_U == _Ptr)
					break;
		return (npos); }
	size_type find_last_not_of(const _E *_S,
		size_type _P = npos) const
		{return (find_last_not_of(_S, _P, _Tr::length(_S))); }
	size_type find_last_not_of(_E _C, size_type _P = npos) const
		{return (find_last_not_of((const _E *)&_C, _P, 1)); }
	_Myt substr(size_type _P = 0, size_type _M = npos) const
		{return (_Myt(*this, _P, _M)); }
	int compare(const _Myt& _X) const
		{return (compare(0, length(), _X.c_str(), _X.size())); }
	int compare(size_type _P0, size_type _N0,
		const _Myt& _X) const
		{return (compare(_P0, _N0, _X, 0, npos)); }
	int compare(size_type _P0, size_type _N0, const _Myt& _X,
		size_type _P, size_type _M) const
		{gpassert(!(_X.size() < _P));
		if (_X.length() - _P < _M)
			_M = _X.length() - _P;
		return (compare(_P0, _N0, _X.c_str() + _P, _M)); }
	int compare(const _E *_S) const
		{return (compare(0, length(), _S, _Tr::length(_S))); }
	int compare(size_type _P0, size_type _N0, const _E *_S) const
		{return (compare(_P0, _N0, _S, _Tr::length(_S))); }
	int compare(size_type _P0, size_type _N0, const _E *_S,
		size_type _M) const
		{gpassert(!(length() < _P0));
		if (length() - _P0 < _N0)
			_N0 = length() - _P0;
		size_type _Ans = _Tr::compare(_Psum(_Ptr, _P0), _S,
			_N0 < _M ? _N0 : _M);
		return (_Ans != 0 ? _Ans : _N0 < _M ? -1
			: _N0 == _M ? 0 : +1); }
	_A get_allocator() const
		{return (*this); }
private:
	enum  {  _MIN_SIZE = 15  };

	void _Copy( size_type _N )
	{
		// $$ realloc opportunity

		if ( _N < _MIN_SIZE )
		{
			_N = _MIN_SIZE;
		}

		Header* header = (Header*)allocate( _N + HEADER_SIZE_ELEMENTS + 1, 0 );
		header->m_RefCount = 0;
		header->m_Reserve  = _N;

		_E* _S = (_E*)(header + 1);

		size_t oldLen = length();
		if ( oldLen > 0 )
		{
			if ( oldLen > _N )
			{
				oldLen = _N;
			}
			_Tr::copy( _S, _Ptr, oldLen );
		}

		_Tidy();
		_Ptr = _S;
		_Eos( oldLen );

#		if GPSTRING_TRACKING
		header->Link();
#		endif // GPSTRING_TRACKING
	}

	void _Eos( size_type _N )
	{
		((Header*)_Ptr)[ -1 ].m_Length = _N;
		_Tr::assign( _Ptr[ _N ], _E( 0 ) );
	}

	void _Freeze()
	{
		if ( _Ptr )
		{
			Header* header = (Header*)_Ptr - 1;
			if ( (header->m_RefCount != 0) && (header->m_RefCount != _FROZEN) )
			{
				_Grow( header->m_Length );
			}
		}
		if ( _Ptr )
		{
			_Refcnt( _Ptr ) = _FROZEN;
		}
	}

	bool _Grow( size_type _N, bool _Trim = false )
	{
		if ( _Ptr )
		{
			Header* header = (Header*)_Ptr - 1;
			if ( (header->m_RefCount != 0) && (header->m_RefCount != _FROZEN) )
			{
				if ( _N == 0 )
				{
					--header->m_RefCount;
					_Ptr = 0;
					return ( false );
				}
				else
				{
					_Copy( _N );
					return ( true );
				}
			}
		}

		if ( _N == 0 )
		{
			if ( _Trim )
			{
				_Tidy();
			}
			else if ( _Ptr )
			{
				_Eos( 0 );
			}
			return ( false );
		}

		size_type res = capacity();
		if ( _Trim && ((res > _MIN_SIZE) || (res < _N)) )
		{
			// $$ realloc opportunity
			_Tidy();
			_Copy( _N );
		}
		else if ( !_Trim && (res < _N) )
		{
			_Copy( _N );
		}

		return ( true );
	}

	static const _E * __cdecl _Nullstr()
		{static const _E _C = _E(0);
		return (&_C); }
	static size_type _Pdif(const_pointer _P2, const_pointer _P1)
		{return (_P2 == 0 ? 0 : _P2 - _P1); }
	static const_pointer _Psum(const_pointer _P, size_type _N)
		{return (_P == 0 ? 0 : _P + _N); }
	static pointer _Psum(pointer _P, size_type _N)
		{return (_P == 0 ? 0 : _P + _N); }
	unsigned short& _Refcnt(const _E *_U)
		{return (((Header*)_U)[-1].m_RefCount); }

protected:
	const Header& get_header( void ) const
	{
		gpassert( _Ptr != NULL );
		return ( ((const Header*)_Ptr)[ -1 ] );
	}

	Header& get_header( void )
	{
		gpassert( _Ptr != NULL );
		return ( ((Header*)_Ptr)[ -1 ] );
	}

	int get_ref_count( void ) const
	{
		if ( _Ptr == NULL )
		{
			return ( 0 );
		}
		unsigned short refCount = ((Header*)_Ptr)[ -1 ].m_RefCount;
		return ( (refCount == _FROZEN) ? 1 : (refCount + 1) );
	}

	void _Split()
	{
		if ( _Ptr )
		{
			Header* header = (Header*)_Ptr - 1;
			if ( (header->m_RefCount != 0) && (header->m_RefCount != _FROZEN) )
			{
				_E *_Temp = _Ptr;
				_Tidy();
				assign( _Temp );
			}
		}
	}

	void _Tidy()
	{
		if ( _Ptr )
		{
			Header* header = (Header*)_Ptr - 1;
			if ( (header->m_RefCount == 0) || (header->m_RefCount == _FROZEN) )
			{
#				if GPSTRING_TRACKING
				header->Unlink( sizeof( _E ) == 1 );
#				endif // GPSTRING_TRACKING

				deallocate( header, header->m_Reserve + HEADER_SIZE_ELEMENTS + 1 );
			}
			else
			{
				--header->m_RefCount;
			}
			_Ptr = 0;
		}
	}

private:
	_E *_Ptr;
	};
template<class _E, class _Tr, class _A>
	const gp_basic_string<_E, _Tr, _A>::size_type
		gp_basic_string<_E, _Tr, _A>::npos = (size_type)-1;

        // gp_basic_string TEMPLATE OPERATORS
template<class _E, class _Tr, class _A> inline
    gp_basic_string<_E, _Tr, _A> __cdecl operator+(
        const gp_basic_string<_E, _Tr, _A>& _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (gp_basic_string<_E, _Tr, _A>(_L) += _R); }
template<class _E, class _Tr, class _A> inline
    gp_basic_string<_E, _Tr, _A> __cdecl operator+(const _E *_L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (gp_basic_string<_E, _Tr, _A>(_L) += _R); }
template<class _E, class _Tr, class _A> inline
    gp_basic_string<_E, _Tr, _A> __cdecl operator+(
        const _E _L, const gp_basic_string<_E, _Tr, _A>& _R)
    {return (gp_basic_string<_E, _Tr, _A>(1, _L) += _R); }
template<class _E, class _Tr, class _A> inline
    gp_basic_string<_E, _Tr, _A> __cdecl operator+(
        const gp_basic_string<_E, _Tr, _A>& _L,
        const _E *_R)
    {return (gp_basic_string<_E, _Tr, _A>(_L) += _R); }
template<class _E, class _Tr, class _A> inline
    gp_basic_string<_E, _Tr, _A> __cdecl operator+(
        const gp_basic_string<_E, _Tr, _A>& _L, const _E _R)
    {return (gp_basic_string<_E, _Tr, _A>(_L) += _R); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator==(const gp_basic_string<_E, _Tr, _A>& _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (_L.compare(_R) == 0); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator==(const _E * _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (_R.compare(_L) == 0); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator==(const gp_basic_string<_E, _Tr, _A>& _L,
        const _E *_R)
    {return (_L.compare(_R) == 0); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator!=(const gp_basic_string<_E, _Tr, _A>& _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (!(_L == _R)); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator!=(const _E *_L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (!(_L == _R)); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator!=(const gp_basic_string<_E, _Tr, _A>& _L,
        const _E *_R)
    {return (!(_L == _R)); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator<(const gp_basic_string<_E, _Tr, _A>& _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (_L.compare(_R) < 0); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator<(const _E * _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (_R.compare(_L) > 0); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator<(const gp_basic_string<_E, _Tr, _A>& _L,
        const _E *_R)
    {return (_L.compare(_R) < 0); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator>(const gp_basic_string<_E, _Tr, _A>& _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (_R < _L); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator>(const _E * _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (_R < _L); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator>(const gp_basic_string<_E, _Tr, _A>& _L,
        const _E *_R)
    {return (_R < _L); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator<=(const gp_basic_string<_E, _Tr, _A>& _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (!(_R < _L)); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator<=(const _E * _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (!(_R < _L)); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator<=(const gp_basic_string<_E, _Tr, _A>& _L,
        const _E *_R)
    {return (!(_R < _L)); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator>=(const gp_basic_string<_E, _Tr, _A>& _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (!(_L < _R)); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator>=(const _E * _L,
        const gp_basic_string<_E, _Tr, _A>& _R)
    {return (!(_L < _R)); }
template<class _E, class _Tr, class _A> inline
    bool __cdecl operator>=(const gp_basic_string<_E, _Tr, _A>& _L,
        const _E *_R)
    {return (!(_L < _R)); }
        // gp_basic_string INSERTERS AND EXTRACTORS
template<class _E, class _Tr, class _A> inline
    basic_istream<_E, _Tr>& __cdecl operator>>(
        basic_istream<_E, _Tr>& _I,
        gp_basic_string<_E, _Tr, _A>& _X)
    {typedef ctype<_E> _Ctype;
    typedef basic_istream<_E, _Tr> _Myis;
    ios_base::iostate _St = ios_base::goodbit;
    bool _Chg = false;
    _X.erase();
    const _Myis::sentry _Ok(_I);
    if (_Ok)
        {const _Ctype& _Fac = _USE(_I.getloc(), _Ctype);
        _TRY_IO_BEGIN
        _A::size_type _N = 0 < _I.width()
                ? _I.width() : _X.max_size();
        _Tr::int_type _C = _I.rdbuf()->sgetc();
        for (; 0 < --_N; _C = _I.rdbuf()->snextc())
            if(_Tr::eq_int_type(_Tr::eof(), _C))
                {_St |= ios_base::eofbit;
                break; }
            else if (_Fac.is(_Ctype::space,
                _Tr::to_char_type(_C)))
                break;
            else
                {_X.append(1, _Tr::to_char_type(_C));
                _Chg = true; }
        _CATCH_IO_(_I); }
    _I.width(0);
    if (!_Chg)
        _St |= ios_base::failbit;
    _I.setstate(_St);
    return (_I); }
template<class _E, class _Tr, class _A> inline
    basic_istream<_E, _Tr>& __cdecl getline(basic_istream<_E, _Tr>& _I,
        gp_basic_string<_E, _Tr, _A>& _X)
    {return (getline(_I, _X, _I.widen('\n'))); }
template<class _E, class _Tr, class _A> inline
    basic_istream<_E, _Tr>& __cdecl getline(basic_istream<_E, _Tr>& _I,
        gp_basic_string<_E, _Tr, _A>& _X, const _E _D)
    {typedef basic_istream<_E, _Tr> _Myis;
    ios_base::iostate _St = ios_base::goodbit;
    bool _Chg = false;
    _X.erase();
    const _Myis::sentry _Ok(_I, true);
    if (_Ok)
        {_TRY_IO_BEGIN
        _Tr::int_type _C = _I.rdbuf()->sgetc();
        for (; ; _C = _I.rdbuf()->snextc())
            if (_Tr::eq_int_type(_Tr::eof(), _C))
                {_St |= ios_base::eofbit;
                break; }
            else if (_Tr::eq((_E)_C, _D))
                {_Chg = true;
                _I.rdbuf()->sbumpc();
                break; }
            else
                _X += _Tr::to_char_type(_C), _Chg = true;
        _CATCH_IO_(_I); }
    if (!_Chg)
        _St |= ios_base::failbit;
    _I.setstate(_St);
    return (_I); }
template<class _E, class _Tr, class _A> inline
    basic_ostream<_E, _Tr>& __cdecl operator<<(
        basic_ostream<_E, _Tr>& _O,
        const gp_basic_string<_E, _Tr, _A>& _X)
    {
    _O << _X.c_str();
    return (_O); }


typedef gp_basic_string<char, char_traits<char>, allocator<char> >
	gp_string;

typedef gp_basic_string<wchar_t, char_traits<wchar_t>,
	allocator<wchar_t> > gp_wstring;

_STD_END
#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* _GPSTRING_STD_ */

/*
 * Copyright (c) 1995 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 */

