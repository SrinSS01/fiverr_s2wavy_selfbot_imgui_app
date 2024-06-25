//
// Created by Srinj on 22-06-2024.
//

#ifndef SELFBOT_CHANNEL_H
#define SELFBOT_CHANNEL_H
#include <string>

struct Channel {
    std::string id;
    int type;
    std::string guildId;
    std::string name;
//    std::string parentId;
    bool configured;
};


#endif //SELFBOT_CHANNEL_H
