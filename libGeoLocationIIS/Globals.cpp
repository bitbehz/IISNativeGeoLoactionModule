#include "pch.h"
#include "Globals.h"

HANDLE g_hEventLog = NULL;
MMDB_s g_mmdb;
std::mutex g_eventLogMutex;
std::mutex g_mmdbMutex;
std::mutex g_configMutex;
Config* g_configs = nullptr;
bool g_bDataLoaded = false;

IHttpServer* g_pHttpServer = NULL;