#pragma once

#include "ImageRect.h"
#include <cstdio>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// OpenCV 2 API info
// Mat:				http://www.wakayama-u.ac.jp/~chen/opencv/opencv2.html
// resize:			http://opencv.jp/opencv-2svn/cpp/geometric_image_transformations.html
// imread/imwrite:	http://opencv.jp/opencv-2.1/cpp/reading_and_writing_images_and_video.html

typedef float Real;

class OpenCVImageIO
{
public:
	// converters --------------------------------------------------------------

	template <class T>
	void convertMat2ImageRect(const cv::Mat &inImg, ImageRect<T> &outImg)
	{
		fprintf(stderr, __FUNCTION__ ": unknown image type\n");
	}

	template <class T>
	void convertImageRect2Mat(const ImageRect<T> &inImg, cv::Mat &outImg)
	{
		fprintf(stderr, __FUNCTION__ ": unknown image type\n");
	}

	template <>
	void convertMat2ImageRect(const cv::Mat &inImg, ImageRect<IntVec::ubyte> &outImg)
	{
		if ( inImg.type() != CV_8UC1 )
		{
			fprintf(stderr, __FILE__ ": CV_8UC1 expected\n");
			return;
		}

		const int w = inImg.cols;
		const int h = inImg.rows;

		outImg.allocate( w, h );

		for (int yi=0; yi<h; yi++)
		{
			unsigned char *srcData = inImg.data + (h - 1 - yi) * w;
			unsigned char *dstData = outImg.getData() + yi * w;

			for (int xi=0; xi<w; xi++)
			{
				*dstData++ = *srcData++;
			}
		}
	}

	template <>
	void convertMat2ImageRect(const cv::Mat &inImg, ImageRect<IntVec::ubvec3> &outImg)
	{
		if ( inImg.type() != CV_8UC3 )
		{
			fprintf(stderr, __FILE__ ": CV_8UC3 expected\n");
			return;
		}

		const int w = inImg.cols;
		const int h = inImg.rows;

		outImg.allocate( w, h );

		for (int yi=0; yi<h; yi++)
		{
			unsigned char *srcData = inImg.data + 3 * (h - 1 - yi) * w;
			unsigned char *dstData = ((unsigned char *)outImg.getData()) + 3 * yi * w;

			for (int xi=0; xi<w; xi++)
			{
				// 2014/04/11
				// OpenCV‚ÍBGR, ImageRect‚ÍRGB‚Ì‡‚ÅF‚ª“ü‚Á‚Ä‚¢‚é‚Ì‚Å‰º‹L‚ðC³
				/*
				*dstData++ = *srcData++;
				*dstData++ = *srcData++;
				*dstData++ = *srcData++;
				*/

				*(dstData  ) = *(srcData+2);
				*(dstData+1) = *(srcData+1);
				*(dstData+2) = *(srcData  );

				dstData += 3;
				srcData += 3;
			}
		}
	}

	template <>
	void convertMat2ImageRect(const cv::Mat &inImg, ImageRect<IntVec::ubvec4> &outImg)
	{
		if ( inImg.type() != CV_8UC4 )
		{
			fprintf(stderr, __FILE__ ": CV_8UC3 expected\n");
			return;
		}

		const int w = inImg.cols;
		const int h = inImg.rows;

		outImg.allocate( w, h );

		for (int yi=0; yi<h; yi++)
		{
			unsigned char *srcData = inImg.data + 4 * (h - 1 - yi) * w;
			unsigned char *dstData = ((unsigned char *)outImg.getData()) + 4 * yi * w;

			for (int xi=0; xi<w; xi++)
			{
				*dstData++ = *srcData++;
				*dstData++ = *srcData++;
				*dstData++ = *srcData++;
				*dstData++ = *srcData++;
			}
		}
	}

