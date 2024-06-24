#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "ServerListener.h"
#include "GlobalVars.h"

using namespace geode::prelude;

$execute{
    ServerListener::connectAsync();
}

class $modify(LevelInfoLayer) {

    bool init(GJGameLevel* level, bool p1) {
		if (!LevelInfoLayer::init(level, p1)) return false;

		if (!GlobalVars::getSharedInstance()->m_isServerOpen) {
			ServerListener::connectAsync();
		}

		if(auto layer = GlobalVars::getSharedInstance()->m_currentLevelInfoLayer){
			layer->release();
		}

		GlobalVars::getSharedInstance()->m_currentLevelInfoLayer = this;
		this->retain();

		ServerListener::sendMessage(fmt::format("{}", level->m_levelID.value()));

		return true;
	}
};