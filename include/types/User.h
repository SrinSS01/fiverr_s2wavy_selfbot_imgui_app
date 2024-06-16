#pragma once

struct User {
    const std::string user_id;
    const std::string name;
//    const std::string avatar;
    bool bot_running;
    const GLuint textureID;

    User(
            std::string userId,
            std::string name,
//            std::string avatar,
            bool bot_running,
            unsigned int textureID
    ) : user_id(std::move(userId)), name(std::move(name))/*, avatar(std::move(avatar))*/, bot_running(bot_running), textureID(textureID) {}
};