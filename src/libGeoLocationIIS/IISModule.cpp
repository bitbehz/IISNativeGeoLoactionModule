#define _WINSOCKAPI_
#include "pch.h"


#define MODULE_TITLE L"GeoLocation IIS Module"

HRESULT InitializeEventViewer() {
    std::lock_guard<std::mutex> lock(g_eventLogMutex);
    if (g_hEventLog == nullptr)
    {
        g_hEventLog = RegisterEventSource(NULL, MODULE_TITLE);
        if (g_hEventLog == nullptr) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return S_OK;
}

HRESULT LoadConfiguration(IHttpContext* pHttpContext, Config*& pConfig) {

    HRESULT hr = S_OK;
    IAppHostAdminManager* pAdminManager = nullptr;
    IAppHostElement* pElement = nullptr;
    IAppHostProperty* pProperty = nullptr;
    BSTR    bstrSectionName = SysAllocString(L"system.webServer/IISGeoLocation");
    std::lock_guard<std::mutex> lock(g_configMutex);
    // Get the Admin Manager
    /*hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        LogError(L"Error calling CoInitializeEx for initializing COM library");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = CoCreateInstance(__uuidof(AppHostAdminManager), nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(IAppHostAdminManager), (VOID**)&pAdminManager);
    if (FAILED(hr)) {        
        return HRESULT_FROM_WIN32(GetLastError());
    }*/

    PCWSTR pszConfigPath = pHttpContext->GetMetadata()->GetMetaPath();
    BSTR bstrUrlPath = SysAllocString(pszConfigPath);
    
    pAdminManager = g_pHttpServer->GetAdminManager();

    if (pAdminManager == nullptr)
    {
        LogError(L"Unable to initialize AdminManager.");
        return E_UNEXPECTED;
    }
           
    // Get the IISGeoLocation section
    hr = pAdminManager->GetAdminSection(bstrSectionName,
        bstrUrlPath, &pElement);

    SysFreeString(bstrSectionName);

    if (FAILED(hr)) {
        //pAdminManager->Release();
        LogError(L"Unable to load confgiguration section");
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    pConfig = new Config();

    VARIANT var;
    
    // Read enabled property
    hr = ReadConfigProperty(pElement, L"enabled", var, [&](VARIANT& var) {
        pConfig->setEnabled(var.boolVal == VARIANT_TRUE);
        });
    if (FAILED(hr)) return hr;

    // Read databasePath property
    hr = ReadConfigProperty(pElement, L"databasePath", var, [&](VARIANT& var) {
        pConfig->setDatabasePath(var.bstrVal);
        });
    if (FAILED(hr)) return hr;

    // Read mode property   
    hr = ReadConfigProperty(pElement, L"mode", var, [&](VARIANT& var) {
        pConfig->setMode(var.bstrVal);
        });
    if (FAILED(hr)) return hr;

    if (pConfig->getModeEnum() == ModuleMode::Undefiend)
    {
        LogError(L"Mode property is undefiend.");
        return E_UNEXPECTED;
    }

    // Read countryList property
    hr = ReadConfigProperty(pElement, L"countryList", var, [&](VARIANT& var) {
        pConfig->setCountryList(var.bstrVal);
        });
    if (FAILED(hr)) return hr;

    // Read exceptionIPs property   
    hr = ReadConfigProperty(pElement, L"exceptionIPs", var, [&](VARIANT& var) {
        pConfig->setExceptionIPs(var.bstrVal);
        });
    if (FAILED(hr)) return hr;

    // Read isBehindProxy property   
    hr = ReadConfigProperty(pElement, L"isBehindProxy", var, [&](VARIANT& var) {
        pConfig->setIsBehindProxy(var.boolVal == VARIANT_TRUE);
        });
    if (FAILED(hr)) return hr;


    pElement->Release();
    
    //pAdminManager->Release();  // IIS will release it. (ntdll.dll)
    //pAdminManager = nullptr;      
   
    LogDebug(
        L"Configuration loaded successfully." +
        L"\n | CountryList : " + g_configs->getCountryList() +
        L"\n | DatabasePath : " + g_configs->getDatabasePath() +
        L"\n | ExceptionIPs : " + g_configs->getExceptionIPs() +
        L"\n | Mode : " + g_configs->getMode() +
        L"\n | Enabled : " + (g_configs->isEnabled() ? L"true" : L"false") + 
        L"\n | BehindProxy : " + (g_configs->isBehindProxy() ? L"true" : L"false")
    
    );

    return hr;
}

HRESULT LoadGeoDB(Config* pConfig)
{    
    std::lock_guard<std::mutex> lock(g_mmdbMutex);
    std::string dbPath = wstringToString(pConfig->getDatabasePath());
    int status = MMDB_open(dbPath.c_str(), MMDB_MODE_MMAP, &g_mmdb);
     

    if (status != MMDB_SUCCESS) {
        LogError(L"Could not open MaxMind DB.");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    LogDebug(L"MaxMind database successfully loaded.");

    const char* version = MMDB_lib_version();
    std::wstring wVersion = std::wstring(version, version + strlen(version));
    LogInfo(L"Database version : " +  wVersion);
    
    return S_OK;
}

HRESULT LoadData(IHttpContext* pHttpContext, Config*& pConfig)
{
    HRESULT hr = S_OK;
    if (g_configs == nullptr)
    {
        hr = LoadConfiguration(pHttpContext, g_configs);

        if (FAILED(hr)) {
            LogError(L"Failed to load configuration");
            return hr;
        }
    }
    if (g_mmdb.filename == nullptr)
    {
        hr = LoadGeoDB(g_configs);

        if (FAILED(hr))
        {
            LogError(L"Failed to load GeoLocation database.");
            return hr;
        }
    }
    
    return hr;
}

HRESULT InitializeModule() {

    HRESULT    hr = S_OK;    

    hr = InitializeEventViewer();

    if (FAILED(hr)) return hr;
    

    /*hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        LogError(L"Error calling CoInitializeEx for initializing COM library");
        return HRESULT_FROM_WIN32(GetLastError());
    }
    */

    LogInfo(std::wstring(MODULE_TITLE) + L" initialized successfully.");

    return S_OK;
}

void CleanupModule() {    
    LogDebug(L"Cleaning up module...");
    if (g_hEventLog != nullptr) {
        DeregisterEventSource(g_hEventLog);
        g_hEventLog = nullptr;
    }

    delete g_configs;
    
    if (g_mmdb.filename != nullptr)
    {
        MMDB_close(&g_mmdb);
    }
        
}

std::string GetClientIP(IHttpContext* pHttpContext , bool isBehindProxy) {
    
    if (isBehindProxy) {
        PCSTR psxRealIP;
        USHORT ccxRealIP;
        
        psxRealIP = pHttpContext->GetRequest()->GetHeader("X-Real-IP", &ccxRealIP);
        if (ccxRealIP != 0) {
            LogDebug(L"X-Real-IP header detected.");
            return std::string(psxRealIP);
        }

        PCSTR psxForwardedFor;
        USHORT ccxForwardedFor;

        psxForwardedFor = pHttpContext->GetRequest()->GetHeader("X-Forwarded-For", &ccxForwardedFor);
        if (ccxForwardedFor != 0) {
            LogDebug(L"X-Forwarded-For header detected.");
            // Extract the first IP address in the list (if multiple proxies are involved)
            std::string xForwardedFor = std::string(psxForwardedFor);
            size_t pos = xForwardedFor.find(',');
            if (pos != std::string::npos) {
                return trim(xForwardedFor.substr(0, pos));
            }
            return xForwardedFor;
        }


        PCSTR psxForwarded;
        USHORT ccxForwarded;

        psxForwarded = pHttpContext->GetRequest()->GetHeader("Forwarded" ,&ccxForwarded);
        if (ccxForwarded != 0 ) {
            LogDebug(L"Forwarded header detected.");
            std::string xForwarded = std::string(psxForwarded);
            // Parse 'for=' part from the Forwarded header
            size_t pos = xForwarded.find("for=");
            if (pos != std::string::npos) {
                pos += 4; // Move past "for="
                size_t end = xForwarded.find(';', pos);
                return xForwarded.substr(pos, end - pos);
            }
        }

        PCSTR psxClientIP;
        USHORT ccxClientlIP;

        psxClientIP = pHttpContext->GetRequest()->GetHeader("X-Client-IP", &ccxClientlIP);
        if (ccxClientlIP != 0) {
            LogDebug(L"X-Client-IP header detected.");
            return std::string(psxClientIP);
        }


        PCSTR psCFIP;
        USHORT ccCFIP;

        psCFIP = pHttpContext->GetRequest()->GetHeader("CF-Connecting-IP", &ccCFIP);
        if (ccCFIP != 0) {
            LogDebug(L"CF-Connecting-IP header detected.");
            return std::string(psCFIP);
        }
       
        PCSTR psTrueClientIP;
        USHORT ccTrueClientIP;

        psTrueClientIP = pHttpContext->GetRequest()->GetHeader("True-Client-IP", &ccTrueClientIP);
        if (ccTrueClientIP != 0) {
            LogDebug(L"True-Client-IP header detected.");
            return std::string(psTrueClientIP);
        }
    }

    
    const SOCKADDR* pSockAddr = pHttpContext->GetRequest()->GetRemoteAddress();
    if (pSockAddr == nullptr) {
        return "";
    }

    char ipStr[INET6_ADDRSTRLEN] = { 0 };

    if (pSockAddr->sa_family == AF_INET) {
        const SOCKADDR_IN* pSockAddrIn = reinterpret_cast<const SOCKADDR_IN*>(pSockAddr);
        inet_ntop(AF_INET, &pSockAddrIn->sin_addr, ipStr, sizeof(ipStr));
    }
    else if (pSockAddr->sa_family == AF_INET6) {
        const SOCKADDR_IN6* pSockAddrIn6 = reinterpret_cast<const SOCKADDR_IN6*>(pSockAddr);
        inet_ntop(AF_INET6, &pSockAddrIn6->sin6_addr, ipStr, sizeof(ipStr));
    }

    return std::string(ipStr);
}

REQUEST_NOTIFICATION_STATUS TagRequest(IHttpRequest* pHttpRequest , std::string& countryISO)
{
    pHttpRequest->SetHeader(
        "X-COUNTRY",
        countryISO.c_str(),
        (USHORT)countryISO.length(),
        TRUE);

    LogDebug(L"Set country header to " + std::wstring(countryISO.begin(), countryISO.end()));
        
    return RQ_NOTIFICATION_CONTINUE;
}

REQUEST_NOTIFICATION_STATUS UnknownRequest(IHttpRequest* pHttpRequest)
{
    pHttpRequest->SetHeader("X-COUNTRY", "unknown", (USHORT)strlen("unknown"), TRUE);
    LogDebug(L"Client IP not found, setting country to unknown.");
    return RQ_NOTIFICATION_CONTINUE;
}

REQUEST_NOTIFICATION_STATUS BlockRequest(IHttpResponse* pHttpResponse , IHttpEventProvider* pProvider) {
    LogDebug(L"Blocking Request.");
    
    DWORD cbSent = 0;// Buffer to store the byte count.
    BOOL fCompletionExpected = false; // Buffer to store if asyncronous completion is pending.
    
    pHttpResponse->SetStatus(451, "Unavailable For Legal Reasons");
    
    HRESULT hr = pHttpResponse->Flush(false, true, &cbSent, &fCompletionExpected);
    if (FAILED(hr)) pProvider->SetErrorStatus(hr);
    
    //pHttpResponse->Clear();

    return RQ_NOTIFICATION_FINISH_REQUEST;    
}



bool IsStringMatchedInList(const std::string& str, const std::wstring& list)
{
    if (list.length() == 0) return false;
    
    std::wstring wstr = stringToWstring(str);      
    std::wstring pattern = std::wstring(L"|" + wstr + L"|");
    std::wstring modifiedExceptionList = L"|" + list + L"|";

    bool result = modifiedExceptionList.find(pattern) != std::string::npos;
  
    LogDebug(
        L"Matching result: looking " +
        wstr +
        L" inside list " +
        list +
        L" results in " +
        (result ? L"True" : L"False"));
   
    return result;
}

bool IsIpInExceptionList(const std::string& ip, const std::wstring& exceptionList) {
    return IsStringMatchedInList(ip, exceptionList);
}

bool IsCountryMatch(const std::string& countryISO, const std::wstring& countryList) {
    return IsStringMatchedInList(countryISO, countryList);
}


class CGeoLocationIIS : public CHttpModule
{    

public:    

    REQUEST_NOTIFICATION_STATUS
        OnBeginRequest(
            IN IHttpContext* pHttpContext,
            IN IHttpEventProvider* pProvider
        )
    {
        UNREFERENCED_PARAMETER(pProvider);
        HRESULT hr = E_UNEXPECTED;

		try
		{
            if (!g_bDataLoaded)
            {
                hr = LoadData(pHttpContext, g_configs);
                if(FAILED(hr)) return RQ_NOTIFICATION_CONTINUE;
                g_bDataLoaded = true;
            }            

			if(!g_configs->isEnabled()) // module is not enable
                return RQ_NOTIFICATION_CONTINUE;

			
			std::string clientIP = GetClientIP(pHttpContext , g_configs->isBehindProxy());

            IHttpRequest* pHttpRequest = pHttpContext->GetRequest();
            if (pHttpRequest == nullptr) {
                LogError(L"Unable to get http request");
                return RQ_NOTIFICATION_CONTINUE;
            }

            IHttpResponse* pHttpResponse = pHttpContext->GetResponse();
            if (pHttpResponse == nullptr) {
                LogError(L"Unable to get http response");
                return RQ_NOTIFICATION_CONTINUE;
            }

			if (clientIP.empty()) {
                LogError(L"Client IP not found.");                
                return UnknownRequest(pHttpRequest);
			}

            LogDebug(L"Detected Ip address is :" + stringToWstring(clientIP));
#if _DEBUG
            pHttpRequest->SetHeader("X-IP", clientIP.c_str(), (USHORT)clientIP.length(), TRUE);
#endif              
            bool bIsIpException = IsIpInExceptionList(clientIP, g_configs->getExceptionIPs());

			int gai_error, mmdb_error;
            std::lock_guard<std::mutex> lock(g_mmdbMutex);
            MMDB_lookup_result_s result =
                MMDB_lookup_string(&g_mmdb, clientIP.c_str(), &gai_error, &mmdb_error);

            if (result.found_entry) {
                MMDB_entry_data_s entry_data;
                int status = MMDB_get_value(
                                &result.entry, 
                                &entry_data, 
                                "country", 
                                "iso_code", 
                                NULL);

                if (status == MMDB_SUCCESS && entry_data.has_data) {
                    std::string countryISO(entry_data.utf8_string, entry_data.data_size);
                    
                    bool bCountryMatch = IsCountryMatch(countryISO, g_configs->getCountryList());
                    
                    ModuleMode mode = g_configs->getModeEnum();

                    if (mode == ModuleMode::Tag || bIsIpException) {                        
                        return TagRequest(pHttpRequest, countryISO);
                    }

                    if (mode == ModuleMode::Block) {
                        return bCountryMatch ? BlockRequest(pHttpResponse, pProvider)
                            : TagRequest(pHttpRequest, countryISO);
                    }

                    if (mode == ModuleMode::Allow) {
                        return bCountryMatch ? TagRequest(pHttpRequest, countryISO)
                            : BlockRequest(pHttpResponse, pProvider);
                    }

                    LogWarning(L"Unpredicted logic.");
                    return BlockRequest(pHttpResponse, pProvider);
                    
                                        
                }
                else {
                    LogWarning(L"Country ISO code not found.");

                    if (g_configs->getModeEnum() == ModuleMode::Tag || bIsIpException) {
                        return UnknownRequest(pHttpRequest);
                    }
                    else {
                        return BlockRequest(pHttpResponse, pProvider);
                    } 
                }
            }
            else {
                LogWarning(L"GeoLocation lookup failed.");

                if (g_configs->getModeEnum() == ModuleMode::Tag || bIsIpException) {                    
                    return UnknownRequest(pHttpRequest);
                }
                else {
                    return BlockRequest(pHttpResponse, pProvider);
                }
                
            }

			return RQ_NOTIFICATION_CONTINUE;
        }
        catch (const std::exception& e)
        {
            std::wstring wideMessage = std::wstring(e.what(), e.what() + strlen(e.what()));
            LogError(L"Exception on request handling: " + wideMessage);
            //LogLastError(L"Associated Windows error:");
            return RQ_NOTIFICATION_CONTINUE;
        }
        
    }    
};


class CGeoLocationIISFactory : public IHttpModuleFactory
{
public:
    HRESULT
        GetHttpModule(
            OUT CHttpModule** ppModule,
            IN IModuleAllocator* pAllocator
        )
    {
        UNREFERENCED_PARAMETER(pAllocator);

        // Create a new instance.
        CGeoLocationIIS* pModule = new CGeoLocationIIS;

        // Test for an error.
        if (!pModule)
        {
            // Return an error if the factory cannot create the instance.
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
        else
        {
            // Return a pointer to the module.
            *ppModule = pModule;
            pModule = nullptr;
            // Return a success status.
            return S_OK;
        }
    }

    void 
        Terminate() 
    {
        LogInfo(L"Terminating module.");
        CleanupModule();
        delete this;
    }
};

// Create the module's exported registration function.
HRESULT
__stdcall
RegisterModule(
    DWORD dwServerVersion,
    IHttpModuleRegistrationInfo* pModuleInfo,
    IHttpServer* pHttpServer
)
{
    UNREFERENCED_PARAMETER(dwServerVersion);    

    HRESULT hr;

    if (pModuleInfo == nullptr || pHttpServer == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        return hr;
    }

    g_pHttpServer = pHttpServer;

    // Initialize the module
    hr = InitializeModule();
    if (FAILED(hr)) {
        return hr;
    }

    

    // Set the request notifications and exit.
    hr = pModuleInfo->SetRequestNotifications(
        new CGeoLocationIISFactory,
        RQ_BEGIN_REQUEST,
        0
    );

    if (FAILED(hr)) {        
        LogError(L"Failed to register GeoLocation module.");
        CleanupModule();
        return hr;
    }

    hr = pModuleInfo->SetPriorityForRequestNotification(RQ_BEGIN_REQUEST, PRIORITY_ALIAS_FIRST);
    //hr = pModuleInfo->SetPriorityForRequestNotification(RQ_SEND_RESPONSE, PRIORITY_ALIAS_LAST);

    if (FAILED(hr)) {
        LogError(L"Failed to register GeoLocation module priority.");
        CleanupModule();
        return hr;
    }

    return hr;
}