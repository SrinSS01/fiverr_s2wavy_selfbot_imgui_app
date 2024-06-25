#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
#pragma once
#include "LibCurl.h"
#include "types/User.h"
#include "LinkedList.h"
#include "types/Guild.h"
#include <map>
#include <vector>
#include "types/Channel.h"
#include "types/Tag.h"
#include "types/MessageSchedule.h"

class Api {
public:
    Api(LibCurl& curl = LibCurl::Instance());
    Response get_users(std::map<std::string, User>& users) const;
    [[nodiscard]] Response DownloadImageFromURL(const std::string &url) const;
    static GLuint LoadTextureFromMemory(const unsigned char*data, int size);

    [[nodiscard]] Response delete_user(const std::string& user_id) const;

    Response add_user(const std::string& token, std::map<std::string, User> &users) const;

    Response load_guilds(const std::string& user_id, LinkedList<Guild> &guilds) const;
    [[nodiscard]] Response DownloadGuildIcon(const std::string &guild_id, const std::string &icon) const;

    Response load_channels(const std::string &user_id, const std::string &guild_id, LinkedList<Channel> &channel_list) const;
    Response schedules(const std::string &user_id, const std::string &guild_id, const std::string &channel_id, LinkedList<MessageSchedule> &schedules) const;
    [[nodiscard]] Response post_schedule(const MessageSchedule &schedule) const;
    [[nodiscard]] Response start_bot(const std::string &user_id) const;
    [[nodiscard]] Response stop_bot(const std::string &user_id) const;
    Response load_tags(std::string const &user_id, LinkedList<Tag> &tags) const;
    [[nodiscard]] Response create_tag(const std::string &user_id, Tag tag) const;

    Response delete_tag(const std::string &user_id, std::string &tag_name) const;

private:
    LibCurl& curl;

};
#pragma clang diagnostic pop
