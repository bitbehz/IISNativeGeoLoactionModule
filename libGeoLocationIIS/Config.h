#pragma once
#include "pch.h"

enum ModuleMode
{
    Tag,
    Allow,
    Block,
    Undefiend
};

class Config {
public:
    // Constructor
    Config();

    Config(bool enabled, const std::wstring& databasePath, const std::wstring& mode,
        const std::wstring& countryList, const std::wstring& exceptionIPs, bool isBehindProxy);

    // Getters
    bool isEnabled() const;
    bool isBehindProxy() const;
    std::wstring getDatabasePath() const;
    std::wstring getMode() const;
    ModuleMode getModeEnum() const;
    static std::wstring ModeToWString(ModuleMode mode);
    std::wstring getCountryList() const;
    std::wstring getExceptionIPs() const;

    // Setters
    void setEnabled(bool enabled);
    void setIsBehindProxy(bool isBehindProxy);
    void setDatabasePath(const std::wstring& databasePath);
    void setMode(const std::wstring& mode);
    void setCountryList(const std::wstring& countryList);
    void setExceptionIPs(const std::wstring& exceptionIPs);

private:
    bool m_enabled;
    bool m_isBehindProxy;
    std::wstring m_databasePath;
    std::wstring m_mode;
    std::wstring m_countryList;
    std::wstring m_exceptionIPs;
};


HRESULT ReadConfigProperty(
    IAppHostElement* pElement,
    const wchar_t* propertyName,
    VARIANT& var,
    std::function<void(VARIANT&)> setConfigProperty);

