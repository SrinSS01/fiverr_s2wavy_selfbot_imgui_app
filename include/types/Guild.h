#pragma once
#include <string>
#include <utility>

struct Guild {
    std::string id;
    std::string name;
    std::string icon;
    std::string icon_data;
    unsigned int iconTextureID;
    bool configured;
    bool should_link_texture;
};