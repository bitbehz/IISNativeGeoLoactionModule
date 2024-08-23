#include "pch.h"
#include "StringUtils.h"

extern std::wstring stringToWstring(const std::string& str) {
    size_t size_needed;
    mbstowcs_s(&size_needed, nullptr, 0, str.c_str(), 0);

    std::wstring wstr(size_needed, L'\0');
    mbstowcs_s(&size_needed, &wstr[0], size_needed, str.c_str(), size_needed);
    return wstr;
}

extern std::string wstringToString(const std::wstring& wstr) {
    size_t size_needed;
    wcstombs_s(&size_needed, nullptr, 0, wstr.c_str(), 0);

    std::string str(size_needed, '\0');
    wcstombs_s(&size_needed, &str[0], size_needed, wstr.c_str(), size_needed);
    return str;
}

extern std::string trim(const std::string& str) {
    std::string result = str;

    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));

    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), result.end());

    return result;
}