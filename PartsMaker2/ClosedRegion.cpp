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

	// 一回り大きいダミーデータを作り、外側の領域を塗りつぶす
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
			float v = dummy.at<unsigned char>(h - y, x+1); // Matは(行,列)という指定
			// 外側の領域でなければすべて自分の領域になる
			if(v != sDummyValue)
			{
				IntVec::ubvec4 color(m_RegionColor.r, m_RegionColor.g, m_RegionColor.b, 255);
				m_RegionMap->setValue(x, y, color);
			}
		}
	}

	traceRegionBoundaries();

#if 0 // デバッグ
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

	// スクリブルバッファの値をIDマップにコピー
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
	// 自分の領域とつながっていない部分を削除する
	//
	// cv::floodFillは4チャンネル画像を扱えないので、グレースケール画像にしてfloodfillを行う
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
	
	// regionValueの部分だけを自分の領域とする
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

	// 特徴点とラインの修正
	if(m_RegionLinkDataPtr)
	{
		m_RegionLinkDataPtr->calcDepth();
		m_RegionLinkDataPtr->resetFeaturePoint();
		m_RegionLinkDataPtr->createLines();
	}

#if 0 // デバッグ
	{
		OpenCVImageIO io;
		io.save("ResultImage/testmodify.png", *m_RegionMap);
	}
#endif
	return true;
}

/*!
	@brief	領域マップの書き換え
*/
bool ClosedRegion::replaceRegionMap(QImage& replaceRegion)
{
	RegionMap& map = getRegionMap();
	Utility::convertQImage2ImageRGBAu(replaceRegion, map);
	traceRegionBoundaries();

	// 特徴点とラインの修正
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

	// 領域の境界のピクセルをクリアしておく
	m_BoundaryPixels.clear();
	
	ImageRect<bool> visitedMap(w,h);
	visitedMap.fill( false );
	for (int yi=0; yi<h; yi++)
	{
		for (int xi=0; xi<w; xi++)
		{
			IntVec::ubvec4 color = regionMap(xi, yi);
			
			if ( color != IntVec::ubvec4(0,0,0,0) && isBoundary(color,xi,yi,regionMap) && ! visitedMap(xi,yi) ) // そのピクセルが境界のピクセルなら
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
						break;	// なにも動かなかった
					}
				} while ( ! (xj==xi && yj==yi) );
			}
		}
	}
	
	// バウンディングボックスの計算
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

	// 境界の頂点を順番に整列
	serializeRegionBoundaries();
}

/*!
	@brief	特徴点リストリセット
*/
void ClosedRegion::resetFeaturePoint()
{
	// 0番目は必ず0
	m_FeaturePointIndices.clear();
	m_FeaturePointIndices.append(0);
}


/*!
	@brief	特徴点追加
	@note	特徴点となるピクセルインデックスを入れる
			データは昇順に入る
*/
int ClosedRegion::addFeaturePoint(int pixelIndex)
{
	Q_ASSERT( 0 < pixelIndex && pixelIndex < m_BoundaryPixels.size() );
	
	// すでに特徴点に設定されていたら何もしない
	if(m_FeaturePointIndices.indexOf(pixelIndex) != -1)
		return -1;

	int index = -1;
	if(m_FeaturePointIndices.size() > 1)
	{
		for(int i = 0; i < m_FeaturePointIndices.size() - 1; i++)
		{
			int prev = m_FeaturePointIndices.at(i);
			int next = m_FeaturePointIndices.at(i+1);

			// データの中間に挿入する場合
			if(prev < pixelIndex && pixelIndex < next)
			{
				index = i+1;
				m_FeaturePointIndices.insert(index, pixelIndex);
				return index;
			}
		}
		// データの最後に挿入
		m_FeaturePointIndices.append(pixelIndex);
		index = m_FeaturePointIndices.size() - 1;
		return index;
	}
	else
	{
		// 特徴点が始点しかない場合
		m_FeaturePointIndices.append(pixelIndex);
		index = m_FeaturePointIndices.size() - 1;
	}
	return index;
}

/*!
	@brief	特徴点追加
	@note	mappedRatioが区間内に入ればその値をセット、入らなければ前後の中間値をセット
*/
void ClosedRegion::addFeaturePoint(int insertIndex, float mappedRatio)
{
	// ここから
	Q_ASSERT( 0 < insertIndex && insertIndex <= m_FeaturePointIndices.size() );

	int refPixelIndex = m_BoundaryPixels.size() * mappedRatio;
	int prev = m_FeaturePointIndices.at(insertIndex - 1);
	int next = (m_FeaturePointIndices.size() <= insertIndex) ? m_BoundaryPixels.size() - 1 : m_FeaturePointIndices.at(insertIndex);
	int pixelIndex = (prev < refPixelIndex && refPixelIndex < next) ? refPixelIndex : (prev + next) / 2;

	m_FeaturePointIndices.insert(insertIndex, pixelIndex);
}

