


class Extension
{
public:

    LPRDATA rdPtr;
    LPRH    rhPtr;

    Edif::Runtime Runtime;

    static const int MinimumBuild = 251;
    static const int Version = 1;

	static const int OEFLAGS =	OEFLAG_SPRITES|OEFLAG_BACKSAVE|OEFLAG_MOVEMENTS;
	static const int OEPREFS =	OEPREFS_BACKSAVE;

	static const int WindowProcPriority = 100;

	static const int INVALIDVALUE = -100;
	static const int ERRORSIZE = 1000;

    Extension(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr);
    ~Extension();
	void StartChangeDeviceThread(int id);
	void StartCapturingThread();
	void StartColorTrackThread();
	void StartObjectTrackThread();
	void StartFeatureTrackThread();
	void StartQRCodeTrackThread();
	void StopAllThreads();
	void DrawRectangleOnImage(int posX,int posY,int width,int height);
	void DrawEllipseOnImage(Rect faceRect);
	void UpdateOriginalImage(Mat image);
	void CopyOriginalImageToSurface();
	void ExtractBlobFromImage(Mat image, int colorId);
	void ShowBackground();
	void DrawFeatureOnImage();
	int* RGB2HSVColor(int r, int g, int b);
	Transformation FindPose3D(vector< Point2f > scene_points,vector< Point2f > points2d, CameraCalibration calibration);
	void decode_image(Mat &image);
	Rect DetectBarcodeFromImage(Mat image);

	// Thread Safe Properties
	//Getters
	VideoCapture*	GetCapture();
	VideoWriter		GetVideoWriter();
	bool			GetChangeDevice();
	int				GetDeviceId();
	TCHAR*			GetRawImagePath();
	Mat				GetOriginalImage();
	Mat				GetOriginalImageHSV();
	Mat				GetOriginalImageGRAY();
	TCHAR*			GetLastError();
	bool			GetIsColor1Tracked();
	bool			GetIsColor2Tracked();
	int*			GetColor1Range();
	int*			GetColor1Pos();
	int*			GetColor1Size();
	int*			GetColor2Range();
	int*			GetColor2Pos();
	int*			GetColor2Size();
	bool			GetIsCaptureEnabled();
	bool			GetIsCaptureDisplayed();
	COLORREF		GetBackgroundColor();
	CascadeClassifier GetObjectCascade();
	bool			GetIsObjectTracked();
	int*			GetObjectPos();
	int*			GetObjectSize();
	bool			GetIsFeatureTracked();
	Mat				GetFeatureObjectImage();
	int				GetFeatureNumberOfGoodMatches();
	Point2f			GetFeatureCorner(int cornerId);
	TCHAR*			GetQRCodeText();
	bool		    GetIsQRCodeTracked();
	bool			GetIsQRCodeAutoDetected();
	bool			GetIsQRCodeSharpened();

	//Setters
	void SetCaptureDevice(int deviceId);
	void SetVideoWriter(TCHAR* filename,int codec, int fps);
	void SetChangeDevice(bool status);
	void SetDeviceId(int id);
	void SetRawImagePath(TCHAR* filename);
	void SetOriginalImage(Mat image);
	void FlipOriginalImage();
	void SwitchIsImageFlipped();
	void SetOriginalImageHSV();
	void SetOriginalImageGRAY();
	void SetLastError(const TCHAR* error);
	void SetIsColor1Tracked(bool status);
	void SetIsColor2Tracked(bool status);
	void SetColorPos(int colorId, int index, int value);
	void SetColorSize(int colorId, int index, int value);
	void SetColorRange(int colorId, int index, int value);
	void SetIsCaptureEnabled(bool status);
	void SetIsCaptureDisplayed(bool status);
	void SetBackgroundColor(COLORREF color);
	void SetObjectCascade(TCHAR* filename);
	void SetIsObjectTracked(bool status);
	void SetObjectPos(int index, int value);
	void SetObjectSize(int index, int value);
	void SetIsFeatureTracked(bool status);
	void SetFeatureObjectImage(TCHAR* filename);
	void SetFeatureNumberOfGoodMatches(int matchesN);
	void SetFeatureObjectCorners(vector<Point2f> corners);
	void SetQRCodeText(TCHAR* text);
	void SetIsQRCodeTracked(bool status);
	void SetIsQRCodeAutoDetected(bool status);
	void SetIsQRCodeSharpened(bool status);
	
