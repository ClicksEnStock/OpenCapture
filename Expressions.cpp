
#include "Common.h"

TCHAR * Extension::LastError()
{
	_tcscpy(errorTemp, lastError);
	_snwprintf_s(lastError, _countof(lastError), ERRORSIZE, ErrorS[noError]);
	return  errorTemp;
}

int Extension::ColorPosX(int colorId)
{
	if(colorId==1){
	
		return GetColor1Pos()[0];
	}
	else if(colorId==2){

		return GetColor2Pos()[0];
	}
	else{
		SetLastError(ErrorS[colorPosXIdInvalid]);
		return -1000;
	}
}

int Extension::ColorPosY(int colorId)
{
	if(colorId==1){
	
		return GetColor1Pos()[1];
	}
	else if(colorId==2){

		return GetColor2Pos()[1];
	}
	else{
		SetLastError(ErrorS[colorPosYIdInvalid]);
		return -1000;
	}
}

int Extension::ColorElementHSVOnPos(int posX, int posY, int index)
{
	try
	 {
		int colorE = INVALIDVALUE;
		if(GetOriginalImageHSV().empty())
		{
				SetOriginalImageHSV();
		}
		if(GetChangeDevice()==false && index>0 && index<=3 &&
			posX <= GetOriginalImageHSV().cols && posY <= GetOriginalImageHSV().rows)
		{
			Vec3b bgrPixel = GetOriginalImageHSV().at<Vec3b>(posY, posX);
			//Scalar scol =cvGet2D(originalImageHSV,posY,posX);
			colorE = bgrPixel.val[index-1];
		}
		return colorE;
	}
	catch(exception ex)
	{
		SetLastError(ErrorS[HSVColorOnPos]);
		return 0;
	}
}

//int Extension::ColorElementHSVOnPos(int posX, int posY, int index)
//{
//	try
//	 {
//		int r,g,b;
//		r = ColorElementRGBOnPos(posX, posY, 1);
//		g = ColorElementRGBOnPos(posX, posY, 2);
//		b = ColorElementRGBOnPos(posX, posY, 3);
//		return RGB2HSVColor(r,g,b)[index];
//	}
//	catch(exception ex)
//	{
//		_snprintf_s(lastError, _countof(lastError),ERRORSIZE,"[10] Encounter problem by getting HSV color on current frame (%s)",ex.what());
//	}
//}

int Extension::ColorElementRGBOnPos(int posX, int posY, int index)
{
	 try
	 {
		int colorE = INVALIDVALUE;
		if(GetChangeDevice()==false && !GetOriginalImage().empty() && index>0 && index<=3 &&
			posX <=GetOriginalImage().cols && posY <= GetOriginalImage().rows)
		{
			Vec3b bgrPixel = GetOriginalImage().at<Vec3b>(posY, posX);
			//CvScalar scol =cvGet2D(originalImage,posY,posX);
			colorE = bgrPixel.val[3-index];
		}
		return colorE;
	}
	catch(exception ex)
	{
		SetLastError(ErrorS[RGBColorOnPos]);
		return 0;
	}
}

int Extension::ColorObjectWidth(int colorId)
{
	if(colorId==1)
	{
		return GetColor1Size()[0];
	}
	else if(colorId==2)
	{
		return GetColor2Size()[0];
	}
	else
	{
		return INVALIDVALUE;
	}
}

int Extension::ColorObjectHeight(int colorId)
{
	if(colorId==1)
	{
		return GetColor1Size()[1];
	}
	else if(colorId==2)
	{
		return GetColor2Size()[1];
	}
	else
	{
		return INVALIDVALUE;
	}
}

int Extension::ObjectPosX()
{
	return GetObjectPos()[0];
}

int Extension::ObjectPosY()
{
	return GetObjectPos()[1];
}

int Extension::ObjectWidth()
{
	return GetObjectSize()[0];
}

int Extension::ObjectHeight()
{
	return GetObjectSize()[1];
}

int Extension::FeatureMatches()
{
	return GetFeatureNumberOfGoodMatches();
}
	
int Extension::FeatureCornerPosX(int cornerIndex)
{
	if(cornerIndex>=1 && cornerIndex<=4)
	{
		return ((int)GetFeatureCorner(cornerIndex-1).x);
	}
	else
	{
		SetLastError(ErrorS[featureCornerInvalidIndex]);
		return 0;
	}
}
	
int Extension::FeatureCornerPosY(int cornerIndex)
{
	if(cornerIndex>=1 && cornerIndex<=4)
	{
		return ((int)GetFeatureCorner(cornerIndex-1).y);
	}
	else
	{
		SetLastError(ErrorS[featureCornerInvalidIndex]);
		return 0;
	}
}

TCHAR * Extension::QRCode()
{
	_tcscpy(QRCodeTemp, QRCodeText);
	_snwprintf_s(QRCodeText, _countof(QRCodeText), 1024, _T(""));
	return  QRCodeTemp;
}