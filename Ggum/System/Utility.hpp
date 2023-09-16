#pragma once

#include <string>
#include <stringapiset.h>

namespace GG {
namespace Utility {

inline std::wstring string_to_wstring(const std::string& str)
{
	int len = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
	auto* buf = new wchar_t[len + sizeof(wchar_t)];
	buf[len] = '\0';
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.length()), buf, len + 1);
	std::wstring wstr{ buf };
	delete[] buf;

	return wstr;
}

inline std::string wstring_to_string(const std::wstring& wstr)
{
	int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<int>(wstr.length()), nullptr, 0, nullptr, nullptr);
	auto buf = new char[len + sizeof(char)];
	buf[len] = '\0';
	::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<int>(wstr.length()), buf, len + 1, nullptr, nullptr);
	std::string str{ buf };
	delete[] buf;

	return str;
}

}
}