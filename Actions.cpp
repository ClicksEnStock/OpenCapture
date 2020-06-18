
#include "Common.h"

void Extension::ChooseDevice(int deviceId)
{
	if(deviceId>=0){
		StartChangeDeviceThread(deviceId);
	}
}

void Extension::StartCapture()
{
	SetIsCaptureEnabled(true);
	StartCapturingThread();
}

void Extension::StopCapture()
{
	SetIsCaptureEnabled(false);
}

void Extension::StartPreview()
{
	SetIsCaptureDisplayed(true);
}

void Extension::StopPreview()
{
	SetIsCaptureDisplayed(false);
}

void Extension::SetColor(int colorId,int H1,int S1, int V1, int H2,int S2, int V2)
{
	if(colorId==1){
		SetColorRange(1, 0, __max(__min(H1,179),0));
		SetColorRange(1, 1, __max(__min(S1,255),0));
		SetColorRange(1, 2, __max(__min(V1,255),0));
		SetColorRange(1, 3, __max(__min(H2,179),0));
		SetColorRange(1, 4, __max(__min(S2,255),0));
		SetColorRange(1, 5, __max(__min(V2,255),0));
	}
	else{
		SetColorRange(2, 0, __max(__min(H1,179),0));
		SetColorRange(2, 1, __max(__min(S1,255),0));
		SetColorRange(2, 2, __max(__min(V1,255),0));
		SetColorRange(2, 3, __max(__min(H2,179),0));
		SetColorRange(2, 4, __max(__min(S2,255),0));
		SetColorRange(2, 5, __max(__min(V2,255),0));
	}
}

void Extension::EnableTrackColor(int colorId)
{
	if (GetOriginalImage().empty())
	{
		return;
	}
	if(colorId==1){
	
		SetIsColor1Tracked(true);
	}
	else if(colorId==2){

		SetIsColor2Tracked(true);
	}
	else if(colorId==3){
		SetIsColor1Tracked(true);
		SetIsColor2Tracked(true);
	}
	else{
		SetLastError(ErrorS[startTrackingColor]);
	}
	if(GetIsColor1Tracked() || GetIsColor2Tracked())
	{
		StartColorTrackThread();
	}
}

void Extension::DisableTrackColor(int colorId)
{
	if(colorId==1){
	
		SetIsColor1Tracked(false);
	}
	else if(colorId==2){

		SetIsColor2Tracked(false);
	}
	else if(colorId==3){
		SetIsColor1Tracked(false);
		SetIsColor2Tracked(false);
	}
	else{
		SetLastError(ErrorS[stopTrackingColor]);
	}
}

void Extension::SaveImage(TCHAR* filename)
{
	try
	{
		int lg;
		LPSTR ps;
		UINT codePage = CP_ACP;
		lg = WideCharToMultiByte(codePage, 0, filename, -1, NULL, 0, NULL, NULL);
		ps = (LPSTR)malloc(lg*sizeof(char));
		WideCharToMultiByte(codePage, 0, filename, -1, ps, lg, NULL, NULL);

		if (!GetOriginalImage().empty()){
			imwrite(ps, GetOriginalImage());
		}
		free(ps);
	}
	catch (exception ex) {
		SetLastError(ErrorS[saveImage]);
	}
}

void Extension::SaveImageAtResolution(TCHAR* filename)
{
	try
	{
		SetRawImagePath(filename);
	}
	catch (exception ex) {
		SetLastError(ErrorS[saveImage]);
	}
}

void Extension::StartSavingVideoStream(TCHAR* filename, int fps, int codecId)
{
	if (!GetOriginalImage().empty())
	{
		if (GetVideoWriter().isOpened())
		{
			GetVideoWriter().release();
		}
		if (fps<0)
		{
			SetLastError(ErrorS[createVideo]);
			return;
		}
		switch (codecId)
		{
		case -1: // let the user choose the codecs
			SetVideoWriter(filename, -1, fps);
			break;
		case 0: // creates an uncompressed AVI file (the filename must have a .avi extension)
			SetVideoWriter(filename, 0, fps);
			break;
		case 1: // MPEG-1 codec
			SetVideoWriter(filename, CV_FOURCC('P', 'I', 'M', '1'), fps);
			break;
		case 2: // MPEG4 codec
			SetVideoWriter(filename, CV_FOURCC('D', 'I', 'V', 'X'), fps);
			break;
		case 3: // MPEG-4.2 codec
			SetVideoWriter(filename, CV_FOURCC('M', 'P', '4', '2'), fps);
			break;
		case 4: // H264
			SetVideoWriter(filename, CV_FOURCC('H', '2', '6', '4'), fps);
			break;
		case 5: //CYUV
			SetVideoWriter(filename, CV_FOURCC('C', 'Y', 'U', 'V'), fps);
			break;
		default:
			SetLastError(ErrorS[createVideoCodec]);
			break;
		}
	}
}

