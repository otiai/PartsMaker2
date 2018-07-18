#include "RegionMatchHandler.h"
#include "ClosedRegion.h"
#include <QMatrix4x4>
#include "Utility.h"
#include "Config.h"
#include "ContourFourierDescriptor.h"
#include "ObjectManager.h"

int RegionLinkData::createdDataNum = 0;
void RegionLinkData::init()
{
	for(int i = 0; i < VIEW_MAX; i++)
	{
		regions[i] = NULL;
	}
	uniqueID = createdDataNum;
	createdDataNum++;
}

void RegionLinkData::clear()
{
	for(int i = 0; i < VIEW_MAX; i++)
	{
		ClosedRegion* r = regions[i];
		if(r)
		{
			r->setRegionLinkData(NULL);
		}
		regions[i] = NULL;
	}
}

void RegionLinkData::setData(ClosedRegion* r, int viewID)
{
	Q_ASSERT((0 <= viewID && viewID < VIEW_MAX));
	regions[viewID] = r;
}

ClosedRegion* RegionLinkData::getRegion(int viewID)
{
	Q_ASSERT((0 <= viewID && viewID < VIEW_MAX));
	return regions[viewID];
}

void RegionLinkData::resetFeaturePoint()
{
	for(int i = 0; i < VIEW_MAX; i++)
	{
		ClosedRegion* r = regions[i];
		if(r)
		{
			r->resetFeaturePoint();
		}
	}
}

void RegionLinkData::createLines()
{
	for(int i = 0; i < VIEW_MAX; i++)
	{
		ClosedRegion* r = regions[i];
		if(r)
		{
			r->createLines();
		}
	}
}

/*
	// We start with a point at (0,0,0). Then for each key view, we project the current point onto the line defined by that 
	// key view's orientation and center. We average each of these projections and continue. We do this 100 times.
*/
void RegionLinkData::calcDepth()
{
	QVector3D current(0,0,0);
	QVector3D ave(0,0,0);

	int viewNum = 0;
	ObjectManager* mgr = ObjectManager::getInstance();
	for(int j = 0; j < 100; j++)
	{
		viewNum = 0.0;
		ave = QVector3D(0,0,0);
		for(int i = 0; i < VIEW_MAX; i++)
		{
			ClosedRegion* r = regions[i];
			if(!r)
				continue;

			// ビュー行列の逆行列を求める
			QMatrix4x4 matProjInv, matViewInv;
			matProjInv.setToIdentity(); // プロジェクション行列は単位行列に設定しているので、逆行列は単位行列でよい
			if(i == VIEW_FRONT)
			{
				matViewInv = QMatrix4x4(mgr->getSrcPoseMatrix());
			}
			else if(i == VIEW_SIDE_RIGHT)
			{
				matViewInv = QMatrix4x4(mgr->getDstPoseMatrix());
			}
			matViewInv = matViewInv.inverted();
			QVector3D lineStart, lineEnd; // スクリーン座標上の点（2Dイラスト上の点）からビュー方向に伸びた線
			calc3DLine(r, &matProjInv, &matViewInv, &lineStart, &lineEnd);

			// currentをline上に投影した点を求める
			QVector3D pos;
			Utility::linePointDistance(lineStart, lineEnd, current, pos);

			ave += pos;
			viewNum++;
		}
		ave /= viewNum;
		current = ave;
	}


	// 各視点での3D位置を求める
	// スクリーン座標系での位置に合うようにするため、
	// 視線方向のベクトルにcurrentを投影する
	for(int i = 0; i < VIEW_MAX; i++)
	{
		ClosedRegion* r = regions[i];
		if(!r)
			continue;

		// ビュー行列の逆行列を求める
		QMatrix4x4 matProjInv, matViewInv;
		matProjInv.setToIdentity(); // プロジェクション行列は単位行列に設定しているので、逆行列は単位行列でよい
		if(i == VIEW_FRONT)
		{
			matViewInv = QMatrix4x4(mgr->getSrcPoseMatrix());
		}
		else if(i == VIEW_SIDE_RIGHT)
		{
			matViewInv = QMatrix4x4(mgr->getDstPoseMatrix());
		}
		matViewInv = matViewInv.inverted();
		QVector3D lineStart, lineEnd; // スクリーン座標上の点（2Dイラスト上の点）からビュー方向に伸びた線
		calc3DLine(r, &matProjInv, &matViewInv, &lineStart, &lineEnd);

		// currentをline上に投影した点を求める
		QVector3D pos;
		Utility::linePointDistance(lineStart, lineEnd, current, pos);

		r->setPos3D(pos);
	}
}

