#include "ObjectManager.h"
#include <stdio.h>
#include "AnimeFrame.h"
#include "Config.h"
#include "MainWindow.h"
#include "SegmentationDriver.h"
#include <vector>
#include "OpenCVImageIO.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "DepthViewBase.h"


#if _DEBUG
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_core241d.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_highgui241d.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_imgproc241d.lib")
#else
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_core241.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_highgui241.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_imgproc241.lib")
#endif

#define USE_SIMIRALITY_OURS 1 // �ގ��x�̌v�Z�͖{��@���g��

ObjectManager* ObjectManager::instance_ = NULL;
int	 ObjectManager::currentSrcRegionID_ = 0;
int	 ObjectManager::currentDstRegionID_ = 0;

ObjectManager* ObjectManager::create()
{
	instance_ = new ObjectManager();
	return instance_;
}

void ObjectManager::destroy()
{
	delete instance_;
}

void ObjectManager::initialize(MainWindow* w)
{
	srcFrame_ = NULL;
	dstFrame_ = NULL;

	isRegionLinking_ = false;
	editMode_ = MODE_REGION_MATCH;
	refMainWindow_ = w;

	srcRot_ = QVector2D(0.0, 0.0);
	dstRot_ = QVector2D(0.0, 45.0);
	edgeWidth_ = 2;

	//loadImageFiles("../PartsMaker2/resources/input3.png", "../PartsMaker2/resources/input3_dst.png");
	
}

void ObjectManager::loadImageFiles()
{
	if(srcImageFileName_.isEmpty() || dstImageFileName_.isEmpty())
		return;

	finalize();

	srcFrame_ = new AnimeFrame;
	srcFrame_->loadInputImage(srcImageFileName_.toAscii());
	dstFrame_ = new AnimeFrame;
	dstFrame_->loadInputImage(dstImageFileName_.toAscii());

	setCurrentSrcRegionID(srcFrame_->getNumRegions());
	setCurrentDstRegionID(dstFrame_->getNumRegions());

	// ���C���r���[�̗̈�i���C���̈�j�̐������Ή��f�[�^�쐬
	for(int i = 0; i < srcFrame_->getRegions().size(); i++)
	{
		ClosedRegion* r = srcFrame_->getRegions().at(i);
		regionLinkDataManager_.createData(r);
	}

	//? �̈�}�b�`���O�e�X�g
	regionMatching();
}

void ObjectManager::finalize()
{
	// ���C���̈�(srcFrame_)�������O�Ƀ����N�f�[�^���N���A����K�v������
	regionLinkDataManager_.deleteAll();

	if(srcFrame_)
	{
		delete srcFrame_;
	}
	srcFrame_ = NULL;

	if(dstFrame_)
	{
		delete dstFrame_;
	}
	dstFrame_ = NULL;

	selectRegionData_.clearAll();

	currentSrcRegionID_ = 0;
	currentDstRegionID_ = 0;
}