	template <>
	void convertMat2ImageRect(const cv::Mat &inImg, ImageRect<Real> &outImg)
	{
		if ( inImg.type() != CV_8UC1 && inImg.type() != CV_32FC1 )
		{
			fprintf(stderr, __FILE__ ": CV_8UC1 or CV_32FC1 expected\n");
			return;
		}

		const int w = inImg.cols;
		const int h = inImg.rows;

		outImg.allocate( w, h );

		if ( inImg.type() == CV_8UC1 )
		{
			for (int yi=0; yi<h; yi++)
			{
				unsigned char *srcData = inImg.data + (h - 1 - yi) * w;
				float *dstData = outImg.getData() + yi * w;

				for (int xi=0; xi<w; xi++)
				{
					*dstData++ = (*srcData++) / 255.f;
				}
			}
		}
		else
		{
			for (int yi=0; yi<h; yi++)
			{
				float *srcData = ((float *)inImg.data) + (h - 1 - yi) * w;
				float *dstData = outImg.getData() + yi * w;

				for (int xi=0; xi<w; xi++)
				{
					*dstData++ = *srcData++;
				}
			}
		}
	}

	template <>
	void convertMat2ImageRect(const cv::Mat &inImg, ImageRect<MyAlgebra::vec3> &outImg)
	{
		if ( inImg.type() != CV_8UC3 && inImg.type() != CV_32FC3 )
		{
			fprintf(stderr, __FILE__ ": CV_8UC3 or CV_32FC3 expected\n");
			return;
		}

		const int w = inImg.cols;
		const int h = inImg.rows;

		outImg.allocate( w, h );

		if ( inImg.type() == CV_8UC3 )
		{
			for (int yi=0; yi<h; yi++)
			{
				unsigned char *srcData = inImg.data +  3 * (h - 1 - yi) * w;
				float *dstData = ((float *)outImg.getData()) + 3 * yi * w;

				for (int xi=0; xi<w; xi++)
				{
					*dstData++ = (*srcData++) / 255.f;
					*dstData++ = (*srcData++) / 255.f;
					*dstData++ = (*srcData++) / 255.f;
				}
			}
		}
		else
		{
			for (int yi=0; yi<h; yi++)
			{
				float *srcData = ((float *)inImg.data) + 3 * (h - 1 - yi) * w;
				float *dstData = ((float *)outImg.getData()) + 3 * yi * w;

				for (int xi=0; xi<w; xi++)
				{
					*dstData++ = *srcData++;
					*dstData++ = *srcData++;
					*dstData++ = *srcData++;
				}
			}
		}
	}

	template <>
	void convertMat2ImageRect(const cv::Mat &inImg, ImageRect<MyAlgebra::vec4> &outImg)
	{
		if ( inImg.type() != CV_8UC4 && inImg.type() != CV_32FC4 )
		{
			fprintf(stderr, __FILE__ ": CV_8UC4 or CV_32FC4 expected\n");
			return;
		}

		const int w = inImg.cols;
		const int h = inImg.rows;

		outImg.allocate( w, h );

		if ( inImg.type() ==	CV_8UC4 )
		{
			for (int yi=0; yi<h; yi++)
			{
				unsigned char *srcData = inImg.data + 4 * (h - 1 - yi) * w;
				float *dstData = ((float *)outImg.getData()) + 4 * yi * w;

				for (int xi=0; xi<w; xi++)
				{
					*dstData++ = (*srcData++) / 255.f;
					*dstData++ = (*srcData++) / 255.f;
					*dstData++ = (*srcData++) / 255.f;
					*dstData++ = (*srcData++) / 255.f;
				}
			}
		}
		else
		{
			for (int yi=0; yi<h; yi++)
			{
				float *srcData = ((float *)inImg.data) + 4 * (h - 1 - yi) * w;
				float *dstData = ((float *)outImg.getData()) + 4 * yi * w;

				for (int xi=0; xi<w; xi++)
				{
					*dstData++ = *srcData++;
					*dstData++ = *srcData++;
					*dstData++ = *srcData++;
					*dstData++ = *srcData++;
				}
			}
		}
	}

	template <>
	void convertImageRect2Mat(const ImageRect<IntVec::ubyte> &inImg, cv::Mat &outImg)
	{
		const int w = inImg.getWidth();
		const int h = inImg.getHeight();

		outImg.create( h, w, CV_8UC1 );

		for (int yi=0; yi<h; yi++)
		{
			const unsigned char *srcData = inImg.getData() + yi * w;
			unsigned char *dstData = outImg.data + (h - 1 - yi) * w;

			int idx = 0;

			for (int xi=0; xi<w; xi++)
			{
				dstData[idx] = srcData[idx++];
			}
		}
	}

