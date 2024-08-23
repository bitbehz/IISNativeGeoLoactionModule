#include "pch.h"
#include "Log.h"

void LogEvent(const std::wstring& message, WORD eventType) {
    std::lock_guard<std::mutex> lock(g_eventLogMutex);

    if (g_hEventLog != NULL) {
        const wchar_t* strings[1] = { message.c_str() };
        ReportEvent(g_hEventLog, eventType, 0, 0, NULL, 1, 0, strings, NULL);
    }
}

void LogLastError(const std::wstring& context) {
    DWORD errorCode = GetLastError();
    LPWSTR messageBuffer = nullptr;

    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&messageBuffer,
        0,
        NULL
    );

    std::wostringstream woss;
    woss << context << L" Error code: " << errorCode << L". Message: ";

    if (messageBuffer) {
        woss << std::wstring(messageBuffer, size);
        LocalFree(messageBuffer);
    }
    else {
        woss << L"Unable to retrieve error message";
    }

    LogError(woss.str());
}

void LogError(const std::wstring& message) {
    LogEvent(message, EVENTLOG_ERROR_TYPE);
}

void LogInfo(const std::wstring& message) {
    LogEvent(message, EVENTLOG_INFORMATION_TYPE);
}

void LogWarning(const std::wstring& message) {
    LogEvent(message, EVENTLOG_WARNING_TYPE);
}