    // Camera Data members
	VideoCapture* capture;
	VideoWriter videoW;
	bool changeDevice;
	int deviceId;
	cSurface*		rawImageSurface;
	TCHAR rawImagePath[1024];
	Mat originalImage;
	Mat originalImageHSV;
	Mat imageThreshed;
	Mat originalImageGRAY;
	TCHAR lastError[ERRORSIZE];
	TCHAR errorTemp[ERRORSIZE];
	bool isColor1Tracked;
	bool isColor2Tracked;
	int color1Range[6];//1st color in HSV format
	int color1Pos[2];
	int color1Size[2];
	int color2Range[6];//2nd color in HSV format
	int color2Pos[2];
	int color2Size[2];
	bool isCaptureEnabled;
	bool isCaptureDisplayed;
	bool isImageFlipped;
	int orientation;
	bool isDebugShowTracking;
	COLORREF backgroundColor;
	CascadeClassifier objectCascade;
	thread* changeDeviceThread;
	thread* captureThread;
	thread* trackColorThread;
	thread* trackObjectThread;
	thread* trackFeatureThread;
	thread* trackQRCodeThread;
	fast_mutex cameraMutex;
	bool isObjectTracked;
	int objectPos[2];
	int objectSize[2];
	bool isFeatureTracked;
	Mat featureObjectImage;
	Mat featureMatches;
	FeatureDetector * detector;
	DescriptorExtractor * extractor;
	DescriptorMatcher* matcher;
	int numberOfGoodMatches;
	vector< Point2f > featureObjectCorners;
	TCHAR QRCodeText[1024];
	TCHAR QRCodeTemp[1024];
	bool isQRCodeTracked;
	bool isQRCodeAutoDetectionEnabled;
	bool isQRCodeSharpeningEnabled;
	CameraEnumerator camEnum;
	std::map<int, Camera> cameraList;
			
    /*  Add your actions, conditions and expressions as real class member
        functions here. The arguments (and return type for expressions) must
        match EXACTLY what you defined in the JSON.

        Remember to link the actions, conditions and expressions to their
        numeric IDs in the class constructor (Extension.cpp)
    */

    /// Actions
	void ChooseDevice(int deviceId);
	void StartCapture();
	void StopCapture();
	void StartPreview();
	void StopPreview();
    void SetColor(int colorId,int H1,int H2,int S1, int S2, int V1, int V2);
	void EnableTrackColor(int colorId);
	void DisableTrackColor(int colorId);
	void SaveImage(TCHAR* filename);
	void SaveImageAtResolution(TCHAR* filename);
	void StartSavingVideoStream(TCHAR* filename, int fps, int codecId);
	void StopSavingVideoStream();
	void ChangeBackgroundColor(COLORREF color);
	void LoadObjectClassifier(TCHAR* filename);
	void EnableTrackObject();
	void DisableTrackObject();
	void LoadFeatureObject(TCHAR* filename);
	void EnableTrackFeature();
	void DisableTrackFeature();
	void MirrorImage();
	void SetOrientation(int type);
	void SetResolution(int width, int height);
	void SetSize(int width, int height);
	void SetCameraProperty(TCHAR* propertyName, float value);
	void StartQRCodeTracking();
	void StopQRCodeTracking();
	void EnableQRCodeAutoDetection();
	void DisableQRCodeAutoDetection();
	void EnableQRCodeSharpening();
	void DisableQRCodeSharpening();
	void EnumerateCameras();

    /// Conditions
	bool IsDeviceInitializing();
    bool IsDeviceRunning();
	bool HasError();
	bool OnQRCodeFound();

    /// Expressions
    TCHAR * LastError();
	int ColorPosX(int colorId);
	int ColorPosY(int colorId);
	int ColorElementHSVOnPos(int posX, int posY, int index);
	int ColorElementRGBOnPos(int posX, int posY, int index);
	COLORREF ColorRGBOnPos(int posX, int posY);
	int ColorObjectWidth(int colorId);
	int ColorObjectHeight(int colorId);
	int ObjectPosX();
	int ObjectPosY();
	int ObjectWidth();
	int ObjectHeight();
	int FeatureMatches();
	int FeatureCornerPosX(int cornerIndex);
	int FeatureCornerPosY(int cornerIndex);
	TCHAR * QRCode();
	LPCWSTR CameraName(int deviceId);
	int NumberOfAvailableCameras();
		
    /* These are called if there's no function linked to an ID */

    void Action(int ID, LPRDATA rdPtr, long param1, long param2);
    long Condition(int ID, LPRDATA rdPtr, long param1, long param2);
    long Expression(int ID, LPRDATA rdPtr, long param);
	
    /*  These replace the functions like HandleRunObject that used to be
        implemented in Runtime.cpp. They work exactly the same, but they're
        inside the extension class.
    */

	short Handle();
    short Display();

    short Pause();
    short Continue();

    bool Save(HANDLE File);
    bool Load(HANDLE File);

};

