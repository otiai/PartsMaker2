#include "ClosedRegion.h"
#include <cstdlib>
#include <QGLWidget>
#include "OpenCVImageIO.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Config.h"
#include "Utility.h"
#include <QVector2D>
#include "RegionMatchHandler.h"
#include "ObjectManager.h"


using namespace std;

#if _DEBUG
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_core241d.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_highgui241d.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_imgproc241d.lib")
#else
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_core241.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_highgui241.lib")
#pragma comment(lib,"C:/opencv/build/x86/vc10/lib/opencv_imgproc241.lib")
#endif

static const float sDummyValue = 128;


void Line::setPoints(QVector<QVector2D>& points)
{
	points_.clear();
	for(int i = 0; i < points.size(); i++)
	{
		points_.append(points.at(i));
	}
}

ClosedRegion::ClosedRegion()
{
	m_RegionColor = IntVec::ubvec3(0,0,0);
	m_RegionMap = new RegionMap;
	m_RegionLinkDataPtr = NULL;
	m_Pos3D = QVector3D(0,0,0);
}

ClosedRegion::~ClosedRegion()
{
	if(m_RegionMap)
	{
		delete m_RegionMap;
	}

	for(int i = 0; i < m_Lines.size(); i++)
	{
		Line* line = m_Lines.at(i);
		delete line;
	}
	m_Lines.clear();

	resetFeaturePoint();
}

void ClosedRegion::drawBoundary() const
{
	if (m_BoundaryPixels.empty()) return;

	glBegin(GL_POINTS);
	for (int i=0; i<(int)m_BoundaryPixels.size(); i++)
		glVertex2iv(m_BoundaryPixels[i]);
	glEnd();
}

void ClosedRegion::fillHoles()
{
	OpenCVImageIO io;
	cv::Mat matRegion;
	io.convertImageRect2Mat(*m_RegionMap, matRegion);
	cv::Mat matRegionGray;
	cvtColor(matRegion, matRegionGray,CV_RGB2GRAY);
	cv::Mat matBin;
	cv::threshold(matRegionGray, matBin, 0, 255, cv::THRESH_BINARY);

	// ����傫���_�~�[�f�[�^�����A�O���̗̈��h��Ԃ�
	cv::Mat dummy(matBin.rows + 2, matBin.cols + 2, CV_8UC1);
	dummy = 0;
	int w = matBin.cols;
	int h = matBin.rows;
	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			dummy.at<unsigned char>(y+1, x+1) = matBin.at<unsigned char>(y, x);
		}
	}
	cv::floodFill(dummy, cv::Point(0,0), sDummyValue);

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			float v = dummy.at<unsigned char>(h - y, x+1); // Mat��(�s,��)�Ƃ����w��
			// �O���̗̈�łȂ���΂��ׂĎ����̗̈�ɂȂ�
			if(v != sDummyValue)
			{
				IntVec::ubvec4 color(m_RegionColor.r, m_RegionColor.g, m_RegionColor.b, 255);
				m_RegionMap->setValue(x, y, color);
			}
		}
	}

	traceRegionBoundaries();

#if 0 // �f�o�b�O
	char str[256];
	sprintf(str, "ResultImage/dummy_%d.png", getID());
	io.save(str, *m_RegionMap);
#endif
}

bool ClosedRegion::modifyRegion(ImageRGBAu& scribbleBuffer)
{
	int w = m_RegionMap->getWidth();
	int h = m_RegionMap->getHeight();

	if(w != scribbleBuffer.getWidth() || h != scribbleBuffer.getHeight())
	{
		return false;
	}

	// �X�N���u���o�b�t�@�̒l��ID�}�b�v�ɃR�s�[
	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			IntVec::ubvec4 color = scribbleBuffer(x, y);
			if(color.a != 0)
			{
				IntVec::ubvec4 regionColor(m_RegionColor.r, m_RegionColor.g, m_RegionColor.b, 255);
				m_RegionMap->setValue(x, y, regionColor);
			}
		}
	}

	//
	// �����̗̈�ƂȂ����Ă��Ȃ��������폜����
	//
	// cv::floodFill��4�`�����l���摜�������Ȃ��̂ŁA�O���[�X�P�[���摜�ɂ���floodfill���s��
	OpenCVImageIO io;
	cv::Mat matRegion;
	io.convertImageRect2Mat(*m_RegionMap, matRegion);
	cv::Mat matRegionGray;
	cvtColor(matRegion, matRegionGray,CV_RGB2GRAY);

	int seedX = m_BoundaryPixels.at(0).x;
	int seedY = h - m_BoundaryPixels.at(0).y - 1;

	unsigned char regionValue = matRegionGray.at<unsigned char>(seedY, seedX);
	unsigned char dummy = 128;
	if(regionValue == dummy)
	{
		dummy = 200;
	}

	cv::floodFill(matRegionGray, cv::Point(seedX, seedY), dummy);
	
	// regionValue�̕��������������̗̈�Ƃ���
	m_RegionMap->fill(IntVec::ubvec4(0,0,0,0));
	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			unsigned char v = matRegionGray.at<unsigned char>(h - y - 1, x);
			if(v == dummy)
			{
				IntVec::ubvec4 regionColor(m_RegionColor.r, m_RegionColor.g, m_RegionColor.b, 255);
				m_RegionMap->setValue(x, y, regionColor);
			}
		}
	}


	fillHoles();

	// �����_�ƃ��C���̏C��
	if(m_RegionLinkDataPtr)
	{
		m_RegionLinkDataPtr->calcDepth();
		m_RegionLinkDataPtr->resetFeaturePoint();
		m_RegionLinkDataPtr->createLines();
	}

