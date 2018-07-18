#ifndef CONTOUR_FOURIER_DESCRIPTOR_H
#define CONTOUR_FOURIER_DESCRIPTOR_H

#include <QVector>
#include <QVector2D>

class ContourFourierDescriptor
{
public:
	static QVector<QVector2D> resample( const QVector<QVector2D> &points, int nResample );
};

#endif // CONTOUR_FOURIER_DESCRIPTOR_H