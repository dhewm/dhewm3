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

/**
 * @brief This file provides a way for getting information about a member function.
 * @file sh_memfuncinfo.h
 */

#ifndef __SHINT_MEMFUNC_INFO_H__
#define __SHINT_MEMFUNC_INFO_H__

namespace SourceHook
{

	// Don Clugston:
	//		implicit_cast< >
	// I believe this was originally going to be in the C++ standard but 
	// was left out by accident. It's even milder than static_cast.
	// I use it instead of static_cast<> to emphasize that I'm not doing
	// anything nasty. 
	// Usage is identical to static_cast<>
	template <class OutputClass, class InputClass>
	inline OutputClass implicit_cast(InputClass input){
		return input;
	}


	struct MemFuncInfo
	{
		bool isVirtual;		// Is the function virtual?
		int thisptroffs;	// The this pointer the function expects to be called with
							// If -1, you need to call the GetFuncInfo_GetThisPtr function
		int vtblindex;		// The function's index in the vtable (0-based, 1=second entry, 2=third entry, ...)
		int vtbloffs;		// The vtable pointer
	};

	// Ideas by Don Clugston.
	// Check out his excellent paper: http://www.codeproject.com/cpp/FastDelegate.asp

	template<int N> struct MFI_Impl
	{
		template<class MFP> static inline void GetFuncInfo(MFP *mfp, MemFuncInfo &out)
		{
			static char weird_memfunc_pointer_exclamation_mark_arrow_error[N-1000];
		}
	}; 

# if SH_COMP == SH_COMP_GCC

	template<> struct MFI_Impl<2*SH_PTRSIZE>   // All of these have size==8/16
	{
		struct GCC_MemFunPtr
		{
			union
			{
				void *funcadr;				// always even
				intptr_t vtable_index_plus1;		//  = vindex+1, always odd
			};
			intptr_t delta;
		};
		template<class MFP> static inline void GetFuncInfo(MFP mfp, MemFuncInfo &out)
		{
			GCC_MemFunPtr *mfp_detail = (GCC_MemFunPtr*)&mfp;
			out.thisptroffs = mfp_detail->delta;
			if (mfp_detail->vtable_index_plus1 & 1)
			{
				out.vtblindex = (mfp_detail->vtable_index_plus1 - 1) / SH_PTRSIZE;
				out.vtbloffs = 0;
				out.isVirtual = true;
			}
			else
				out.isVirtual = false;
		}
	}; 

# elif SH_COMP == SH_COMP_MSVC

	namespace
	{
		int MFI_GetVtblOffset(void *mfp)
		{
			unsigned char *addr = (unsigned char*)mfp;
			if (*addr == 0xE9)		// Jmp
			{
				// May or may not be!
				// Check where it'd jump
				addr += 5 /*size of the instruction*/ + *(unsigned long*)(addr + 1);
			}
	
			// Check whether it's a virtual function call
			// They look like this:
			// 004125A0 8B 01            mov         eax,dword ptr [ecx] 
			// 004125A2 FF 60 04         jmp         dword ptr [eax+4]
			//		==OR==
			// 00411B80 8B 01            mov         eax,dword ptr [ecx] 
			// 00411B82 FF A0 18 03 00 00 jmp         dword ptr [eax+318h]

			// However, for vararg functions, they look like this:
			// 0048F0B0 8B 44 24 04      mov         eax,dword ptr [esp+4]
			// 0048F0B4 8B 00            mov         eax,dword ptr [eax]
			// 0048F0B6 FF 60 08         jmp         dword ptr [eax+8]
			//		==OR==
			// 0048F0B0 8B 44 24 04      mov         eax,dword ptr [esp+4]
			// 0048F0B4 8B 00            mov         eax,dword ptr [eax]
			// 00411B82 FF A0 18 03 00 00 jmp         dword ptr [eax+318h]
			
			// With varargs, the this pointer is passed as if it was the first argument

			bool ok = false;
			if (addr[0] == 0x8B && addr[1] == 0x44 && addr[2] == 0x24 && addr[3] == 0x04 &&
				addr[4] == 0x8B && addr[5] == 0x00)
			{
				addr += 6;
				ok = true;
			}
			else if (addr[0] == 0x8B && addr[1] == 0x01)
			{
				addr += 2;
				ok = true;
			}
			if (!ok)
				return -1;

			if (*addr++ == 0xFF)
			{
				if (*addr == 0x60)
				{
					return *++addr / 4;
				}
				else if (*addr == 0xA0)
				{
					return *((unsigned int*)++addr) / 4;
				}
				else if (*addr == 0x20)
					return 0;
				else
					return -1;
			}
			return -1;
		}
	}

