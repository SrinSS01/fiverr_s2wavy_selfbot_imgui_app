#include <iostream>
#include <GLFW/glfw3.h>
#include <json/json.h>
#include <stb_image.h>
#include "api/Api.h"

static std::string API_URL = "http://localhost:8000";

Api::Api(LibCurl &curl):curl(curl) {}

ResponsePair<std::map<std::string, User>> Api::get_users() {
    auto response = curl.Get(API_URL + "/self_bot_users");
    if (response.status != 200) {
        return {{}, response};
    }

    Json::CharReaderBuilder readerBuilder;
    Json::CharReader *reader = readerBuilder.newCharReader();
    Json::Value root;
    std::string errors;
    bool parsingSuccessful = reader->parse(response.data.c_str(),response.data.c_str() + response.data.size(), &root, &errors);
    delete reader;

    if (!parsingSuccessful) {
        return { {},{errors, 400} };
    }

    std::map<std::string, User> users;
    std::stringstream error_stream;

    for (const auto &item: root) {
        std::string user_id = item["user_id"].asString();
        auto avatar = item["avatar"];
        std::string url;
        if (avatar.isNull()) {
            url.append(API_URL).append("/avatars/").append(user_id).append("/").append("null");
        } else {
            url.append(API_URL).append("/avatars/").append(user_id).append("/").append(avatar.asString());
        }
        auto textureIDResponse = LoadTextureFromURL(url);

        if (!is_ok_status(textureIDResponse.second.status)) {
            error_stream << textureIDResponse.second.data << '\n';
        }

        users.emplace(
            user_id,
            User { user_id, item["name"].asString(), item["bot_running"].asBool(), textureIDResponse.first }
        );
    }
    const auto &err_str = error_stream.str();
    if (!err_str.empty()) {
        return {{},{err_str}};
    }

    return {users, response};
}

Response Api::delete_user(const std::string& user_id) {
    return curl.Delete(API_URL + "/self_bot_users/" + user_id);
}

Response Api::add_user(const std::string& token, std::map<std::string, User> &users) {
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
    auto textureIDResponse = LoadTextureFromURL(url);

    if (!is_ok_status(textureIDResponse.second.status)) {
        return textureIDResponse.second;
    }

    users.emplace(
            user_id,
            User{ user_id, user["username"].asString(), false, textureIDResponse.first }
    );
    response.data = root["message"].asString();
    return response;
}

ResponsePair<GLuint> Api::LoadTextureFromURL(const std::string &url) {
    auto response = curl.Get(url);
    if (!is_ok_status(response.status)) {
        return { 0, response };
    }
    return { LoadTextureFromMemory(reinterpret_cast<const stbi_uc *>(response.data.c_str()), response.data.size()), response };
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