/*!
	@brief	AnimeFrame���̊e�p�[�c�̃}�b�`���O
*/
void ObjectManager::regionMatching()
{
	std::vector<ClosedRegion*>& srcRegions = srcFrame_->getRegions();
	std::vector<ClosedRegion*>& dstRegions = dstFrame_->getRegions();

	int numSrc = srcFrame_->getRegions().size();
	int numDst = dstFrame_->getRegions().size();
	float** data = new float*[ numSrc ];
	for(int i = 0; i < numSrc; i++)
	{
		data[i] = new float[ numDst ];
	}

	// �ގ��x�̌v�Z
	for(int j = 0; j < dstRegions.size(); j++)
	{
		ClosedRegion* rd = dstRegions.at(j);
		for(int i = 0; i < srcRegions.size(); i++)
		{
			ClosedRegion* rs = srcRegions.at(i);

			float s;
#if USE_SIMIRALITY_OURS
			s = ClosedRegion::calcSimirarity_Ours(*rs, *rd);
#else
			s = ClosedRegion::calcSimirarity(*rs, *rd);
#endif
			//qDebug("[%d][%d]:%f", i, j, s);
			data[i][j] = s;
		}
	}

	// ���݂��ɍł��ގ��x���������̂�Ή��t����
	QVector<int> maxDataIndex_src; // src�f�[�^�ɑ΂��čł��悭�}�b�`������̂�dst�̒�����I�񂾂���
	for(int i = 0; i < numSrc; i++)
	{
		float maxValue = 0.0;
		int maxIndex = -1;
		for(int j = 0; j < numDst; j++)
		{
			if(data[i][j] > maxValue)
			{
				maxValue = data[i][j];
				maxIndex = j;
			}
		}
		maxDataIndex_src.append(maxIndex);
	}

	QVector<int> maxDataIndex_dst;	// dst�f�[�^�ɑ΂��čł��悭�}�b�`������̂�src�̒�����I�񂾂���
	for(int j = 0; j < numDst; j++)
	{
		float maxValue = 0.0;
		int maxIndex = -1;
		for(int i = 0; i < numSrc; i++)
		{
			if(data[i][j] > maxValue)
			{
				maxValue = data[i][j];
				maxIndex = i;
			}
		}
		maxDataIndex_dst.append(maxIndex);
	}

	// maxDataIndex_src�őI��dst�̈悪src�ƍł��悭�}�b�`����Η̈��Ή��t����
	for(int i = 0; i < maxDataIndex_src.size(); i++)
	{
		int srcIndex = i;
		int dstIndex = maxDataIndex_src.at(i);

		if(dstIndex != -1)
		{
			if(maxDataIndex_dst.at(dstIndex) == srcIndex)
			{
				//qDebug("match %d - %d", srcIndex, dstIndex);
				// �����N�f�[�^�Ή��t��
				ClosedRegion* src = srcRegions.at(srcIndex);
				ClosedRegion* dst = dstRegions.at(dstIndex);
				regionLinkDataManager_.link(src, dst, VIEW_SIDE_RIGHT);
			}
		}
	}

#if 0 // �f�o�b�O�\��
	regionLinkDataManager_.debugPrint();
#endif

	for(int i = 0; i < numSrc; i++)
	{
		delete [] data[i];
	}
	delete [] data;
}

/*!
	@brief	�����N�f�[�^���X�V���ꂽ�炱�ꂪ��΂��
*/
void ObjectManager::linkDataUpdated()
{
	refMainWindow_->noticeLinkDataUpdated();
}

/*!
	@brief	�����ɗ̈��ǉ�
	@args: r �����̗̈�, selfViewID �����̃r���[ID
*/
bool ObjectManager::createMatchedRegion(ClosedRegion* r, int selfViewID)
{
	// �Ή��t�������݂��Ȃ��ꍇ�����A�V�����̈����邱�Ƃ��ł���
	RegionLinkData* linkData = r->getRegionLinkData();
	int addViewID = (selfViewID == VIEW_MAIN) ? VIEW_SIDE_RIGHT : VIEW_MAIN; //? �Ƃ肠����VIEW_SIDE_RIGHT�ɒǉ�����

	if(addViewID == VIEW_MAIN)
	{
		if(linkData)
			return false;
	}
	else
	{
		Q_ASSERT(linkData);
		if(linkData->getRegion(addViewID))
			return false;
	}

	int newRegionID;
	const IntVec::ubvec3 regionColor = r->getRegionColor();

	ClosedRegion* newRegion = new ClosedRegion;
	newRegion->setRegionColor(regionColor);
	
	if(addViewID == VIEW_MAIN)
	{
		newRegion->setID(currentSrcRegionID_);
		srcFrame_->getRegions().push_back(newRegion);
		currentSrcRegionID_++;
	}
	else
	{
		newRegion->setID(currentDstRegionID_);
		dstFrame_->getRegions().push_back(newRegion);
		currentDstRegionID_++;
	}

	// �̈�}�b�v�쐬
	//? �Ƃ肠�����R�s�[����i�����͎����ŉ�������������悳�����j
	RegionMap& regionMap = newRegion->getRegionMap();
	regionMap.copy(r->getRegionMap());
	newRegion->traceRegionBoundaries();

	// �����N�f�[�^�X�V
	if(addViewID == VIEW_MAIN)
	{
		regionLinkDataManager_.createData(newRegion);
		regionLinkDataManager_.link(newRegion, r, selfViewID);
	}
	else
	{
		regionLinkDataManager_.link(r, newRegion, addViewID);
	}
	
	return true;
}

