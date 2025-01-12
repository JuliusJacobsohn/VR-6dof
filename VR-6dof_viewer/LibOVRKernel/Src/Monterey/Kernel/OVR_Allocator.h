/************************************************************************************

Filename    :   OVR_Allocator.h
Content     :   Installable memory allocator
Created     :   September 19, 2012
Notes       :

Copyright   :   Copyright 2014-2016 Oculus VR, LLC All Rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License");
you may not use the Oculus VR Rift SDK except in compliance with the License,
which is provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3

Unless required by applicable law or agreed to in writing, the Oculus VR SDK
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_Allocator_h
#define OVR_Allocator_h

#include "OVR_Types.h"

// ***** Disable template-unfriendly MS VC++ warnings
#if defined(OVR_CC_MSVC)
// Pragma to prevent long name warnings in in VC++
#pragma warning(disable : 4503)
#pragma warning(disable : 4786)
// In MSVC 7.1, warning about placement new POD default initializer
#pragma warning(disable : 4345)
#endif

// Un-define new so that placement constructors work
#undef new

//-----------------------------------------------------------------------------------
// ***** Placement new overrides

// Calls constructor on own memory created with "new(ptr) type"
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE

#if defined(OVR_CC_MWERKS) || defined(OVR_CC_BORLAND) || defined(OVR_CC_GNU)
#include <new>
#else
// Useful on MSVC
OVR_FORCE_INLINE void* operator new(size_t n, void* ptr) {
  OVR_UNUSED(n);
  return ptr;
}
OVR_FORCE_INLINE void operator delete(void*, void*) {}
#endif

#endif // __PLACEMENT_NEW_INLINE

//------------------------------------------------------------------------
// ***** Macros to redefine class new/delete operators
//
// Types specifically declared to allow disambiguation of address in
// class member operator new.

#define OVR_MEMORY_REDEFINE_NEW_IMPL(class_name, check_delete) \
  void* operator new(size_t sz) {                              \
    void* p = OVR_ALLOC_DEBUG(sz, __FILE__, __LINE__);         \
    return p;                                                  \
  }                                                            \
  void* operator new(size_t sz, const char* file, int line) {  \
    OVR_UNUSED2(file, line);                                   \
    void* p = OVR_ALLOC_DEBUG(sz, file, line);                 \
    return p;                                                  \
  }                                                            \
  void operator delete(void* p) {                              \
    check_delete(class_name, p);                               \
    OVR_FREE(p);                                               \
  }                                                            \
  void operator delete(void* p, const char*, int) {            \
    check_delete(class_name, p);                               \
    OVR_FREE(p);                                               \
  }

#define OVR_MEMORY_DEFINE_PLACEMENT_NEW         \
  void* operator new(size_t n, void* ptr) {     \
    OVR_UNUSED(n);                              \
    return ptr;                                 \
  }                                             \
  void operator delete(void* ptr, void* ptr2) { \
    OVR_UNUSED2(ptr, ptr2);                     \
  }

// Used by OVR_MEMORY_REDEFINE_NEW
#define OVR_MEMORY_CHECK_DELETE_NONE(class_name, p)

// Redefine all delete/new operators in a class without custom memory initialization.
#define OVR_MEMORY_REDEFINE_NEW(class_name) \
  OVR_MEMORY_REDEFINE_NEW_IMPL(class_name, OVR_MEMORY_CHECK_DELETE_NONE)

namespace OVR {

//-----------------------------------------------------------------------------------
// ***** Construct / Destruct

// Construct/Destruct functions are useful when new is redefined, as they can
// be called instead of placement new constructors.

template <class T>
OVR_FORCE_INLINE T* Construct(void* p) {
  return ::new (p) T;
}

template <class T>
OVR_FORCE_INLINE T* Construct(void* p, const T& source) {
  return ::new (p) T(source);
}

// Same as above, but allows for a different type of constructor.
template <class T, class S>
OVR_FORCE_INLINE T* ConstructAlt(void* p, const S& source) {
  return ::new (p) T(source);
}

template <class T, class S1, class S2>
OVR_FORCE_INLINE T* ConstructAlt(void* p, const S1& src1, const S2& src2) {
  return ::new (p) T(src1, src2);
}

// Note: These ConstructArray functions don't properly support the case of a C++ exception occurring
// midway
// during construction, as they don't deconstruct the successfully constructed array elements before
// returning.
template <class T>
OVR_FORCE_INLINE void ConstructArray(void* p, size_t count) {
  uint8_t* pdata = (uint8_t*)p;
  for (size_t i = 0; i < count; ++i, pdata += sizeof(T)) {
    Construct<T>(pdata);
  }
}

template <class T>
OVR_FORCE_INLINE void ConstructArray(void* p, size_t count, const T& source) {
  uint8_t* pdata = (uint8_t*)p;
  for (size_t i = 0; i < count; ++i, pdata += sizeof(T)) {
    Construct<T>(pdata, source);
  }
}

template <class T>
OVR_FORCE_INLINE void Destruct(T* pobj) {
  pobj->~T();
  OVR_UNUSED1(pobj); // Fix incorrect 'unused variable' MSVC warning.
}

template <class T>
OVR_FORCE_INLINE void DestructArray(T* pobj, size_t count) {
  for (size_t i = 0; i < count; ++i, ++pobj)
    pobj->~T();
}

//-----------------------------------------------------------------------------------
// ***** Allocator

// Allocator defines a memory allocation interface that developers can override
// to to provide memory for OVR; an instance of this class is typically created on
// application startup and passed into System or OVR::System constructor.
//
//
// Users implementing this interface must provide three functions: Alloc, Free,
// and Realloc. Implementations of these functions must honor the requested alignment.
// Although arbitrary alignment requests are possible, requested alignment will
// typically be small, such as 16 bytes or less.

//-----------------------------------------------------------------------------------
// ***** Allocator
//
class Allocator {
  friend class System;

 public:
  // *** Standard Alignment Alloc/Free

  virtual ~Allocator() {}

  // Allocate memory of specified size with default alignment.
  // Alloc of size==0 will allocate a tiny block & return a valid pointer;
  // this makes it suitable for new operator.
  virtual void* Alloc(size_t size) = 0;

  // Same as Alloc, but provides an option of passing debug data.
  virtual void* AllocDebug(size_t size, const char* /*file*/, unsigned /*line*/) {
    return Alloc(size);
  }

  // Reallocate memory block to a new size, copying data if necessary. Returns the pointer to
  // new memory block, which may be the same as original pointer. Will return 0 if reallocation
  // failed, in which case previous memory is still valid.
  // Realloc to decrease size will never fail.
  // Realloc of pointer == 0 is equivalent to Alloc
  // Realloc to size == 0, shrinks to the minimal size, pointer remains valid and requires Free().
  virtual void* Realloc(void* p, size_t newSize) = 0;

  // Frees memory allocated by Alloc/Realloc.
  // Free of null pointer is valid and will do nothing.
  virtual void Free(void* p) = 0;

  // *** Standard Alignment Alloc/Free

  // Allocate memory of specified alignment.
  // Memory allocated with AllocAligned MUST be freed with FreeAligned.
  // Default implementation will delegate to Alloc/Free after doing rounding.
  virtual void* AllocAligned(size_t size, size_t align);
  // Frees memory allocated with AllocAligned.
  virtual void FreeAligned(void* p);

  // Returns the pointer to the current globally installed Allocator instance.
  // This pointer is used for most of the memory allocations.
  static Allocator* GetInstance() {
    return pInstance;
  }

 protected:
  // onSystemShutdown is called on the allocator during System::Shutdown.
  // At this point, all allocations should've been freed.
  virtual void onSystemShutdown() {}

 public:
  static void setInstance(Allocator* palloc) {
    OVR_ASSERT((pInstance == 0) || (palloc == 0));
    pInstance = palloc;
  }

 private:
  static Allocator* pInstance;
};

