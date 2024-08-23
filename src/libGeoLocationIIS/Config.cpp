#include "pch.h"
#include "Config.h"

// Constructor
Config::Config():m_enabled(false),m_isBehindProxy(false) {}

Config::Config(
    bool enabled, 
    const std::wstring& databasePath, 
    const std::wstring& mode,
    const std::wstring& countryList, 
    const std::wstring& exceptionIPs , 
    bool isBehindProxy):
    m_enabled(enabled), 
    m_databasePath(databasePath), 
    m_mode(mode),
    m_countryList(countryList), 
    m_exceptionIPs(exceptionIPs) , 
    m_isBehindProxy(isBehindProxy) {}

// Getters
bool Config::isEnabled() const {
    return m_enabled;
}

bool Config::isBehindProxy() const
{
    return m_isBehindProxy;
}

std::wstring Config::getDatabasePath() const {
    return m_databasePath;
}

std::wstring Config::getMode() const {
    return m_mode;
}


ModuleMode Config::getModeEnum() const {
    std::wstring lowerStr = m_mode;    
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), towlower);  
    if (lowerStr == L"tag"){
        return ModuleMode::Tag;
    }
    else if (lowerStr == L"allow") {
        return ModuleMode::Allow;
    }
    else if (lowerStr == L"block") {
        return ModuleMode::Block;
    }

    return ModuleMode::Undefiend;
}

std::wstring Config::ModeToWString(ModuleMode mode) {
    switch (mode) {
    case ModuleMode::Tag:
        return L"Tag";
    case ModuleMode::Allow:
        return L"Allow";
    case ModuleMode::Block:
        return L"Block";
    case ModuleMode::Undefiend:
        return L"Undefiend";
    default:
        return L"Unknown";
    }
}

std::wstring Config::getCountryList() const {
    return m_countryList;
}

std::wstring Config::getExceptionIPs() const {
    return m_exceptionIPs;
}

// Setters
void Config::setEnabled(bool enabled) {
    m_enabled = enabled;
}

void Config::setIsBehindProxy(bool isBehindProxy)
{
    m_isBehindProxy = isBehindProxy;
}

void Config::setDatabasePath(const std::wstring& databasePath) {
    m_databasePath = databasePath;
}

void Config::setMode(const std::wstring& mode) {
    m_mode = mode;
}

void Config::setCountryList(const std::wstring& countryList) {
    m_countryList = countryList;
}

void Config::setExceptionIPs(const std::wstring& exceptionIPs) {
    m_exceptionIPs = exceptionIPs;
}


// Config helpers

HRESULT ReadConfigProperty(
    IAppHostElement* pElement,
    const wchar_t* propertyName,
    VARIANT& var,
    std::function<void(VARIANT&)> setConfigProperty) {

    BSTR bstrPropertyName = SysAllocString(propertyName);
    if (!bstrPropertyName) return E_OUTOFMEMORY;

    // Getting property by name
    IAppHostProperty* pProperty = nullptr;
    HRESULT hr = pElement->GetPropertyByName(bstrPropertyName, &pProperty);
    SysFreeString(bstrPropertyName);

    if (FAILED(hr)) return hr;

    // Initializing VARIANT and get the value
    VariantInit(&var);
    hr = pProperty->get_Value(&var);
    if (SUCCEEDED(hr)) setConfigProperty(var);

    // Cleaning up
    VariantClear(&var);
    pProperty->Release();

    return hr;
}