bool ObjectManager::deleteClosedRegion(ClosedRegion* r, int selfViewID)
{
	AnimeFrame* frame = (selfViewID == VIEW_FRONT) ? srcFrame_ : dstFrame_;
	std::vector<ClosedRegion*>& regions = frame->getRegions();

	// �����N����
	regionLinkDataManager_.clearLink(r);

	// �����N�f�[�^�폜(���C���r���[�̂�)
	if(selfViewID == VIEW_MAIN)
	{
		regionLinkDataManager_.deleteData(r->getRegionLinkData());
	}

	// �̈�폜
	int dataIndex = -1;
	for(int i = 0; i < frame->getRegions().size(); i++)
	{
		int id = frame->getRegions().at(i)->getID();
		if(id == r->getID())
		{
			dataIndex = i;
			break;
		}
	}
	Q_ASSERT(dataIndex != -1);
	delete r;
	regions.erase(frame->getRegions().begin() + dataIndex);

	return true;
}

/*!
	@brief	�̈挋��
*/
bool ObjectManager::combineRegion(ClosedRegion& selectRegion, ImageRGBAu& scribbleBuffer, int viewID)
{
	IDMap dummyIDMap(selectRegion.getRegionMap().getWidth(), selectRegion.getRegionMap().getHeight());
	dummyIDMap.fill(Config::FalseRegionID);

	AnimeFrame* frame = (viewID == VIEW_FRONT) ? srcFrame_ : dstFrame_;

	// �I��̈�Ɠ����F�̗̈��ID�}�b�v���쐬
	//? ID�}�b�v�̓v���C�I���e�B���ɍ��ׂ������ۗ�
	int w = frame->getIDMap().getWidth();
	int h = frame->getIDMap().getHeight();
	for(int i = 0; i < frame->getRegions().size(); i++)
	{
		ClosedRegion* thisRegion = frame->getRegions().at(i);
		if(selectRegion.getID() == thisRegion->getID())
			continue;

		if(selectRegion.getRegionColor() != thisRegion->getRegionColor())
			continue;

		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{ 
				ImageRGBAu& regionMap = thisRegion->getRegionMap();
				if(regionMap(x, y).a != 0)
				{
					RegionID regionID = thisRegion->getID();
					dummyIDMap.setValue(x, y, regionID);
				}
			}
		}
	}

	// �X�N���u�����I�����Ă���̈���X�N���u���o�b�t�@�ɒǉ�
	QVector<int> selectIDs;
	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			bool isScribble = (scribbleBuffer(x, y).a != 0) ? true : false;
			if(isScribble)
			{
				int regionID = dummyIDMap(x, y);
				if(!selectIDs.contains(regionID) && regionID != Config::FalseRegionID)
				{
					selectIDs.append(regionID);
				}
			}
		}
	}

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			int regionID = dummyIDMap(x, y);
			if(regionID != Config::FalseRegionID && selectIDs.contains(regionID))
			{
				IntVec::ubvec4 addColor(0, 255, 0, 255); // �����F�͉��F�ł�����
				scribbleBuffer.setValue(x, y, addColor);
			}
		}
	}

#if 0 // �f�o�b�O
	{
		OpenCVImageIO io;
		io.save("ResultImage/testcombine.png", scribbleBuffer);
	}
