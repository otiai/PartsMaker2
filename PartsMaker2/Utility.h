#ifndef UTILITY_H
#define UTILITY_H

#include "ImageRect.h"
#include <QGLWidget>
#include <gl/glu.h>
#include <QMatrix4x4>
#include <QColor>
#include <QVector3D>
#include "ivec.h"
#include <vector>

#define PrintGLErrorMacro {		\
	GLenum err_code = glGetError();	\
									\
	if (err_code != GL_NO_ERROR) {							\
		const GLubyte *err_str = gluErrorString(err_code);	\
		qDebug("OpenGL Error: %s (File: %s, line %d)\n",	\
			err_str, __FILE__, __LINE__);					\
	}														\
}

class Utility
{
public:
	static void convertQImage2ImageRGBAu(QImage& src, ImageRGBAu& out);
	static void convertImageRGBAu2QImage(ImageRGBAu* src, QImage** dst);
	static void multMatrix(const QMatrix4x4& m);
	static void getMatrix(QMatrix4x4& m, int matrixMode);
	static float calcColorDistance(IntVec::ubvec3& a, IntVec::ubvec3& b);
	static float linePointDistance(QVector3D& p1, QVector3D& p2, QVector3D& p, QVector3D& closestPoint);
	static bool isClockwise(QVector<QVector2D>& points);
	static bool isClockwise(std::vector<IntVec::ivec2>& points);
	static void gaussianFilter(QVector<QVector2D>* inData, QVector2D* outData, int kernelSize, double sigma);

private:
	static unsigned char clampUbyte(float f) { return (f < 0.f) ? 0 : (f > 255.f) ? 255 : (unsigned char)f; };
};
#endif // UTILITY_H