/*!
	@brief	領域のバウンディングボックスを通る直線を求める
*/
void RegionLinkData::calc3DLine(ClosedRegion* r, QMatrix4x4* matProjInv, QMatrix4x4* matViewInv, QVector3D* outStart, QVector3D* outEnd)
{
	// スクリーン座標を射影空間(left, right, bottom, top) = (-1, 1, -1, 1)での座標Psに変換
	QVector3D Ps;
	int width = r->getRegionMap().getWidth();
	int height = r->getRegionMap().getHeight();
	IntVec::ivec2 bBoxMin = r->getBboxMin();
	IntVec::ivec2 bBoxMax = r->getBboxMax();
	QVector3D Ps0 = QVector3D((float)(bBoxMin.x + bBoxMax.x) / 2.0f, (float)(bBoxMin.y + bBoxMax.y) / 2.0f, 0.0);
	Ps0.setX(((Ps0.x() * 2.0) / (float)width) - 1.0f);
	Ps0.setY(((Ps0.y() * 2.0) / (float)height) - 1.0f);
	QVector3D Ps1 = QVector3D(Ps0.x(), Ps0.y(), 1.0);

	// 射影空間の座標からワールド座標系での座標Pにするには、
	// MvInv: ビュー行列の逆行列, MpInv: プロジェクション行列の逆行列
	// P = MvInv * MpInv * Ps
	QVector3D Vs0, Vs1;
	Vs0 = (*matViewInv) * (*matProjInv) * Ps0;
	Vs1 = (*matViewInv) * (*matProjInv) * Ps1;

	*outStart = Vs0;
	*outEnd = Vs1;
}


