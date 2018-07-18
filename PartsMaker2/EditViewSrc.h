#ifndef EDIT_VIEW_SRC_H
#define EDIT_VIEW_SRC_H

#include "EditViewBase.h"

class EditViewSrc : public EditViewBase
{
protected:
	AnimeFrame* getAnimeFrame();
	int getViewID(){ return VIEW_FRONT; }
};
#endif // EDIT_VIEW_SRC_H