#include "EditViewDst.h"
#include "ObjectManager.h"
#include "RegionMatchHandler.h"

AnimeFrame* EditViewDst::getAnimeFrame()
{
	return ObjectManager::getInstance()->getDstFrame();
}
