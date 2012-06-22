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
* Author(s): David "BAILOPAN" Anderson
* ============================
*/

#ifndef _INCLUDE_SMM_LIST_H
#define _INCLUDE_SMM_LIST_H

#include <new>
#include <malloc.h>

namespace SourceHook
{
//This class is from CSDM for AMX Mod X
template <class T>
class List
{
public:
	class iterator;
	friend class iterator;
	class ListNode
	{
	public:
		ListNode(const T & o) : obj(o) { };
		ListNode() { };
		T obj;
		ListNode *next;
		ListNode *prev;
	};
private:
	ListNode *_Initialize()
	{
		ListNode *n = (ListNode *)malloc(sizeof(ListNode));
		n->next = NULL;
		n->prev = NULL;
		return n;
	}
public:
	List() : m_Head(_Initialize()), m_Size(0)
	{
	}
	List(const List &src) : m_Head(_Initialize()), m_Size(0)
	{
		iterator iter;
		for (iter=src.begin(); iter!=src.end(); iter++)
			push_back( (*iter) );
	}
	~List()
	{
		clear();
		if (m_Head)
		{
			free(m_Head);
			m_Head = NULL;
		}
	}
	void push_back(const T &obj)
	{
		ListNode *node = new ListNode(obj);

		if (!m_Head->prev)
		{
			//link in the node as the first item 
			node->next = m_Head;
			node->prev = m_Head;
			m_Head->prev = node;
			m_Head->next = node;
		} else {
			node->prev = m_Head->prev;
			node->next = m_Head;
			m_Head->prev->next = node;
			m_Head->prev = node;
		}
		m_Size++;
	}
	size_t size()
	{
		return m_Size;
	}
	void clear()
	{
		ListNode *node = m_Head->next;
		ListNode *temp;
		m_Head->next = NULL;
		m_Head->prev = NULL;
		while (node && node != m_Head)
		{
			temp = node->next;
			delete node;
			node = temp;
		}
		m_Size = 0;
	}
	bool empty()
	{
		return (m_Head->next == NULL);
	}
	T & back()
	{
		return m_Head->prev->obj;
	}
private:
	ListNode *m_Head;
	size_t m_Size;
public:
	class iterator
	{
	friend class List;
	public:
		iterator()
		{
			m_This = NULL;
		}
		iterator(const List &src)
		{
			m_This = src.m_Head;
		}
		iterator(ListNode *n) : m_This(n)
		{
		}
		iterator(const iterator &where)
		{
			m_This = where.m_This;
		}
		//pre decrement
		iterator & operator--()
		{
			if (m_This)
				m_This = m_This->prev;
			return *this;
		}
		//post decrement
		iterator operator--(int)
		{
			iterator old(*this);
			if (m_This)
				m_This = m_This->prev;
			return old;
		}	
		
		//pre increment
		iterator & operator++()
		{
			if (m_This)
				m_This = m_This->next;
			return *this;
		}
		//post increment
		iterator operator++(int)
		{
			iterator old(*this);
			if (m_This)
				m_This = m_This->next;
			return old;
		}
		
		const T & operator * () const
		{
			return m_This->obj;
		}
		T & operator * ()
		{
			return m_This->obj;
		}
		
		T * operator -> ()
		{
			return &(m_This->obj);
		}
		const T * operator -> () const
		{
			return &(m_This->obj);
		}
		
		bool operator != (const iterator &where) const
		{
			return (m_This != where.m_This);
		}
		bool operator ==(const iterator &where) const
		{
			return (m_This == where.m_This);
		}
	private:
		ListNode *m_This;
	};
public:
	iterator begin() const
	{
		if (m_Size)
			return iterator(m_Head->next);
		else
			return iterator(m_Head);
	}
	iterator end() const
	{
		return iterator(m_Head);
	}
	iterator erase(iterator &where)
	{
		ListNode *pNode = where.m_This;
		iterator iter(where);
		iter++;

		//If we are both the head and tail...
		if (m_Head->next == pNode && m_Head->prev == pNode)
		{
			m_Head->next = NULL;
			m_Head->prev = NULL;
		} else if (m_Head->next == pNode) {
			//we are only the first
			pNode->next->prev = m_Head;
			m_Head->next = pNode->next;
		} else if (m_Head->prev == pNode) {
			//we are the tail
			pNode->prev->next = m_Head;
			m_Head->prev = pNode->prev;
		} else {
			//middle unlink
			pNode->prev->next = pNode->next;
			pNode->next->prev = pNode->prev;
		}

		delete pNode;
		m_Size--;

		return iter;
	}
public:
	void remove(const T & obj)
	{
		iterator b;
		for (b=begin(); b!=end(); b++)
		{
			if ( (*b) == obj )
			{
				erase( b );
				break;
			}
		}
	}
	template <typename U>
	iterator find(const U & equ)
	{
		iterator iter;
		for (iter=begin(); iter!=end(); iter++)
		{
			if ( (*iter) == equ )
				return iter;
		}
		return end();
	}
	List & operator =(List &src)
	{
		clear();
		iterator iter;
		for (iter=src.begin(); iter!=src.end(); iter++)
			push_back( (*iter) );
		return *this;
	}
};
};	//NAMESPACE

#endif //_INCLUDE_CSDM_LIST_H
