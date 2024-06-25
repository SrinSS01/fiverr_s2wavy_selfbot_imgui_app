#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui_freetype.h>
#include "Window.h"
#include <iostream>
#include <thread>

#include "icons/EditDarkMode.png.h"
#include "icons/Delete.png.h"
#include "icons/Play.png.h"
#include "icons/Stop.png.h"
#include "icons/OnlineStatus.png.h"
#include "icons/OfflineStatus.png.h"
#include "icons/EditLightMode.png.h"
#include "icons/Close.png.h"
#include "icons/dark_mode.png.h"
#include "icons/light_mode.png.h"
#include "icons/Send.png.h"
#include "font/JetBrainsMonoNerdFont-Bold.ttf.h"

GLuint Window::DeleteIcon;
GLuint Window::EditDarkModeIcon;
GLuint Window::EditLightModeIcon;
GLuint Window::PlayIcon;
GLuint Window::StopIcon;
GLuint Window::OfflineStatusIcon;
GLuint Window::OnlineStatusIcon;
GLuint Window::CloseIcon;
GLuint Window::DarkModeIcon;
GLuint Window::LightModeIcon;
GLuint Window::SendIcon;

Window::Window(int width, int height) {
    glfwSetErrorCallback([](int error, const char *description) -> void { std::cerr << "GLFW Error " << error << ": " << description << std::endl; });
    if (!glfwInit()) {
        throw std::runtime_error("Unable to init glfw");
    }
    const char *glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "Selfbot Control Panel", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("Unable to create glfw window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    glViewport(0, 0, width, height);

//    io.Fonts->AddFontDefault();

    ImFontConfig jetbrains_font_config;
    jetbrains_font_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(JetBrainsMonoNerdFont_Bold, JetBrainsMonoNerdFont_BoldSize, 18.0f, &jetbrains_font_config);
    ImFontConfig config;
    static const ImWchar ranges[] = { 0x1, 0x1FFFF, 0 };
    config.OversampleH = config.OversampleV = 1;
    config.PixelSnapH = true;
    config.MergeMode = true;
    config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\seguiemj.ttf)", 18.0f, &config, ranges);
    io.Fonts->Build();

    io.IniFilename = nullptr;

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowPadding = {20, 20};
    style.FramePadding = {10, 5};
    style.FrameRounding = 6;
}

Window::~Window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

Window &Window::Instance() {
    static Window instance{480, 640};
    Response users_result = instance.api.get_users(instance.users);
    if (!is_ok_status(users_result.status)) {
        instance.state.messages.push_front(users_result);
        std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            instance.state.messages.pop_back();
        }).detach();
    }
    DeleteIcon = Api::LoadTextureFromMemory(Delete, DeleteSize);
    EditDarkModeIcon = Api::LoadTextureFromMemory(EditDarkMode, EditDarkModeSize);
    EditLightModeIcon = Api::LoadTextureFromMemory(EditLightMode, EditLightModeSize);
    PlayIcon = Api::LoadTextureFromMemory(Play, PlaySize);
    StopIcon = Api::LoadTextureFromMemory(Stop, StopSize);
    OfflineStatusIcon = Api::LoadTextureFromMemory(OfflineStatus, OfflineStatusSize);
    OnlineStatusIcon = Api::LoadTextureFromMemory(OnlineStatus, OnlineStatusSize);
    CloseIcon = Api::LoadTextureFromMemory(Close, CloseSize);
    DarkModeIcon = Api::LoadTextureFromMemory(dark_mode, dark_modeSize);
    LightModeIcon = Api::LoadTextureFromMemory(light_mode, light_modeSize);
    SendIcon = Api::LoadTextureFromMemory(Send, SendSize);
    return instance;
}

void Window::CircleImage(ImTextureID user_texture_id, float diameter, const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col) {
    ImVec2 p_min = ImGui::GetCursorScreenPos();
    ImVec2 p_max = ImVec2(p_min.x + diameter, p_min.y + diameter);
    ImGui::GetWindowDrawList()->AddImageRounded(user_texture_id, p_min, p_max, uv0, uv1, ImGui::GetColorU32(tint_col), diameter * 0.5f);
    ImGui::Dummy(ImVec2(diameter, diameter));
}

void Window::loop(std::function<void()> const &runnable) {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        switch (state.theme) {
            case LIGHT:
                ImGui::StyleColorsLight();
                break;
            case DARK:
                ImGui::StyleColorsDark();
                break;
        }

        runnable();

        ImGui::Render();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

Window::State &Window::get_state() {
    return state;
}

GLuint Window::get_theme_mode_icon() const {
    return state.theme == DARK ? LightModeIcon : DarkModeIcon;
}

GLuint Window::get_edit_icon() const {
    return state.theme == DARK ? EditDarkModeIcon : EditLightModeIcon;
}

void Window::toggle_theme() {
    switch (state.theme) {
        case DARK: {
            state.theme = LIGHT;
        }
            break;
        case LIGHT: {
            state.theme = DARK;
        }
    }
}