/*!
	@brief	回転角に対応する領域のバウンディングボックスを求める
	@note	srcとdstの比率を求める
			それぞれのビューでZ軸上の点を求め、現在の点との距離を比率とする
*/
QRect RegionLinkData::calcBoundaryPixels(float rotX, float rotY)
{
	ClosedRegion* front = regions[VIEW_FRONT];
	ClosedRegion* side = regions[VIEW_SIDE_RIGHT];
	if(!front || !side)
		return QRect(0,0,0,0);

	QVector<Line*>* frontLines = front->getBoundaryLines();
	QVector<Line*>* sideLines = side->getBoundaryLines();

	Q_ASSERT(frontLines->size() == sideLines->size());


	ObjectManager* mgr = ObjectManager::getInstance();
	float t;
	
	const QMatrix4x4& matFrontRot = mgr->getSrcPoseMatrix();
	QVector3D frontPos(0,0,1);
	frontPos = matFrontRot * frontPos;

	const QMatrix4x4& matSideRot = mgr->getDstPoseMatrix();
	QVector3D sidePos(0,0,1);
	sidePos = matSideRot * sidePos;

	float currentRotX, currentRotY;
	currentRotX = rotX;
	currentRotY = rotY;
	QMatrix4x4 matCurrentRotX, matCurrentRotY, matCurrentRot;
	matCurrentRotX.rotate(currentRotX, 1, 0, 0);
	matCurrentRotY.rotate(currentRotY, 0, 1, 0);
	matCurrentRot = matCurrentRotY * matCurrentRotX;
	QVector3D currentPos(0,0,1);
	currentPos = matCurrentRot * currentPos;

	QVector2D dist;
	dist.setX((currentPos - frontPos).length());
	dist.setY((currentPos - sidePos).length());
	dist.normalize();
	t = dist.x();


	// バウンディングボックスの計算（frontとsideを補間）
	float frontWidth = abs(front->getBboxMax().x - front->getBboxMin().x);
	float frontHeight = abs(front->getBboxMax().y - front->getBboxMin().y);
	QVector2D frontCenter((front->getBboxMax().x + front->getBboxMin().x)/2.0f, (front->getBboxMax().y + front->getBboxMin().y)/2.0f);
	float sideWidth = abs(side->getBboxMax().x - side->getBboxMin().x);
	float sideHeight = abs(side->getBboxMax().y - side->getBboxMin().y);
	QVector2D sideCenter((side->getBboxMax().x + side->getBboxMin().x)/2.0f, (side->getBboxMax().y + side->getBboxMin().y)/2.0f);

	float outWidth = (1.0f - t) * frontWidth + t * sideWidth;
	float outHeight = (1.0f - t) * frontHeight + t * sideHeight;
	QVector2D outCenter = (1.0f - t) * frontCenter + t * sideCenter;
	QRect outRect(outCenter.x() - outWidth/2.0f, outCenter.y() - outHeight / 2.0f, outWidth, outHeight);

	// 頂点数を多い方に合わせる
	for(int i = 0; i < frontLines->size(); i++)
	{
		Line* frontLine = frontLines->at(i);
		Line* sideLine = sideLines->at(i);
		int vertexNum;
		if(frontLine->getPoints().size() > sideLine->getPoints().size())
		{
			QVector<QVector2D> buf;
			vertexNum = frontLine->getPoints().size();
			buf = ContourFourierDescriptor::resample(sideLine->getPoints(), vertexNum);
			sideLine->setPoints(buf);
		}
		else
		{
			QVector<QVector2D> buf;
			vertexNum = sideLine->getPoints().size();
			buf = ContourFourierDescriptor::resample(frontLine->getPoints(), vertexNum);
			frontLine->setPoints(buf);
		}
		Q_ASSERT(frontLine->getPoints().size() == sideLine->getPoints().size());
	}

	boundaryPixels_.clear();
	for(int i = 0; i < frontLines->size(); i++)
	{
		Line* frontLine = frontLines->at(i);
		Line* sideLine = sideLines->at(i);
		for(int j = 0; j < frontLine->getPoints().size(); j++)
		{
			// 中心を(0,0)上を(0.5,0.5)となるような座標系に変換
			QVector2D vFront = frontLine->getPoints().at(j);
			vFront = vFront - frontCenter;
			float frontX = vFront.x() / frontWidth;
			float frontY = vFront.y() / frontHeight;

			QVector2D vSide = sideLine->getPoints().at(j);
			vSide = vSide - sideCenter;
			float sideX = vSide.x() / sideWidth;
			float sideY = vSide.y() / sideHeight;

			float outX = (1.0f - t) * frontX + t * sideX;
			float outY = (1.0f - t) * frontY + t * sideY;
			boundaryPixels_.append(QVector2D(outX, outY));
		}
	}

	// 3D位置の計算(frontとsideの補間)
	QVector3D posFront = front->getPos3D();
	QVector3D posSide = side->getPos3D();
	pos3D_ = (1.0f - t) * posFront + t * posSide;

	return outRect;
}


//===========================================
// RegionLinkDataManager
//===========================================
RegionLinkDataManager::RegionLinkDataManager()
{
	for(int i = 0; i < COLOR_MAX; i++)
	{
		colorTable_[i] = QColor( rand()%256, rand()%256, rand()%256 );
	}
	noneColor_ = QColor(64, 64, 64);
}

RegionLinkDataManager::~RegionLinkDataManager()
{
	deleteAll();
}


QColor& RegionLinkDataManager::getIndexColor(ClosedRegion* r)
{
	Q_ASSERT(r);
	RegionLinkData* data = r->getRegionLinkData();

	if(data)
	{
		int index = data->getUniqueID();
		index = index % COLOR_MAX;
		return colorTable_[index];
	}
	else
	{
		return noneColor_;
	}
}

QColor& RegionLinkDataManager::getIndexColor(int index)
{
	index = index % COLOR_MAX;
	return colorTable_[index];
}

void RegionLinkDataManager::createData(ClosedRegion* r)
{
	Q_ASSERT(r);

	RegionLinkData* data = new RegionLinkData();
	data->setData(r, VIEW_MAIN);
	regionLinkDatas_.append(data);
	r->setRegionLinkData(data);
}