#endif
	selectRegion.modifyRegion(scribbleBuffer);


	// �������ꂽ�̈�̍폜
	for(int i = 0; i < selectIDs.size(); i++)
	{
		int dataIndex = -1;

		// �����������`�F�b�N
		int deleteRegionID = selectIDs.at(i);
		for(int j = 0; j < frame->getRegions().size(); j++)
		{
			if(deleteRegionID == frame->getRegions().at(j)->getID())
			{
				dataIndex = j;
				break;
			}
		}
		Q_ASSERT(dataIndex != -1);
		ClosedRegion* deleteRegion = frame->getRegions().at(dataIndex);
		IntVec::ivec2 v0 = deleteRegion->getBoundaryPixels().at(0);
		RegionMap& combinedMap = selectRegion.getRegionMap();
		// ������̗̈�}�b�v�Ɍ������̗̈悪�܂܂�Ă���Ό��������Ƃ݂Ȃ�
		if(combinedMap(v0.x, v0.y).a != 0)
		{
			deleteClosedRegion(deleteRegion, viewID);
		}
	}

	return true;
}

/*!
	@brief	�̈敪��
*/
bool ObjectManager::divideRegion(ClosedRegion& r, ImageRGBAu& scribbleBuffer, int viewID)
{
	
	int w = scribbleBuffer.getWidth();
	int h = scribbleBuffer.getHeight();

	ImageRGBAu dummyScribble;
	dummyScribble.allocate(w, h);
	dummyScribble.fill(IntVec::ubvec4(255,255,255,255));

	// �̈���ɂ���X�N���u�������𒊏o
	bool isEnableDivide = false;
	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			if(scribbleBuffer(x, y).a != 0)
			{
				if(r.getRegionMap().getValue(x, y).a != 0)
				{
					dummyScribble.setValue(x, y, IntVec::ubvec4(0,0,0,0));
					isEnableDivide = true;
				}
			}
		}
	}
	if(!isEnableDivide)
		return false;

	// �̈悩��X�N���u���������폜���A����ɑ΂��̈敪�����s��
	ImageRGBu dummyRegionMap;
	dummyRegionMap.allocate(w, h);
	dummyRegionMap.fill(IntVec::ubvec3(0,0,0));
	ImageRGBAu& regionMap = r.getRegionMap();

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			if(dummyScribble.getValue(x, y).a == 0)
			{
				dummyRegionMap.setValue(x, y, IntVec::ubvec3(0,0,0));
			}
			else
			{
				IntVec::ubvec4 color = regionMap.getValue(x, y);
				dummyRegionMap.setValue(x, y, IntVec::ubvec3(color.r, color.g, color.b));
			}	
		}
	}
	
	SegmentationDriver driver;
	IDMap candidateIDMap;
	candidateIDMap.allocate( w, h );
	candidateIDMap.fill( Config::FalseRegionID );
	std::vector<ClosedRegion*> addCandidateRegions; // �ǉ�����\���̂���̈�
	int *currentRegionID = (viewID == VIEW_MAIN) ? &currentSrcRegionID_ : &currentDstRegionID_;
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			if (candidateIDMap(x, y) == Config::FalseRegionID)
			{
				if(dummyRegionMap(x, y) == IntVec::ubvec3(0,0,0))
				{
					driver.colorFloodFill(x, y, dummyRegionMap, candidateIDMap, Config::BackRegionID);
				}
				else
				{
					// �ǉ����̈���쐬
					driver.colorFloodFill(x, y, dummyRegionMap, candidateIDMap, *currentRegionID);
					const IntVec::ubvec3 regionColor = dummyRegionMap(x, y);
					ClosedRegion* r = new ClosedRegion();
					r->setRegionColor(regionColor);
					r->setID(*currentRegionID);
					addCandidateRegions.push_back( r );
					qDebug("candidate %x", r);
					(*currentRegionID)++;
				}
			}
		}
	}

	// �ǉ����̈�̗̈�}�b�v�쐬
	for(int i = 0; i < addCandidateRegions.size(); i++)
	{
		ClosedRegion* addCandidateRegion = addCandidateRegions.at(i);
		RegionMap& newMap = addCandidateRegion->getRegionMap();
		newMap.allocate(w, h);
		newMap.fill(IntVec::ubvec4(0,0,0,0));
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{
				if(candidateIDMap(x, y) == addCandidateRegion->getID())
				{
					IntVec::ubvec3 rColor = dummyRegionMap(x, y);
					newMap.setValue(x, y, IntVec::ubvec4(rColor.r, rColor.g, rColor.b, 255));
				}
			}
		}
			
