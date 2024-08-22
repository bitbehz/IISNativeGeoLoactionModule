#pragma once
#include "pch.h"


void LogEvent(const std::wstring& message, WORD eventType);
void LogError(const std::wstring& message);
void LogInfo(const std::wstring& message);
void LogWarning(const std::wstring& message);
void LogLastError(const std::wstring& context);

#ifdef _DEBUG
#define LogDebug(message) LogInfo(std::wstring(L"[DEBUG]") + message)
#else
#define LogDebug(message) ((void)0) // No-op in release mode
#endif