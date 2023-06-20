#include "Common.h"

// THREADS
// Change Device Thread
void ChangeDeviceThread(LPVOID lpParam)
{
	Extension* ext;
	ext = ((Extension*)lpParam);
	try{
		ext->SetCaptureDevice(ext->GetDeviceId());
		ext->SetChangeDevice(false);
	}
	catch(exception ex){
		ext->SetLastError(ErrorS[changingCameraDevice]);
	}
}
// Capture Thread
void CaptureThread(LPVOID lpParam)
{
	Extension* ext;
	ext = ((Extension*)lpParam);

	for(;;)
	{
		if(ext->GetCapture()!=NULL && ext->GetIsCaptureEnabled())
		{
			if(ext->rdPtr->rs->rsFlags<=3)
			{
				continue;
			}
			if(!ext->GetChangeDevice())
			{
				Mat newRawImage;
				ext->GetCapture()->read(newRawImage);
				if(newRawImage.empty())
				{
					continue;
				}
				else if (wcscmp(ext->GetRawImagePath(), _T("")) != 0)
				{
					int lg;
					LPSTR ps;
					UINT codePage = CP_ACP;
					lg = WideCharToMultiByte(codePage, 0, ext->GetRawImagePath(), -1, NULL, 0, NULL, NULL);
					ps = (LPSTR)malloc(lg*sizeof(char));
					WideCharToMultiByte(codePage, 0, ext->GetRawImagePath(), -1, ps, lg, NULL, NULL);
					imwrite(ps, newRawImage);
					ext->SetRawImagePath((_TCHAR*)(""));
					free(ps);
				}
				if (ext->orientation == 1)//vertical orientation
				{
					transpose(newRawImage, newRawImage);
					flip(newRawImage, newRawImage, 1);
				}
	
				ext->UpdateOriginalImage(newRawImage);
				
				if(ext->isDebugShowTracking)
				{
					//Add debug infos on the image
					if(ext->GetIsColor1Tracked())
					{
						ext->DrawRectangleOnImage(ext->GetColor1Pos()[0],ext->GetColor1Pos()[1],ext->GetColor1Size()[0],ext->GetColor1Size()[1]);
					}
					if(ext->GetIsColor2Tracked())
					{
						ext->DrawRectangleOnImage(ext->GetColor2Pos()[0],ext->GetColor2Pos()[1],ext->GetColor2Size()[0],ext->GetColor2Size()[1]);
					}
					if(ext->GetIsObjectTracked())
					{
						Rect objRect;
						objRect.x=ext->GetObjectPos()[0]-ext->GetObjectSize()[0]/2;
						objRect.y=ext->GetObjectPos()[1]-ext->GetObjectSize()[1]/2;
						objRect.width=ext->GetObjectSize()[0];
						objRect.height=ext->GetObjectSize()[1];
						if(objRect.x>=0 && objRect.y>=0 && objRect.width>0 && objRect.height>0)
						{
							ext->DrawEllipseOnImage(objRect);
						}
					}
					if(ext->GetIsFeatureTracked())
					{
						ext->DrawFeatureOnImage();
					}
				}
				// copy image to the displayed surface
				if(ext->GetIsCaptureDisplayed())
				{
					ext->CopyOriginalImageToSurface();
				}
				// store image in stream
				if(ext->GetVideoWriter().isOpened())
				{
					ext->GetVideoWriter() << ext->GetOriginalImage();
				}
			}
		}
		else
		{
			break;
		}
	}
}
void Extension::UpdateOriginalImage(Mat image)
{
	//this function must be performed in one pass to avoid other thread to access originalImage before rezising or flipping
	lock_guard<fast_mutex> lock(cameraMutex);
	if (originalImage.empty() || originalImage.cols != rdPtr->rHo.hoImgWidth || originalImage.rows != rdPtr->rHo.hoImgHeight)
	{
		delete rawImageSurface;//delete display surface, it will be recreated with object size
		rawImageSurface = NULL;
		originalImage = Mat(cv::Size(rdPtr->rHo.hoImgWidth,rdPtr->rHo.hoImgHeight),CV_MAKETYPE(image.depth(), image.channels()));
	}
	resize(image,originalImage,originalImage.size(),0,0,INTER_LINEAR);

	if(originalImage.empty())
	{
		return;
	}
	if (isImageFlipped)
	{
		flip(originalImage,originalImage,1);
	}
}
void Extension::CopyOriginalImageToSurface()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(rawImageSurface &&
		rawImageSurface->GetWidth() == originalImage.cols &&
		rawImageSurface->GetHeight() == originalImage.rows)
	{
		for(int i=0;i<originalImage.rows;i++)
		{
			for(int j=0;j<originalImage.cols;j++) {
					int cols = originalImage.cols;
					int cn   = originalImage.channels();
					uchar* data    = (uchar*)originalImage.data;
					COLORREF color=RGB(data[i*cols*cn+j*cn+2],
						data[i*cols*cn+j*cn+1],
						data[i*cols*cn+j*cn+0]);// it is stored as BGR
					rawImageSurface->SetPixelFast(j,i,color);
				}
		}
	}
}
void Extension::DrawRectangleOnImage(int posX,int posY,int width,int height)
{
	cv::Point pt1,pt2;  
	pt1.x = posX-width/2;  
	pt1.y = posY-height/2;
	pt2.x = posX+width/2;  
	pt2.y = posY+height/2; 
	//Attach bounding rect to blob in orginal video input 
	cv::Mat img = GetOriginalImage();
	cv::rectangle(img,pt1,pt2,cvScalar(0, 0, 0, 0),1,8,0 );
}
void Extension::DrawEllipseOnImage(Rect faceRect)
{
	Point center( faceRect.x + faceRect.width*0.5, faceRect.y + faceRect.height*0.5 );
	ellipse( originalImage, center, cv::Size( faceRect.width*0.5, faceRect.height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
}
// Color Tracking Thread
void ColorTrackThread(LPVOID lpParam)
{
	Extension* ext;
	ext = ((Extension*)lpParam);
	
	for(;;)
	{
		if (ext->GetOriginalImage().empty())
		{
			break;
		}
		if(ext->GetIsColor1Tracked() || ext->GetIsColor2Tracked())
		{
			if (!ext->GetChangeDevice())
			{
				if(ext->imageThreshed.empty())
				{
					ext->imageThreshed = Mat(ext->GetOriginalImage().size(),CV_8UC1);
				}

				//store the original image in HSV format
				ext->SetOriginalImageHSV();
			
				//TO DO: Resize the captured image (width/4,height/4, this will reduce a lot of the noise and will speed up the processing time)
				// Start track color
				if(ext->GetIsColor1Tracked() 
					&& ext->GetColor1Range()[0]!=ext->INVALIDVALUE 
					&& ext->GetColor1Range()[1]!=ext->INVALIDVALUE 
					&& ext->GetColor1Range()[2]!=ext->INVALIDVALUE
					&& ext->GetColor1Range()[3]!=ext->INVALIDVALUE
					&& ext->GetColor1Range()[4]!=ext->INVALIDVALUE
					&& ext->GetColor1Range()[5]!=ext->INVALIDVALUE)
				{
					inRange(ext->originalImageHSV, Scalar(ext->GetColor1Range()[0], ext->GetColor1Range()[1], ext->GetColor1Range()[2]), Scalar(ext->GetColor1Range()[3], ext->GetColor1Range()[4], ext->GetColor1Range()[5]), ext->imageThreshed);
					dilate(ext->imageThreshed,ext->imageThreshed,Mat());
					dilate(ext->imageThreshed,ext->imageThreshed,Mat());
					medianBlur(ext->imageThreshed,ext->imageThreshed,3);
					ext->ExtractBlobFromImage(ext->imageThreshed, 1);
				}
				if(ext->GetIsColor2Tracked()
					&& ext->GetColor2Range()[0]!=ext->INVALIDVALUE 
					&& ext->GetColor2Range()[1]!=ext->INVALIDVALUE 
					&& ext->GetColor2Range()[2]!=ext->INVALIDVALUE
					&& ext->GetColor2Range()[3]!=ext->INVALIDVALUE
					&& ext->GetColor2Range()[4]!=ext->INVALIDVALUE
					&& ext->GetColor2Range()[5]!=ext->INVALIDVALUE)
				{
					inRange(ext->GetOriginalImageHSV(), Scalar(ext->GetColor2Range()[0], ext->GetColor2Range()[1], ext->GetColor2Range()[2]), Scalar(ext->GetColor2Range()[3], ext->GetColor2Range()[4], ext->GetColor2Range()[5]), ext->imageThreshed);
					dilate(ext->imageThreshed,ext->imageThreshed,Mat());
					dilate(ext->imageThreshed,ext->imageThreshed,Mat());
					medianBlur(ext->imageThreshed,ext->imageThreshed,3);
					ext->ExtractBlobFromImage(ext->imageThreshed, 2);
				}
			}
		}
		else
		{
			break;
		}
	}
}
void Extension::ExtractBlobFromImage(Mat image, int colorId)
{
	std::vector<std::vector<cv::Point>> contours;
	// Detect the contour of the white blobs (black&white image)
	cv::findContours(image,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	//// Eliminate too short or too long contours
	//int cmin = 100; // minimum contour length
	//int cmax = 10000; // maximum contour length
	//std::vector<std::vector<cv::Point>>::const_iterator itc = contours.begin();
	//while (itc != contours.end())
	//{
	//	if (itc->size() < cmin || itc->size() > cmax)
	//	{
	//		itc = contours.erase(itc);
	//	}
	//	else
	//	{
	//		++itc;
	//	}
	//}

	Rect cvRect;
	// Find the biggest bounding rectangle for each blob discovered  
	int num_blobs = contours.size();
	int iMax = 0;
	int maxSize = 0;
	if (num_blobs>0)
	{
		for (int i = 0; i < num_blobs; i++)
		{
			cvRect = boundingRect(Mat(contours[i]));
			if ((cvRect.width*cvRect.height) >= maxSize)
			{
				iMax = i;
				maxSize = cvRect.width*cvRect.height;
			}
		}

		cvRect = boundingRect(Mat(contours[iMax]));
		CvScalar debugColor = cvScalar(0, 0, 0, 0);

		if (colorId == 1)
		{
			SetColorPos(1, 0, cvRect.x + cvRect.width / 2);
			SetColorPos(1, 1, cvRect.y + cvRect.height / 2);
			SetColorSize(1, 0, cvRect.width / 2);
			SetColorSize(1, 1, cvRect.height / 2);

		}
		else
		{
			SetColorPos(2, 0, cvRect.x + cvRect.width / 2);
			SetColorPos(2, 1, cvRect.y + cvRect.height / 2);
			SetColorSize(2, 0, cvRect.width / 2);
			SetColorSize(2, 1, cvRect.height / 2);
			debugColor = cvScalar(255, 0, 0, 0);
		}
	}
	else
	{
		SetLastError(ErrorS[noColorFound]);
	}
}
int* Extension::RGB2HSVColor(int r, int g,int b)
{
	int i;
    const int hsv_shift = 12;
    static int sdiv_table[256];
    static int hdiv_table180[256];

   const int* hdiv_table = hdiv_table180;

    sdiv_table[0] = hdiv_table180[0] = 0;
    for( i = 1; i < 256; i++ )
    {
        sdiv_table[i] = saturate_cast<int>((255 << hsv_shift)/(1.*i));
        hdiv_table180[i] = saturate_cast<int>((180 << hsv_shift)/(6.*i));
    }
         
	int h, s, v = b;
    int vmin = b , diff;
    int vr, vg;
    
	v=__max(__max(v,g),r);
	vmin=__min(__min(v,g),r);
   /* CV_CALC_MAX_8U( v, g );
    CV_CALC_MAX_8U( v, r );
    CV_CALC_MIN_8U( vmin, g );
    CV_CALC_MIN_8U( vmin, r );*/
            
    diff = v - vmin;
    vr = v == r ? -1 : 0;
    vg = v == g ? -1 : 0;
            
    s = (diff * sdiv_table[v] + (1 << (hsv_shift-1))) >> hsv_shift;
    h = (vr & (g - b)) +
        (~vr & ((vg & (b - r + 2 * diff)) + ((~vg) & (r - g + 4 * diff))));
    h = (h * hdiv_table[diff] + (1 << (hsv_shift-1))) >> hsv_shift;
    h += h < 0 ? 180 : 0;
            
	int hsv[3]={h,s,v};
	return hsv;
}
// Object Tracking Thread
void TrackObjectThread(LPVOID lpParam)
{
	//get reference to extension
	Extension* ext;
	ext = ((Extension*)lpParam);

	for(;;)
	{
		if(ext->GetIsObjectTracked())
		{
			if(!ext->GetChangeDevice() && !ext->GetObjectCascade().empty())
			{
				vector<Rect> faces;
				ext->SetOriginalImageGRAY();
				ext->GetObjectCascade().detectMultiScale(ext->GetOriginalImageGRAY(), faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(80, 80) );
				if(faces.size()>0)
				{
					ext->SetObjectPos(0,faces[0].x+faces[0].width*0.5);
					ext->SetObjectPos(1,faces[0].y+faces[0].height*0.5);
					ext->SetObjectSize(0,faces[0].width);
					ext->SetObjectSize(1,faces[0].height);
				}
			}
		}
		else
		{
			break;
		}
	}
}
// Feature Tracking Thread
void TrackFeatureThread(LPVOID lpParam)
{
	//get reference to extension
	Extension* ext;
	ext = ((Extension*)lpParam);

	for(;;)
	{
		if(false && ext->GetIsFeatureTracked())
		{
			if(!ext->GetChangeDevice())
			{
				try
				{
					ext->SetOriginalImageGRAY();
					bool notEmpty = !ext->GetFeatureObjectImage().empty() && !ext->GetOriginalImageGRAY().empty();
					if (notEmpty)
					{
						vector<KeyPoint> objectKeypoints, sceneKeypoints, matchSceneKeypoints;
						Mat objectDescriptors, sceneDescriptors;
						vector<vector<DMatch>> matches;
						vector< DMatch > good_matches;
						vector<Point2f> obj;
						vector<Point2f> scene;
 						vector< Point2f > obj_corners(4);
						vector< Point2f > scene_corners(4);
					
						if(ext->detector==NULL)
						{
							//ext->detector = new BRISK(30,3,1.0f);
							ext->detector = new ORB(2000);
						}
						if(ext->extractor==NULL)
						{
							//ext->extractor = new BRISK(30,3,1.0f);
							ext->extractor = new ORB(2000);
						}
						if(ext->matcher==NULL)
						{
							ext->matcher = new BFMatcher(NORM_HAMMING);
							//ext->matcher = new FlannBasedMatcher(new cv::flann::LshIndexParams(20,10,2));
						}

						/// extract keypoints
						ext->detector->detect(ext->GetFeatureObjectImage(), objectKeypoints);
						ext->detector->detect(ext->GetOriginalImageGRAY(), sceneKeypoints);

						/// extract descriptors
						ext->extractor->compute(ext->GetFeatureObjectImage(), objectKeypoints, objectDescriptors);
						ext->extractor->compute(ext->GetOriginalImageGRAY(), sceneKeypoints, sceneDescriptors);

						//drawKeypoints(ext->GetOriginalImageGRAY(),sceneKeypoints,ext->featureMatches);

						ext->matcher->knnMatch(objectDescriptors, sceneDescriptors, matches, 2);
						//matcher.match(objectDescriptors,sceneDescriptors,test_matches);
					
						good_matches.reserve(matches.size()); 
						for (size_t i = 0; i < matches.size(); ++i)
						{
							if (matches[i].size() < 2)
										continue;
   
							const DMatch &m1 = matches[i][0];
							const DMatch &m2 = matches[i][1];
							//good_matches.push_back(m1);
							if(m1.distance <= (0.7 * m2.distance)) 
							{
								good_matches.push_back(m1);
							}
						}
										
						/*drawMatches(ext->GetFeatureObjectImage(), objectKeypoints, ext->GetOriginalImageGRAY(), sceneKeypoints, good_matches, ext->featureMatches,
								 Scalar::all(-1), Scalar::all(-1),vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );*/
											
						//ext->featureMatches=ext->GetOriginalImage();
						ext->SetFeatureNumberOfGoodMatches(good_matches.size());
						scene_corners[0]=scene_corners[1]=scene_corners[2]=scene_corners[3]=cvPoint(0,0);
						if( (good_matches.size() >= 8))
						{
							for( unsigned int i = 0; i < good_matches.size(); i++ )
							{
								//-- Get the keypoints from the good matches
								obj.push_back( objectKeypoints[ good_matches[i].queryIdx ].pt );
								scene.push_back( sceneKeypoints[ good_matches[i].trainIdx ].pt );
							}
 
							Mat H = findHomography(obj, scene, CV_RANSAC );
 
							//-- Get the corners from the feature image ( the object to be "detected" )
						
							obj_corners[0] = cvPoint(0,0); 
							obj_corners[1] = cvPoint( ext->GetFeatureObjectImage().cols, 0 );
							obj_corners[2] = cvPoint( ext->GetFeatureObjectImage().cols, ext->GetFeatureObjectImage().rows );
							obj_corners[3] = cvPoint( 0, ext->GetFeatureObjectImage().rows );
						 
							perspectiveTransform(obj_corners, scene_corners, H);
							/*CameraCalibration calibration(526.58037684199849f, 524.65577209994706f, 318.41744018680112f, 202.96659047014398f);
							Transformation p = ext->FindPose3D(scene_corners,obj_corners,calibration);*/
						}
						//-- Draw lines between the corners (the mapped object in the scene)
						ext->SetFeatureObjectCorners(scene_corners);
					}
				}
				catch(cv::Exception& ex)
				{
					ext->SetLastError(_T("Error by Feature Tracking"));
				}
			}
		}
		else
		{
			break;
		}
	}
}
// QR Decoding Thread
void TrackQRCodeThread(LPVOID lpParam)
{
	//get reference to extension
	Extension* ext;
	ext = ((Extension*)lpParam);
	Mat image, crop, cropS;
	for (;;)
	{
		if (ext->GetIsQRCodeTracked())
		{
			if (!ext->GetChangeDevice())
			{
				ext->SetOriginalImageGRAY();
				if (!ext->GetOriginalImageGRAY().empty())
				{
					image = ext->GetOriginalImageGRAY().clone();
					if (ext->GetIsQRCodeAutoDetected())
					{
						Rect r = ext->DetectBarcodeFromImage(image);
						if (r.width>0)
						{
							crop = image(r);
							//imwrite("test6.png", crop);
							if (ext->GetIsQRCodeSharpened())
							{
								GaussianBlur(crop, cropS, cv::Size(0, 0), 3);
								addWeighted(crop, 1.5, cropS, -0.5, 0, cropS);
								//imwrite("test7.png", cropS);
								ext->decode_image(cropS);
							}
							else
							{
								ext->decode_image(crop);
							}
						}
					}
					else
					{
						ext->decode_image(image);
					}
				}
			}
		}
		else
		{
			break;
		}
	}
}
Rect Extension::DetectBarcodeFromImage(Mat image)
{
	
	Mat img,imgInv, imgx, imgy, imgthresh;
	std::vector<std::vector<cv::Point>> contours;
	Rect cvRect(0,0,0,0);
	int num_blobs, iMax, maxSize;
	try
	{
		
		img = image.clone();
		bitwise_not(img, imgInv);
		//imwrite("orig.png", img);
		//imwrite("origInv.png", imgInv);
		imgx = Mat(image.size(), IPL_DEPTH_16S, 1);
		imgy = Mat(image.size(), IPL_DEPTH_16S, 1);
		imgthresh = Mat(image.size(), 8, 1);

		//find horiz and vert gradients
		Sobel(img, imgx, IPL_DEPTH_16S,1, 0);
		imgx = abs(imgx);
		Sobel(img, imgy, IPL_DEPTH_16S, 0, 1);
		imgy = abs(imgy);
		//subtract one from each other
		subtract(imgx, imgy, imgx);
		imgx.convertTo(img, -1, 3,2);
		
		Sobel(imgInv, imgx, IPL_DEPTH_16S, 1, 0);
		imgx = abs(imgx);
		Sobel(imgInv, imgy, IPL_DEPTH_16S, 0, 1);
		imgy = abs(imgy);
		//subtract one from each other
		subtract(imgx, imgy, imgx);
		imgx.convertTo(imgInv, -1, 3, 2);

		//Mat sub_mat = Mat::ones(image.size(), image.type()) * 255
		//imwrite("test0.png", img);
		//imwrite("test1.png", imgInv);
		//erode(img, img, Mat());
		//erode(imgInv, imgInv, Mat());
		// Low pass filtering
		//threshold(img,imgthresh, 130, 255, CV_THRESH_BINARY);
		inRange(img,100,255,img);
		inRange(imgInv, 100, 255, imgInv);
		//imwrite("test2a.png", img);
		//imwrite("test2b.png", imgInv);
		add(imgInv, img, imgthresh);
		erode(imgthresh, imgthresh, Mat());
		dilate(imgthresh, imgthresh, Mat(),Point(-1,-1),5);
		//GaussianBlur(imgthresh, imgthresh, Size(3,1), 3, 1);
		//GaussianBlur(imgthresh, imgthresh, cvSize(7, 7), 0);
		//medianBlur(imgthresh, imgthresh, 3);
		//imwrite("test3.png", imgthresh);
		//cvSmooth(img, img, CV_GAUSSIAN, 7, 7, 0);
		//cvThreshold(img, thresh, 100, 255, cv.CV_THRESH_BINARY)
		//cvErode(thresh, thresh, None, 2)
		//cvDilate(thresh, thresh, None, 5)

		// Detect the contour of the white blobs (black&white image)
		cv::findContours(imgthresh,
			contours, // a vector of contours
			CV_RETR_EXTERNAL, // retrieve the external contours
			CV_CHAIN_APPROX_NONE); // all pixels of each contours
		//cv::findContours(imgthresh,
		//	contours, // a vector of contours
		//	CV_RETR_CCOMP, // retrieve the external contours
		//	CV_CHAIN_APPROX_SIMPLE); // all pixels of each contours
		//imwrite("test4.png", imgthresh);
		// Find the biggest bounding rectangle for each blob discovered  
		num_blobs = contours.size();
		iMax = 0;
		maxSize = 0;
		if (num_blobs > 0)
		{
			for (int i = 0; i < num_blobs; i++)
			{
				cvRect = boundingRect(Mat(contours[i]));
				if ((cvRect.width*cvRect.height) >= maxSize)
				{
					iMax = i;
					maxSize = cvRect.width*cvRect.height;
				}
			}

			cvRect = boundingRect(Mat(contours[iMax]));
		}
		
		rectangle(imgthresh, cvRect, cvScalar(255, 0, 0, 0));
		//imwrite("test5.png", imgthresh);
	}
	catch(cv::Exception e)
	{
		int i = 0 ;
	}

	return cvRect;
}

inline ZXing::ImageView ImageViewFromMat(const cv::Mat& image)
{
	using ZXing::ImageFormat;

	auto fmt = ImageFormat::None;
	switch (image.channels()) {
	case 1: fmt = ImageFormat::Lum; break;
	case 3: fmt = ImageFormat::BGR; break;
	case 4: fmt = ImageFormat::BGRX; break;
	}

	if (image.depth() != CV_8U || fmt == ImageFormat::None)
		return { nullptr, 0, 0, ImageFormat::None };

	return { image.data, image.cols, image.rows, fmt };
}

inline ZXing::Result ReadBarcode(const cv::Mat& image, const ZXing::DecodeHints& hints = {})
{
	return ZXing::ReadBarcode(ImageViewFromMat(image), hints);
}

void Extension::decode_image(Mat &image)
{
	try
	{
		ZXing::Result result = ReadBarcode(image);//DecodeHints(DecodeHints::TRYHARDER_HINT)
		CA2T out(result.text().c_str());
		SetQRCodeText(out);
	}
	catch (ZXing::Error& e)
	{
		string str = e.msg();
	}
}

Transformation Extension::FindPose3D(vector< Point2f > scene_points,vector< Point2f > points2d, CameraCalibration calibration)
{
	cv::Mat Rvec;
	cv::Mat_<float> Tvec;
	cv::Mat raux,taux;
	std::vector<cv::Point3f>  points3d;
	points3d.push_back(Point3f(scene_points[0].x, scene_points[0].y, 0));
	points3d.push_back(Point3f(scene_points[1].x, scene_points[1].y, 0));
	points3d.push_back(Point3f(scene_points[2].x, scene_points[2].y, 0));
	points3d.push_back(Point3f(scene_points[3].x, scene_points[3].y, 0));
	
	cv::solvePnP(points3d, points2d, calibration.getIntrinsic(), calibration.getDistorsion(),raux,taux);
	raux.convertTo(Rvec,CV_32F);
	taux.convertTo(Tvec ,CV_32F);

	cv::Mat_<float> rotMat(3,3); 
	cv::Rodrigues(Rvec, rotMat);

	Transformation	pose3d;
	// Copy to transformation matrix
	for (int col=0; col<3; col++)
	{
		for (int row=0; row<3; row++)
		{        
			pose3d.r().mat[row][col] = rotMat(row,col); // Copy rotation component
		}
			pose3d.t().data[col] = Tvec(col); // Copy translation component
	}

	// Since solvePnP finds camera location, w.r.t to marker pose, to get marker pose w.r.t to the camera we invert it.
	pose3d = pose3d.getInverted();

	return pose3d;
}
void Extension::DrawFeatureOnImage()
{
	if (featureObjectCorners.size() == 4)
	{
		cv::Mat img = GetOriginalImage();
		line(img, GetFeatureCorner(0), GetFeatureCorner(1), Scalar(0, 255, 0), 2); //TOP line
		line(img, GetFeatureCorner(1), GetFeatureCorner(2), Scalar(0, 255, 0), 2);
		line(img, GetFeatureCorner(2), GetFeatureCorner(3), Scalar(0, 255, 0), 2);
		line(img, GetFeatureCorner(3), GetFeatureCorner(0), Scalar(0, 255, 0), 2);
	}
}

// START THREADS
void Extension::StartChangeDeviceThread(int id)
{
	if(GetChangeDevice()==false){
			StopAllThreads();
			SetChangeDevice(true);
			SetDeviceId(id);
			changeDeviceThread = new thread(ChangeDeviceThread,this);
	}	
}
void Extension::StartCapturingThread()
{
	if(!captureThread || !captureThread->joinable())
	{

		captureThread = new thread(CaptureThread,this);
		//SetThreadPriority(captureThread->native_handle(),THREAD_PRIORITY_LOWEST);
	}
	else
	{
		SetLastError(ErrorS[captureAlreadyStarted]);
	}
}
void Extension::StartColorTrackThread()
{
	if(!trackColorThread || !trackColorThread->joinable())
	{
		trackColorThread = new thread(ColorTrackThread,this);
		//SetThreadPriority(trackColorThread->native_handle(),THREAD_PRIORITY_LOWEST);
	}
	else
	{
		SetLastError(ErrorS[colorAlreadyTracked]);
	}
}
void Extension::StartObjectTrackThread()
{
	SetIsObjectTracked(true);
	if(!trackObjectThread || !trackObjectThread->joinable())
	{
		trackObjectThread = new thread(TrackObjectThread,this);
		//SetThreadPriority(trackObjectThread.native_handle(),THREAD_PRIORITY_LOWEST);
	}
	else
	{
		SetLastError(ErrorS[objectAlreadyTracked]);
	}
}
void Extension::StartFeatureTrackThread()
{
	SetIsFeatureTracked(true);
	if(!trackFeatureThread || !trackFeatureThread->joinable())
	{
		trackFeatureThread = new thread(TrackFeatureThread,this);
		//SetThreadPriority(trackObjectThread.native_handle(),THREAD_PRIORITY_LOWEST);
	}
	else
	{
		SetLastError(ErrorS[featureAlreadyTracked]);
	}
}
void Extension::StartQRCodeTrackThread()
{
	SetIsQRCodeTracked(true);
	if (!trackQRCodeThread || !trackQRCodeThread->joinable())
	{
		trackQRCodeThread = new thread(TrackQRCodeThread, this);
		//SetThreadPriority(trackObjectThread.native_handle(),THREAD_PRIORITY_LOWEST);
	}
	else
	{
		SetLastError(ErrorS[QRCodeAlreadyTracked]);
	}
}
// STOP/START THREADS
void Extension::StopAllThreads()
{
	if(changeDeviceThread && changeDeviceThread->joinable())
	{
		changeDeviceThread->join();
	}
	isFeatureTracked=false;
	if(trackFeatureThread && trackFeatureThread->joinable())
	{
		trackFeatureThread->join();
	}
	isObjectTracked=false;
	if(trackObjectThread && trackObjectThread->joinable())
	{
		trackObjectThread->join();
	}
	isColor1Tracked=false;
	isColor2Tracked=false;
	if(trackColorThread && trackColorThread->joinable())
	{
		trackColorThread->join();
	}
	isQRCodeTracked = false;
	if (trackQRCodeThread && trackQRCodeThread->joinable())
	{
		trackQRCodeThread->join();
	}
	isCaptureEnabled=false;
	if(captureThread && captureThread->joinable())
	{
		captureThread->join();
	}
}

// Thread Safe Getter/Setter
// Getters
VideoCapture* Extension::GetCapture()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return capture;
}
VideoWriter	Extension::GetVideoWriter()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return videoW;
}
bool		Extension::GetChangeDevice()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return changeDevice;
}
int			Extension::GetDeviceId()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return deviceId;
}
TCHAR*		Extension::GetRawImagePath()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return rawImagePath;
}
Mat			Extension::GetOriginalImage()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return originalImage;
}
Mat			Extension::GetOriginalImageHSV()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return originalImageHSV;
}
Mat			Extension::GetOriginalImageGRAY()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return originalImageGRAY;
	//return imread("box_in_scene.png",IMREAD_GRAYSCALE);
}
TCHAR*		Extension::GetLastError()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return lastError;
}
bool		Extension::GetIsColor1Tracked()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isColor1Tracked;
}
bool		Extension::GetIsColor2Tracked()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isColor2Tracked;
}
int*		Extension::GetColor1Range()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return color1Range;
}
int*		Extension::GetColor1Pos()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return color1Pos;
}
int*		Extension::GetColor1Size()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return color1Size;
}
int*		Extension::GetColor2Range()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return color2Range;
}
int*		Extension::GetColor2Pos()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return color2Pos;
}
int*		Extension::GetColor2Size()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return color2Size;
}
bool		Extension::GetIsCaptureEnabled()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isCaptureEnabled;
}
bool		Extension::GetIsCaptureDisplayed()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isCaptureDisplayed;
}
COLORREF	Extension::GetBackgroundColor()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return backgroundColor;
}
CascadeClassifier Extension::GetObjectCascade()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return objectCascade;
}
bool		Extension::GetIsObjectTracked()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isObjectTracked;
}
int*		Extension::GetObjectPos()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return objectPos;
}
int*		Extension::GetObjectSize()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return objectSize;
}
bool		Extension::GetIsFeatureTracked()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isFeatureTracked;
}
Mat			Extension::GetFeatureObjectImage()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return featureObjectImage;
	//return imread("box.png",IMREAD_GRAYSCALE);
}
int			Extension::GetFeatureNumberOfGoodMatches()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return numberOfGoodMatches;
}
Point2f		Extension::GetFeatureCorner(int cornerId)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return featureObjectCorners[cornerId];
}
TCHAR*		Extension::GetQRCodeText()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return QRCodeText;
}
bool		Extension::GetIsQRCodeTracked()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isQRCodeTracked;
}
bool		Extension::GetIsQRCodeAutoDetected()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isQRCodeAutoDetectionEnabled;
}
bool		Extension::GetIsQRCodeSharpened()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	return isQRCodeSharpeningEnabled;
}