#if 0 // �f�o�b�O
		OpenCVImageIO io;
		char str[256];
		sprintf(str, "ResultImage/divide_%d.png", r->getID());
		io.save(str, (r->getRegionMap()) );
#endif
	}


	// �e�̈�ɑ΂���G�b�W�̋������v�Z���A��ԋ߂����̂����̃G�b�W�s�N�Z���̗̈�Ƃ���
	OpenCVImageIO io;
	cv::Mat scribbleMat, grayImage, edgeImage;
	io.convertImageRect2Mat(dummyScribble, scribbleMat);
	cvtColor(scribbleMat, grayImage, CV_RGB2GRAY);
	cv::threshold(grayImage, edgeImage, 0, 255, cv::THRESH_BINARY);

	cv::imwrite("ResultImage/addgray.png", grayImage);
	cv::imwrite("ResultImage/addmono.png", edgeImage);

	ImageRect<float> tmpDistBuffer;
	IDMap   tmpIDBuffer;
	tmpDistBuffer.allocate(w, h);
	tmpDistBuffer.fill(0xffff);
	tmpIDBuffer.allocate(w, h);
	tmpIDBuffer.fill(Config::FalseRegionID);
	ImageGrayf edgeImageRect;
	io.convertMat2ImageRect(edgeImage, edgeImageRect);
	
	for(int i = 0; i < addCandidateRegions.size(); i++)
	{
		ClosedRegion* r = addCandidateRegions.at(i);
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

	std::vector<ClosedRegion*> addRegions; // �ŏI�I�ɒǉ�����̈�
	IDMap   outIDBuffer;
	outIDBuffer.allocate(w, h);
	outIDBuffer.fill(Config::FalseRegionID);
	for(int i = 0; i < addCandidateRegions.size(); i++)
	{
		ClosedRegion* addCandidateRegion = addCandidateRegions.at(i);
		RegionMap& regionMap = addCandidateRegion->getRegionMap();
	
		ImageRGBu tempMap;
		tempMap.allocate(w, h);
		tempMap.fill(IntVec::ubvec3(0,0,0));

		// �X�N���u��������؂������̈�摜�ɁA�����̃X�N���u���̈悾���𑫂��Afloodfill���s��
		// �X�N���u����������Ȃ��Ƃ��͕��f����Ă��Ă��A�X�N���u�����܂߂�ƌ������Ă���̈悪���邽�߁A���̏������s��
		int xSeed, ySeed;
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{
				// �X�N���u���ȊO�̗̈敔���܂��́A�X�N���u�����Ŏ����̗̈�̕���
				if(dummyRegionMap(x, y) != IntVec::ubvec3(0,0,0) || tmpIDBuffer(x, y) == addCandidateRegion->getID())
				{
					IntVec::ubvec3 regionColor = addCandidateRegion->getRegionColor();
					tempMap(x, y) = IntVec::ubvec3(regionColor.r, regionColor.g, regionColor.b);

					if(tmpIDBuffer(x, y) == addCandidateRegion->getID())
					{
						xSeed = x;
						ySeed = y;
					}
				}
			}
		}

#if 1 // �f�o�b�O
		OpenCVImageIO io;
		char str[256];
		sprintf(str, "ResultImage/divide_%d.png", addCandidateRegion->getID());
		io.save(str, (addCandidateRegion->getRegionMap()) );
#endif

		if(outIDBuffer(xSeed, ySeed) == Config::FalseRegionID)
		{
			driver.colorFloodFill(xSeed, ySeed, tempMap, outIDBuffer, addCandidateRegion->getID());
			const IntVec::ubvec3 regionColor = tempMap(xSeed, ySeed);
			ClosedRegion* addRegion = new ClosedRegion();
			addRegion->setRegionColor(regionColor);
			//addRegion->setID(*currentRegionID);
			addRegion->setID(addCandidateRegion->getID());
			addRegions.push_back( addRegion );
			//(*currentRegionID)++;

			qDebug("size %d", addRegions.size());
		}

	}
	
	// �̈�}�b�v�쐬
	for(int i = 0; i < addRegions.size(); i++)
	{
		ClosedRegion* addRegion = addRegions.at(i);
		RegionMap& regionMap = addRegion->getRegionMap();
		regionMap.allocate(w, h);
		regionMap.fill(IntVec::ubvec4(0,0,0,0));

		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{
				if(outIDBuffer(x, y) == addRegion->getID())
				{
					IntVec::ubvec4 color(addRegion->getRegionColor().r, addRegion->getRegionColor().g, addRegion->getRegionColor().b, 255);
					regionMap.setValue(x, y, color);
				}
			}
		}
		
		addRegion->traceRegionBoundaries();

		// �����N�f�[�^�ǉ�
		if(viewID == VIEW_MAIN)
		{
			regionLinkDataManager_.createData(addRegion);
			srcFrame_->getRegions().push_back(addRegion);
		}
		else
		{
			dstFrame_->getRegions().push_back(addRegion);
		}
		qDebug("add %x, id %d", addRegion, addRegion->getID());
	}

	// ���̈���폜
	for(int i = 0; i < addCandidateRegions.size(); i++)
	{
		ClosedRegion* r = addCandidateRegions.at(i);
		qDebug("delete candidate %x", r);
		delete r;
	}
	addCandidateRegions.clear();

	// �I��̈���폜
	deleteClosedRegion(&r, viewID);

	return true;
}

