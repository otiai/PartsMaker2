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
	initialize();//メンバ変数の初期化
}

AnimeFrame::~AnimeFrame()
{
	finalize();//処理の終了メソッド
}

bool AnimeFrame::loadInputImage(const char* filePath)
{
	// opencvを使った処理
	cv::Mat inputImage = cv::imread(filePath);
	if(inputImage.empty())//空なら
	{
        qDebug("cannot open files");
        return false;
    }

	//
	// ���͉摜��G�b�W�̈�ƃJ���[�̈�ɕ�����
	//

	// 画像のグレースケール化
	cv::Mat grayImage;
	cvtColor(inputImage, grayImage,CV_RGB2GRAY);
	cv::imwrite("ResultImage/gray.png", grayImage);//指定したファイルに画像を保存する

	// エッジ画像
	cv::Mat edgeImage;
	cv::threshold(grayImage, edgeImage, 0, 255, cv::THRESH_BINARY);//画像の二値化
	cv::imwrite("ResultImage/mono.png", edgeImage);

	// �J���[�̈�摜���o
	cv::Mat colorImage;
	colorImage = inputImage.clone();
	for(int y = 0; y < colorImage.rows; y++)//行
	{
		for(int x = 0; x < colorImage.cols; x++)//列
		{
			unsigned char m = edgeImage.at<unsigned char>(y, x);//unsigned char:８ビットの領域を占有する
			if(m <= 0)
			{
				colorImage.at<cv::Vec3b>(y,x) = cv::Vec3b(Config::BackColor.r, Config::BackColor.g, Config::BackColor.b);//カラー画像の画素値の設定
			}
		}
	}
	cv::imwrite("ResultImage/color.png", colorImage);

	// cv::Mat����ImageRect�쐬,カラー画像を置き替えている？
	OpenCVImageIO io;
	io.convertMat2ImageRect(colorImage, m_ColorImage);

	SegmentationDriver driver;
	driver.applySegmentation(*this);


	// �e�̈�ɑ΂���G�b�W�̋�����v�Z���A��ԋ߂���̂�̃G�b�W�s�N�Z���̗̈�Ƃ���
	int w = m_ColorImage.getWidth();//画像の幅を獲得
	int h = m_ColorImage.getHeight();
	ImageRect<float> tmpDistBuffer;//float型とする？
	IDMap   tmpIDBuffer;//?
	tmpDistBuffer.allocate(w, h);//float型として代入
	tmpDistBuffer.fill(0xffff);//この色で染める
	tmpIDBuffer.allocate(w, h);
	tmpIDBuffer.fill(Config::FalseRegionID);//誤った領域
	ImageGrayf edgeImageRect;
	io.convertMat2ImageRect(edgeImage, edgeImageRect);

	for(int i = 0; i < getNumRegions(); i++)//領域取得を繰り返す
	{
		ClosedRegion* r = m_Regions.at(i);
		RegionMap& regionMap = r->getRegionMap();
		cv::Mat matRegion;
		io.convertImageRect2Mat(regionMap, matRegion);
		cv::Mat matRegionGray;
		cvtColor(matRegion, matRegionGray,CV_RGB2GRAY);//グレースケール
		cv::Mat matBin;
		cv::threshold(matRegionGray, matBin, 0, 255, cv::THRESH_BINARY);//閾値処理
		matBin = ~matBin;
		cv::Mat matDist;
		cv::distanceTransform(matBin, matDist, CV_DIST_C, 3);//マスクのサイズ、距離の計算

#if 1 // �f�o�b�O
		char str[256];
		sprintf(str, "ResultImage/dist_%d.png", r->getID());
		cv::imwrite(str, matDist);
#endif
		for(int y = 0; y < edgeImageRect.getHeight(); y++)//繰り返し条件：エッジ画像の高さ
		{
			for(int x = 0; x < edgeImageRect.getWidth(); x++)//繰り返し条件：エッジ画像の幅
			{
				if(edgeImageRect(x, y) == 0)
				{
					float dist = matDist.at<float>(edgeImageRect.getHeight()- y -1, x); // at(�s, ��), ImageRect��cv::Mat�͏㉺���t
					if(dist < tmpDistBuffer(x, y))//距離
					{
						tmpDistBuffer(x, y) = dist;
						tmpIDBuffer(x, y) = r->getID();
					}
				}
			}
		}
	}
	
	// �e�̈�f�[�^�ɃG�b�W������ǉ�
	for(int i = 0; i < getNumRegions(); i++)//繰り返し条件：領域の数
	{
		ClosedRegion* r = m_Regions.at(i);//閉領域
		RegionMap& regionMap = r->getRegionMap();//領域範囲の習得
	
		for(int y = 0; y < tmpIDBuffer.getHeight(); y++)
		{
			for(int x = 0; x < tmpIDBuffer.getWidth(); x++)
			{
				if(tmpIDBuffer(x, y) == r->getID())
				{
					IntVec::ubvec3 regionColor = r->getRegionColor();//色領域の習得
					regionMap(x, y) = IntVec::ubvec4(regionColor.r, regionColor.g, regionColor.b, 255);

					// IDMap��X�V
					m_IDMap(x, y) = r->getID();
				}
			}
		}

		// ������
		r->fillHoles();//穴を埋める

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