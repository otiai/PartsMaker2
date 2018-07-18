#include "ContourFourierDescriptor.h"

using namespace std;


QVector<QVector2D> ContourFourierDescriptor::resample( const QVector<QVector2D> &points, int nResample )
{
	QVector<QVector2D> results;

	const int nSegments = points.size();

	if ( nSegments <= 1 ) return results;

	float totalLength = 0.f;
	float *segmentLengths = new float[ nSegments ];

	for (int si=0; si<nSegments-1; si++)
	{
		const float len = QVector2D( points[si] - points[si+1] ).length();
		segmentLengths[si] = len;
		totalLength += len;
	}

	const float uniformLength = totalLength / (float)(nResample-1);

	results.clear();
	results.push_back( points[0] );

	int originalCurveIndex = 0;
	float currentCurveLength = 0.f;

	for (int si=1; si<nResample-1; si++)
	{
		const float targetLength = si * uniformLength;

		while (originalCurveIndex < nSegments-1)
		{
			if (currentCurveLength + segmentLengths[originalCurveIndex] >= targetLength)
				break;
			currentCurveLength += segmentLengths[originalCurveIndex];
			originalCurveIndex++;
		}

		QVector2D unitDir = points[originalCurveIndex+1] - points[originalCurveIndex];
		unitDir.normalize();

		const float residualLength = targetLength - currentCurveLength;
		results.push_back( points[originalCurveIndex] + residualLength*unitDir);
	}
	results.push_back(points.back());
	delete [] segmentLengths;

	return results;
}
