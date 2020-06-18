#include "Common.h"

FeatureObject::FeatureObject()
{
}

FeatureObject::FeatureObject(TCHAR* filename)
{
	int lg;
	LPSTR ps;
	UINT codePage = CP_ACP;
	lg = WideCharToMultiByte(codePage, 0, filename, -1, NULL, 0, NULL, NULL);
	ps = (LPSTR)malloc(lg*sizeof(char));
	WideCharToMultiByte(codePage, 0, filename, -1, ps, lg, NULL, NULL);
	image = imread(ps, IMREAD_GRAYSCALE);
	//imread does not have unicode support (see loadsave.cpp in opencv project)
	free(ps);
}

bool FeatureObject::Train(Ptr<FeatureDetector> detector, Ptr<DescriptorExtractor> extractor)
{
	if (!image.empty()){
		detector->detect(image, keyPoints);
		extractor->compute(image, keyPoints, descriptors);
		isTrained = true;
	}
	else{
		isTrained = false;
	}
	return isTrained;
}

bool  FeatureObject::IsTrained()
{
	return isTrained;
}