// GG Engine Precompiled Header
// 프로젝트 안에서 사용되는 모든 stl이나 외부 API들은 여기 선언합니다.
// 모든 (Engine).cpp 파일에서 최상단에 #include "EnginePch.h"를 작성해 pch를 참조하도록 합니다.

#pragma once

#ifdef _DEBUG
#define GG_ASSERTION_ENABLED
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#endif
#include <Windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#else
#error GG Only Support Windows!
#endif

#include <iostream>
#include <algorithm>
#include <utility>
#include <functional>
#include <memory>
#include <sstream>

#include <string>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>


#ifdef GG_GRAPHICS_API_VULKAN
#pragma comment(lib, "vulkan-1.lib")
#include <vulkan/vulkan.h>
#endif