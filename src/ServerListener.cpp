
#include "easywsclient.hpp"
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>
#include "ServerListener.h"
#include <Geode/Geode.hpp>
#include <thread>
#include "GlobalVars.h"
#include "Macros.h"
#include <iostream>
#include <sstream>

using namespace cocos2d;

using namespace easywsclient;

using namespace geode::prelude;

std::unique_ptr<WebSocket> ws(nullptr);

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

void ServerListener::connectAsync() {
    std::thread t = std::thread(connect);
    t.detach();
}

void ServerListener::connect(){

    open();

    using easywsclient::WebSocket;

#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        return;
    }
#endif

    if (ws) {
        GlobalVars::getSharedInstance()->m_isServerOpen = true;

        while (ws->getReadyState() != WebSocket::CLOSED) {
            WebSocket::pointer wsp = &*ws; // a unique_ptr cannot be copied into a lambda
            ws->poll();
            ws->dispatch([wsp](const std::string& message) {
                onMessage(message);
                });
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    GlobalVars::getSharedInstance()->m_isServerOpen = false;
    ws.release();

#ifdef _WIN32
    WSACleanup();
#endif

}

void ServerListener::onMessage(std::string message) {
    geode::Loader::get()->queueInMainThread([message]() {

        LevelInfoLayer* levelInfoLayer = GlobalVars::getSharedInstance()->m_currentLevelInfoLayer;

        if (!levelInfoLayer) return;

        auto level = levelInfoLayer->m_level;

        int ID = level->m_levelID;

        std::vector<std::string> parts = split(message, ",");

        int sentID = std::stoi(parts.at(0));
        bool isSent;
        std::istringstream(parts.at(1)) >> std::boolalpha >> isSent;

        if(sentID == ID && isSent){

            if(level->m_stars > 0) return;

            CCNode* difficultySprite = levelInfoLayer->getChildByID("difficulty-sprite");

            CCSprite* sentCoin = CCSprite::create("sentCoin.png"_spr);
            sentCoin->setZOrder(-2);
            sentCoin->setPosition({difficultySprite->getContentSize().width/2, difficultySprite->getContentSize().height/2});
            difficultySprite->addChild(sentCoin);

        }
    });
}

void ServerListener::sendMessage(std::string message) {
    if(GlobalVars::getSharedInstance()->m_isServerOpen) {
        ws->send(message);
    }
}

void ServerListener::open() {

    ws.release();
    ws.reset(WebSocket::from_url("ws://164.152.25.111:15346"));

}




