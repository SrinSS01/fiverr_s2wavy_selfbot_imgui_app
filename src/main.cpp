#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <json/json.h>
#include <thread>

#include "LoginWindow.h"
#include "LibCurl.h"
#include "api/Api.h"
#include "types/User.h"
#include "icons/EditDarkMode.png.h"
#include "icons/Delete.png.h"
#include "icons/Play.png.h"
#include "icons/Stop.png.h"
#include "icons/OnlineStatus.png.h"
#include "icons/OfflineStatus.png.h"
#include "icons/EditLightMode.png.h"

static const int width = 480;
static const int height = 640;

enum Appearance {
    DARK, LIGHT
};

void CircleImage(ImTextureID user_texture_id, float diameter, const ImVec2 &uv0 = ImVec2(0, 0), const ImVec2 &uv1 = ImVec2(1, 1), const ImVec4 &tint_col = ImVec4(1, 1, 1, 1)) {
    ImVec2 p_min = ImGui::GetCursorScreenPos();
    ImVec2 p_max = ImVec2(p_min.x + diameter, p_min.y + diameter);
    ImGui::GetWindowDrawList()->AddImageRounded(user_texture_id, p_min, p_max, uv0, uv1, ImGui::GetColorU32(tint_col), diameter * 0.5f);
    ImGui::Dummy(ImVec2(diameter, diameter));
}