	template<> struct MFI_Impl<1*SH_PTRSIZE>   // simple ones
	{
		template<class MFP> static inline void GetFuncInfo(MFP mfp, MemFuncInfo &out)
		{
			out.vtblindex = MFI_GetVtblOffset(*(void**)&mfp);
			out.isVirtual = out.vtblindex >= 0 ? true : false;
			out.thisptroffs = 0;
			out.vtbloffs = 0;
		}
	};

	template<> struct MFI_Impl<2*SH_PTRSIZE>   // more complicated ones!
	{
		struct MSVC_MemFunPtr2
		{
			void *funcadr;
			int delta;
		};
		template<class MFP> static inline void GetFuncInfo(MFP mfp, MemFuncInfo &out)
		{
			out.vtblindex = MFI_GetVtblOffset(*(void**)&mfp);
			out.isVirtual = out.vtblindex >= 0 ? true : false;
			out.thisptroffs = reinterpret_cast<MSVC_MemFunPtr2*>(&mfp)->delta;
			out.vtbloffs = 0;
		}
	};

	// By Don Clugston, adapted
	template<> struct MFI_Impl<3*SH_PTRSIZE>   // WOW IT"S GETTING BIGGER OMGOMOGMG
	{
		class __single_inheritance GenericClass;
		class GenericClass {};

		struct MicrosoftVirtualMFP {
			void (GenericClass::*codeptr)(); // points to the actual member function
			int delta;		// #bytes to be added to the 'this' pointer
			int vtable_index; // or 0 if no virtual inheritance
		};

		struct GenericVirtualClass : virtual public GenericClass
		{
			typedef GenericVirtualClass * (GenericVirtualClass::*ProbePtrType)();
			GenericVirtualClass * GetThis() { return this; }
		};

		template<class MFP> static inline void GetFuncInfo(MFP mfp, MemFuncInfo &out)
		{
			out.vtblindex = MFI_GetVtblOffset(*(void**)&mfp);
			out.isVirtual = out.vtblindex >= 0 ? true : false;
			// This pointer
			/*
			union {
				MFP func;
				GenericClass* (T::*ProbeFunc)();
				MicrosoftVirtualMFP s;
			} u;
			u.func = mfp;
			union {
				GenericVirtualClass::ProbePtrType virtfunc;
				MicrosoftVirtualMFP s;
			} u2;

			// Check that the horrible_cast<>s will work
			typedef int ERROR_CantUsehorrible_cast[sizeof(mfp)==sizeof(u.s)
				&& sizeof(mfp)==sizeof(u.ProbeFunc)
				&& sizeof(u2.virtfunc)==sizeof(u2.s) ? 1 : -1];
			// Unfortunately, taking the address of a MF prevents it from being inlined, so 
			// this next line can't be completely optimised away by the compiler.
			u2.virtfunc = &GenericVirtualClass::GetThis;
			u.s.codeptr = u2.s.codeptr;
			out.thisptroffs = (reinterpret_cast<T*>(NULL)->*u.ProbeFunc)();
			*/
			out.thisptroffs = -1;
			out.vtbloffs = 0;
		}
	};

