#ifndef DST_DEPTH_VIEW_H
#define DST_DEPTH_VIEW_H

#include "DepthViewBase.h"

class AnimeFrame;
class DepthViewDst : public DepthViewBase
{
protected:
	AnimeFrame* getAnimeFrame();
};
#endif // DST_DEPTH_VIEW_H