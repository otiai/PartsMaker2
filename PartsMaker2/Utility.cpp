#include "Utility.h"
#include <QGLWidget>
#include <QVector3D>
#include <QVector2D>


void Utility::convertImageRGBAu2QImage(ImageRGBAu* src, QImage** dst)
{
	const int w = src->getWidth();
	const int h = src->getHeight();

	*dst = new QImage(QSize(w, h), QImage::Format_ARGB32);

	for (int yi = 0; yi < h; yi++)
	{
		unsigned char *srcData = ((unsigned char *)src->getData()) + 4 * (h - 1 - yi) * w;
		unsigned char *dstData = ((unsigned char *)(*dst)->bits()) + 4 * yi * w;

		for (int xi = 0; xi < w; xi++)
		{
			dstData[0] = srcData[2];
			dstData[1] = srcData[1];
			dstData[2] = srcData[0];
			dstData[3] = srcData[3];

			dstData += 4;
			srcData += 4;
		}
	}
}

void Utility::convertQImage2ImageRGBAu(QImage& src, ImageRGBAu& out)
{
	const int w = src.width();
	const int h = src.height();

	out.allocate(w, h);

	for (int yi = 0; yi < h; yi++)
	{
		unsigned char *srcData = ((unsigned char *)src.bits()) + 4 * (h - 1 - yi) * w;
		unsigned char *dstData = ((unsigned char *)out.getData()) + 4 * yi * w;

		for (int xi = 0; xi < w; xi++)
		{
			dstData[0] = srcData[2];
			dstData[1] = srcData[1];
			dstData[2] = srcData[0];
			dstData[3] = srcData[3];

			dstData += 4;
			srcData += 4;
		}
	}

}


void Utility::multMatrix(const QMatrix4x4& m)
{
	static GLfloat mat[16];
	const qreal *data = m.constData();
	for (int index = 0; index < 16; ++index)
		mat[index] = data[index];
	glMultMatrixf(mat);
}

/*!
	arg matrixMode: GL_PROJECTION_MATRIX, GL_MODELVIEW_MATRIX
*/
void Utility::getMatrix(QMatrix4x4& m, int matrixMode)
{
	static GLfloat mat[16];
	qreal *data = m.data();
	glGetFloatv( matrixMode, mat );
	for (int index = 0; index < 16; ++index)
		data[index] = mat[index];
}

/*!
	@ �F��Ԃ̃��[�N���b�h�������v�Z
*/
float Utility::calcColorDistance(IntVec::ubvec3& a, IntVec::ubvec3& b)
{
	float ret = 0.0f;
	QVector3D colA(a.r / 255.0f, a.g / 255.0f, a.b / 255.0f);
	QVector3D colB(b.r / 255.0f, b.g / 255.0f, b.b / 255.0f);

	ret = QVector3D(colA - colB).length();

	return ret;
}

float Utility::linePointDistance(QVector3D& p1, QVector3D& p2, QVector3D& p, QVector3D& closestPoint)
{
	double bot;
	double dist;
	double t;

	bot = (p2 - p1).lengthSquared();

	if (bot == 0.0E+00)
	{
		closestPoint = p1;
	}
	//
	//  (P-P1) dot (P2-P1) = Norm(P-P1) * Norm(P2-P1) * Cos(Theta).
	//
	//  (P-P1) dot (P2-P1) / Norm(P2-P1)**2 = normalized coordinate T
	//  of the projection of (P-P1) onto (P2-P1).
	//
	else
	{
		t = (
			(p.x() - p1.x()) * (p2.x() - p1.x()) + 
			(p.y() - p1.y()) * (p2.y() - p1.y()) + 
			(p.z() - p1.z()) * (p2.z() - p1.z()) ) / bot;

		closestPoint = p1 + t * (p2 - p1);
	}
	//
	//  Now compute the distance between the projection point and P.
	//
	dist = (closestPoint - p).length();

	return dist;
}

/*!
	@brief ���_�z�񂪎��v�܂�肩���ׂ�
*/
bool Utility::isClockwise(QVector<QVector2D>& points)
{
	float sum = 0;
	for(int i = 0; i < points.size() - 1; i++)
	{
		QVector2D p0 = points.at(i);
		QVector2D p1 = points.at(i+1);

		sum += p0.x() * p1.y() - p1.x() * p0.y();
	}
	if(sum < 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Utility::isClockwise(std::vector<IntVec::ivec2>& points)
{
	float sum = 0;
	for(int i = 0; i < points.size() - 1; i++)
	{
		IntVec::ivec2 p0 = points.at(i);
		IntVec::ivec2 p1 = points.at(i+1);

		sum += p0.x * p1.y - p1.x * p0.y;
	}
	if(sum < 0)
	{
		return true;
	}
	else
	{
		return false;
	}
	return true;
}
/*!
	@brief	1�����̃K�E�X�t�B���^��������
	@note	���_�̎n�_�ƏI�_�̓��[�v���Ă�����̂Ƃ���i�Ȃ����Ă���K�v�͂Ȃ��j
	@param	inData: ���͒��_�f�[�^
	@param	outData: �o�͗p�̃f�[�^
	@param	kernelSize: �J�[�l���T�C�Y�@�����̗�����kernelSize�̃J�[�l�����쐬����
	@param	sigma: �ڂ����̋���
*/
void Utility::gaussianFilter(QVector<QVector2D>* inData, QVector2D* outData, int kernelSize, double sigma)
{
	double sumWeight = 0;
	for(int i = -kernelSize; i <= kernelSize; i++)
	{
		sumWeight += exp(- double(i*i) / (sigma*sigma));
	}

	for(int j = 0; j < inData->size(); j++)
	{
		QVector2D sum;
		for(int i = -kernelSize; i <= kernelSize; i++)
		{
			double w = (1.0 / sumWeight) * exp(- (double)(i*i) / (sigma*sigma));
			int index = (j+i+inData->size())%inData->size();
			QVector2D v = inData->at(index);
			sum += v * w;
		}
		outData[j] = sum;
	}
}