	// Don: Nasty hack for Microsoft and Intel (IA32 and Itanium)
	// unknown_inheritance classes go here 
	// This is probably the ugliest bit of code I've ever written. Look at the casts!
	// There is a compiler bug in MSVC6 which prevents it from using this code.
	template<> struct MFI_Impl<4*SH_PTRSIZE>   // THE BIGGEST ONE!!!1GABEN
	{
		template<class MFP> static inline void GetFuncInfo(MFP mfp, MemFuncInfo &out)
		{
			out.vtblindex = MFI_GetVtblOffset(*(void**)&mfp);
			out.isVirtual = out.vtblindex >= 0 ? true : false;

			// The member function pointer is 16 bytes long. We can't use a normal cast, but
			// we can use a union to do the conversion.
			union {
				MFP func;
				// In VC++ and ICL, an unknown_inheritance member pointer 
				// is internally defined as:
				struct {
					void *m_funcaddress; // points to the actual member function
					int delta;		// #bytes to be added to the 'this' pointer
					int vtordisp;		// #bytes to add to 'this' to find the vtable
					int vtable_index; // or 0 if no virtual inheritance
				} s;
			} u;
			// Check that the horrible_cast will work
			typedef int ERROR_CantUsehorrible_cast[sizeof(u.func)==sizeof(u.s)? 1 : -1];
			u.func = mfp;
			int virtual_delta = 0;
			if (u.s.vtable_index) { // Virtual inheritance is used
				/*
				// First, get to the vtable. 
				// It is 'vtordisp' bytes from the start of the class.
				int * vtable = *reinterpret_cast<int **>(
					reinterpret_cast<char *>(thisptr) + u.s.vtordisp );

				// 'vtable_index' tells us where in the table we should be looking.
				virtual_delta = u.s.vtordisp + *reinterpret_cast<const int *>( 
					reinterpret_cast<const char *>(vtable) + u.s.vtable_index);
			// The int at 'virtual_delta' gives us the amount to add to 'this'.
			// Finally we can add the three components together. Phew!
			out.thisptr = reinterpret_cast<void *>(
				reinterpret_cast<char *>(thisptr) + u.s.delta + virtual_delta);
				*/
				out.vtbloffs = u.s.vtordisp;
				out.thisptroffs = -1;
			}
			else
			{
				out.vtbloffs = out.vtblindex < 0 ? 0 : u.s.delta;
				out.thisptroffs = u.s.delta;
			}
		};
	};
# else
#  error Unsupported compiler
# endif

	// This version does not take a this pointer
	// Useful for hookdecls, as they ensure that mfp is correct through a static_cast
	template<class X> inline void GetFuncInfo(X mfp, MemFuncInfo &out)
	{
		MFI_Impl<sizeof(mfp)>::GetFuncInfo(mfp, out);
	}

	// Versions which do take a this
	template<class X, class Y, class RetType>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)() = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)() const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)() const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18, class Param19>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18, class Param19>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18, class Param19, class Param20>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, Param20), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, Param20) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18, class Param19, class Param20>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, Param20) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, Param20) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}


	// GCC & MSVC 7.1 need this, MSVC 7.0 doesn't like it
#if SH_COMP != SH_COMP_MSVC || _MSC_VER > 1300

	template<class X, class Y, class RetType>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18, class Param19>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18, class Param19>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18, class Param19, class Param20>
	inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, Param20, ...), MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, Param20, ...) = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}

	template<class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10, class Param11, class Param12, class Param13, class Param14, class Param15, class Param16, class Param17, class Param18, class Param19, class Param20>
		inline void GetFuncInfo(Y *ptr, RetType(X::*mfp)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, Param20, ...) const, MemFuncInfo &out)
	{
		RetType(Y::*mfp2)(Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16, Param17, Param18, Param19, Param20, ...) const = mfp;
		MFI_Impl<sizeof(mfp2)>::GetFuncInfo(mfp2, out);
	}


#endif

}

#endif