/*!
	@brief	特徴点データ書き換え
	@note	0番目は書き換えられない
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
	@brief	境界ピクセルの始点の位置を変える
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
	@brief 境界ピクセルを順番に並べる
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

		// 極端に離れすぎた場合は、境界頂点リストに入れない
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
	@brief	境界ピクセルを特徴点毎に分割
	@note	領域を対応付けたときと特徴点をセットしたときにラインを作る
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
		// 特徴点のときは新しいラインクラス生成
		// 一番最後が特徴点の場合は無視する
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
	@brief	2つの領域の類似度を計算
	@note	本手法
*/
float ClosedRegion::calcSimirarity_Ours(ClosedRegion& a, ClosedRegion& b)
{
	float ret = 0.0f;

	//
	// Jabを計算
	//
	float Jab, Tc, Cab, Rab, Hc, Hn;
	Tc = 0.3f; // 定数


	// RGB色空間でのユークリッド距離を求める
	IntVec::ubvec3 colorA, colorB;
	colorA.r = a.getRegionColor().r;	colorA.g = a.getRegionColor().g;	colorA.b = a.getRegionColor().b;
	colorB.r = b.getRegionColor().r;	colorB.g = b.getRegionColor().g;	colorB.b = b.getRegionColor().b;
	Cab = Utility::calcColorDistance(colorA, colorB);
	if(Tc - Cab < 0)
		return 0;
	else
		Hc = 1.0f;

	// 領域間の距離を求める
	// 座標はバウンディングボックスの中心とする
	// 回転軸方向に移動すると類似度が下がる
	float ax = (a.getBboxMax().x + a.getBboxMin().x) / 2.0f;
	float ay = (a.getBboxMax().y + a.getBboxMin().y) / 2.0f;
	float bx = (b.getBboxMax().x + b.getBboxMin().x) / 2.0f;
	float by = (b.getBboxMax().y + b.getBboxMin().y) / 2.0f;

	// Ｙ軸まわりの回転はスクリーン座標のx成分に影響し、X軸周りの回転はy成分に影響する
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

	// 面積の類似度を求める
	float Ra, Rb, e;
	Ra = calcSize(a);
	Rb = calcSize(b);
	e = - abs(Ra - Rb) / ((Ra + Rb)*0.5f);
	Rab = exp(e);

	ret = Jab * Rab;
	return ret;
}

/*!
	@brief	2つの領域の類似度を計算
	@note	"Stereoscopizing Cel Animations"のRegion Correspondence and Smoothness Costの計算
*/
float ClosedRegion::calcSimirarity(ClosedRegion& a, ClosedRegion& b)
{
	float ret = 0.0f;

	//
	// Jabを計算
	//
	float Jab, Tc, Cab, Tn, Nab, Hc, Hn;
	Tc = 0.3f; // 定数
	Tn = 0.1f; // 定数


	// RGB色空間でのユークリッド距離を求める
	IntVec::ubvec3 colorA, colorB;
	colorA.r = a.getRegionColor().r;	colorA.g = a.getRegionColor().g;	colorA.b = a.getRegionColor().b;
	colorB.r = b.getRegionColor().r;	colorB.g = b.getRegionColor().g;	colorB.b = b.getRegionColor().b;
	Cab = Utility::calcColorDistance(colorA, colorB);
	if(Tc - Cab < 0)
		return 0;
	else
		Hc = 1.0f;

	// 領域間の距離を求める、重なっているときは距離は0
	Nab = calcSmallestEuclideanDistance(a, b);
	if(Tn - Nab < 0)
		return 0;
	else
		Hn = 1.0f;

	Jab = Hc * Hn;

	//
	// 右側の式を計算
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
	@brief	領域の最小ユークリッド距離を求める
*/
float ClosedRegion::calcSmallestEuclideanDistance(ClosedRegion& a, ClosedRegion& b)
{
	float ret = 0xffffff;

	// 領域が重なっていれば、領域間の距離は0
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
	@brief	２つの領域の重なった部分の面積を求める
*/
int ClosedRegion::calcOverlapSize(ClosedRegion& a, ClosedRegion& b)
{
	int ret = 0;
	RegionMap& ra = a.getRegionMap();
	RegionMap& rb = b.getRegionMap();

	if(ra.getWidth() != rb.getWidth() || ra.getHeight() != rb.getHeight())
	{
		// 領域マップの大きさは同じでないといけない
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
	@brief	面積を計算
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
	@brief	周囲の長さを計算
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
	
	// 終点と始点までの距離も加える
	p1.setX(boundaryPixels.at(0).x);
	p1.setY(boundaryPixels.at(0).y);
	float dist = QVector2D(p1 - p0).length();
	ret += dist;

	return ret;
}
