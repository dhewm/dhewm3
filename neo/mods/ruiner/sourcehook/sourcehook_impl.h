/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 22:23:28 +0530 (Tue, 16 Oct 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/
/* ======== SourceHook ========
* Copyright (C) 2004-2005 Metamod:Source Development Team
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Pavol "PM OnoTo" Marko
* ============================
*/

#ifndef __SOURCEHOOK_IMPL_H__
#define __SOURCEHOOK_IMPL_H__

#include "sourcehook.h"
#include "sh_list.h"
#include "sh_vector.h"
#include "sh_tinyhash.h"
#include "sh_stack.h"

namespace SourceHook
{
	/**
	*	@brief The SourceHook implementation class
	*/
	class CSourceHookImpl : public ISourceHook
	{
	private:
		/**
		*	@brief A hook can be removed if you have this information
		*/
		struct RemoveHookInfo
		{
			RemoveHookInfo(Plugin pplug, void *piface, int tpo, HookManagerPubFunc phookman,
				ISHDelegate *phandler, bool ppost)
				: plug(pplug), iface(piface), thisptr_offs(tpo),
				hookman(phookman), handler(phandler), post(ppost)
			{
			}

			Plugin plug;
			void *iface;
			int thisptr_offs;
			HookManagerPubFunc hookman;
			ISHDelegate *handler;
			bool post;
		};

		struct HookInfo
		{
			ISHDelegate *handler;			//!< Pointer to the handler
			bool paused;					//!< If true, the hook should not be executed
			Plugin plug;					//!< The owner plugin
			int thisptr_offs;				//!< This pointer offset
		};

		class CHookList : public IHookList
		{
		public:
			List<HookInfo> m_List;

			friend class CIter;

			class CIter : public IHookList::IIter
			{
				friend class CHookList;

				CHookList *m_pList;

				void SkipPaused();
			public:

				List<HookInfo>::iterator m_Iter;

				CIter(CHookList *pList);

				virtual ~CIter();

				void GoToBegin();

				bool End();
				void Next();
				ISHDelegate *Handler();
				int ThisPtrOffs();

				void Clear();

				CIter *m_pNext;		// When stored in m_FreeIters and m_UsedIters
				CIter *m_pPrev;		// Only used when stored in m_UsedIters
			};

			CIter *m_FreeIters;
			CIter *m_UsedIters;

			CHookList();
			CHookList(const CHookList &other);
			virtual ~CHookList();

			void operator=(const CHookList &other);

			IIter *GetIter();
			void ReleaseIter(IIter *pIter);
		};

		// I know, data hiding... But I'm a lazy bastard!

		class CIface : public IIface
		{
		public:
			void *m_Ptr;
			CHookList m_PreHooks;
			CHookList m_PostHooks;
		public:
			CIface(void *ptr);
			virtual ~CIface();

			void *GetPtr();
			IHookList *GetPreHooks();
			IHookList *GetPostHooks();

			bool operator==(void *ptr)
			{
				return m_Ptr == ptr;
			}
		};

		class CVfnPtr : public IVfnPtr
		{
		public:
			typedef List<CIface> IfaceList;
			typedef IfaceList::iterator IfaceListIter;

			void *m_Ptr;
			void *m_OrigEntry;

			IfaceList m_Ifaces;

		public:
			CVfnPtr(void *ptr);
			virtual ~CVfnPtr();

			void *GetVfnPtr();
			void *GetOrigEntry();

			virtual IIface *FindIface(void *ptr);

			bool operator==(void *ptr)
			{
				return m_Ptr == ptr;
			}
		};

		class CHookManagerInfo : public IHookManagerInfo
		{
		public:
			typedef List<CVfnPtr> VfnPtrList;
			typedef VfnPtrList::iterator VfnPtrListIter;

			Plugin m_Plug;
			HookManagerPubFunc m_Func;

			int m_VtblOffs;
			int m_VtblIdx;
			const char *m_Proto;
			void *m_HookfuncVfnptr;

			VfnPtrList m_VfnPtrs;

		public:
			virtual ~CHookManagerInfo();

			IVfnPtr *FindVfnPtr(void *vfnptr);

			void SetInfo(int vtbl_offs, int vtbl_idx, const char *proto);
			void SetHookfuncVfnptr(void *hookfunc_vfnptr);
		};
		/**
		*	@brief A list of CHookManagerInfo classes
		*/
		typedef List<CHookManagerInfo> HookManInfoList;

		class CCallClassImpl : public GenericCallClass
		{
		public:

			typedef SourceHook::CVector<void*> OrigFuncs;
			typedef SourceHook::THash<int, OrigFuncs> OrigVTables;

			void *m_Ptr;			//!< Pointer to the actual object
			size_t m_ObjSize;		//!< Size of the instance
			OrigVTables m_VT;		//!< Info about vtables & functions

			int m_RefCounter;

			CCallClassImpl(void *ptr, size_t size);
			virtual ~CCallClassImpl();

			bool operator==(void *other)
			{
				return m_Ptr == other;
			}

			void *GetThisPtr();
			void *GetOrigFunc(int vtbloffs, int vtblidx);

			void ApplyCallClassPatch(int vtbl_offs, int vtbl_idx, void *orig_entry);
			void RemoveCallClassPatch(int vtbl_offs, int vtbl_idx);
		};

		/**
		*	@brief A list of CallClass structures
		*/
		typedef List<CCallClassImpl> Impl_CallClassList;

		Impl_CallClassList m_CallClasses;			//!< A list of already generated callclasses
		HookManInfoList m_HookMans;					//!< A list of hook managers

		struct HookLoopInfo
		{
			META_RES *pStatus;
			META_RES *pPrevRes;
			META_RES *pCurRes;

			bool shouldContinue;

			IIface *pCurIface;
			const void *pOrigRet;
			const void *pOverrideRet;
			void **pIfacePtrPtr;
		};
		typedef CStack<HookLoopInfo> HookLoopInfoStack;

		/**
		*	@brief Finds a hook manager for a function based on a text-prototype, a vtable offset and a vtable index
		*/
		HookManInfoList::iterator FindHookMan(HookManInfoList::iterator begin, HookManInfoList::iterator end,
			const char *proto, int vtblofs, int vtblidx);

		void ApplyCallClassPatches(CCallClassImpl &cc);
		void ApplyCallClassPatches(void *ifaceptr, int vtbl_offs, int vtbl_idx, void *orig_entry);
		void RemoveCallClassPatches(void *ifaceptr, int vtbl_offs, int vtbl_idx);

		void SetPluginPaused(Plugin plug, bool paused);

		HookLoopInfoStack m_HLIStack;
	public:
		CSourceHookImpl();
		virtual ~CSourceHookImpl();

		/**
		*	@brief Returns the interface version
		*/
		int GetIfaceVersion();

		/**
		*	@brief Returns the implemnetation version
		*/
		int GetImplVersion();

		/**
		*	@brief Make sure that a plugin is not used by any other plugins anymore, and unregister all its hook managers
		*/
		void UnloadPlugin(Plugin plug);

		/**
		*	@brief Shut down the whole system, unregister all hook managers
		*/
		void CompleteShutdown();

		/**
		*	@brief Add a hook.
		*
		*	@return True if the function succeeded, false otherwise
		*
		*	@param plug The unique identifier of the plugin that calls this function
		*	@param iface The interface pointer
		*	@param ifacesize The size of the class iface points to
		*	@param myHookMan A hook manager function that should be capable of handling the function
		*	@param handler A pointer to a FastDelegate containing the hook handler
		*	@param post Set to true if you want a post handler
		*/
		bool AddHook(Plugin plug, void *iface, int thisptr_offs, HookManagerPubFunc myHookMan, ISHDelegate *handler, bool post);

		/**
		*	@brief Removes a hook.
		*
		*	@return True if the function succeeded, false otherwise
		*
		*	@param plug The unique identifier of the plugin that calls this function
		*	@param iface The interface pointer
		*	@param thisptr_offs This pointer adjuster
		*	@param myHookMan A hook manager function that should be capable of handling the function
		*	@param handler A pointer to a FastDelegate containing the hook handler
		*	@param post Set to true if you want a post handler
		*/
		bool RemoveHook(Plugin plug, void *iface, int thisptr_offs, HookManagerPubFunc myHookMan, ISHDelegate *handler, bool post);

		/**
		*	@brief Removes a hook.
		*
		*	@ return True if the function succeeded, false otherwise
		*
		*	@param info A RemoveHookInfo structure, describing the hook
		*/
		bool RemoveHook(RemoveHookInfo info);

		/**
		*	@brief Checks whether a plugin has (a) hook manager(s) that is/are currently used by other plugins
		*
		*	@param plug The unique identifier of the plugin in question
		*/
		bool IsPluginInUse(Plugin plug);

		/**
		*	@brief Pauses all hooks of a plugin
		*
		*	@param plug The unique identifier of the plugin
		*/
		void PausePlugin(Plugin plug);

		/**
		*	@brief Unpauses all hooks of a plugin
		*
		*	@param plug The unique identifier of the plugin
		*/
		void UnpausePlugin(Plugin plug);

		/**
		*	@brief Return a pointer to a callclass. Generate a new one if required.
		*
		*	@param iface The interface pointer
		*	@param size Size of the class instance
		*/
		GenericCallClass *GetCallClass(void *iface, size_t size);

		/**
		*	@brief Release a callclass
		*
		*	@param ptr Pointer to the callclass
		*/
		virtual void ReleaseCallClass(GenericCallClass *ptr);

		virtual void SetRes(META_RES res);				//!< Sets the meta result
		virtual META_RES GetPrevRes();					//!< Gets the meta result of the previously called handler
		virtual META_RES GetStatus();					//!< Gets the highest meta result
		virtual const void *GetOrigRet();				//!< Gets the original result. If not in post function, undefined
		virtual const void *GetOverrideRet();			//!< Gets the override result. If none is specified, NULL
		virtual void *GetIfacePtr();					//!< Gets the interface pointer

		//////////////////////////////////////////////////////////////////////////
		// For hook managers
		void HookLoopBegin(IIface *pIface);			//!< Should be called when a hook loop begins
		void HookLoopEnd();							//!< Should be called when a hook loop exits
		void SetCurResPtr(META_RES *mres);			//!< Sets pointer to the current meta result
		void SetPrevResPtr(META_RES *mres);			//!< Sets pointer to previous meta result
		void SetStatusPtr(META_RES *mres);			//!< Sets pointer to the status variable
		void SetIfacePtrPtr(void **pp);				//!< Sets pointer to the interface this pointer
		void SetOrigRetPtr(const void *ptr);		//!< Sets the original return pointer
		void SetOverrideRetPtr(const void *ptr);	//!< Sets the override result pointer
		bool ShouldContinue();						//!< Returns false if the hook loop should exit
	};
}

#endif

