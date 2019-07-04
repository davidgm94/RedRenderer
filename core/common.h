#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

#define USING_EASTL
#ifdef USING_EASTL
#include <EAStdC/EASprintf.h>
#include <EASTL/vector.h>
using eastl::vector;
#include <EASTL/array.h>
using eastl::array;
#include <EASTL/string.h>
using eastl::string;
using eastl::wstring;
#include <EASTL/algorithm.h>
using eastl::min;
using eastl::max;
#else
#include <vector>
using std::vector;
#include <array>
using std::array;
#include <string>
using std::string;
using std::wstring;
#include <algorithm>
using std::min;
using std::max;
#endif

#define STRING_CONCAT(a, b) (a "" b)
#define WSTRING_CONCAT(a, b) (a L"" b)

inline wstring stringToWString(const char* string)
{
	size_t stringLength = strlen(string);
	wstring wstr; wstr.reserve(stringLength);
	mbstowcs(&wstr[0], string, strlen(string));

	return wstr;
}

#ifndef CONTAINER_BYTES
#define CONTAINER_BYTES(container) (sizeof(container[0]) * container.size())
#endif