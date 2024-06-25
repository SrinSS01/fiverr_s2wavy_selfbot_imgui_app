#pragma once

#include <GLFW/glfw3.h>
#include "LibCurl.h"
#include "LinkedList.h"
#include "api/Api.h"
#include "types/Channel.h"
#include "types/Tag.h"
#include <map>
#include <mutex>
#include <functional>

#ifndef SELFBOT_WINDOW_H
#define SELFBOT_WINDOW_H

class Window {
public:
    enum Theme {
        DARK, LIGHT
    };
    struct State {
        LinkedList<Response> messages;
        LinkedList<MessageSchedule> message_schedules;
        LinkedList<Guild> guilds;
        LinkedList<Channel> channels;
        LinkedList<Tag> tags;
        bool show_tags;
        std::string tag_name;
        std::string tag_reply;
        Theme theme { DARK };
        std::string token;
        std::string guild_search;
        std::unique_ptr<User> selected_user{ nullptr };
        std::unique_ptr<Guild> selected_guild { nullptr };
        std::unique_ptr<Channel> selected_channel { nullptr };
        std::string channel_search;
        std::string message;
    };
    struct Cache {
        std::map<std::string, GLuint> icon_cache{};
    };

private:
    Window(int width, int height);

public:
    ~Window();
    static Window& Instance();
    static void CircleImage(ImTextureID user_texture_id, float diameter, const ImVec2 &uv0 = ImVec2(0, 0), const ImVec2 &uv1 = ImVec2(1, 1), const ImVec4 &tint_col = ImVec4(1, 1, 1, 1));
    void loop(std::function<void()> const& runnable);
    State& get_state();

    [[nodiscard]] GLuint get_theme_mode_icon() const;
    void toggle_theme();
    [[nodiscard]] GLuint get_edit_icon() const;

private:
    GLFWwindow* window;
    State state {};

public:
    static GLuint DeleteIcon;
    static GLuint EditDarkModeIcon;
    static GLuint EditLightModeIcon;
    static GLuint PlayIcon;
    static GLuint StopIcon;
    static GLuint OfflineStatusIcon;
    static GLuint OnlineStatusIcon;
    static GLuint CloseIcon;
    static GLuint DarkModeIcon;
    static GLuint LightModeIcon;
    static GLuint SendIcon;

    const Api api{};
    std::map<std::string, User> users;
    Cache cache {};
};

#endif //SELFBOT_WINDOW_H
