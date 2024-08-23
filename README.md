
# GeoLocation Filtering IIS Module

This is a native IIS module that utilizes [MaxMind](https://www.maxmind.com)'s GeoIP2 database to filter or tag HTTP requests based on geolocation. The module supports three working modes: **Tag**, **Allow**, and **Block**.

## Features

### Working Modes
1. **Tag**: 
   - Adds an `X-COUNTRY` HTTP header with the value set to the country's [ISO 3166-1 alpha-2 code](https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2).
   - This header can be used by your web application or other tools, such as IIS URL Rewrite, to make decisions based on the country code.
  
2. **Allow**:
   - Allows requests only from countries listed in the `countryList` property of the module.

3. **Block**:
   - Blocks requests from countries listed in the `countryList` property of the module.

### Exception IPs
- IP addresses provided in the `exceptionIPs` property will bypass the Allow or Block modes and will only have the `X-COUNTRY` header added.

### Debug Mode
- If the module is built in **Debug** mode, an additional header named `X-IP` will be added with the value of the detected IP address for testing and debugging purposes.

### Behind Proxy Support
- If your application server is behind a proxy, you can enable the `isBehindProxy` property. The module will then attempt to detect the client's IP address from the following standard HTTP headers:
  - `X-Forwarded-For` (de-facto standard)
  - `X-Client-IP`
  - `X-Cluster-Client-IP`
  - `X-Original-Forwarded-For` (used when `X-Forwarded-For` is modified or overwritten by a proxy)
  - `True-Client-IP` (used by some CDNs)
  - `CF-Connecting-IP` (used by Cloudflare)
  - `Fastly-Client-IP` (used by Fastly CDN)
  - `X-ProxyUser-IP`

## Installation

### Manual Installation
#### Prerequisites
- Ensure that IIS is installed and running on your server.
- This module was developed using Visual Studio 2022.

#### Steps
1. **Build the Project**: Compile the project to generate the module DLL.

2. **Install the Module**: 
2. - Copy `libGeoLocationIIS.dll` into `%SystemRoot%\SysWOW64\inetsrv` directory.
2. - Add the following sections to your IIS configuration file located at `%windir%\system32\inetsrv\config\applicationHost.config`:

    ```xml
    <sectionGroup name="system.webServer">
        <section name="IISGeoLocation" overrideModeDefault="Allow" allowDefinition="Everywhere" />
    </sectionGroup>

    <globalModules>
        <add name="GeoLocation IIS" image="%SystemRoot%\SysWOW64\inetsrv\libGeoLocationIIS.dll" preCondition="bitness64" />
    </globalModules>
    ```

3. **Copy XML Configuration**:
   - Copy the `IISGeoLocation.xml` file included in the root directory of the project to the following directory: `%windir%\system32\inetsrv\config\schema`.

4. **Restart IIS**: After configuring, restart IIS to apply the changes.

### Installation using installer
Installation using an installer is currently under development.

### Sample configuration

```xml
<configuration>
  <system.webServer>
    <IISGeoLocation 
      enabled="true" 
      databasePath="[Path to your geo database]" 
      mode="Allow"
      countryList="IR|US"
      exceptionIPs="127.0.0.1|1:::|192.168.1.100"
      isBehindProxy="true" />
  </system.webServer>
</configuration>
```

### Debugging
- In **Debug** mode, the module is verbose and can be used for diagnostic purposes.

## License

This project is licensed under the Mozilla Public License 2.0 (MPL 2.0).
