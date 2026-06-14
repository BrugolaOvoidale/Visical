#ifdef _WIN32
#include "DirectShow.hpp"
#include <dshow.h>


std::vector<std::string> DirectShow::listDeviceIds()
{
    std::vector<std::string> ids;

    // Initialize COM
    CoInitialize(nullptr);

    ICreateDevEnum* pDevEnum = nullptr;
    IEnumMoniker* pEnum = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr,
        CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum,
        (void**)&pDevEnum);

    if (SUCCEEDED(hr) && pDevEnum)
    {
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

        if (hr == S_OK && pEnum)  // Only proceed if enumerator exists
        {
            IMoniker* pMoniker = nullptr;
            while (pEnum->Next(1, &pMoniker, nullptr) == S_OK)
            {
                LPOLESTR displayName = nullptr;
                if (SUCCEEDED(pMoniker->GetDisplayName(nullptr, nullptr, &displayName)) && displayName)
                {
                    int size_needed = WideCharToMultiByte(CP_UTF8, 0, displayName, -1, nullptr, 0, nullptr, nullptr);

                    std::string id(size_needed - 1, '\0');

                    WideCharToMultiByte(CP_UTF8, 0, displayName, -1, &id[0], size_needed, nullptr, nullptr);

                    ids.push_back(id);

                    CoTaskMemFree(displayName);
                }
                pMoniker->Release();
            }
            pEnum->Release();
        }
        // If hr == S_FALSE, no devices found, just return empty vector
        pDevEnum->Release();
    }

    CoUninitialize();
    return ids;
}
#endif
