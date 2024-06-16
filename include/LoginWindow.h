//
// Created by Srinj on 26-05-2024.
//
#pragma once
#ifndef DISCORDRPC_LOGINWINDOW_H
#define DISCORDRPC_LOGINWINDOW_H
#include <imgui.h>
#include <imgui_stdlib.h>

class LoginWindow {
public:
    bool Show();

private:
    std::string username;
    std::string password;
    std::string code;
    bool show_login_error { false };
    bool captcha_error { false };
    bool mfa_error { false };
    bool mfa { false };
    std::string ticket {};
};

#endif //DISCORDRPC_LOGINWINDOW_H
