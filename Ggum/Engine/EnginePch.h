// GG Engine Precompiled Header
// ������Ʈ �ȿ��� ���Ǵ� ��� stl�̳� �ܺ� API���� ���� �����մϴ�.
// ��� (Engine).cpp ���Ͽ��� �ֻ�ܿ� #include "EnginePch.h"�� �ۼ��� pch�� �����ϵ��� �մϴ�.

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