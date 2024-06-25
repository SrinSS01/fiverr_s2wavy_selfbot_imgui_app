//
// Created by Srinj on 25-06-2024.
//

#ifndef SELFBOT_TAG_H
#define SELFBOT_TAG_H
#include <string>
struct Tag {
    std::string name;
    std::string reply;

    bool operator==(Tag const& tag) const {
        return this->name == tag.name && this->reply == tag.reply;
    }
};
#endif //SELFBOT_TAG_H
