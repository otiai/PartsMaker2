#include "EditViewSrc.h"
#include "ObjectManager.h"
#include "RegionMatchHandler.h"

AnimeFrame* EditViewSrc::getAnimeFrame()
{
	return ObjectManager::getInstance()->getSrcFrame();
}