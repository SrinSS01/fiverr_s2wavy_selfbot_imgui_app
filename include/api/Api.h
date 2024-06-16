#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
#pragma once
#include "LibCurl.h"
#include "types/User.h"
#include <vector>

template<typename T> using ResponsePair = std::pair<T, Response>;

class Api {
public:
    Api(LibCurl& curl);
    ResponsePair<std::map<std::string, User>> get_users();
    GLuint LoadTextureFromURL(std::string const& url);
    static GLuint LoadTextureFromMemory(const unsigned char*data, int size);

    Response delete_user(const std::string& user_id);

    Response add_user(const std::string& token, std::map<std::string, User> &users);

private:
    LibCurl& curl;

};
#pragma clang diagnostic pop