	template <>
	void convertImageRect2Mat(const ImageRect<IntVec::ubvec3> &inImg, cv::Mat &outImg)
	{
		const int w = inImg.getWidth();
		const int h = inImg.getHeight();

		outImg.create( h, w, CV_8UC3 );

		for (int yi=0; yi<h; yi++)
		{
			const unsigned char *srcData = ((unsigned char *)inImg.getData()) + 3 * yi * w;
			unsigned char *dstData = outImg.data + 3 * (h - 1 - yi) * w;

			int idx = 0;

			for (int xi=0; xi<w; xi++)
			{
				dstData[idx] = srcData[idx++];
				dstData[idx] = srcData[idx++];
				dstData[idx] = srcData[idx++];
			}
		}
	}

	template <>
	void convertImageRect2Mat(const ImageRect<IntVec::ubvec4> &inImg, cv::Mat &outImg)
	{
		const int w = inImg.getWidth();
		const int h = inImg.getHeight();

		outImg.create( h, w, CV_8UC4 );

		for (int yi=0; yi<h; yi++)
		{
			const unsigned char *srcData = ((unsigned char *)inImg.getData()) + 4 * yi * w;
			unsigned char *dstData = outImg.data + 4 * (h - 1 - yi) * w;

			int idx = 0;

			for (int xi=0; xi<w; xi++)
			{
				dstData[idx] = srcData[idx++];
				dstData[idx] = srcData[idx++];
				dstData[idx] = srcData[idx++];
				dstData[idx] = srcData[idx++];
			}
		}
	}

	inline unsigned char clampUbyte(float f) const { return (f < 0.f) ? 0 : (f > 255.f) ? 255 : (unsigned char)f; }

	template <>
	void convertImageRect2Mat(const ImageRect<Real> &inImg, cv::Mat &outImg)
	{
		const int w = inImg.getWidth();
		const int h = inImg.getHeight();

		outImg.create( h, w, CV_8UC1 );	// no float support

		for (int yi=0; yi<h; yi++)
		{
			const float *srcData = inImg.getData() + yi * w;
			unsigned char *dstData = outImg.data + (h - 1 - yi) * w;

			int idx = 0;

			for (int xi=0; xi<w; xi++)
			{
				dstData[idx] =  clampUbyte( 255.f * (srcData[idx++]) );
			}
		}
	}

	template <>
	void convertImageRect2Mat(const ImageRect<MyAlgebra::vec3> &inImg, cv::Mat &outImg)
	{
		const int w = inImg.getWidth();
		const int h = inImg.getHeight();

		outImg.create( h, w, CV_8UC3 );	// no float support

		for (int yi=0; yi<h; yi++)
		{
			const float *srcData = ((float *)inImg.getData()) + 3 * yi * w;
			unsigned char *dstData = outImg.data + 3 * (h - 1 - yi) * w;

			int idx = 0;

			for (int xi=0; xi<w; xi++)
			{
				dstData[idx] =  clampUbyte( 255.f * (srcData[idx++]) );
				dstData[idx] =  clampUbyte( 255.f * (srcData[idx++]) );
				dstData[idx] =  clampUbyte( 255.f * (srcData[idx++]) );
			}
		}
	}

	template <>
	void convertImageRect2Mat(const ImageRect<MyAlgebra::vec4> &inImg, cv::Mat &outImg)
	{
		const int w = inImg.getWidth();
		const int h = inImg.getHeight();

		outImg.create( h, w, CV_8UC4 );	// no float support

		for (int yi=0; yi<h; yi++)
		{
			const float *srcData = ((float *)inImg.getData()) + 4 * yi * w;
			unsigned char *dstData = outImg.data + 4 * (h - 1 - yi) * w;

			int idx = 0;

			for (int xi=0; xi<w; xi++)
			{
				dstData[idx] =  clampUbyte( 255.f * (srcData[idx++]) );
				dstData[idx] =  clampUbyte( 255.f * (srcData[idx++]) );
				dstData[idx] =  clampUbyte( 255.f * (srcData[idx++]) );
				dstData[idx] =  clampUbyte( 255.f * (srcData[idx++]) );
			}
		}
	}

	// io ----------------------------------------------------------------------

	template <class T>
	bool load(const char *filename, ImageRect<T> &imgRect, int nLimitPixels = 0)
	{
		fprintf(stderr, __FUNCTION__ ": unknown image type\n");
		return false;
	}

