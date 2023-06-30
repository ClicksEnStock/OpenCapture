#pragma once

#include <Windows.h>
#include <mfidl.h>
#include <mfapi.h>

#pragma comment(lib, "strmiids")

#include <map>
#include <string>

struct Camera {
	int id; // This can be used to open the device in OpenCV
	LPWSTR deviceName; // This can be used to show the devices to the user
};

class CameraEnumerator {

public:

	CameraEnumerator() = default;
	std::map<int, Camera> getCameraMap();

};
