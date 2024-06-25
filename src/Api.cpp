#include <iostream>
#include <GLFW/glfw3.h>
#include <json/json.h>
#include <stb_image.h>
#include "api/Api.h"

static std::string API_URL = "http://localhost:8000";

Api::Api(LibCurl &curl):curl(curl) {}

Response Api::get_users(std::map<std::string, User>& users) const {
    auto response = curl.Get(API_URL + "/self_bot_users");
    if (response.status != 200) {
        return response;
    }

    Json::CharReaderBuilder readerBuilder;
    Json::CharReader *reader = readerBuilder.newCharReader();
    Json::Value root;
    std::string errors;
    bool parsingSuccessful = reader->parse(response.data.c_str(),response.data.c_str() + response.data.size(), &root, &errors);
    delete reader;

    if (!parsingSuccessful) {
        return {errors, 400};
    }

    std::stringstream error_stream;

    for (const auto &item: root) {
        std::string user_id = item["user_id"].asString();
        auto avatar = item["avatar"];
        std::string url;
        url.append(API_URL).append("/avatars/");
        if (avatar.isNull()) {
            url.append(user_id).append("/").append("null");
        } else {
            url.append(user_id).append("/").append(avatar.asString());
        }
        auto textureIDResponse = DownloadImageFromURL(url);

        if (!is_ok_status(textureIDResponse.status)) {
            error_stream << textureIDResponse.data << '\n';
        }

        const GLuint textureID = LoadTextureFromMemory(reinterpret_cast<const unsigned char *>(textureIDResponse.data.c_str()), textureIDResponse.data.size());

        users.emplace(
            user_id,
            User { user_id, item["name"].asString(), item["bot_running"].asBool(), textureID }
        );
    }
    const auto &err_str = error_stream.str();
    if (!err_str.empty()) {
        return {err_str, 400};
    }

    return response;
}

Response Api::delete_user(const std::string& user_id) const {
    return curl.Delete(API_URL + "/self_bot_users/" + user_id);
}

Response Api::add_user(const std::string& token, std::map<std::string, User> &users) const {
    Response response = curl.Post(API_URL + "/self_bot_users", std::string(R"({"token":")").append(token).append(R"("})"));
    if (!(is_ok_status(response.status))) {
        return response;
    }
    Json::CharReaderBuilder readerBuilder;
    Json::CharReader *reader = readerBuilder.newCharReader();
    Json::Value root;
    std::string errors;
    bool parsingSuccessful = reader->parse(response.data.c_str(),response.data.c_str() + response.data.size(), &root, &errors);
    delete reader;

    if (!parsingSuccessful) {
        return { errors, 400 };
    }

    auto user = root["user"];

    std::string user_id = user["id"].asString();
    auto avatar = user["avatar"];
    std::string url;
    if (avatar.isNull()) {
        url.append(API_URL).append("/avatars/").append(user_id).append("/").append("null");
    } else {
        url.append(API_URL).append("/avatars/").append(user_id).append("/").append(avatar.asString());
    }
    auto textureIDResponse = DownloadImageFromURL(url);

    if (!is_ok_status(textureIDResponse.status)) {
        return textureIDResponse;
    }

    const GLuint textureID = LoadTextureFromMemory(reinterpret_cast<const unsigned char *>(textureIDResponse.data.c_str()), response.data.size());

    users.emplace(
            user_id,
            User{ user_id, user["username"].asString(), false, textureID }
    );
    response.data = root["message"].asString();
    return response;
}

Response Api::load_guilds(const std::string& user_id, LinkedList<Guild> &guilds) const {
    auto url = API_URL + "/servers/" + user_id;
    auto response = curl.Get(url);
    if (!is_ok_status(response.status)) {
        return response;
    }
    Json::CharReaderBuilder readerBuilder;
    Json::CharReader *reader = readerBuilder.newCharReader();
    Json::Value root;
    std::string errors;
    bool parsingSuccessful = reader->parse(response.data.c_str(),response.data.c_str() + response.data.size(), &root, &errors);
    delete reader;

    if (!parsingSuccessful) {
        return { errors, 400 };
    }

    for (const auto &item: root) {
        auto icon = item["icon"];

        guilds.push_back({
            .id = item["id"].asString(),
            .name = item["name"].asString(),
            .icon = icon.isNull()? "null": icon.asString(),
            .configured = item["configured"].asBool(),
            .should_link_texture = false
        });
    }

    return response;
}

