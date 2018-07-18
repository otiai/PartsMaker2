#include "DepthViewSrc.h"
#include "ObjectManager.h"

AnimeFrame* DepthViewSrc::getAnimeFrame()
{
	return ObjectManager::getInstance()->getSrcFrame();
}