#if 0 // �f�o�b�O
	{
		OpenCVImageIO io;
		io.save("ResultImage/testmodify.png", *m_RegionMap);
	}
#endif
	return true;
}

/*!
	@brief	�̈�}�b�v�̏�������
*/
bool ClosedRegion::replaceRegionMap(QImage& replaceRegion)
{
	RegionMap& map = getRegionMap();
	Utility::convertQImage2ImageRGBAu(replaceRegion, map);
	traceRegionBoundaries();

	// �����_�ƃ��C���̏C��
	if(m_RegionLinkDataPtr)
	{
		m_RegionLinkDataPtr->calcDepth();
		m_RegionLinkDataPtr->resetFeaturePoint();
		m_RegionLinkDataPtr->createLines();
	}

	return true;
}

void ClosedRegion::traceRegionBoundaries()
{
	const RegionMap &regionMap(*m_RegionMap);
	const int w = regionMap.getWidth();
	const int h = regionMap.getHeight();

	// �̈�̋��E�̃s�N�Z�����N���A���Ă���
	m_BoundaryPixels.clear();
	
	ImageRect<bool> visitedMap(w,h);
	visitedMap.fill( false );
	for (int yi=0; yi<h; yi++)
	{
		for (int xi=0; xi<w; xi++)
		{
			IntVec::ubvec4 color = regionMap(xi, yi);
			
			if ( color != IntVec::ubvec4(0,0,0,0) && isBoundary(color,xi,yi,regionMap) && ! visitedMap(xi,yi) ) // ���̃s�N�Z�������E�̃s�N�Z���Ȃ�
			{
				vector<IntVec::ivec2> &boundaryPixels = getBoundaryPixels();

				int xj = xi;
				int yj = yi;

				do {
					visitedMap(xj,yj) = true;
					boundaryPixels.push_back( IntVec::ivec2(xj,yj) );

					if ( boundaryPixels.size() >= 100000 )
					{
						cerr << __FUNCTION__ << ": too many boundary pixels" << endl;
						break;
					}

					if (xj>0 && yj>0 && isBoundary(color,xj-1,yj-1,regionMap) && !visitedMap(xj-1,yj-1))
					{
						xj--;
						yj--;
					}
					else if (yj>0 && isBoundary(color,xj,yj-1,regionMap) && !visitedMap(xj,yj-1))
					{
						yj--;
					}
					else if (xj<w-1 && yj>0 && isBoundary(color,xj+1,yj-1,regionMap) && !visitedMap(xj+1,yj-1))
					{
						xj++;
						yj--;
					}
					else if (xj<w-1 && isBoundary(color,xj+1,yj,regionMap) && !visitedMap(xj+1,yj))
					{
						xj++;
					}
					else if (xj<w-1 && yj<h-1 && isBoundary(color,xj+1,yj+1,regionMap) && !visitedMap(xj+1,yj+1))
					{
						xj++;
						yj++;
					}
					else if (yj<h-1 && isBoundary(color,xj,yj+1,regionMap) && !visitedMap(xj,yj+1))
					{
						yj++;
					}
					else if (xj>0 && yj<h-1 && isBoundary(color,xj-1,yj+1,regionMap) && !visitedMap(xj-1,yj+1))
					{
						xj--;
						yj++;
					}
					else if (xj>0 && isBoundary(color,xj-1,yj,regionMap) && !visitedMap(xj-1,yj))
					{
						xj--;
					}
					else
					{
						break;	// �Ȃɂ������Ȃ�����
					}
				} while ( ! (xj==xi && yj==yi) );
			}
		}
	}
	
	// �o�E���f�B���O�{�b�N�X�̌v�Z
	setBboxMin( w, h );
	setBboxMax( 0, 0 );
	const vector<IntVec::ivec2> &boundary = getBoundaryPixels();
	for (int bi=0; bi<(int)boundary.size(); bi++)
	{
		const IntVec::ivec2 p = boundary[bi];
		const IntVec::ivec2 bMin = getBboxMin();
		const IntVec::ivec2 bMax = getBboxMax();

		setBboxMin( min(bMin.x,p.x), min(bMin.y,p.y) );
		setBboxMax( max(bMax.x,p.x), max(bMax.y,p.y) );
	}

	// ���E�̒��_�����Ԃɐ���
	serializeRegionBoundaries();
}

