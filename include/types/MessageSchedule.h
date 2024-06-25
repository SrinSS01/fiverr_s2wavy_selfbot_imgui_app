//
// Created by Srinj on 23-06-2024.
//

#ifndef SELFBOT_MESSAGESCHEDULE_H
#define SELFBOT_MESSAGESCHEDULE_H
#include <string>

struct MessageSchedule {
    std::string guild_id;
    std::string channel_id;
    std::string selfbot_user_id;
    std::string message_content;
    std::string initiate_time;
    int interval;
    bool expired;
};
#endif //SELFBOT_MESSAGESCHEDULE_H
