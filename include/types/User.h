#pragma once
#include <string>

struct User {
    const std::string user_id;
    const std::string name;
    bool bot_running;
    const unsigned int textureID;

    User(
        std::string userId,
        std::string name,
        bool bot_running,
        unsigned int textureID
    ) : user_id(std::move(userId)), name(std::move(name)), bot_running(bot_running), textureID(textureID) {}
};