	template <class T>
	bool save(const char *filename, const ImageRect<T> &imgRect)
	{
		fprintf(stderr, __FUNCTION__ ": unknown image type\n");
		return false;
	}

	// template specifications for load

	template <> bool load(const char *filename, ImageRect<IntVec::ubyte> &imgRect, int nLimitPixels)
	{
		cv::Mat img = cv::imread( filename, 0 );	// flag == 0: enforce grayscale format
		if ( img.empty() )
		{
			fprintf(stderr, __FUNCTION__ ": cannot open: %s\n", filename);
			return false;
		}

		if (nLimitPixels) shrinkImage( img, nLimitPixels );

		const int w = img.cols;
		const int h = img.rows;
		imgRect.allocate( w, h );

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const unsigned char *srcPtr = img.data + xi + (h - 1 - yi) * w;
				IntVec::ubyte *dstPtr = imgRect.getData() + xi + yi * w;
				*dstPtr     = *srcPtr;
			}
		}

		return true;
	}

	template <> bool load(const char *filename, ImageRect<IntVec::ubvec3> &imgRect, int nLimitPixels)
	{
		cv::Mat img = cv::imread( filename, 1 );	// flag == 1: enforce RGB color format
		if ( img.empty() )
		{
			fprintf(stderr, __FUNCTION__ ": cannot open: %s\n", filename);
			return false;
		}

		if (nLimitPixels) shrinkImage( img, nLimitPixels );

		const int w = img.cols;
		const int h = img.rows;
		imgRect.allocate( w, h );

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const unsigned char *srcPtr = img.data + 3 * (xi + (h - 1 - yi) * w);
				IntVec::ubvec3 *dstPtr = imgRect.getData() + xi + yi * w;
				(*dstPtr).b = *(srcPtr  );
				(*dstPtr).g = *(srcPtr+1);
				(*dstPtr).r = *(srcPtr+2);
			}
		}

		return true;
	}

	template <> bool load(const char *filename, ImageRect<IntVec::ubvec4> &imgRect, int nLimitPixels)
	{
		cv::Mat img = cv::imread( filename, -1 );	// flag == -1: load an image as is
		if ( img.empty() )
		{
			fprintf(stderr, __FUNCTION__ ": cannot open: %s\n", filename);
			return false;
		}

		if (nLimitPixels) shrinkImage( img, nLimitPixels );

		const int w = img.cols;
		const int h = img.rows;
		imgRect.allocate( w, h );

		const int nChannels = img.channels();

		if ( nChannels == 4 )
		{
			for (int yi=0; yi<h; yi++)
			{
				for (int xi=0; xi<w; xi++)
				{
					const unsigned char *srcPtr = img.data + 4 * (xi + (h - 1 - yi) * w);
					IntVec::ubvec4 *dstPtr = imgRect.getData() + xi + yi * w;
					(*dstPtr).b = *(srcPtr  );
					(*dstPtr).g = *(srcPtr+1);
					(*dstPtr).r = *(srcPtr+2);
					(*dstPtr).a = *(srcPtr+3);
				}
			}
		}
		else if ( nChannels == 3 )
		{
			for (int yi=0; yi<h; yi++)
			{
				for (int xi=0; xi<w; xi++)
				{
					const unsigned char *srcPtr = img.data + 3 * (xi + (h - 1 - yi) * w);
					IntVec::ubvec4 *dstPtr = imgRect.getData() + xi + yi * w;
					(*dstPtr).b = *(srcPtr  );
					(*dstPtr).g = *(srcPtr+1);
					(*dstPtr).r = *(srcPtr+2);
					(*dstPtr).a = 255;
				}
			}
		}
		else
		{
			fprintf(stderr, __FUNCTION__ ": error: invalid num channels: %d\n", nChannels);
			return false;
		}

		return true;
	}

	template <> bool load(const char *filename, ImageRect<float> &imgRect, int nLimitPixels)
	{
		cv::Mat img = cv::imread( filename, 0 );	// flag == 0: enforce grayscale format
		if ( img.empty() )
		{
			fprintf(stderr, __FUNCTION__ ": cannot open: %s\n", filename);
			return false;
		}

		if (nLimitPixels) shrinkImage( img, nLimitPixels );

		const int w = img.cols;
		const int h = img.rows;
		imgRect.allocate( w, h );

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const unsigned char *srcPtr = img.data + xi + (h - 1 - yi) * w;
				float *dstPtr = imgRect.getData() + xi + yi * w;
				*dstPtr     = (*srcPtr) / 255.f;
			}
		}

		return true;
	}

	template <> bool load(const char *filename, ImageRect<MyAlgebra::vec3> &imgRect, int nLimitPixels)
	{
		cv::Mat img = cv::imread( filename, 1 );	// flag == 1: enforce RGB color format
		if ( img.empty() )
		{
			fprintf(stderr, __FUNCTION__ ": cannot open: %s\n", filename);
			return false;
		}

		if (nLimitPixels) shrinkImage( img, nLimitPixels );

		const int w = img.cols;
		const int h = img.rows;
		imgRect.allocate( w, h );

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const unsigned char *srcPtr = img.data + 3 * (xi + (h - 1 - yi) * w);
				MyAlgebra::vec3 *dstPtr = imgRect.getData() + xi + yi * w;
				(*dstPtr).b = *(srcPtr  ) / 255.f;
				(*dstPtr).g = *(srcPtr+1) / 255.f;
				(*dstPtr).r = *(srcPtr+2) / 255.f;
			}
		}

		return true;
	}

	template <> bool load(const char *filename, ImageRect<MyAlgebra::vec4> &imgRect, int nLimitPixels)
	{
		cv::Mat img = cv::imread( filename, -1 );	// flag == -1: load an image as is
		if ( img.empty() )
		{
			fprintf(stderr, __FUNCTION__ ": cannot open: %s\n", filename);
			return false;
		}

		if (nLimitPixels) shrinkImage( img, nLimitPixels );

		const int w = img.cols;
		const int h = img.rows;
		imgRect.allocate( w, h );

		const int nChannels = img.channels();

		if ( nChannels == 4 )
		{
			for (int yi=0; yi<h; yi++)
			{
				for (int xi=0; xi<w; xi++)
				{
					const unsigned char *srcPtr = img.data + 4 * (xi + (h - 1 - yi) * w);
					MyAlgebra::vec4 *dstPtr = imgRect.getData() + xi + yi * w;
					(*dstPtr).b = *(srcPtr  ) / 255.f;
					(*dstPtr).g = *(srcPtr+1) / 255.f;
					(*dstPtr).r = *(srcPtr+2) / 255.f;
					(*dstPtr).a = *(srcPtr+3) / 255.f;
				}
			}
		}
		else if ( nChannels == 3 )
		{
			for (int yi=0; yi<h; yi++)
			{
				for (int xi=0; xi<w; xi++)
				{
					const unsigned char *srcPtr = img.data + 3 * (xi + (h - 1 - yi) * w);
					MyAlgebra::vec4 *dstPtr = imgRect.getData() + xi + yi * w;
					(*dstPtr).b = *(srcPtr  ) / 255.f;
					(*dstPtr).g = *(srcPtr+1) / 255.f;
					(*dstPtr).r = *(srcPtr+2) / 255.f;
					(*dstPtr).a = 1.f;
				}
			}
		}
		else
		{
			fprintf(stderr, __FUNCTION__ ": error: invalid num channels: %d\n", nChannels);
			return false;
		}

		return true;
	}

	// template specifications for save

	template <> bool save(const char *filename, const ImageRect<IntVec::ubyte> &imgRect)
	{
		const int w = imgRect.getWidth();
		const int h = imgRect.getHeight();

		cv::Mat img(h, w, CV_8UC1);

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const IntVec::ubyte *srcPtr = imgRect.getData() + xi + (h - 1 - yi) * w;
				unsigned char *dstPtr = img.data + (xi + yi * w);
				*dstPtr     = *srcPtr;
			}
		}

		if ( ! cv::imwrite(filename, img) )
		{
			fprintf(stderr, __FUNCTION__ ": cannot save: %s\n", filename);
			return false;
		}

		return true;
	}

	template <> bool save(const char *filename, const ImageRect<IntVec::ubvec3> &imgRect)
	{
		const int w = imgRect.getWidth();
		const int h = imgRect.getHeight();

		cv::Mat img(h, w, CV_8UC3);

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const IntVec::ubvec3 *srcPtr = imgRect.getData() + xi + (h - 1 - yi) * w;
				unsigned char *dstPtr = img.data + 3 * (xi + yi * w);
				*dstPtr     = (*srcPtr).b;
				*(dstPtr+1) = (*srcPtr).g;
				*(dstPtr+2) = (*srcPtr).r;
			}
		}

		if ( ! cv::imwrite(filename, img) )
		{
			fprintf(stderr, __FUNCTION__ ": cannot save: %s\n", filename);
			return false;
		}

		return true;
	}

	template <> bool save(const char *filename, const ImageRect<IntVec::ubvec4> &imgRect)
	{
		const int w = imgRect.getWidth();
		const int h = imgRect.getHeight();

		cv::Mat img(h, w, CV_8UC4);

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const IntVec::ubvec4 *srcPtr = imgRect.getData() + xi + (h - 1 - yi) * w;
				unsigned char *dstPtr = img.data + 4 * (xi + yi * w);
				*dstPtr     = (*srcPtr).b;
				*(dstPtr+1) = (*srcPtr).g;
				*(dstPtr+2) = (*srcPtr).r;
				*(dstPtr+3) = (*srcPtr).a;
			}
		}

		if ( ! cv::imwrite(filename, img) )
		{
			fprintf(stderr, __FUNCTION__ ": cannot save: %s\n", filename);
			return false;
		}

		return true;
	}

	template <> bool save(const char *filename, const ImageRect<float> &imgRect)
	{
		const int w = imgRect.getWidth();
		const int h = imgRect.getHeight();

		cv::Mat img(h, w, CV_8UC1);

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const float *srcPtr = imgRect.getData() + xi + (h - 1 - yi) * w;
				unsigned char *dstPtr = img.data + (xi + yi * w);
				*dstPtr     = (unsigned char)(255 * (*srcPtr));
			}
		}

		if ( ! cv::imwrite(filename, img) )
		{
			fprintf(stderr, __FUNCTION__ ": cannot save: %s\n", filename);
			return false;
		}

		return true;
	}

	template <> bool save(const char *filename, const ImageRect<MyAlgebra::vec3> &imgRect)
	{
		const int w = imgRect.getWidth();
		const int h = imgRect.getHeight();

		cv::Mat img(h, w, CV_8UC3);

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const MyAlgebra::vec3 *srcPtr = imgRect.getData() + xi + (h - 1 - yi) * w;
				unsigned char *dstPtr = img.data + 3 * (xi + yi * w);
				*dstPtr     = (unsigned char)(255 * (*srcPtr).b);
				*(dstPtr+1) = (unsigned char)(255 * (*srcPtr).g);
				*(dstPtr+2) = (unsigned char)(255 * (*srcPtr).r);
			}
		}

		if ( ! cv::imwrite(filename, img) )
		{
			fprintf(stderr, __FUNCTION__ ": cannot save: %s\n", filename);
			return false;
		}

		return true;
	}

	template <> bool save(const char *filename, const ImageRect<MyAlgebra::vec4> &imgRect)
	{
		const int w = imgRect.getWidth();
		const int h = imgRect.getHeight();

		cv::Mat img(h, w, CV_8UC4);

		for (int yi=0; yi<h; yi++)
		{
			for (int xi=0; xi<w; xi++)
			{
				const MyAlgebra::vec4 *srcPtr = imgRect.getData() + xi + (h - 1 - yi) * w;
				unsigned char *dstPtr = img.data + 4 * (xi + yi * w);
				*dstPtr     = (unsigned char)(255 * (*srcPtr).b);
				*(dstPtr+1) = (unsigned char)(255 * (*srcPtr).g);
				*(dstPtr+2) = (unsigned char)(255 * (*srcPtr).r);
				*(dstPtr+3) = (unsigned char)(255 * (*srcPtr).a);
			}
		}

		if ( ! cv::imwrite(filename, img) )
		{
			fprintf(stderr, __FUNCTION__ ": cannot save: %s\n", filename);
			return false;
		}

		return true;
	}

private:
	void shrinkImage(cv::Mat &img, int nLimitPixels)
	{
		const int nPixels = img.cols * img.rows;

		if ( nPixels > nLimitPixels)
		{
			const float scale = sqrtf( nLimitPixels/(float)nPixels );
			cv::Mat tmp;
			cv::resize( img, tmp, cv::Size(0,0), scale, scale, cv::INTER_AREA );
			tmp.copyTo( img );	// deep copy
		}
	}

};