/*!
	@brief	�e�r���[�ł̌����Z�b�g�iX������̉�]�̂��ƁA�x������̉�]���s���j
*/
void ObjectManager::setSrcRotation(QVector2D rot)
{
	QMatrix4x4 matRotX, matRotY;
	float x = rot.x();
	x = (x > 90.0f) ? 90.0f : x;
	x = (x < -90.0f) ? -90.0f : x;

	float y = rot.y();
	y = (y > 180.0f) ? 180.0f : y;
	y = (y < -180.0f) ? -180.0f : y;

	srcRot_ = QVector2D(x, y);

	matRotX.rotate(x, 1, 0, 0);
	matRotY.rotate(y, 0, 1, 0);
	srcPoseMatrix_ = matRotY * matRotX;
}

void ObjectManager::setDstRotation(QVector2D rot)
{
	QMatrix4x4 matRotX, matRotY;
	float x = rot.x();
	x = (x > 90.0f) ? 90.0f : x;
	x = (x < -90.0f) ? -90.0f : x;

	float y = rot.y();
	y = (y > 180.0f) ? 180.0f : y;
	y = (y < -180.0f) ? -180.0f : y;

	dstRot_ = QVector2D(x, y);
	matRotX.rotate(x, 1, 0, 0);
	matRotY.rotate(y, 0, 1, 0);
	dstPoseMatrix_ = matRotY * matRotX;
}

void ObjectManager::reCalcDepth()
{
	regionLinkDataManager_.reCalcDepth();
}

void ObjectManager::changeEdgeWidth(int w)
{
	edgeWidth_ = w;
	DepthViewBase* refDepthView = refMainWindow_->getDepthView();
	if(refDepthView)
	{
		refDepthView->update();
	}
}
