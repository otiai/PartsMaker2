#pragma once

#include <cstdlib>
#include <cassert>
#include "my_algebra.h"
#include "ivec.h"

// 次のマクロ CLAMP_TO_EDGE_VALUES が定義されている場合、画像にアクセスするときの
// x 座標が x < 0 のときは x = 0 に、x >= m_Width のときは x = m_Width-1 に、
// y 座標が y < 0 のときは y = 0 に、y >= m_Height のときは y = m_Height に切り詰める
#define CLAMP_TO_EDGE_VALUES

template <class T>
class ImageRect
{
protected:
	int m_Width;
	int m_Height;
	T *m_pData;

public:
	ImageRect()
		: m_pData(0)
	{
	}

	ImageRect(int w, int h)
		: m_Width(w), m_Height(h)
	{
		m_pData = new T[ m_Width*m_Height ];
	}

	ImageRect(int w, int h, const T &v)
		: m_Width(w), m_Height(h)
	{
		m_pData = new T[ m_Width*m_Height ];
		fill( v );
	}

	ImageRect(int w, int h, T *_data)
		: m_Width(w), m_Height(h)
	{
		assert(_data);

		m_pData = new T[ m_Width*m_Height ];
		memcpy(m_pData, _data, m_Width*m_Height*sizeof(T));
	}

	ImageRect(const ImageRect &img)
		: m_Width(img.m_Width), m_Height(img.m_Height)
	{
		m_pData = new T[ m_Width*m_Height ];

		if (img.m_pData)
		{
			memcpy(m_pData, img.m_pData, m_Width*m_Height*sizeof(T));
		}
	}

	~ImageRect()
	{
		if (m_pData) delete [] m_pData;
	}

	inline int getWidth() const { return m_Width; }
	inline int getHeight() const { return m_Height; }
	inline T * getData() { return m_pData; }
	inline T const * getData() const { return m_pData; }

	void allocate(int w, int h)
	{
		if (m_pData) delete [] m_pData;
		m_pData = new T[w*h];
		m_Width = w;
		m_Height = h;
	}

	void allocate(int w, int h, T *_data)
	{
		if (m_pData) delete [] m_pData;
		m_pData = new T[w*h];
		memcpy(m_pData, _data, w*h*sizeof(T));
		m_Width = w;
		m_Height = h;
	}

	void copy(const ImageRect &img)
	{
		if (m_pData) delete [] m_pData;

		m_Width = img.m_Width;
		m_Height = img.m_Height;
		m_pData = new T[m_Width*m_Height];
		memcpy(m_pData, img.m_pData, m_Width*m_Height*sizeof(T));
	}

	inline const T getValue(int xi, int yi) const
	{
		assert(m_pData);
#ifdef CLAMP_TO_EDGE_VALUES
		xi = (xi < 0) ? 0 : (xi >= m_Width) ? (m_Width-1) : xi;
		yi = (yi < 0) ? 0 : (yi >= m_Height) ? (m_Height-1) : yi;
#else
		assert(0 <= xi && xi < m_Width);
		assert(0 <= yi && yi < m_Height);
#endif

		return m_pData[xi + m_Width*yi];
	}

	const T bilinearInterp(float x, float y) const
	{
		assert(m_pData);

		// clamped to edge

		//const float _x = std::max(0.f, std::min(x, (float)(width-1)));
		//const float _y = std::max(0.f, std::min(y, (float)(height-1)));
		const float _x = max(0.f, min(x, (float)(m_Width-1)));
		const float _y = max(0.f, min(y, (float)(m_Height-1)));

		const int x0 = (int)floorf(_x);
		//const int x1 = std::min(x0+1, width-1);
		const int x1 = min(x0+1, m_Width-1);
		const float s = _x - x0;

		const int y0 = (int)floorf(_y);
		//const int y1 = std::min(y0+1, height-1);
		const int y1 = min(y0+1, m_Height-1);
		const float t = _y - y0;

		const T v00 = m_pData[x0 + m_Width*y0];
		const T v10 = m_pData[x1 + m_Width*y0];
		const T v01 = m_pData[x0 + m_Width*y1];
		const T v11 = m_pData[x1 + m_Width*y1];

		const T r0 = v00 + s*(v10 - v00);
		const T r1 = v01 + s*(v11 - v01);

		return r0 + t*(r1 - r0);
	}

	inline void setValue(int xi, int yi, T& v)
	{
		assert(m_pData);
#ifdef CLAMP_TO_EDGE_VALUES
		xi = (xi < 0) ? 0 : (xi >= m_Width) ? (m_Width-1) : xi;
		yi = (yi < 0) ? 0 : (yi >= m_Height) ? (m_Height-1) : yi;
#else
		assert(0 <= xi && xi < m_Width);
		assert(0 <= yi && yi < m_Height);
#endif

		m_pData[xi + m_Width*yi] = v;
	}

