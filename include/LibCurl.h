//
// Created by Srinj on 27-05-2024.
//

#ifndef DISCORDRPC_LIBCURL_H
#define DISCORDRPC_LIBCURL_H
#include <string>
#include <curl/curl.h>
#define is_ok_status(code) (code >= 200 && code <= 299)
#define StatusInternalServerError 500

struct Response {
    std::string data;
    long status;
};

class LibCurl {
public:
    LibCurl();
    ~LibCurl();
    LibCurl(const LibCurl&) = delete;
    LibCurl& operator=(const LibCurl&) = delete;

    Response Get(std::string const& url);
    Response Post(std::string const& url, std::string const& data);
    Response Post(std::string const& url);
    void setAuthorization(std::string&& token);

    static LibCurl& Instance();

    Response Delete(std::string const& url);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    curl_slist* global_header {};
};

#endif //DISCORDRPC_LIBCURL_H