Response Api::load_channels(const std::string &user_id, const std::string &guild, LinkedList<Channel> &channels) const {
    std::string url = API_URL;
    Response response = curl.Get(url.append("/channels/").append(user_id).append("/").append(guild));
    if (!is_ok_status(response.status)) {
        return response;
    }
    Json::CharReaderBuilder readerBuilder;
    Json::CharReader *reader = readerBuilder.newCharReader();
    Json::Value root;
    std::string errors;
    bool parsingSuccessful = reader->parse(response.data.c_str(),response.data.c_str() + response.data.size(), &root, &errors);
    delete reader;

    if (!parsingSuccessful) {
        return { errors, 400 };
    }
    for (const auto &item: root) {
        channels.push_back({
            .id = item["id"].asString(),
            .type = item["type"].asInt(),
            .guildId = item["guild_id"].asString(),
            .name = item["name"].asString(),
            .configured = item["configured"].asBool(),
        });
    }

    return response;
}

Response Api::load_tags(std::string const &user_id, LinkedList<Tag> &tags) const {
    auto response = curl.Get(API_URL + "/tag/" + user_id);
    if (!is_ok_status(response.status)) {
        return response;
    }
    Json::CharReaderBuilder readerBuilder;
    Json::CharReader *reader = readerBuilder.newCharReader();
    Json::Value root;
    std::string errors;
    bool parsingSuccessful = reader->parse(response.data.c_str(),response.data.c_str() + response.data.size(), &root, &errors);
    delete reader;
    if (!parsingSuccessful) {
        return { errors, 400 };
    }
    for (const auto &item: root) {
        tags.push_back({
            .name = item["name"].asString(),
            .reply = item["reply"].asString()
        });
    }
    return response;
}

Response Api::create_tag(const std::string &user_id, Tag tag) const {
    Json::Value root;
    root["name"] = tag.name;
    root["reply"] = tag.reply;
    Json::StreamWriterBuilder writer;
    std::string json_string = Json::writeString(writer, root);
    return curl.Post(API_URL + "/tag/" + user_id, json_string);
}

Response Api::delete_tag(const std::string &user_id, std::string &tag_name) const {
    return curl.Delete(API_URL + "/tag/" + user_id + "/" + tag_name);
}

Response Api::schedules(const std::string& user_id, const std::string &guild_id, const std::string & channel_id, LinkedList<MessageSchedule>& schedules) const {
    std::string url = API_URL;
    Response response = curl.Get(url.append("/schedules/").append(user_id).append("/").append(guild_id).append("/").append(channel_id));
    if (!is_ok_status(response.status)) {
        return response;
    }
    Json::CharReaderBuilder readerBuilder;
    Json::CharReader *reader = readerBuilder.newCharReader();
    Json::Value root;
    std::string errors;
    bool parsingSuccessful = reader->parse(response.data.c_str(),response.data.c_str() + response.data.size(), &root, &errors);
    delete reader;

    if (!parsingSuccessful) {
        return { errors, 400 };
    }
    for (const auto &item: root) {
        schedules.push_back({
            .guild_id = item["guild_id"].asString(),
            .channel_id = item["channel_id"].asString(),
            .selfbot_user_id = item["selfbot_user_id"].asString(),
            .message_content = item["message_content"].asString(),
            .initiate_time = item["initiate_time"].asString(),
            .interval = item["interval"].asInt(),
            .expired = item["expired"].asBool(),
        });
    }

    return response;
}

Response Api::post_schedule(MessageSchedule const& schedule) const {
    Json::Value root;
    root["guild_id"] = schedule.guild_id;
    root["channel_id"] = schedule.channel_id;
    root["selfbot_user_id"] = schedule.selfbot_user_id;
    root["message_content"] = schedule.message_content;
    root["initiate_time"] = schedule.initiate_time;
    root["interval"] = schedule.interval;
    root["expired"] = schedule.expired;

    Json::StreamWriterBuilder writer;
    std::string json_string = Json::writeString(writer, root);
    return curl.Post(API_URL + "/schedule", json_string);
}

Response Api::start_bot(const std::string& user_id) const {
    return curl.Post(API_URL + "/start_bot/" + user_id);
}
Response Api::stop_bot(const std::string& user_id) const {
    return curl.Post(API_URL + "/stop_bot/" + user_id);
}

Response Api::DownloadImageFromURL(const std::string &url) const {
    return curl.Get(url);
}
Response Api::DownloadGuildIcon(const std::string &id, const std::string& icon) const {
    std::string url;
    url.append(API_URL).append("/guild_icons/");
    if (icon == "null") {
        url.append(id).append("/").append("null");
    } else {
        url.append(id).append("/").append(icon);
    }
    return DownloadImageFromURL(url);
}

GLuint Api::LoadTextureFromMemory(const unsigned char*data, int size) {
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory(data, size, &image_width, &image_height, nullptr, 4);
    if (image_data == nullptr) {
        return 0;
    }

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    return image_texture;
}


