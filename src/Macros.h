#define getLayer(type) typeinfo_cast<type>(CCDirector::sharedDirector()->getRunningScene()->getChildren()->objectAtIndex(0))