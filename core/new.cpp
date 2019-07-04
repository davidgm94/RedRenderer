#include "common.h"
#ifdef USING_EASTL
#include <malloc.h>
#include <EABase/config/eaplatform.h>
#include <EASTL/allocator_malloc.h>




void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new u8[size];
}

namespace
{
	eastl::allocator_malloc dummyAllocator;
}

void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
#if 0

#if EA_PLATFORM_MICROSOFT
	return _aligned_offset_malloc(size, alignment, alignmentOffset);
#else
#error "Platform not supported"
#endif

#endif
	return dummyAllocator.allocate(size, alignment, alignmentOffset, flags);
}
#endif