/*!
	@brief	�����_���X�g���Z�b�g
*/
void ClosedRegion::resetFeaturePoint()
{
	// 0�Ԗڂ͕K��0
	m_FeaturePointIndices.clear();
	m_FeaturePointIndices.append(0);
}


/*!
	@brief	�����_�ǉ�
	@note	�����_�ƂȂ�s�N�Z���C���f�b�N�X������
			�f�[�^�͏����ɓ���
*/
int ClosedRegion::addFeaturePoint(int pixelIndex)
{
	Q_ASSERT( 0 < pixelIndex && pixelIndex < m_BoundaryPixels.size() );
	
	// ���łɓ����_�ɐݒ肳��Ă����牽�����Ȃ�
	if(m_FeaturePointIndices.indexOf(pixelIndex) != -1)
		return -1;

	int index = -1;
	if(m_FeaturePointIndices.size() > 1)
	{
		for(int i = 0; i < m_FeaturePointIndices.size() - 1; i++)
		{
			int prev = m_FeaturePointIndices.at(i);
			int next = m_FeaturePointIndices.at(i+1);

			// �f�[�^�̒��Ԃɑ}������ꍇ
			if(prev < pixelIndex && pixelIndex < next)
			{
				index = i+1;
				m_FeaturePointIndices.insert(index, pixelIndex);
				return index;
			}
		}
		// �f�[�^�̍Ō�ɑ}��
		m_FeaturePointIndices.append(pixelIndex);
		index = m_FeaturePointIndices.size() - 1;
		return index;
	}
	else
	{
		// �����_���n�_�����Ȃ��ꍇ
		m_FeaturePointIndices.append(pixelIndex);
		index = m_FeaturePointIndices.size() - 1;
	}
	return index;
}

/*!
	@brief	�����_�ǉ�
	@note	mappedRatio����ԓ��ɓ���΂��̒l���Z�b�g�A����Ȃ���ΑO��̒��Ԓl���Z�b�g
*/
void ClosedRegion::addFeaturePoint(int insertIndex, float mappedRatio)
{
	// ��������
	Q_ASSERT( 0 < insertIndex && insertIndex <= m_FeaturePointIndices.size() );

	int refPixelIndex = m_BoundaryPixels.size() * mappedRatio;
	int prev = m_FeaturePointIndices.at(insertIndex - 1);
	int next = (m_FeaturePointIndices.size() <= insertIndex) ? m_BoundaryPixels.size() - 1 : m_FeaturePointIndices.at(insertIndex);
	int pixelIndex = (prev < refPixelIndex && refPixelIndex < next) ? refPixelIndex : (prev + next) / 2;

	m_FeaturePointIndices.insert(insertIndex, pixelIndex);
}

/*!
	@brief	�����_�f�[�^��������
	@note	0�Ԗڂ͏����������Ȃ�
*/
void ClosedRegion::setFeaturePoint(int dataIndex, int pixelIndex)
{
	Q_ASSERT( 0 < dataIndex && dataIndex < m_FeaturePointIndices.size() );
	Q_ASSERT( 0 < pixelIndex && pixelIndex < m_BoundaryPixels.size() );

	int prev = m_FeaturePointIndices.at(dataIndex - 1);
	int next = (m_FeaturePointIndices.size() <= dataIndex + 1) ?  m_BoundaryPixels.size() - 1 : m_FeaturePointIndices.at(dataIndex+1);
	Q_ASSERT( prev < pixelIndex && pixelIndex < next);
	m_FeaturePointIndices.replace(dataIndex, pixelIndex);
}