void Extension::StopSavingVideoStream()
{
	if(GetVideoWriter().isOpened())
	{
		GetVideoWriter().release();
	}
}

void Extension::ChangeBackgroundColor(COLORREF color)
{
	SetBackgroundColor(color);
}

void Extension::LoadObjectClassifier(TCHAR* filename)
{
	SetObjectCascade(filename);
	if (GetObjectCascade().empty())
	{
		SetLastError(ErrorS[loadingClassifier]);
	}
}

void Extension::EnableTrackObject()
{
	StartObjectTrackThread();
}

void Extension::DisableTrackObject()
{
	SetIsObjectTracked(false);
}

void Extension::LoadFeatureObject(TCHAR* filename)
{
	SetFeatureObjectImage(filename);
}

void Extension::EnableTrackFeature()
{
	StartFeatureTrackThread();
}

void Extension::DisableTrackFeature()
{
	SetIsFeatureTracked(false);
}

void Extension::MirrorImage()
{
	SwitchIsImageFlipped();
}

void Extension::SetOrientation(int type)
{
	if (type==0 || type==1)
	{
		orientation = type;
	}
}

void Extension::SetResolution(int width, int height)
{
	if(GetCapture() != NULL)
	{
		try
		{
			GetCapture()->set(CV_CAP_PROP_FRAME_WIDTH,width);
			GetCapture()->set(CV_CAP_PROP_FRAME_HEIGHT,height);
		}
		catch(exception ex)
		{
			SetLastError(ErrorS[changeResException]);
		}
	}
	else
	{
		SetLastError(ErrorS[changeResImpossible]);
	}
}

void Extension::SetSize(int width, int height)
{
	try
	{
		rdPtr->rHo.hoImgWidth = width;
		rdPtr->rHo.hoImgHeight = height;
	}
	catch(exception ex)
	{
		SetLastError(ErrorS[setSizeException]);
	}
}

void Extension::SetCameraProperty(TCHAR* propertyName, float value)
{
	/*
	ALL,
	FPS,
	BRIGHTNESS,
	CONTRAST,
	HUE,
	SATURATION,
	SHARPNESS,
	GAMMA,
	MONOCROME,
	WHITE_BALANCE_BLUE_U,
	BACKLIGHT,
	GAIN
	*/
	if (propertyName != NULL)
	{
		if (wcscmp(propertyName, _T("ALL")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_SETTINGS, 0))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("FPS")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_FPS, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("BRIGHTNESS")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_BRIGHTNESS, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("CONTRAST")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_CONTRAST, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("HUE")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_HUE, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("SATURATION")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_SATURATION, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("GAMMA")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_GAMMA, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("MONOCHROME")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_MONOCROME, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("WHITEBALANCE")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_WHITE_BALANCE_BLUE_U, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("BACKLIGHT")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_BACKLIGHT, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else if (wcscmp(propertyName, _T("GAIN")) == 0)
		{
			if (!GetCapture()->set(CV_CAP_PROP_GAIN, value))
			{
				SetLastError(ErrorS[setPropertyError]);
			}
		}
		else
		{
			SetLastError(ErrorS[setPropertyError]);
		}
	}
	else
	{
		SetLastError(ErrorS[setPropertyError]);
	}
}


void Extension::StartQRCodeTracking()
{
	StartQRCodeTrackThread();
}

void Extension::StopQRCodeTracking()
{

}

void Extension::EnableQRCodeAutoDetection()
{
	SetIsQRCodeAutoDetected(true);
}
void Extension::DisableQRCodeAutoDetection()
{
	SetIsQRCodeAutoDetected(false);
}
void Extension::EnableQRCodeSharpening()
{
	SetIsQRCodeSharpened(true);
}
void Extension::DisableQRCodeSharpening()
{
	SetIsQRCodeSharpened(false);
}