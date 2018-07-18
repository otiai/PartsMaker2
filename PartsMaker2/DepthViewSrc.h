#ifndef SRC_DEPTH_VIEW_H
#define SRC_DEPTH_VIEW_H

#include "DepthViewBase.h"

class AnimeFrame;
class DepthViewSrc : public DepthViewBase
{
protected:
	AnimeFrame* getAnimeFrame();
};
#endif // SRC_DEPTH_VIEW_H