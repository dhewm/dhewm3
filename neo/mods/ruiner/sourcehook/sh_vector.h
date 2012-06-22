/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 22:23:28 +0530 (Tue, 16 Oct 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/
/* ======== SourceMM ========
* Copyright (C) 2004-2005 Metamod:Source Development Team
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Pavol "PM OnoTo" Marko
* ============================
*/

#ifndef __CVECTOR_H__
#define __CVECTOR_H__

#include <assert.h>

//This file originally from AMX Mod X
namespace SourceHook
{
// Vector
template <class T> class CVector
{
	bool Grow()
	{
		// automatic grow
		size_t newSize = m_Size * 2;
		if (newSize == 0)
			newSize = 8;					// a good init value
		T *newData = new T[newSize];
		if (!newData)
			return false;
		if (m_Data)
		{
			for (size_t i=0; i<m_CurrentUsedSize; i++)
				newData[i] = m_Data[i];
			delete [] m_Data;
		}
		m_Data = newData;
		m_Size = newSize;
		return true;
	}

	bool GrowIfNeeded()
	{
		if (m_CurrentUsedSize >= m_Size)
			return Grow();
		else
			return true;
	}

	bool ChangeSize(size_t size)
	{
		// change size
		if (size == m_Size)
			return true;

		if (!size)
		{
			if (m_Data)
			{
				delete [] m_Data;
				m_Data = NULL;
				m_Size = 0;
			}
			return true;
		}

		T *newData = new T[size];
		if (!newData)
			return false;
		if (m_Data)
		{
			size_t end = (m_CurrentUsedSize < size) ? (m_CurrentUsedSize) : size;
			for (size_t i=0; i<end; i++)
				newData[i] = m_Data[i];
			delete [] m_Data;
		}
		m_Data = newData;
		m_Size = size;
		if (m_CurrentUsedSize > m_Size)
			m_CurrentUsedSize = m_Size;

		return true;
	}

	void FreeMemIfPossible()
	{
		if (!m_Data)
			return;

		if (!m_CurrentUsedSize)
		{
			ChangeSize(0);
			return;
		}

		size_t newSize = m_Size;
		while (m_CurrentUsedSize <= newSize / 2)
			newSize /= 2;

		if (newSize != m_Size)
			ChangeSize(newSize);
	}
protected:
	T *m_Data;
	size_t m_Size;
	size_t m_CurrentUsedSize;
public:
	class iterator
	{
	protected:
		T *m_Ptr;
	public:
		// constructors / destructors
		iterator()
		{
			m_Ptr = NULL;
		}

		iterator(T * ptr)
		{
			m_Ptr = ptr;
		}

		// member functions
		T * base()
		{
			return m_Ptr;
		}

		const T * base() const
		{
			return m_Ptr;
		}

		// operators
		T & operator*()
		{
			return *m_Ptr;
		}

		T * operator->()
		{
			return m_Ptr;
		}

		iterator & operator++()		// preincrement
		{
			++m_Ptr;
			return (*this);
		}

		iterator operator++(int)	// postincrement
		{
			iterator tmp = *this;
			++m_Ptr;
			return tmp;
		}

		iterator & operator--()		// predecrement
		{
			--m_Ptr;
			return (*this);
		}

		iterator operator--(int)	// postdecrememnt
		{
			iterator tmp = *this;
			--m_Ptr;
			return tmp;
		}

		bool operator==(T * right) const
		{
			return (m_Ptr == right);
		}

		bool operator==(const iterator & right) const
		{
			return (m_Ptr == right.m_Ptr);
		}

		bool operator!=(T * right) const
		{
			return (m_Ptr != right);
		}

		bool operator!=(const iterator & right) const
		{
			return (m_Ptr != right.m_Ptr);
		}

		iterator & operator+=(size_t offset)
		{
			m_Ptr += offset;
			return (*this);
		}

		iterator & operator-=(size_t offset)
		{
			m_Ptr -= offset;
			return (*this);
		}

		iterator operator+(size_t offset) const
		{
			iterator tmp(*this);
			tmp.m_Ptr += offset;
			return tmp;
		}

		iterator operator-(size_t offset) const
		{
			iterator tmp(*this);
			tmp.m_Ptr -= offset;
			return tmp;
		}

		T & operator[](size_t offset)
		{
			return (*(*this + offset));
		}

		const T & operator[](size_t offset) const
		{
			return (*(*this + offset));
		}

		bool operator<(const iterator & right) const
		{
			return m_Ptr < right.m_Ptr;
		}

		bool operator>(const iterator & right) const
		{
			return m_Ptr > right.m_Ptr;
		}

		bool operator<=(const iterator & right) const
		{
			return m_Ptr <= right.m_Ptr;
		}

		bool operator>=(const iterator & right) const
		{
			return m_Ptr >= right.m_Ptr;
		}

		size_t operator-(const iterator & right) const
		{
			return m_Ptr - right.m_Ptr;
		}
	};

	// constructors / destructors
	CVector<T>()
	{
		m_Size = 0;
		m_CurrentUsedSize = 0;
		m_Data = NULL;
	}

	CVector<T>(const CVector<T> & other)
	{
		// copy data
		m_Data = new T [other.m_CurrentUsedSize];
		m_Size = other.m_CurrentUsedSize;
		m_CurrentUsedSize = other.m_CurrentUsedSize;
		for (size_t i=0; i<other.m_CurrentUsedSize; i++)
			m_Data[i] = other.m_Data[i];
	}

	~CVector<T>()
	{
		clear();
	}

	// interface
	size_t size() const
	{
		return m_CurrentUsedSize;
	}

	size_t capacity() const
	{
		return m_Size;
	}

	iterator begin() const
	{
		return iterator(m_Data);
	}

	iterator end() const
	{
		return iterator(m_Data + m_CurrentUsedSize);
	}

	iterator iterAt(size_t pos)
	{
		if (pos > m_CurrentUsedSize)
			assert(0);
		return iterator(m_Data + pos);
	}

	bool reserve(size_t newSize)
	{
		if (newSize > m_Size)
			return ChangeSize(newSize);
		return true;
	}

	bool push_back(const T & elem)
	{
		++m_CurrentUsedSize;
		if (!GrowIfNeeded())
		{
			--m_CurrentUsedSize;
			return false;
		}

		m_Data[m_CurrentUsedSize - 1] = elem;
		return true;
	}

	void pop_back()
	{
		--m_CurrentUsedSize;
		if (m_CurrentUsedSize < 0)
			m_CurrentUsedSize = 0;

		FreeMemIfPossible();
	}

	bool resize(size_t newSize)
	{
		if (!ChangeSize(newSize))
			return false;
		m_CurrentUsedSize = newSize;
		return true;
	}

	bool empty() const
	{
		return (m_CurrentUsedSize == 0);
	}

	T & at(size_t pos)
	{
		if (pos > m_CurrentUsedSize)
		{
			assert(0);
		}
		return m_Data[pos];
	}

	const  T & at(size_t pos) const
	{
		if (pos > m_CurrentUsedSize)
		{
			assert(0);
		}
		return m_Data[pos];
	}

	T & operator[](size_t pos)
	{
		return at(pos);
	}

	const T & operator[](size_t pos) const
	{
		return at(pos);
	}

	T & front()
	{
		if (m_CurrentUsedSize < 1)
		{
			assert(0);
		}
		return m_Data[0];
	}

	const T & front() const
	{
		if (m_CurrentUsedSize < 1)
		{
			assert(0);
		}
		return m_Data[0];
	}

	T & back()
	{
		if (m_CurrentUsedSize < 1)
		{
			assert(0);
		}
		return m_Data[m_CurrentUsedSize - 1];
	}

	const T & back() const
	{
		if (m_CurrentUsedSize < 1)
		{
			assert(0);
		}
		return m_Data[m_CurrentUsedSize - 1];
	}

	iterator insert(iterator where, const T & value)
	{
		// validate iter
		if (where < m_Data || where > (m_Data + m_CurrentUsedSize))
			return iterator(0);

		size_t ofs = where - begin();

		++m_CurrentUsedSize;
		if (!GrowIfNeeded())
		{
			--m_CurrentUsedSize;
			return false;
		}

		where = begin() + ofs;

		// Move subsequent entries
		for (T *ptr = m_Data + m_CurrentUsedSize - 2; ptr >= where.base(); --ptr)
			*(ptr + 1) = *ptr;

		*where.base() = value;

		return where;
	}

	iterator erase(iterator where)
	{
		// validate iter
		if (where < m_Data || where >= (m_Data + m_CurrentUsedSize))
			return iterator(0);

		size_t ofs = where - begin();

		if (m_CurrentUsedSize > 1)
		{
			// move
			T *theend = m_Data + m_CurrentUsedSize;
			for (T *ptr = where.base() + 1; ptr < theend; ++ptr)
				*(ptr - 1) = *ptr;
		}

		--m_CurrentUsedSize;

		FreeMemIfPossible();

		return begin() + ofs;
	}

	void clear()
	{
		m_Size = 0;
		m_CurrentUsedSize = 0;
		if (m_Data)
		{
			delete [] m_Data;
			m_Data = NULL;
		}
	}
};
};	//namespace SourceHook

#endif // __CVECTOR_H__

