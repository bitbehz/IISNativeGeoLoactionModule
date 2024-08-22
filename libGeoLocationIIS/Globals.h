#pragma once
#include "pch.h"

extern HANDLE g_hEventLog;
extern MMDB_s g_mmdb;
extern std::mutex g_eventLogMutex;
extern std::mutex g_mmdbMutex;
extern std::mutex g_configMutex;
extern Config* g_configs;
extern bool g_bDataLoaded;

extern IHttpServer* g_pHttpServer;


