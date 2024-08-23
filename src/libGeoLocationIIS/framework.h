#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <sal.h>
#include <httpserv.h>
#include <string>
#include <sstream>
#include <mutex>
#include <maxminddb.h>
#include <unordered_set>
#include <ws2tcpip.h>
#include <functional>
#include <algorithm>

// Link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
