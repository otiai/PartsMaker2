#pragma once

#include "ImageRect.h"

class ScribbleBrush
{
public:
	ScribbleBrush()
		: m_CurrentRGBA(255, 128, 128, 255), m_BrushRadius(1), m_IsUsed(false)
	{
	}

	void paint( ImageRGBAu &scribbleBuf, int xi, int yi );

	bool isUsed() const { return m_IsUsed; }
	void release() { m_IsUsed = false; }

	void setBrushRadius(int r) { m_BrushRadius = r; }
	void changeColor();
	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

	int getBrushRadius() const { return m_BrushRadius; }
	IntVec::ubvec4 getCurrentRGB() const { return m_CurrentRGBA; }

private:
	bool m_IsUsed;
	IntVec::ubvec4 m_CurrentRGBA;
	int m_BrushRadius;
	int m_PrevX, m_PrevY;

	void paintCircle( ImageRGBAu &scribbleBuf, int xi, int yi );
};
