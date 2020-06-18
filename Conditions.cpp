
#include "Common.h"

bool Extension::IsDeviceInitializing()
{
	return (GetChangeDevice());
}

bool Extension::IsDeviceRunning()
{
	if(GetCapture()!=NULL)
	{
		return (!GetChangeDevice() && GetCapture()->isOpened() && GetIsCaptureEnabled());
	}
	else{
		return false;
	}
}

bool Extension::HasError()
{
	return ((wcscmp(lastError, ErrorS[noError])) != 0);
}

bool Extension::OnQRCodeFound()
{
	return ((wcscmp(QRCodeText, _T(""))) != 0);
}