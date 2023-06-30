#include "Common.h"

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(LPRDATA _rdPtr, LPEDATA edPtr, fpcob cobPtr)
    : rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.hoAdRunHeader), Runtime(_rdPtr)
{
    /*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */

    LinkAction(0, ChooseDevice);
    LinkAction(1, StartCapture);
	LinkAction(2, StopCapture);
	LinkAction(3, StartPreview);
	LinkAction(4, StopPreview);
	LinkAction(5, SetColor);
	LinkAction(6, EnableTrackColor);
	LinkAction(7, DisableTrackColor);
	LinkAction(8, SaveImage);
	LinkAction(9, StartSavingVideoStream);
	LinkAction(10, StopSavingVideoStream);
	LinkAction(11, ChangeBackgroundColor);
	LinkAction(12, LoadObjectClassifier);
	LinkAction(13, EnableTrackObject);
	LinkAction(14, DisableTrackObject);
	LinkAction(15, LoadFeatureObject);
	LinkAction(16, EnableTrackFeature);
	LinkAction(17, DisableTrackFeature);
	LinkAction(18, MirrorImage);
	LinkAction(19, SetResolution);
	LinkAction(20, SetSize);
	LinkAction(21, SaveImageAtResolution);
	LinkAction(22, SetCameraProperty);
	LinkAction(23, StartQRCodeTracking);
	LinkAction(24, StopQRCodeTracking);
	LinkAction(25, EnableQRCodeAutoDetection);
	LinkAction(26, DisableQRCodeAutoDetection);
	LinkAction(27, EnableQRCodeSharpening);
	LinkAction(28, DisableQRCodeSharpening);
	LinkAction(29, SetOrientation);
	LinkAction(30, EnumerateCameras);

	
	LinkCondition(0, IsDeviceInitializing);
    LinkCondition(1, IsDeviceRunning);
	LinkCondition(2, HasError); 
	LinkCondition(3, OnQRCodeFound);

    LinkExpression(0, LastError);
	LinkExpression(1, ColorPosX);
	LinkExpression(2, ColorPosY);
	LinkExpression(3, ColorElementHSVOnPos);
	LinkExpression(4, ColorElementRGBOnPos);
	LinkExpression(5, ColorObjectWidth);
	LinkExpression(6, ColorObjectHeight);
	LinkExpression(7, ObjectPosX);
	LinkExpression(8, ObjectPosY);
	LinkExpression(9, ObjectWidth);
	LinkExpression(10,ObjectHeight);
	LinkExpression(11,FeatureMatches);
	LinkExpression(12,FeatureCornerPosX);
	LinkExpression(13,FeatureCornerPosY);
	LinkExpression(14, QRCode);
	LinkExpression(15, CameraName);
	LinkExpression(16, NumberOfAvailableCameras);

    /*
        This is where you'd do anything you'd do in CreateRunObject in the original SDK

        It's the only place you'll get access to edPtr at runtime, so you should transfer
        anything from edPtr to the extension class here.
    */	
	rdPtr->rHo.hoImgWidth = edPtr->swidth;
	rdPtr->rHo.hoImgHeight = edPtr->sheight;
	isImageFlipped=edPtr->isFlipped;
	backgroundColor = edPtr->backgroundColor;
	isDebugShowTracking = edPtr->isDebugShowTracking;

	capture=NULL;
	rawImageSurface	= NULL;
	changeDevice=false;
	deviceId=INVALIDVALUE;
	_snwprintf_s(rawImagePath, _countof(rawImagePath), 1024, _T(""));
	_snwprintf_s(QRCodeText, _countof(QRCodeText), 1024, _T(""));
	_snwprintf_s(QRCodeTemp, _countof(QRCodeTemp), 1024, _T(""));
	_snwprintf_s(lastError, _countof(lastError), ERRORSIZE, ErrorS[noError]);
	color1Pos[0]=INVALIDVALUE;
	color1Pos[1]=INVALIDVALUE;
	color1Size[0]=INVALIDVALUE;
	color1Size[1]=INVALIDVALUE;
	color2Pos[0]=INVALIDVALUE;
	color2Pos[1]=INVALIDVALUE;
	color2Size[0]=INVALIDVALUE;
	color2Size[1]=INVALIDVALUE;
	objectPos[0]=INVALIDVALUE;
	objectPos[1]=INVALIDVALUE;
	objectSize[0]=INVALIDVALUE;
	objectSize[1]=INVALIDVALUE;
	color1Range[0]=color1Range[1]=color1Range[2]=color1Range[3]=color1Range[4]=color1Range[5]=0;
	color2Range[0]=color2Range[1]=color2Range[2]=color2Range[3]=color2Range[4]=color2Range[5]=0;
	isCaptureEnabled=false;
	isCaptureDisplayed=false;
	isColor1Tracked=false;
	isColor2Tracked=false;
	isFeatureTracked=false;
	isQRCodeAutoDetectionEnabled = false;
	isQRCodeSharpeningEnabled = false;
	changeDeviceThread=NULL;
	captureThread = NULL;
	trackObjectThread=NULL;
	trackColorThread=NULL;
	trackFeatureThread=NULL;
	trackQRCodeThread = NULL;
	featureObjectCorners = vector<Point2f>(4);
	detector=NULL;
	extractor=NULL;
	matcher=NULL;
	orientation = 0;

	cameraList = camEnum.getCameraMap();
	/*HRSRC res = FindResource (hInstLib, MAKEINTRESOURCE (IDR_CAMERA_CLASSIFIER), _T("CAMERA"));
	storage = cvCreateMemStorage(0);
	size_t xmlSize = SizeofResource(hInstLib,res);
	char* xmlFile  = (char*) LockResource(LoadResource (hInstLib, res));
	*/
}

