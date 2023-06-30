#include "CameraEnumerator.h"

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

std::map<int, Camera> CameraEnumerator::getCameraMap()
{
	std::map<int, Camera> deviceMap;

    IMFMediaSource* pSource = NULL;
    IMFAttributes* pAttributes = NULL;
    IMFActivate** ppDevices = NULL;
    UINT32 count = 0;
    // Create an attribute store to specify the enumeration parameters.
    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if (FAILED(hr))
    {
        SafeRelease(&pAttributes);
        return deviceMap;
    }

    // Source type: video capture devices
    hr = pAttributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );
    
    if (FAILED(hr))
    {
        SafeRelease(&pAttributes);
        return deviceMap;
    }

    // Enumerate devices.
    hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
    if (!FAILED(hr) && count!=0)
    {
        LPWSTR deviceName = NULL;
        Camera currentDevice;

        for (UINT32 i = 0; i < count; i++)
        {
            if (ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &deviceName, NULL) == S_OK)
            {
                currentDevice.id = i;
                currentDevice.deviceName = deviceName;
                deviceMap[i] = currentDevice;
            }
        }
    }
    else {
        if (pAttributes != NULL) {
            SafeRelease(&pAttributes);
        }
        if (ppDevices != NULL && count>0) {
            for (DWORD i = 0; i < count; i++)
            {
                SafeRelease(&ppDevices[i]);
            }
        }
        CoTaskMemFree(ppDevices);
        SafeRelease(&pSource);
    }
    return deviceMap;
}