void RegionLinkDataManager::deleteData(RegionLinkData* linkData)
{
	for(int i = 0; i < regionLinkDatas_.size(); i++)
	{
		RegionLinkData* data = regionLinkDatas_.at(i);
		if(data == linkData)
		{
			delete data;
			regionLinkDatas_.remove(i);
			return;
		}
	}
}

void RegionLinkDataManager::deleteAll()
{
	for(int i = 0; i < regionLinkDatas_.size(); i++)
	{
		RegionLinkData* data = regionLinkDatas_.at(i);
		delete data;
	}
	regionLinkDatas_.clear();

	RegionLinkData::resetCreateDataNum();
}

void RegionLinkDataManager::link(ClosedRegion* mainRegion, ClosedRegion* subRegion, int viewID)
{
	Q_ASSERT(mainRegion);
	Q_ASSERT(viewID != VIEW_MAIN);

	RegionLinkData* linkData = mainRegion->getRegionLinkData();
	Q_ASSERT(linkData);

	// メイン領域に対応付けられているデータを消す
	ClosedRegion* mainLinkRegion = linkData->getRegion(viewID);
	if(mainLinkRegion)
	{
		mainLinkRegion->setRegionLinkData(NULL);
	}
	linkData->setData(NULL, viewID);

	// サブ領域に対応付けられているデータを消す
	if(subRegion)
	{
		RegionLinkData* subLinkData = subRegion->getRegionLinkData();
		if(subLinkData)
		{
			subLinkData->setData(NULL, viewID);
		}
		subRegion->setRegionLinkData(NULL);
	}

	// メイン領域とサブ領域を対応付け
	linkData->setData(subRegion, viewID);
	if(subRegion)
	{
		subRegion->setRegionLinkData(linkData);
	}

#if 0
	if(subRegion)
	{
		qDebug("link main ID %d, sub id %d, view id %d", mainRegion->getID(), subRegion->getID(), viewID);
	}
	else
	{
		qDebug("link main ID %d, sub none, view id %d", mainRegion->getID(), viewID);
	}
#endif

	// デプス更新
	linkData->calcDepth();

	// 特徴点リセット
	linkData->resetFeaturePoint();

	// 輪郭線をラインに分割
	linkData->createLines();

	// リンクデータ更新通知
	ObjectManager::getInstance()->linkDataUpdated();

}

/*!
	@brief	リンク解除
*/
void RegionLinkDataManager::clearLink(ClosedRegion* region)
{
	RegionLinkData* linkData = region->getRegionLinkData();

	if(linkData)
	{
		ClosedRegion* subRegion = linkData->getRegion(VIEW_SIDE_RIGHT);
		if(subRegion)
		{
			subRegion->setRegionLinkData(NULL);
		}
		linkData->setData(NULL, VIEW_SIDE_RIGHT);

		// デプス更新
		linkData->calcDepth();

		// 特徴点リセット
		linkData->resetFeaturePoint();

		// 輪郭線をラインに分割
		linkData->createLines();

		// リンクデータ更新通知
		ObjectManager::getInstance()->linkDataUpdated();
	}
}


void RegionLinkDataManager::reCalcDepth()
{
	for(int i = 0; i < regionLinkDatas_.size(); i++)
	{
		RegionLinkData* data = regionLinkDatas_.at(i);
		data->calcDepth();
	}
}

//==========================
void RegionLinkDataManager::debugPrint()
{
	qDebug("--- Region Link Data Debug ---");
	for(int i = 0; i < regionLinkDatas_.size(); i++)
	{
		RegionLinkData* data = regionLinkDatas_.at(i);
		ClosedRegion* main = data->getRegion(VIEW_MAIN);
		ClosedRegion* sub = data->getRegion(VIEW_SIDE_RIGHT);
		if(sub)
		{
			qDebug("[%d] main ID %d, sub ID %d", i, main->getID(), sub->getID());
		}
		else
		{
			qDebug("[%d] main ID %d, sub None", i, main->getID());
		}
	}
	qDebug("------------------------------");
}