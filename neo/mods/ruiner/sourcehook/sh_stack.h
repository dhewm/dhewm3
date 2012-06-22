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

#ifndef __SH_STACK_H__
#define __SH_STACK_H__

#define SH_STACK_DEFAULT_SIZE 4

namespace SourceHook
{
	// Vector
	template <class T> class CStack
	{
		T *m_Elements;
		size_t m_AllocatedSize;
		size_t m_UsedSize;
	public:
		friend class iterator;
		class iterator
		{
			CStack<T> *m_pParent;
			size_t m_Index;
		public:
			iterator(CStack<T> *pParent, size_t id) : m_pParent(pParent), m_Index(id)
			{
			}

			iterator(CStack<T> *pParent) : m_pParent(pParent), m_Index(0)
			{
			}
			
			iterator() : m_pParent(NULL), m_Index(0)
			{
			}

			T &operator *()
			{
				return m_pParent->m_Elements[m_Index];
			}
			const T &operator *() const
			{
				return m_pParent->m_Elements[m_Index];
			}
			
			T * operator->()
			{
				return m_pParent->m_Elements + m_Index;
			}

			const T * operator->() const
			{
				return m_pParent->m_Elements + m_Index;
			}

			iterator & operator++()		// preincrement
			{
				++m_Index;
				return (*this);
			}

			iterator operator++(int)	// postincrement
			{
				iterator tmp = *this;
				++m_Index;
				return tmp;
			}

			iterator & operator--()		// predecrement
			{
				--m_Index;
				return (*this);
			}

			iterator operator--(int)	// postdecrememnt
			{
				iterator tmp = *this;
				--m_Index;
				return tmp;
			}

			bool operator==(const iterator & right) const
			{
				return (m_pParent == right.m_pParent && m_Index == right.m_Index);
			}

			bool operator!=(const iterator & right) const
			{
				return !(*this == right);
			}
		};
		CStack() : m_Elements(new T[SH_STACK_DEFAULT_SIZE]),
			m_AllocatedSize(SH_STACK_DEFAULT_SIZE),
			m_UsedSize(0)
		{
		}
		CStack(size_t size) : m_Elements(new T[size]),
			m_AllocatedSize(size),
			m_UsedSize(0)
		{
		}

		CStack(const CStack &other) : m_Elements(NULL),
			m_AllocatedSize(0),
			m_UsedSize(0)
		{
			reserve(other.m_AllocatedSize);
			m_UsedSize = other.m_UsedSize;
			for (size_t i = 0; i < m_UsedSize; ++i)
				m_Elements[i] = other.m_Elements[i];
		}

		~CStack()
		{
			if (m_Elements)
				delete [] m_Elements;
		}
		
		void operator=(const CStack &other)
		{
			if (m_AllocatedSize < other.m_AllocatedSize)
			{
				if (m_Elements)
					delete [] m_Elements;
				m_Elements = new T[other.m_AllocatedSize];
				m_AllocatedSize = other.m_AllocatedSize;
			}
			m_UsedSize = other.m_UsedSize;
			for (size_t i = 0; i < m_UsedSize; ++i)
				m_Elements[i] = other.m_Elements[i];
		}

		bool push(const T &val)
		{
			if (m_UsedSize + 1 == m_AllocatedSize)
			{
				// zOHNOES! REALLOCATE!
				m_AllocatedSize *= 2;
				T *newElements = new T[m_AllocatedSize];
				if (!newElements)
				{
					m_AllocatedSize /= 2;
					return false;
				}
				if (m_Elements)
				{
					for (size_t i = 0; i < m_UsedSize; ++i)
						newElements[i] = m_Elements[i];
					delete [] m_Elements;
				}
				m_Elements = newElements;
			}
			m_Elements[m_UsedSize++] = val;
			return true;
		}
		void pop()
		{
			--m_UsedSize;
		}

		T &front()
		{
			return m_Elements[m_UsedSize - 1];
		}

		const T &front() const
		{
			return m_Elements[m_UsedSize - 1];
		}

		iterator begin()
		{
			return iterator(this, 0);
		}
		iterator end()
		{
			return iterator(this, m_UsedSize);
		}

		size_t size()
		{
			return m_UsedSize;
		}
		size_t capacity()
		{
			return m_AllocatedSize;
		}
		bool empty()
		{
			return m_UsedSize == 0 ? true : false;
		}
		bool reserve(size_t size)
		{
			if (size > m_AllocatedSize)
			{
				T *newElements = new T[size];
				if (!newElements)
					return false;
				if (m_Elements)
				{
					for (size_t i = 0; i < m_UsedSize; ++i)
						newElements[i] = m_Elements[i];
					delete [] m_Elements;
				}
				m_Elements = newElements;
				m_AllocatedSize = size;
			}
			return true;
		}
	};
};	//namespace SourceHook

#endif