/*!
	@brief	���E�s�N�Z���̎n�_�̈ʒu��ς���
*/
void ClosedRegion::setBoundaryStartPoint(int index)
{
	int dataSize = m_BoundaryPixels.size();
	IntVec::ivec2* buf = new IntVec::ivec2[dataSize];
	for(int i = 0; i < dataSize; i++)
	{
		buf[i] = m_BoundaryPixels.at(i);
	}

	m_BoundaryPixels.clear();
	for(int i = 0; i < dataSize; i++)
	{
		IntVec::ivec2 val = buf[ (i + index) % dataSize ];
		m_BoundaryPixels.push_back(val);
	}

	delete [] buf;
}

/*!
	@brief ���E�s�N�Z�������Ԃɕ��ׂ�
*/
void ClosedRegion::serializeRegionBoundaries()
{
	vector<IntVec::ivec2> &boundaryPixels = getBoundaryPixels();
	if(boundaryPixels.size() < 1)
		return;

	QVector<QVector2D> buffer;
	for(int i = 1; i < boundaryPixels.size(); i++)
	{
		buffer.push_back(QVector2D(boundaryPixels.at(i).x, boundaryPixels.at(i).y));
	}

	vector<IntVec::ivec2> outDatas;
	outDatas.push_back(boundaryPixels.at(0));
	QVector2D current;
	current = QVector2D(boundaryPixels.at(0).x, boundaryPixels.at(0).y);
	for(int i = 0; i < boundaryPixels.size() - 1; i++)
	{
		int minIndex = -1;
		float minValue = 0xffff;
		for(int j = 0; j < buffer.size(); j++)
		{
			QVector2D p = buffer.at(j);
			float dist = (p - current).length();
			if(dist < minValue)
			{
				minIndex = j;
				minValue = dist;
			}
		}

		// �ɒ[�ɗ��ꂷ�����ꍇ�́A���E���_���X�g�ɓ���Ȃ�
		if(minValue > 10)
		{
			break;
		}
		IntVec::ivec2 minVec;
		minVec.x = buffer.at(minIndex).x();
		minVec.y = buffer.at(minIndex).y();
		outDatas.push_back(minVec);
		current = buffer.at(minIndex);
		buffer.remove(minIndex);
	}

	boundaryPixels.clear();
	bool isClockwise = Utility::isClockwise(outDatas);
	if(isClockwise)
	{
		for(int i = 0; i < outDatas.size(); i++)
		{
			boundaryPixels.push_back(outDatas.at(i));
		}
	}
	else
	{
		for(int i = 0; i < outDatas.size(); i++)
		{
			boundaryPixels.push_back(outDatas.at(outDatas.size() - i - 1));
		}
	}
}

/*!
	@brief	���E�s�N�Z��������_���ɕ���
	@note	�̈��Ή��t�����Ƃ��Ɠ����_���Z�b�g�����Ƃ��Ƀ��C�������
*/
void ClosedRegion::createLines()
{
	for(int i = 0; i < m_Lines.size(); i++)
	{
		Line* line = m_Lines.at(i);
		delete line;
	}
	m_Lines.clear();

	Line* currentLine = new Line;
	bool* isFeaturePoint = new bool[m_BoundaryPixels.size()];
	for(int i = 0; i < m_BoundaryPixels.size(); i++)
	{
		isFeaturePoint[i] = false;
	}

	for(int i = 0; i < m_FeaturePointIndices.size(); i++)
	{
		int idx = m_FeaturePointIndices.at(i);
		isFeaturePoint[idx] = true;
	}

	for(int i = 0; i < m_BoundaryPixels.size(); i++)
	{
		IntVec::ivec2 currentPoint = m_BoundaryPixels.at(i);
		// �����_�̂Ƃ��͐V�������C���N���X����
		// ��ԍŌオ�����_�̏ꍇ�͖�������
		if(isFeaturePoint[i] && i != m_BoundaryPixels.size() - 1)
		{
			m_Lines.append(currentLine);
			currentLine = new Line;
		}
		currentLine->getPoints().append(QVector2D(currentPoint.x, currentPoint.y));
	}
	m_Lines.append(currentLine);

	delete [] isFeaturePoint;
}


