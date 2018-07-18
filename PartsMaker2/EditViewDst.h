#ifndef EDIT_VIEW_DST_H
#define EDIT_VIEW_DST_H

#include "EditViewBase.h"

class EditViewDst : public EditViewBase
{
protected:
	AnimeFrame* getAnimeFrame();
	int getViewID(){ return VIEW_SIDE_RIGHT; }
};
#endif // EDIT_VIEW_DST_H