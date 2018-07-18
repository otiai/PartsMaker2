#include "DepthViewDst.h"
#include "ObjectManager.h"

AnimeFrame* DepthViewDst::getAnimeFrame()
{
	return ObjectManager::getInstance()->getDstFrame();
}