//------------------------------------------------------------------------
// ***** DefaultAllocator

// This allocator is created and used if no other allocator is installed.
// Default allocator delegates to system malloc.

class DefaultAllocator : public Allocator {
 public:
  virtual void* Alloc(size_t size);
  virtual void* AllocDebug(size_t size, const char* file, unsigned line);
  virtual void* Realloc(void* p, size_t newSize);
  virtual void Free(void* p);

 public:
  static DefaultAllocator* InitSystemSingleton();
};

//------------------------------------------------------------------------
// ***** Memory Allocation Macros
//
// These macros should be used for global allocation. In the future, these
// macros will allows allocation to be extended with debug file/line information
// if necessary.

#define OVR_REALLOC(p, s) OVR::Allocator::GetInstance()->Realloc((p), (s))
#define OVR_FREE(p) OVR::Allocator::GetInstance()->Free((p))
#define OVR_ALLOC_ALIGNED(s, a) OVR::Allocator::GetInstance()->AllocAligned((s), (a))
#define OVR_FREE_ALIGNED(p) OVR::Allocator::GetInstance()->FreeAligned((p))

#ifdef OVR_BUILD_DEBUG
#define OVR_ALLOC(s) OVR::Allocator::GetInstance()->AllocDebug((s), __FILE__, __LINE__)
#define OVR_ALLOC_DEBUG(s, f, l) OVR::Allocator::GetInstance()->AllocDebug((s), f, l)
#else
#define OVR_ALLOC(s) OVR::Allocator::GetInstance()->Alloc((s))
#define OVR_ALLOC_DEBUG(s, f, l) OVR::Allocator::GetInstance()->Alloc((s))
#endif

//------------------------------------------------------------------------
// ***** NewOverrideBase
//
// Base class that overrides the new and delete operators.
// Deriving from this class, even as a multiple base, incurs no space overhead.
class NewOverrideBase {
 public:
  // Redefine all new & delete operators.
  OVR_MEMORY_REDEFINE_NEW(NewOverrideBase)
};

} // OVR

//------------------------------------------------------------------------
// ***** OVR_DEFINE_NEW
//
// Redefine operator 'new' if necessary.
// This allows us to remap all usage of new to something different.
//
#if defined(OVR_DEFINE_NEW)
#define new OVR_DEFINE_NEW
#endif

#endif // OVR_Allocator_h