bool ClosedRegion::isBoundary( IntVec::ubvec4 color, int xi, int yi, const RegionMap &regionMap ) const
{
	const int w = regionMap.getWidth();
	const int h = regionMap.getHeight();

	const IntVec::ubvec4 newColor = regionMap(xi,yi);

	return (color == newColor) && (xi==0 || xi==w-1 || yi==0 || yi==h-1 || 
		color != regionMap(xi-1,yi) || color != regionMap(xi,yi-1) || color != regionMap(xi+1,yi) || color != regionMap(xi,yi+1));
	return true;
}

/*!
	@brief	2�̗̈�̗ގ��x���v�Z
	@note	�{��@
*/
float ClosedRegion::calcSimirarity_Ours(ClosedRegion& a, ClosedRegion& b)
{
	float ret = 0.0f;

	//
	// Jab���v�Z
	//
	float Jab, Tc, Cab, Rab, Hc, Hn;
	Tc = 0.3f; // �萔


	// RGB�F��Ԃł̃��[�N���b�h���������߂�
	IntVec::ubvec3 colorA, colorB;
	colorA.r = a.getRegionColor().r;	colorA.g = a.getRegionColor().g;	colorA.b = a.getRegionColor().b;
	colorB.r = b.getRegionColor().r;	colorB.g = b.getRegionColor().g;	colorB.b = b.getRegionColor().b;
	Cab = Utility::calcColorDistance(colorA, colorB);
	if(Tc - Cab < 0)
		return 0;
	else
		Hc = 1.0f;

	// �̈�Ԃ̋��������߂�
	// ���W�̓o�E���f�B���O�{�b�N�X�̒��S�Ƃ���
	// ��]�������Ɉړ�����Ɨގ��x��������
	float ax = (a.getBboxMax().x + a.getBboxMin().x) / 2.0f;
	float ay = (a.getBboxMax().y + a.getBboxMin().y) / 2.0f;
	float bx = (b.getBboxMax().x + b.getBboxMin().x) / 2.0f;
	float by = (b.getBboxMax().y + b.getBboxMin().y) / 2.0f;

	// �x���܂��̉�]�̓X�N���[�����W��x�����ɉe�����AX������̉�]��y�����ɉe������
	ObjectManager* mgr = ObjectManager::getInstance();
	const QVector2D& srcRot = mgr->getSrcRotation();
	const QVector2D& dstRot = mgr->getDstRotation();
	QVector2D change = srcRot - dstRot;
	float wx = 0.5f;
	float wy = 0.5f;
	if(change.x() != 0)
	{
		wy = 1.0f;
	}
	if(change.y() != 0)
	{
		wx = 1.0f;
	}
	
	bool isChangeX;
	Hn = exp(-abs(ay - by) * wx) * exp(-abs((ax - bx) * wy));
	Jab = Hc * Hn;

	// �ʐς̗ގ��x�����߂�
	float Ra, Rb, e;
	Ra = calcSize(a);
	Rb = calcSize(b);
	e = - abs(Ra - Rb) / ((Ra + Rb)*0.5f);
	Rab = exp(e);

	ret = Jab * Rab;
	return ret;
}