Extension::~Extension()
{
    /*
        This is where you'd do anything you'd do in DestroyRunObject in the original SDK.
        (except calling destructors and other such atrocities, because that's automatic in Edif)
    */
	StopAllThreads();
	delete changeDeviceThread;
	delete trackColorThread;
	delete trackObjectThread;
	delete trackFeatureThread;
	delete trackQRCodeThread;
	delete captureThread;
	if(rawImageSurface!=NULL)
	{
		delete rawImageSurface;
		rawImageSurface=NULL;
	}
	if(detector!=NULL)
	{
		delete detector;
		detector=NULL;
	}
	if(extractor!=NULL)
	{
		delete extractor;
		extractor=NULL;
	}
	if(matcher!=NULL)
	{
		delete matcher;
		matcher=NULL;
	}
	if(capture!=NULL)
	{
		delete capture;
		capture=NULL;
	}
}

short Extension::Handle()
{
    /* If your extension will draw to the MMF window you should first 
     check if anything about its display has changed :*/
	return REFLAG_DISPLAY;
}

short Extension::Display()
{
	
	try{
		// do not show capture if user do not want to
		if(!GetIsCaptureDisplayed())
		{
			ShowBackground();
			return 0;
		}
		LPSURFACE psw = WinGetSurface((int)rhPtr->rhIdEditWin);
		LPSURFACE ps = psw;
	
		if ( rawImageSurface == NULL )
		{
			LPSURFACE pProto;
			rawImageSurface = new cSurface;
			if ( GetSurfacePrototype(&pProto, ps->GetDepth(), ST_MEMORY, SD_DIB) )
			{
				rawImageSurface->Create(rdPtr->rHo.hoImgWidth, rdPtr->rHo.hoImgHeight, pProto);
			}
		}

		RECT destRect;
		destRect.bottom=rdPtr->rHo.hoY+rdPtr->rHo.hoImgHeight;
		destRect.left=rdPtr->rHo.hoX;
		destRect.right=rdPtr->rHo.hoX+rdPtr->rHo.hoImgWidth;
		destRect.top=rdPtr->rHo.hoY;

		//-------------------
		// BLIT
		//-------------------
		rawImageSurface->Blit(
			*psw,
			rdPtr->rHo.hoRect.left,
			rdPtr->rHo.hoRect.top,
			0,
			0,
			rdPtr->rHo.hoImgWidth,
			rdPtr->rHo.hoImgHeight,
			BMODE_OPAQUE,
			BOP_COPY,
			0);
		
		WinAddZone(rhPtr->rhIdEditWin,&destRect);

		//if(!featureMatches.empty())
		//{
		//	//line( featureMatches, featureObjectCorners[0] , featureObjectCorners[1],  Scalar(0, 255, 0), 2 ); //TOP line
		//	//line( featureMatches, featureObjectCorners[1] , featureObjectCorners[2],  Scalar(0, 255, 0), 2 );
		//	//line( featureMatches, featureObjectCorners[2] , featureObjectCorners[3],  Scalar(0, 255, 0), 2 );
		//	//line( featureMatches, featureObjectCorners[3] , featureObjectCorners[0] , Scalar(0, 255, 0), 2 );
		//	//imshow("matches", featureMatches);
		//}
	}
	catch(cv::Exception ex){
		SetLastError(ErrorS[displayImage]);
	}
	//rawImageSurface->Delete();
	// Ok
    return 0;
}

void Extension::ShowBackground()
{
	LPSURFACE ps = WinGetSurface((int)rhPtr->rhIdEditWin);
	if ( ps != NULL )
	{
		RECT    rc;

		int x = rc.left = rdPtr->rHo.hoX - rhPtr->rhWindowX;
		int y = rc.top = rdPtr->rHo.hoY - rhPtr->rhWindowY;
		int w = rdPtr->rHo.hoImgWidth;
		int h = rdPtr->rHo.hoImgHeight;
		rc.right = x + w;
		rc.bottom = y + h;

		ps->Fill(x, y, w, h, GetBackgroundColor());
								
		WinAddZone(rhPtr->rhIdEditWin, &rc);
	}
}

short Extension::Pause()
{

    // Ok
    return 0;
}

short Extension::Continue()
{

    // Ok
    return 0;
}

bool Extension::Save(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Save the object's data here

	    OK = true;

    #endif

	return OK;
}

bool Extension::Load(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Load the object's data here

	    OK = true;

    #endif

	return OK;
}


// These are called if there's no function linked to an ID

void Extension::Action(int ID, LPRDATA rdPtr, long param1, long param2)
{

}

long Extension::Condition(int ID, LPRDATA rdPtr, long param1, long param2)
{

    return false;
}

long Extension::Expression(int ID, LPRDATA rdPtr, long param)
{

    return 0;
}
