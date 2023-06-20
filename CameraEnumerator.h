#pragma once

#include <Windows.h>
#include <dshow.h>

#pragma comment(lib, "strmiids")

#include <map>
#include <string>

struct Camera {
	int id; // This can be used to open the device in OpenCV
	TCHAR* devicePath;
	TCHAR* deviceName; // This can be used to show the devices to the user
};

class CameraEnumerator {

public:

	CameraEnumerator() = default;
	std::map<int, Camera> getDevicesMap(const GUID deviceClass);
	std::map<int, Camera> getVideoDevicesMap();

};
