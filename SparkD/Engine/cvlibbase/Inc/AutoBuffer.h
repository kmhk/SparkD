
/*!
* \file	AutoBuffer.h
* \ingroup cvlibbase
* \brief   class for automatically new and delete memory.
*/

#pragma once

#include "cvlibmacros.h"

namespace cvlib
{

	template<typename _Tp, int fixed_size = 4096 / sizeof(_Tp) + 8> class CVLIB_DECLSPEC AutoBuffer
	{
	public:
		typedef _Tp value_type;

		AutoBuffer();
		AutoBuffer(size_t _size);
		AutoBuffer(const AutoBuffer<_Tp, fixed_size>& buf);
		AutoBuffer<_Tp, fixed_size>& operator=(const AutoBuffer<_Tp, fixed_size>& buf);
		~AutoBuffer();

		void alloc(size_t _size);
		void dealloc();
		void resize(size_t size);
		size_t size() const;
		operator _Tp* ();
		operator const _Tp* () const;

	protected:
		_Tp* ptr;
		size_t sz;
		_Tp buf[(fixed_size > 0) ? fixed_size : 1];
	};

	//////////////////// Generic ref-counting pointer class for C/C++ objects ////////////////////////

	template<typename _Tp> class CVLIB_DECLSPEC Ptr
	{
	public:
		Ptr();
		Ptr(_Tp* _obj);
		~Ptr();
		Ptr(const Ptr& ptr);
		Ptr& operator = (const Ptr& ptr);
		void addref();
		void release();
		void delete_obj();
		bool empty() const;
		int getrefcount() const;
		_Tp* get();
		const _Tp* get() const;
		_Tp* operator -> ();
		const _Tp* operator -> () const;

		operator _Tp* ();
		operator const _Tp*() const;
	protected:
		_Tp* obj;
		int* refcount;
	};

	template<typename _Tp, int fixed_size> inline AutoBuffer<_Tp, fixed_size>::AutoBuffer()
		: ptr(buf), sz(fixed_size) {}

	template<typename _Tp, int fixed_size> inline AutoBuffer<_Tp, fixed_size>::AutoBuffer(size_t _size)
		: ptr(buf), sz(fixed_size) {
		alloc(_size);
	}

	template<typename _Tp, int fixed_size> inline
		AutoBuffer<_Tp, fixed_size>::AutoBuffer(const AutoBuffer<_Tp, fixed_size>& abuf)
	{
		ptr = buf;
		sz = fixed_size;
		alloc(abuf.size());
		for (size_t i = 0; i < sz; i++)
			ptr[i] = abuf.ptr[i];
	}
	template<typename _Tp, int fixed_size> inline AutoBuffer<_Tp, fixed_size>&
		AutoBuffer<_Tp, fixed_size>::operator=(const AutoBuffer<_Tp, fixed_size>& abuf)
	{
		if (this != &abuf)
		{
			dealloc();
			alloc(abuf.size());
			for (size_t i = 0; i < sz; i++)
				ptr[i] = abuf.ptr[i];
		}
		return *this;
	}

	template<typename _Tp, int fixed_size> inline AutoBuffer<_Tp, fixed_size>::~AutoBuffer()
	{
		dealloc();
	}

	template<typename _Tp, int fixed_size> inline void AutoBuffer<_Tp, fixed_size>::alloc(size_t _size)
	{
		if (_size <= sz)
		{
			sz = _size;
			return;
		}
		dealloc();
		if (_size > fixed_size)
		{
			ptr = new _Tp[_size];
			sz = _size;
		}
	}

	template<typename _Tp, int fixed_size> inline void AutoBuffer<_Tp, fixed_size>::dealloc()
	{
		if (ptr != buf)
		{
			delete[]ptr;
			ptr = buf;
			sz = fixed_size;
		}
	}

	template<typename _Tp, int fixed_size> inline AutoBuffer<_Tp, fixed_size>::operator _Tp* ()
	{
		return ptr;
	}

	template<typename _Tp, int fixed_size> inline AutoBuffer<_Tp, fixed_size>::operator const _Tp* () const
	{
		return ptr;
	}

	template<typename _Tp, int fixed_size> inline size_t AutoBuffer<_Tp, fixed_size>::size() const {
		return sz;
	}

	template<typename _Tp, int fixed_size> inline void AutoBuffer<_Tp, fixed_size>::resize(size_t _size) {
		if (_size <= sz)
		{
			sz = _size;
			return;
		}
		size_t i, prevsize = sz, minsize = MIN(prevsize, _size);
		_Tp* prevptr = ptr;

		ptr = _size > fixed_size ? new _Tp[_size] : buf;
		sz = _size;

		if (ptr != prevptr)
			for (i = 0; i < minsize; i++)
				ptr[i] = prevptr[i];
		for (i = prevsize; i < _size; i++)
			ptr[i] = _Tp();

		if (prevptr != buf)
			delete[]prevptr;
	}

	//////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Ptr ////////////////////////////////////////
	inline int _XADD(int* addr, int delta)
	{
		int tmp = *addr; *addr += delta; return tmp;
	}

	template<typename _Tp> inline Ptr<_Tp>::Ptr() : obj(0), refcount(0) {}
	template<typename _Tp> inline Ptr<_Tp>::Ptr(_Tp* _obj) : obj(_obj)
	{
		if (obj)
		{
			refcount = new int;
			*refcount = 1;
		}
		else
			refcount = 0;
	}

	template<typename _Tp> inline void Ptr<_Tp>::addref()
	{
		if (refcount) _XADD(refcount, 1);
	}

	template<typename _Tp> inline void Ptr<_Tp>::release()
	{
		if (refcount && _XADD(refcount, -1) == 1)
		{
			delete_obj();
			delete refcount;
		}
		refcount = 0;
		obj = 0;
	}

	template<typename _Tp> inline void Ptr<_Tp>::delete_obj()
	{
		if (obj) { delete obj; obj = 0; }
	}

	template<typename _Tp> inline Ptr<_Tp>::~Ptr() { release(); }

	template<typename _Tp> inline Ptr<_Tp>::Ptr(const Ptr<_Tp>& ptr)
	{
		obj = ptr.obj;
		refcount = ptr.refcount;
		addref();
	}

	template<typename _Tp> inline Ptr<_Tp>& Ptr<_Tp>::operator = (const Ptr<_Tp>& ptr)
	{
		int* _refcount = ptr.refcount;
		if (_refcount)
			_XADD(_refcount, 1);
		release();
		obj = ptr.obj;
		refcount = _refcount;
		return *this;
	}

	template<typename _Tp> inline _Tp* Ptr<_Tp>::get() { return obj; }
	template<typename _Tp> inline const _Tp* Ptr<_Tp>::get() const { return obj; }

	template<typename _Tp> inline _Tp* Ptr<_Tp>::operator -> () { return obj; }
	template<typename _Tp> inline const _Tp* Ptr<_Tp>::operator -> () const { return obj; }

	template<typename _Tp> inline Ptr<_Tp>::operator _Tp* () { return obj; }
	template<typename _Tp> inline Ptr<_Tp>::operator const _Tp*() const { return obj; }

	template<typename _Tp> inline bool Ptr<_Tp>::empty() const { return obj == 0; }
	template<typename _Tp> inline int Ptr<_Tp>::getrefcount() const {
		return refcount == 0 ? 0 : *refcount;
	}

}