// Setters
void		Extension::SetCaptureDevice(int deviceId)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(capture!=NULL)
	{
		delete capture;
		capture=NULL;
	}
	capture=new VideoCapture(deviceId);
	//bool isOpened = capture.open(deviceId);
}
void		Extension::SetVideoWriter(TCHAR* filename,int codec, int fps)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	int lg;
	LPSTR ps;
	UINT codePage = CP_ACP;
	lg = WideCharToMultiByte(codePage, 0, filename, -1, NULL, 0, NULL, NULL);
	ps = (LPSTR)malloc(lg*sizeof(char));
	if (ps != NULL)
	{
		WideCharToMultiByte(codePage, 0, filename, -1, ps, lg, NULL, NULL);
		videoW = VideoWriter(ps, codec, fps, cv::Size(originalImage.cols, originalImage.rows), 1);
		free(ps);
	}
}
void		Extension::SetChangeDevice(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	changeDevice=status;
}
void		Extension::SetDeviceId(int id)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	deviceId=id;
}
void		Extension::SetRawImagePath(TCHAR* filename)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	_snwprintf_s(rawImagePath, _countof(rawImagePath), 1024, filename);
}
void		Extension::SetOriginalImage(Mat image)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(originalImage.empty() || originalImage.cols!=rdPtr->rHo.hoImgWidth || originalImage.rows!=rdPtr->rHo.hoImgHeight)
	{
		originalImage = Mat(cv::Size(rdPtr->rHo.hoImgWidth,rdPtr->rHo.hoImgHeight),CV_MAKETYPE(image.depth(), image.channels()));
	}
	resize(image,originalImage,originalImage.size(),0,0,INTER_LINEAR);
}
void		Extension::FlipOriginalImage()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(isImageFlipped){
		flip(originalImage,originalImage,1);
	}
}
void		Extension::SwitchIsImageFlipped()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(isFeatureTracked)
	{
		flip(featureObjectImage,featureObjectImage,1);
	}
	if(isImageFlipped){
		isImageFlipped=false;
	}
	else
	{
		isImageFlipped=true;
	}
}
void		Extension::SetOriginalImageHSV()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(originalImageHSV.empty())
	{
		originalImageHSV = Mat(originalImage.size(),CV_MAKETYPE(originalImage.depth(),originalImage.channels()));
	}
	if(!originalImage.empty())
	{
		cvtColor(originalImage, originalImageHSV, CV_BGR2HSV);
	}
}
void		Extension::SetOriginalImageGRAY()
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(originalImageGRAY.empty())
	{
		originalImageGRAY = Mat(originalImage.size(),CV_8UC1);
	}
	if(!originalImage.empty())
	{
		cvtColor(originalImage, originalImageGRAY, CV_BGR2GRAY);
	}
}
void		Extension::SetLastError(const TCHAR* error)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	_snwprintf_s(lastError, _countof(lastError), _countof(lastError), error);
}
void		Extension::SetIsColor1Tracked(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isColor1Tracked=status;
}
void		Extension::SetIsColor2Tracked(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isColor2Tracked=status;
}
void		Extension::SetColorPos(int colorId, int index, int value)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(colorId==1)
	{
		color1Pos[index]=value;
	}
	if(colorId==2)
	{
		color2Pos[index]=value;
	}
}
void		Extension::SetColorSize(int colorId, int index, int value)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(colorId==1)
	{
		color1Size[index]=value;
	}
	if(colorId==2)
	{
		color2Size[index]=value;
	}
}
void		Extension::SetColorRange(int colorId, int index, int value)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(colorId==1)
	{
		color1Range[index]=value;
	}
	if(colorId==2)
	{
		color2Range[index]=value;
	}
}
void		Extension::SetIsCaptureEnabled(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isCaptureEnabled=status;
}
void		Extension::SetIsCaptureDisplayed(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isCaptureDisplayed=status;
}
void		Extension::SetBackgroundColor(COLORREF color)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	backgroundColor=color;
}
void		Extension::SetObjectCascade(TCHAR* filename)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	int lg;
	LPSTR ps;
	UINT codePage = CP_ACP;
	lg = WideCharToMultiByte(codePage, 0, filename, -1, NULL, 0, NULL, NULL);
	ps = (LPSTR)malloc(lg*sizeof(char));
	WideCharToMultiByte(codePage, 0, filename, -1, ps, lg, NULL, NULL);
	objectCascade.load(ps);
	free(ps);
}
void		Extension::SetIsObjectTracked(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isObjectTracked=status;
}
void		Extension::SetObjectPos(int index, int value)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	objectPos[index]=value;
}
void		Extension::SetObjectSize(int index, int value)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	objectSize[index]=value;
}
void		Extension::SetIsFeatureTracked(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isFeatureTracked=status;
}
void		Extension::SetFeatureObjectImage(TCHAR* filename)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	int lg;
	LPSTR ps;
	UINT codePage = CP_ACP;
	lg = WideCharToMultiByte(codePage, 0, filename, -1, NULL, 0, NULL, NULL);
	ps = (LPSTR)malloc(lg*sizeof(char));
	WideCharToMultiByte(codePage, 0, filename, -1, ps, lg, NULL, NULL);
	featureObjectImage = imread(ps, IMREAD_GRAYSCALE);
	//imread does not have unicode support (see loadsave.cpp in opencv project)
	free(ps);
}
void		Extension::SetFeatureNumberOfGoodMatches(int matchesN)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	numberOfGoodMatches=matchesN;
}
void		Extension::SetFeatureObjectCorners(vector<Point2f> corners)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	if(corners.size()==featureObjectCorners.size() && corners.size()>0)
	{
		featureObjectCorners[0]=corners[0];
		featureObjectCorners[1]=corners[1];
		featureObjectCorners[2]=corners[2];
		featureObjectCorners[3]=corners[3];
	}
}
void		Extension::SetQRCodeText(TCHAR* text)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	_snwprintf_s(QRCodeText, _countof(QRCodeText), _countof(QRCodeText), text);
}
void		Extension::SetIsQRCodeTracked(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isQRCodeTracked = status;
}
void		Extension::SetIsQRCodeAutoDetected(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isQRCodeAutoDetectionEnabled = status;
}
void		Extension::SetIsQRCodeSharpened(bool status)
{
	lock_guard<fast_mutex> lock(cameraMutex);
	isQRCodeSharpeningEnabled = status;
}