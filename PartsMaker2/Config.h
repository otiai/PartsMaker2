#ifndef CONFIG_H
#define CONFIG_H

#include "ivec.h"

#define USE_NPAL_MAN  // NPAL���N�̃f�[�^���g��(�f�o�b�O�p)
class Config
{
public:
	static const int FalseRegionID;
	static const int BackRegionID;
	static const IntVec::ubvec3 BackColor;
};

#endif // CONFIG_H