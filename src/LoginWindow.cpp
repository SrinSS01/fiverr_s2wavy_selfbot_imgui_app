//
// Created by Srinj on 26-05-2024.
//

#include <iostream>
#include "LoginWindow.h"
#include "LibCurl.h"
#include <json/json.h>

static void ShowLoginErrorMessage(bool should_show) {
    if (should_show) {
        ImGui::TextColored({ 1, 0, 0, 1 }, "Login or password is invalid.");
    }
}

bool LoginWindow::Show() {
    bool success = false;
    LibCurl& curl = LibCurl::Instance();

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("##login", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        if (mfa) {
            ImGui::InputTextWithHint("##code", "code", &code);
            if (mfa_error && code.empty()) {
                ImGui::TextColored({ 1, 0, 0, 1 }, "Invalid two-factor code.");
            }
            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 100.0f;
            float xPos = (windowWidth - buttonWidth) * 0.5f;

            ImGui::SetCursorPosX(xPos);
            if (ImGui::Button("login")) {
                Json::Value body;
                body["code"] = code;
                body["ticket"] = ticket;
                Json::StreamWriterBuilder writer;
                auto result = curl.Post("https://discord.com/api/v9/auth/mfa/totp", Json::writeString(writer, body));
                Json::CharReaderBuilder readerBuilder;
                Json::CharReader *reader = readerBuilder.newCharReader();
                Json::Value root;
                std::string errors;

                bool parsingSuccessful = reader->parse(result.data.c_str(),result.data.c_str() + result.data.size(), &root, &errors);
                delete reader;

                if (!parsingSuccessful) {
                    std::cerr << "Failed to parse the JSON string: " << errors << std::endl;
                }

                switch (result.status) {
                    case 200: {
                        curl.setAuthorization(root["token"].asString());
                        success = true;
                        code.clear();
                    } break;
                    case 400: {
                        mfa_error = true;
                        code.clear();
                        std::cerr << "ticket: " << ticket << std::endl;
                        std::cerr << result.data << std::endl;
                    } break;
                }
            }
        } else if (captcha_error) {
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 262);
            ImGui::TextColored({ 1, 0, 0, 1 }, "Captcha error, please login from web to fill the captcha");
            ImGui::PushTextWrapPos();

            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 100.0f;
            float xPos = (windowWidth - buttonWidth) * 0.5f;

            ImGui::SetCursorPosX(xPos);
            if (ImGui::Button("retry")) {
                captcha_error = false;
            }
        } else {
            ImGui::InputTextWithHint("##username", "Username", &username);
            ShowLoginErrorMessage(show_login_error && username.empty());
            ImGui::Spacing();
            ImGui::InputTextWithHint("##password", "Password", &password, ImGuiInputTextFlags_Password);
            ShowLoginErrorMessage(show_login_error && username.empty());
            ImGui::Spacing();

            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 100.0f;
            float xPos = (windowWidth - buttonWidth) * 0.5f;

            ImGui::SetCursorPosX(xPos);

            if (ImGui::Button("login")) {
                Json::Value body;
                body["login"] = username;
                body["password"] = password;
                Json::StreamWriterBuilder writer;

                auto result = curl.Post("https://discord.com/api/v9/auth/login", Json::writeString(writer, body));

                Json::CharReaderBuilder readerBuilder;
                Json::CharReader *reader = readerBuilder.newCharReader();
                Json::Value root;
                std::string errors;

                bool parsingSuccessful = reader->parse(result.data.c_str(),result.data.c_str() + result.data.size(), &root, &errors);
                delete reader;

                if (!parsingSuccessful) {
                    std::cerr << "Failed to parse the JSON string: " << errors << std::endl;
                }

                switch (result.status) {
                    case 200: {
                        if (!root["mfa"].isNull()) {
                            ticket = root["ticket"].asString();
                            mfa = true;
                            username.clear();
                            password.clear();
                        } else {
                            auto token = root["token"];
                            std::cout << token << std::endl;
                        }
                    } break;
                    case 400: {
                        if (!root["captcha_sitekey"].isNull()) {
                            captcha_error = true;
                        } else if (!root["errors"].isNull()) {
                            show_login_error = true;
                            username.clear();
                            password.clear();
                        }
                    } break;
                }
            }
        }
        ImGui::End();
    }

    return success;
}