/*!
	@brief	2�̗̈�̗ގ��x���v�Z
	@note	"Stereoscopizing Cel Animations"��Region Correspondence and Smoothness Cost�̌v�Z
*/
float ClosedRegion::calcSimirarity(ClosedRegion& a, ClosedRegion& b)
{
	float ret = 0.0f;

	//
	// Jab���v�Z
	//
	float Jab, Tc, Cab, Tn, Nab, Hc, Hn;
	Tc = 0.3f; // �萔
	Tn = 0.1f; // �萔


	// RGB�F��Ԃł̃��[�N���b�h���������߂�
	IntVec::ubvec3 colorA, colorB;
	colorA.r = a.getRegionColor().r;	colorA.g = a.getRegionColor().g;	colorA.b = a.getRegionColor().b;
	colorB.r = b.getRegionColor().r;	colorB.g = b.getRegionColor().g;	colorB.b = b.getRegionColor().b;
	Cab = Utility::calcColorDistance(colorA, colorB);
	if(Tc - Cab < 0)
		return 0;
	else
		Hc = 1.0f;

	// �̈�Ԃ̋��������߂�A�d�Ȃ��Ă���Ƃ��͋�����0
	Nab = calcSmallestEuclideanDistance(a, b);
	if(Tn - Nab < 0)
		return 0;
	else
		Hn = 1.0f;

	Jab = Hc * Hn;

	//
	// �E���̎����v�Z
	//
	float Oab, Ra, Rb, Ha, Hb;
	Oab = calcOverlapSize(a, b);
	Ra = calcSize(a);
	Rb = calcSize(b);
	float ya = calcPerimeter(a);
	Ha = Ra / ya;
	float yb = calcPerimeter(b);
	Hb = Rb / yb;

	float minR = (Ra < Rb) ? Ra : Rb;
	float minH = (Ha < Hb) ? Ha : Hb;

	float Left, Right;
	Left = Oab / minR;
	float e = -(abs(Ra - Rb) / minR) - (abs(Ha - Hb) / minH);
	Right = exp(e);

	float Value = (Left > Right) ? Left : Right;

	//qDebug("Oab = %f, Ra = %f, Rb = %f, Ha = %f, Hb = %f", Oab, Ra, Rb, Ha, Hb);
	//qDebug("Left = %f, Right = %f", Left, Right);
	ret = Jab * Value;
	return ret;
}
/*!
	@brief	�̈�̍ŏ����[�N���b�h���������߂�
*/
float ClosedRegion::calcSmallestEuclideanDistance(ClosedRegion& a, ClosedRegion& b)
{
	float ret = 0xffffff;

	// �̈悪�d�Ȃ��Ă���΁A�̈�Ԃ̋�����0
	if(calcOverlapSize(a, b) > 0)
		return 0;

	std::vector<IntVec::ivec2>& ba = a.getBoundaryPixels();
	std::vector<IntVec::ivec2>& bb = b.getBoundaryPixels();

	for(int j = 0; j < bb.size(); j++)
	{
		QVector2D vb((float)bb.at(j).x / b.getRegionMap().getWidth(), (float)bb.at(j).y / b.getRegionMap().getHeight());
		
		for(int i = 0; i < ba.size(); i++)
		{
			QVector2D va((float)ba.at(i).x / a.getRegionMap().getWidth(), (float)ba.at(i).y / a.getRegionMap().getHeight());
			float dist = QVector2D(va - vb).length();
			if(dist < ret)
				ret = dist;
		}
	}

	return ret;
}

/*!
	@brief	�Q�̗̈�̏d�Ȃ��������̖ʐς����߂�
*/
int ClosedRegion::calcOverlapSize(ClosedRegion& a, ClosedRegion& b)
{
	int ret = 0;
	RegionMap& ra = a.getRegionMap();
	RegionMap& rb = b.getRegionMap();

	if(ra.getWidth() != rb.getWidth() || ra.getHeight() != rb.getHeight())
	{
		// �̈�}�b�v�̑傫���͓����łȂ��Ƃ����Ȃ�
		return -1;
	}

	int w = ra.getWidth();
	int h = ra.getHeight();

	for(int j = 0; j < h; j++)
	{
		for(int i = 0; i < w; i++)
		{
			IntVec::ubvec4 colorA = ra.getValue(i, j);
			IntVec::ubvec4 colorB = rb.getValue(i, j);
			if((colorA.r != 0.0 || colorA.g != 0.0 || colorA.b != 0.0) &&
			   (colorB.r != 0.0 || colorB.g != 0.0 || colorB.b != 0.0))
			{
				ret++;
			}
		}
	}
	return ret;
}

/*!
	@brief	�ʐς��v�Z
*/
int ClosedRegion::calcSize(ClosedRegion& r)
{
	int ret = 0;
	RegionMap& map = r.getRegionMap();
	for(int j = 0; j < map.getHeight(); j++)
	{
		for(int i = 0; i < map.getWidth(); i++)
		{
			if(map.getValue(i, j).r != 0.0 || map.getValue(i, j).g != 0.0 || map.getValue(i, j).b != 0.0)
			{
				ret++;
			}
		}
	}
	return ret;
}

/*!
	@brief	���͂̒������v�Z
*/
float ClosedRegion::calcPerimeter(ClosedRegion& r)
{
	float ret = 0.0f;
	std::vector<IntVec::ivec2>& boundaryPixels = r.getBoundaryPixels();
	QVector2D p0, p1;
	p0.setX(boundaryPixels.at(0).x);
	p0.setY(boundaryPixels.at(0).y);
	for(int i = 1; i < boundaryPixels.size(); i++)
	{
		p1.setX(boundaryPixels.at(i).x);
		p1.setY(boundaryPixels.at(i).y);
		float dist = QVector2D(p1 - p0).length();
		ret += dist;
		p0 = p1;
	}
	
	// �I�_�Ǝn�_�܂ł̋�����������
	p1.setX(boundaryPixels.at(0).x);
	p1.setY(boundaryPixels.at(0).y);
	float dist = QVector2D(p1 - p0).length();
	ret += dist;

	return ret;
}
