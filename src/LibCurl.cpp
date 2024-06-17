#include "LibCurl.h"
#include <iostream>

LibCurl::LibCurl() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
//    curl = curl_easy_init();
}

LibCurl::~LibCurl() {
    curl_slist_free_all(global_header);
//    if (curl) {
//        curl_easy_cleanup(curl);
//    }
    curl_global_cleanup();
}

Response LibCurl::Get(const std::string &url) {
    Response response;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
//        curl_easy_setopt(curl, CURLOPT_CAINFO, "./curl-ca-bundle.crt");
//        curl_easy_setopt(curl, CURLOPT_CAPATH, "./curl-ca-bundle.crt");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.data);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            response.data = curl_easy_strerror(res);
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);
        curl_easy_cleanup(curl);
    }
    return response;
}

Response LibCurl::Post(const std::string &url, const std::string &data) {
    Response response;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
//        curl_easy_setopt(curl, CURLOPT_CAINFO, "./curl-ca-bundle.crt");
//        curl_easy_setopt(curl, CURLOPT_CAPATH, "./curl-ca-bundle.crt");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.data);
        curl_slist *headers;
        headers = curl_slist_append(global_header, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            response.data = curl_easy_strerror(res);
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return response;
}

Response LibCurl::Delete(std::string const& url) {
    Response response;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
//        curl_easy_setopt(curl, CURLOPT_CAINFO, "./curl-ca-bundle.crt");
//        curl_easy_setopt(curl, CURLOPT_CAPATH, "./curl-ca-bundle.crt");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.data);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            response.data = curl_easy_strerror(res);
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);
        curl_easy_cleanup(curl);
    }
    return response;
}

size_t LibCurl::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

LibCurl &LibCurl::Instance() {
    static LibCurl instance;
    return instance;
}

void LibCurl::setAuthorization(std::string &&token) {
    global_header = curl_slist_append(nullptr, ("Authorization: " + token).c_str());
}