	inline T& operator()(int xi, int yi)
	{
		assert(m_pData);
#ifdef CLAMP_TO_EDGE_VALUES
		xi = (xi < 0) ? 0 : (xi >= m_Width) ? (m_Width-1) : xi;
		yi = (yi < 0) ? 0 : (yi >= m_Height) ? (m_Height-1) : yi;
#else
		assert(0 <= xi && xi < m_Width);
		assert(0 <= yi && yi < m_Height);
#endif

		return m_pData[xi + m_Width*yi];
	}

	inline const T& operator()(int xi, int yi) const
	{
		assert(m_pData);
#ifdef CLAMP_TO_EDGE_VALUES
		xi = (xi < 0) ? 0 : (xi >= m_Width) ? (m_Width-1) : xi;
		yi = (yi < 0) ? 0 : (yi >= m_Height) ? (m_Height-1) : yi;
#else
		assert(0 <= xi && xi < m_Width);
		assert(0 <= yi && yi < m_Height);
#endif

		return m_pData[xi + m_Width*yi];
	}

	inline ImageRect& operator=(const ImageRect &img)
	{
		delete [] m_pData;
		m_Width = img.m_Width;
		m_Height = img.m_Height;
		m_pData = new T[m_Width*m_Height];
		memcpy(m_pData, img.m_pData, m_Width*m_Height*sizeof(T));
		return *this;
	}

	void getSubImage(int x0, int y0, int w, int h, ImageRect &img)
	{
		assert(x0+w < m_Width);
		assert(y0+h < m_Height);
		assert(m_Width);
		assert(m_Height);
		assert(w);
		assert(h);

		if (img.m_pData) delete [] img.m_pData;
		img.m_pData = new T[w*h];
		img.m_Width = w;
		img.m_Height = h;

		for (int yi=0; yi<h; yi++)
		{
			memcpy(&img.m_pData[w*yi], &m_pData[x0+m_Width*(y0+yi)], w*sizeof(T));
		}
	}

	void setSubImage(int x0, int y0, ImageRect &img)
	{
		assert(m_pData);
		assert(x0+img.m_Width < m_Width);
		assert(y0+img.m_Height < m_Height);
		assert(m_Width);
		assert(m_Height);
		assert(img.m_Width);
		assert(img.m_Height);

		for (int yi=0; yi<img.m_Height; yi++)
		{
			memcpy(&m_pData[x0+m_Width*(y0+yi)], &img.m_pData[img.m_Width*yi], img.m_Width*sizeof(T));
		}
	}

	void getRegionData(int x0, int y0, int w, int h, T * _data)
	{
		assert(m_pData);
		assert(x0+w < m_Width);
		assert(y0+h < m_Height);
		assert(m_Width);
		assert(m_Height);
		assert(w);
		assert(h);

		if (_data) delete [] _data;
		_data = new T[w*h];

		for (int yi=0; yi<h; yi++)
		{
			memcpy(&_data[w*yi], &m_pData[x0+m_Width*(y0+yi)], w*sizeof(T));
		}
	}

	void setRegionData(int x0, int y0, int w, int h, T * _data)
	{
		assert(m_pData);
		assert(x0+w < m_Width);
		assert(y0+h < m_Height);
		assert(m_Width);
		assert(m_Height);
		assert(w);
		assert(h);

		for (int yi=0; yi<h; yi++)
		{
			memcpy(&m_pData[x0+m_Width*(y0+yi)], &_data[w*yi], w*sizeof(T));
		}
	}

	void fill(const T& v)
	{
		assert(m_pData);

		for (int yi=0; yi<m_Height; yi++)
			for (int xi=0; xi<m_Width; xi++)
				m_pData[xi + m_Width*yi] = v;
	}

	void trim(int x0, int y0, int w, int h)
	{
		assert(m_pData);
		assert(x0+w < m_Width);
		assert(y0+h < m_Height);
		assert(m_Width);
		assert(m_Height);
		assert(w);
		assert(h);

		T * _tmp = new T[w*h];

		for (int yi=0; yi<h; yi++)
		{
			memcpy(&_tmp[w*yi], &m_pData[x0 + m_Width*(y0+yi)], w*sizeof(T));
		}

		delete [] m_pData;
		m_pData = new T[w*h];
		memcpy(m_pData, _tmp, w*h*sizeof(T));
		m_Width = w;
		m_Height = h;

		delete [] _tmp;
	}

	void resize(int wNew, int hNew)
	{
		ImageRect<T> tmpImg(wNew, hNew);

		const float xScale = m_Width / (float)wNew;
		const float yScale = m_Height / (float)hNew;

		for (int yi=0; yi<hNew; yi++)
		{
			const float yOrg = yScale * yi;

			for (int xi=0; xi<wNew; xi++)
			{
				const float xOrg = xScale * xi;

				tmpImg(xi,yi) = this->bilinearInterp(xOrg, yOrg);
			}
		}

		*this = tmpImg;
	}
};

typedef ImageRect<float> ImageGrayf;
typedef ImageRect<MyAlgebra::vec3> ImageRGBf;
typedef ImageRect<MyAlgebra::vec4> ImageRGBAf;
typedef ImageRect<IntVec::ubyte> ImageGrayu;
typedef ImageRect<IntVec::ubvec3> ImageRGBu;
typedef ImageRect<IntVec::ubvec4> ImageRGBAu;


