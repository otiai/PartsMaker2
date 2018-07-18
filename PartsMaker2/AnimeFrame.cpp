#include "AnimeFrame.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QGLWidget>
#include "OpenCVImageIO.h"
#include "SegmentationDriver.h"
#include "Config.h"
#include "Utility.h"

#if _DEBUG
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_core241d.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_highgui241d.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_imgproc241d.lib")
#else
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_core241.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_highgui241.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_imgproc241.lib")
#endif


AnimeFrame::AnimeFrame()
{
	initialize();
}

AnimeFrame::~AnimeFrame()
{
	finalize();
}

bool AnimeFrame::loadInputImage(const char* filePath)
{
	// �摜�̓ǂݍ���
	cv::Mat inputImage = cv::imread(filePath);
	if(inputImage.empty())
	{
        qDebug("cannot open files");
        return false;
    }

	//
	// ���͉摜���G�b�W�̈�ƃJ���[�̈�ɕ�����
	//

	// �O���[�X�P�[����
	cv::Mat grayImage;
	cvtColor(inputImage, grayImage,CV_RGB2GRAY);
	cv::imwrite("ResultImage/gray.png", grayImage);

	// �G�b�W�摜���o
	cv::Mat edgeImage;
	cv::threshold(grayImage, edgeImage, 0, 255, cv::THRESH_BINARY);
	cv::imwrite("ResultImage/mono.png", edgeImage);

	// �J���[�̈�摜���o
	cv::Mat colorImage;
	colorImage = inputImage.clone();
	for(int y = 0; y < colorImage.rows; y++)
	{
		for(int x = 0; x < colorImage.cols; x++)
		{
			unsigned char m = edgeImage.at<unsigned char>(y, x);
			if(m <= 0)
			{
				colorImage.at<cv::Vec3b>(y,x) = cv::Vec3b(Config::BackColor.r, Config::BackColor.g, Config::BackColor.b);
			}
		}
	}
	cv::imwrite("ResultImage/color.png", colorImage);

	// cv::Mat����ImageRect�쐬
	OpenCVImageIO io;
	io.convertMat2ImageRect(colorImage, m_ColorImage);

	SegmentationDriver driver;
	driver.applySegmentation(*this);


	// �e�̈�ɑ΂���G�b�W�̋������v�Z���A��ԋ߂����̂����̃G�b�W�s�N�Z���̗̈�Ƃ���
	int w = m_ColorImage.getWidth();
	int h = m_ColorImage.getHeight();
	ImageRect<float> tmpDistBuffer;
	IDMap   tmpIDBuffer;
	tmpDistBuffer.allocate(w, h);
	tmpDistBuffer.fill(0xffff);
	tmpIDBuffer.allocate(w, h);
	tmpIDBuffer.fill(Config::FalseRegionID);
	ImageGrayf edgeImageRect;
	io.convertMat2ImageRect(edgeImage, edgeImageRect);

	for(int i = 0; i < getNumRegions(); i++)
	{
		ClosedRegion* r = m_Regions.at(i);
		RegionMap& regionMap = r->getRegionMap();
		cv::Mat matRegion;
		io.convertImageRect2Mat(regionMap, matRegion);
		cv::Mat matRegionGray;
		cvtColor(matRegion, matRegionGray,CV_RGB2GRAY);
		cv::Mat matBin;
		cv::threshold(matRegionGray, matBin, 0, 255, cv::THRESH_BINARY);
		matBin = ~matBin;
		cv::Mat matDist;
		cv::distanceTransform(matBin, matDist, CV_DIST_C, 3);

#if 1 // �f�o�b�O
		char str[256];
		sprintf(str, "ResultImage/dist_%d.png", r->getID());
		cv::imwrite(str, matDist);
#endif
		for(int y = 0; y < edgeImageRect.getHeight(); y++)
		{
			for(int x = 0; x < edgeImageRect.getWidth(); x++)
			{
				if(edgeImageRect(x, y) == 0)
				{
					float dist = matDist.at<float>(edgeImageRect.getHeight()- y -1, x); // at(�s, ��), ImageRect��cv::Mat�͏㉺���t
					if(dist < tmpDistBuffer(x, y))
					{
						tmpDistBuffer(x, y) = dist;
						tmpIDBuffer(x, y) = r->getID();
					}
				}
			}
		}
	}
	
	// �e�̈�f�[�^�ɃG�b�W������ǉ�
	for(int i = 0; i < getNumRegions(); i++)
	{
		ClosedRegion* r = m_Regions.at(i);
		RegionMap& regionMap = r->getRegionMap();
	
		for(int y = 0; y < tmpIDBuffer.getHeight(); y++)
		{
			for(int x = 0; x < tmpIDBuffer.getWidth(); x++)
			{
				if(tmpIDBuffer(x, y) == r->getID())
				{
					IntVec::ubvec3 regionColor = r->getRegionColor();
					regionMap(x, y) = IntVec::ubvec4(regionColor.r, regionColor.g, regionColor.b, 255);

					// IDMap���X�V
					m_IDMap(x, y) = r->getID();
				}
			}
		}

		// ������
		r->fillHoles();

#if 1 // �f�o�b�O
		char str[256];
		sprintf(str, "ResultImage/region_%d.png", r->getID());
		io.save(str, (r->getRegionMap()) );
		driver.dumpIDMaps(*this, "ResultImage/regions.png");
#endif
	}
	return true;
}

bool AnimeFrame::initialize()
{
	return true;
}

bool AnimeFrame::finalize()
{
	for(int i = 0; i < m_Regions.size(); i++)
	{
		ClosedRegion* r = m_Regions.at(i);
		delete r;
	}
	m_Regions.clear();

	return true;
}

