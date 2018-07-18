#include "ScribbleBrush.h"
//?#include "WinUtil.h"

using namespace std;
using namespace MyAlgebra;
using namespace IntVec;

void ScribbleBrush::paint( ImageRGBAu &scribbleBuf, int xi, int yi )
{
	if ( ! m_IsUsed )	// 塗り始め
	{
		paintCircle( scribbleBuf, xi, yi );
		m_IsUsed = true;
	}
	else	// 前に塗った点からある程度離れていたら、その間を補間して塗る
	{
		vec2 d((float)(xi-m_PrevX), (float)(yi-m_PrevY));
		const float dist = d.norm();
		const float interval = 0.9f * m_BrushRadius;
		const int nCircles = (int)ceilf(dist / interval);
		d /= (float)nCircles;

		float x = m_PrevX + d.x;
		float y = m_PrevY + d.y;

		for (int ci=1; ci<=nCircles; ci++)
		{
			paintCircle( scribbleBuf, (int)x, (int)y );
			x += d.x;
			y += d.y;
		}
	}

	m_PrevX = xi;
	m_PrevY = yi;
}

void ScribbleBrush::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	m_CurrentRGBA.set( r, g, b, a );
}

void ScribbleBrush::changeColor()
{
	/*?
	ubvec3 color;
	WinUtil::ChooseColorDialog( color.vec_array );
	m_CurrentRGB.set(  color.r/255.f, color.g/255.f, color.b/255.f );
	?*/
}

void ScribbleBrush::paintCircle( ImageRGBAu &scribbleBuf, int xi, int yi )
{
	const int w = scribbleBuf.getWidth();
	const int h = scribbleBuf.getHeight();

	const int xStart = max(0, xi-m_BrushRadius);
	const int xEnd = min(xi+m_BrushRadius, w-1);
	const int yStart = max(0, yi-m_BrushRadius);
	const int yEnd = min(yi+m_BrushRadius, h-1);

	const int sqrRadius = m_BrushRadius * m_BrushRadius;

	for (int _yi=yStart; _yi<=yEnd; _yi++)
	{
		for (int _xi=xStart; _xi<=xEnd; _xi++)
		{
			const int sqrDist = (_xi-xi)*(_xi-xi) + (_yi-yi)*(_yi-yi);
			if (sqrDist <= sqrRadius)
				scribbleBuf(_xi,_yi).set( m_CurrentRGBA );
		}
	}
}