int main(int, const char **args) {
    glfwSetErrorCallback([](int error, const char *description) -> void { std::cerr << "GLFW Error " << error << ": " << description << std::endl; });
    if (!glfwInit()) {
        return -1;
    }
    const char *glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(width, height, "DiscordRPC", nullptr, nullptr);
    if (window == nullptr) {
        return 1;
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

    io.Fonts->AddFontFromFileTTF((std::string(args[0]) + "../../fonts/JetBrainsMonoNerdFont-Bold.ttf").c_str(), 18.0f);
    io.IniFilename = nullptr;

    Appearance appearance = LIGHT;
    ImGui::StyleColorsLight();
    bool appearanceChecked = true;
    LibCurl &curl = LibCurl::Instance();
    Api api{curl};

    GLuint DeleteIcon = Api::LoadTextureFromMemory(Delete, DeleteSize);
    GLuint EditDarkModeIcon = Api::LoadTextureFromMemory(EditDarkMode, EditDarkModeSize);
    GLuint EditLightModeIcon = Api::LoadTextureFromMemory(EditLightMode, EditLightModeSize);
    GLuint PlayIcon = Api::LoadTextureFromMemory(Play, PlaySize);
    GLuint StopIcon = Api::LoadTextureFromMemory(Stop, StopSize);
    GLuint OfflineStatusIcon = Api::LoadTextureFromMemory(OfflineStatus, OfflineStatusSize);
    GLuint OnlineStatusIcon = Api::LoadTextureFromMemory(OnlineStatus, OnlineStatusSize);
    GLuint EditIcon;

    ResponsePair<std::map<std::string, User>> users_result = api.get_users();
    std::map<time_t, Response> messages{};

    if (!is_ok_status(users_result.second.status)) {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        messages.emplace(now_c, users_result.second);
        std::thread([&](){
            std::this_thread::sleep_for(std::chrono::seconds(3));
            messages.erase(now_c);
        }).detach();
    }
    std::map<std::string, User> users = users_result.first;

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowPadding = {20, 20};
    style.FramePadding = {10, 5};
    style.FrameRounding = 6;

    std::string token;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        switch (appearance) {
            case LIGHT:
                ImGui::StyleColorsLight();
                EditIcon = EditLightModeIcon;
                break;
            case DARK:
                ImGui::StyleColorsDark();
                EditIcon = EditDarkModeIcon;
                break;
        }

        ImGuiViewport *vp = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(vp->WorkSize);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;
        if (ImGui::Begin("##background", nullptr, windowFlags)) {
            const ImVec2 &regionAvail = ImGui::GetContentRegionAvail();
            ImGui::Text("Users");
            ImGui::SameLine();
            ImGui::SetCursorPos({regionAvail.x - 40, 10});
            ImGui::Checkbox(appearanceChecked ? "dark" : "light", &appearanceChecked);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            appearance = appearanceChecked ? DARK : LIGHT;

            if (ImGui::BeginChild("##users", {0, regionAvail.y - 120})) {
                if (!users.empty()) {
                    int i = 0;
                    std::vector<std::string> users_to_delete;
                    for (auto & [user_id, user] : users) {
                        ImGui::BeginChild(i + 1, {}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY); {
                            CircleImage(reinterpret_cast<ImTextureID>(user.textureID), 80);
                            ImGui::SameLine();
                            ImGui::Spacing();
                            ImGui::SameLine();

                            ImGui::BeginChild((i + 1) * 2, {}, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX); {
                                ImGui::SetCursorPosY(20);
                                ImGui::Text(user.name.c_str());
                                ImGui::Spacing();
                                ImGui::Text(user.user_id.c_str());
                            } ImGui::EndChild();

                            /*Spacing*/ {
                                ImGui::SameLine();
                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Spacing();
                                ImGui::SameLine();
                            }

                            ImGui::BeginChild((i + 1) * 4, {}, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX); {
                                ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
                                auto buttonHoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
                                buttonHoverColor.w = 0.3;
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 5, 5 });
                                ImGui::ImageButton(reinterpret_cast<ImTextureID>(user.bot_running ? StopIcon : PlayIcon),{20, 20});
                                ImGui::SameLine();
                                ImGui::ImageButton(reinterpret_cast<ImTextureID>(EditIcon), {20, 20});
                                ImGui::SameLine();
                                if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(DeleteIcon), {20, 20})) {
                                    ImGui::OpenPopup("Delete?");
                                }
                                ImGui::PopStyleVar(1);
                                ImGui::PopStyleColor(2);

                                ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
                                ImGui::SameLine(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::SameLine();
                                ImGui::Image(reinterpret_cast<ImTextureID>(user.bot_running ? OnlineStatusIcon: OfflineStatusIcon),{20, 20});
                                ImGui::SameLine();
                                ImGui::Text(user.bot_running ? "online" : "offline");

                                ImVec2 center = vp->GetCenter();
                                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                                if (ImGui::BeginPopupModal("Delete?", nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
                                    ImGui::Text("Do you want to delete this bot?");
                                    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

                                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                                    ImGui::PushStyleColor(ImGuiCol_Border, ImColor{ 34, 197, 94 }.Value);
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImColor{ 5, 46, 22 }.Value);
                                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor{22, 101, 52}.Value);
                                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor{5, 46, 22}.Value);
                                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                                        auto response = api.delete_user(user.user_id);
                                        auto now = std::chrono::system_clock::now();
                                        auto now_c = std::chrono::system_clock::to_time_t(now);
                                        messages.emplace(now_c, response);
                                        std::thread([&](){
                                            std::this_thread::sleep_for(std::chrono::seconds(3));
                                            messages.erase(now_c);
                                        }).detach();
                                        if (is_ok_status(response.status) || response.status == StatusInternalServerError) {
                                            users_to_delete.push_back(user_id);
                                        }
                                        ImGui::CloseCurrentPopup();
                                    }
                                    ImGui::PopStyleColor(4);
                                    ImGui::SetItemDefaultFocus();
                                    ImGui::SameLine();
                                    ImGui::PushStyleColor(ImGuiCol_Border, ImColor{239, 68, 68}.Value);
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImColor{69, 10, 10}.Value);
                                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor{153, 27, 27}.Value);
                                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor{69, 10, 10}.Value);
                                    if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                                    ImGui::PopStyleColor(4);
                                    ImGui::PopStyleVar(1);
                                    ImGui::EndPopup();
                                }
                            } ImGui::EndChild();
                        } ImGui::EndChild();
                        ImGui::Spacing();
                        ImGui::Spacing();
                        i++;
                    }
                    for (const auto &user_id: users_to_delete) {
                        users.erase(user_id);
                    }
                } ImGui::EndChild();
            }

            if (!messages.empty()) {
                ImGui::SetNextWindowPos({20, 10}, ImGuiCond_Always);
                ImGui::BeginChild("##error", {}, ImGuiChildFlags_AutoResizeY);
                int i = 0;
                for (const auto &[_, message]: messages) {
                    bool is_error = !is_ok_status(message.status);
                    ImGui::SetNextWindowBgAlpha(1);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 30.0f, 25.0f });
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, is_error? ImColor { 69, 10, 10 }.Value: ImColor{5, 46, 22}.Value);
                    ImGui::PushStyleColor(ImGuiCol_Border, is_error? ImColor{ 239, 68, 68 }.Value: ImColor{34, 197, 94}.Value);
                    ImGui::BeginChild((std::string ("##error").append(std::to_string(i + 1))).c_str(), {}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_FrameStyle);
                    ImGui::TextWrapped(message.data.c_str());
                    ImGui::EndChild();
                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar(2);
                    ImGui::Spacing();
                    ImGui::Spacing();
                    i++;
                }
                ImGui::EndChild();
            }

            ImGui::SetNextWindowPos({20, vp->WorkPos.y + vp->WorkSize.y - 20}, ImGuiCond_Always, {0, 1.0f});
            if (ImGui::BeginChild("##token", {0, 0}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) {
                ImGui::PushItemWidth(-80);
                ImGui::InputTextWithHint("##token", "Enter bot token", &token, ImGuiInputTextFlags_CharsNoBlank);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::BeginDisabled(token.empty());
                if (ImGui::Button("add", {-FLT_MIN, 0})) {
                    Response response = api.add_user(token, users);
                    auto now = std::chrono::system_clock::now();
                    auto now_c = std::chrono::system_clock::to_time_t(now);
                    messages.emplace(now_c, response);
                    std::thread([&](){
                        std::this_thread::sleep_for(std::chrono::seconds(3));
                        messages.erase(now_c);
                    }).detach();
                    token = "";
                }
                ImGui::EndDisabled();
                ImGui::EndChild();
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
