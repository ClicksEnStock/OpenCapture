#pragma once

// #define TGFEXT       // TGF2, MMF2, MMF2 Dev
   #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only

//#include "opencv2/objdetect/objdetect.hpp"
//#include "opencv2/core/core.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/legacy/blobtrack.hpp"
#include <windows.h>
#include "atlstr.h"
#include "opencv2/opencv.hpp"

//ZXing
#include "ReadBarcode.h"
#include "Error.h"

//#include <fstream>
#include <tinythread.h>
#include <fast_mutex.h>
#include <CameraCalibration.hpp>
#include <GeometryTypes.hpp>
using namespace tthread;
using namespace cv;
using namespace std;
using namespace ZXing;
/*using namespace oned;
using namespace datamatrix;
using namespace qrcode;*/

#include    "Edif.h"
#include	"Resource.h"

enum ErrorN
{
	noError,
	captureAlreadyStarted,
	displayImage,
	startTrackingColor,
	stopTrackingColor,
	createVideo,
	createVideoCodec,
	noColorFound,
	changingCameraDevice,
	saveImage,
	HSVColorOnPos,
	RGBColorOnPos,
	colorPosXIdInvalid,
	colorPosYIdInvalid,
	loadingClassifier,
	colorAlreadyTracked,
	objectAlreadyTracked,
	featureAlreadyTracked,
	featureCornerInvalidIndex,
	changeResImpossible,
	changeResException,
	setSizeException,
	setPropertyError,
	QRCodeAlreadyTracked,
};

static const TCHAR* ErrorS[24] = {
	_T("[00] No Error"),
	_T("[01] Capture is already started."),
	_T("[02] Encountered problem by displaying camera image:"),
	_T("[03] Cannot start tracking color, color id is invalid"),
	_T("[04] Cannot stop tracking color, color id is invalid"),
	_T("[05] Cannot create video with the specified fps"),
	_T("[06] Cannot create video with the specified codec id"),
	_T("[07] Track color: Cannot find color on image"),
	_T("[08] Encountered problem by changing camera device:"),
	_T("[09] Save Image Error:"),
	_T("[10] Encounter problem by getting HSV color on current frame"),
	_T("[11] Encounter problem by getting RGB color on current frame"),
	_T("[12] ColorPosX: Cannot get color pos X, color id is invalid"),
	_T("[13] ColorPosY: Cannot get color pos Y, color id is invalid"),
	_T("[14] Error loading classifier"),
	_T("[15] Color is already tracked."),
	_T("[16] Object is already tracked."),
	_T("[17] Feature is already tracked."),
	_T("[18] Cannot get feature corner position, invalid index."),
	_T("[19] Cannot change resolution, camera device is not opened."),
	_T("[20] An error occured by changing the camera resolution."),
	_T("[21] An error occured by setting the preview size."),
	_T("[22] An error occured by setting the camera property."),
	_T("[23] QR Code is already tracked.")
};

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files

struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// Object's data
	short			swidth;
	short			sheight;
	bool			isFlipped;
	COLORREF		backgroundColor;
	bool			isDebugShowTracking;
	

};

#include "Extension.h"
