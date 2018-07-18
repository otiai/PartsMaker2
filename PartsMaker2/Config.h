#ifndef CONFIG_H
#define CONFIG_H

#include "ivec.h"

#define USE_NPAL_MAN  // NPAL少年のデータを使う(デバッグ用)
class Config
{
public:
	static const int FalseRegionID;
	static const int BackRegionID;
	static const IntVec::ubvec3 BackColor;
};

#endif // CONFIG_H