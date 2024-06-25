#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_stdlib.h>
#include <thread>
#include "Window.h"
#include "imgui_internal.h"
#include <sstream>
#include <iostream>

#include <imguidatechooser.h>

int main() {
    Window &window = Window::Instance();
    auto &state = window.get_state();
    window.loop([&]() {
        ImGuiViewport *vp = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(vp->WorkSize);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;

        ImGui::Begin("##background", nullptr, windowFlags);
        if (state.show_tags) {
            const ImVec2 &regionAvail = ImGui::GetContentRegionAvail();
            ImGui::BeginChild("##user", {0, 50});
            {
                Window::CircleImage(reinterpret_cast<ImTextureID>(state.selected_user->textureID), 40);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
                auto buttonHoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
                buttonHoverColor.w = 0.3;
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);
                if (state.selected_user->bot_running) {
                    if (ImGui::ImageButton("##stop", reinterpret_cast<ImTextureID>(Window::StopIcon), {30, 30})) {
                        auto res = window.api.stop_bot(state.selected_user->user_id);
                        state.messages.push_front(res);
                        std::thread([&]() {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            state.messages.pop_back();
                        }).detach();
                        if (is_ok_status(res.status)) {
                            state.selected_user->bot_running = false;
                        }
                    }
                } else {
                    if (ImGui::ImageButton("##start", reinterpret_cast<ImTextureID>(Window::PlayIcon), {30, 30})) {
                        auto res = window.api.start_bot(state.selected_user->user_id);
                        state.messages.push_front(res);
                        std::thread([&]() {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            state.messages.pop_back();
                        }).detach();
                        if (is_ok_status(res.status)) {
                            state.selected_user->bot_running = true;
                        }
                    }
                }
                ImGui::SameLine();
                auto textSize = ImGui::CalcTextSize("Tags");
                auto window_width = ImGui::GetWindowWidth();
                ImGui::SetCursorPos({window_width * 0.5f - textSize.x * 0.5f, 10});
                ImGui::Text("Tags");
                ImGui::SameLine();
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {5, 5});
                ImGui::SetCursorPosX(regionAvail.x - 30);
                if (ImGui::ImageButton("##close", reinterpret_cast<ImTextureID>(Window::CloseIcon), {20, 20})) {
                    state.selected_user.reset();
                    state.tags.is_valid = false;
                    state.show_tags = false;
                }
                ImGui::PopStyleVar(1);
                ImGui::PopStyleColor(2);
            }
            ImGui::EndChild();

            ImGui::BeginChild("##tags", ImVec2{0, regionAvail.y - 140}, ImGuiChildFlags_Border);
            state.tags.for_each([&](Tag& tag){
                if (ImGui::TreeNodeEx(tag.name.c_str(), ImGuiTreeNodeFlags_Selected)) {
                    ImGui::TextWrapped(tag.reply.c_str());
                    ImGui::TreePop();
                }
                if (ImGui::BeginPopupContextItem((state.selected_user->user_id + tag.name).c_str())) {
                    if (ImGui::MenuItem("delete")) {
                        std::thread([&](){
                            Response res = window.api.delete_tag(state.selected_user->user_id, tag.name);
                            state.messages.push_front(res);
                            std::thread([&]() {
                                std::this_thread::sleep_for(std::chrono::seconds(5));
                                state.messages.pop_back();
                            }).detach();
                            if (is_ok_status(res.status)) {
                                state.tags.delete_value(tag);
                            }
                        }).detach();
                    }
                    ImGui::EndPopup();
                }
            });
            ImGui::EndChild();

            ImGui::SetNextWindowPos({20, vp->WorkPos.y + vp->WorkSize.y - 20}, ImGuiCond_Always, {0, 1.0f});
            ImGui::BeginChild("##add_tag", {}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
            {
                ImGui::PushItemWidth(80);
                ImGui::InputTextWithHint("##name", "Name", &state.tag_name); ImGui::SameLine();
                ImGui::PopItemWidth();
                ImGui::PushItemWidth(250);
                ImGui::InputTextMultilineWithHint("##reply", "Enter a reply", &state.tag_reply, {0, ImGui::GetItemRectSize().y}); ImGui::SameLine();
                ImGui::PopItemWidth();
                ImGui::BeginDisabled(state.tag_name.empty() || state.tag_reply.empty());
                if (ImGui::Button("add", {-FLT_MIN, 0})) {
                    std::thread([&](){
                        Tag tag {
                            .name = state.tag_name,
                            .reply = state.tag_reply
                        };
                        auto res = window.api.create_tag(state.selected_user->user_id, tag);
                        state.messages.push_front(res);
                        std::thread([&]() {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            state.messages.pop_back();
                        }).detach();
                        if (is_ok_status(res.status)) {
                            state.tags.push_back(tag);
                        }
                    }).detach();
                }
                ImGui::EndDisabled();
            }
            ImGui::EndChild();
        }
        else if (state.selected_guild != nullptr) {
            const ImVec2 &regionAvail = ImGui::GetContentRegionAvail();
            ImGui::BeginChild("##user", {}, ImGuiChildFlags_AutoResizeY);
            {
                Window::CircleImage(reinterpret_cast<ImTextureID>(state.selected_user->textureID), 40);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
                auto buttonHoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
                buttonHoverColor.w = 0.3;
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);

                if (state.selected_user->bot_running) {
                    if (ImGui::ImageButton("##stop", reinterpret_cast<ImTextureID>(Window::StopIcon), {30, 30})) {
                        auto res = window.api.stop_bot(state.selected_user->user_id);
                        state.messages.push_front(res);
                        std::thread([&]() {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            state.messages.pop_back();
                        }).detach();
                        if (is_ok_status(res.status)) {
                            state.selected_user->bot_running = false;
                        }
                    }
                } else {
                    if (ImGui::ImageButton("##start", reinterpret_cast<ImTextureID>(Window::PlayIcon), {30, 30})) {
                        auto res = window.api.start_bot(state.selected_user->user_id);
                        state.messages.push_front(res);
                        std::thread([&]() {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            state.messages.pop_back();
                        }).detach();
                        if (is_ok_status(res.status)) {
                            state.selected_user->bot_running = true;
                        }
                    }
                }
                ImGui::SameLine();


                ImGui::InvisibleButton("##canvas", {240, ImGui::GetTextLineHeight() * 2 + 5});
                const ImVec2 p0 = ImGui::GetItemRectMin();
                const ImVec2 p1 = ImGui::GetItemRectMax();
                ImGui::PushClipRect(p0, p1, true);
                ImDrawList *draw_list = ImGui::GetWindowDrawList();
                draw_list->AddText(p0, ImColor(34, 197, 94), state.selected_guild->name.c_str());
                ImGui::PopClipRect();
                const auto size = ImGui::CalcTextSize(state.selected_guild->name.c_str());
                if (size.x > 240) {
                    draw_list->AddText({p1.x, p0.y}, ImColor(34, 197, 94), "...");
                }
                if (state.selected_channel != nullptr) {
                    const auto hash_size = ImGui::CalcTextSize("# ");
                    draw_list->AddText({p0.x, p0.y + ImGui::GetTextLineHeight() + 5}, state.theme == Window::DARK ? IM_COL32_WHITE : IM_COL32_BLACK, "# ");
                    ImGui::PushClipRect(p0, p1, true);
                    draw_list->AddText({p0.x + hash_size.x, p0.y + ImGui::GetTextLineHeight() + 5}, ImColor(249, 115, 22), state.selected_channel->name.c_str());
                    ImGui::PopClipRect();
                }

                ImGui::SameLine();
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {5, 5});
                ImGui::SetCursorPosX(regionAvail.x - 30);
                if (ImGui::ImageButton("##close", reinterpret_cast<ImTextureID>(Window::CloseIcon), {20, 20})) {
                    if (state.selected_channel == nullptr) {
                        state.selected_guild.reset();
                        state.channels.is_valid = false;
                        state.channel_search.clear();
                    } else {
                        state.selected_channel.reset();
                        state.message_schedules.is_valid = false;
                    }
                }
                ImGui::PopStyleVar(1);
                ImGui::PopStyleColor(2);
            }
            ImGui::EndChild();

            if (state.selected_channel == nullptr) {
                const auto search_box_size = ImGui::CalcTextSize("Search channel by name or ID").x + 20;
                ImGui::PushItemWidth(search_box_size);
                ImGui::SetCursorPosX((vp->WorkSize.x - search_box_size) * 0.5f);
                ImGui::InputTextWithHint("##search_channel", "Search channel by name or ID", &state.channel_search);
                ImGui::PopItemWidth();
                ImGui::Spacing();
                ImGui::Spacing();
            } else {
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
            }
            ImGui::SetNextWindowBgAlpha(0);
            ImGui::BeginChild("##channels", state.selected_channel != nullptr ? ImVec2{0, regionAvail.y - 180} : ImVec2{}, ImGuiChildFlags_Border);
            if (state.selected_channel != nullptr) {
                if (!state.message_schedules.is_empty()) {
                    state.message_schedules.for_each([&](MessageSchedule& schedule){
                        auto draw_list = ImGui::GetWindowDrawList();
                        ImGui::BeginGroup();
                        {
                            auto msg_size = ImGui::CalcTextSize(schedule.message_content.c_str(), nullptr, false, ImGui::GetWindowWidth());
                            ImGui::InvisibleButton("##", msg_size);
                            auto p0 = ImGui::GetItemRectMin();
                            auto p1 = ImGui::GetItemRectMax();
                            draw_list->AddRectFilled({p0.x -  5, p0.y - 5}, {ImGui::GetWindowWidth() + 5, p1.y + 5}, ImColor(31, 41, 55));
                            draw_list->AddText(nullptr, 0, p0, IM_COL32_WHITE, schedule.message_content.c_str(), nullptr, ImGui::GetWindowWidth());
                            ImGui::ItemSize({0, 3});
                            ImGui::BeginGroup();
                            {
                                std::istringstream iss(schedule.initiate_time);
                                time_t timestamp; iss >> timestamp; timestamp /= 1000;
                                auto tm = std::localtime(&timestamp);
                                auto time = std::asctime(tm);
                                auto time_size = ImGui::CalcTextSize(time);
                                ImGui::InvisibleButton("##", time_size);
                                p0 = ImGui::GetItemRectMin();
                                p1 = ImGui::GetItemRectMax();
                                draw_list->AddRectFilled({p0.x - 5, p0.y - 5}, {p1.x + 10, p1.y + 5}, ImColor(75, 85, 99));
                                draw_list->AddText(p0, IM_COL32_WHITE, time);
                                ImGui::SameLine();
                                ImGui::SetCursorPosX(p1.x - 5);
                                std::string interval = "Interval: "+ std::to_string(schedule.interval) +" sec";
                                auto interval_size = ImGui::CalcTextSize(interval.c_str());
                                ImGui::InvisibleButton("##", interval_size);
                                p0 = ImGui::GetItemRectMin();
                                p1 = ImGui::GetItemRectMax();
                                draw_list->AddRectFilled({p0.x - 5, p0.y - 5}, {p1.x + 5, p1.y + 5}, ImColor(156, 163, 175));
                                draw_list->AddText(p0, IM_COL32_BLACK, interval.c_str());
                                ImGui::SameLine();
                                if (schedule.expired) {
                                    auto expired_size = ImGui::CalcTextSize("Expired");
                                    ImGui::InvisibleButton("##", expired_size);
                                    p0 = ImGui::GetItemRectMin();
                                    p1 = ImGui::GetItemRectMax();
                                    draw_list->AddRectFilled({p0.x - 5, p0.y - 5}, {p1.x + 5, p1.y + 5}, ImColor(229, 231, 235));
                                    draw_list->AddText(p0, IM_COL32_BLACK, "Expired");
                                }
                            }
                            ImGui::EndGroup();
                        }
                        ImGui::EndGroup();
                        ImGui::ItemSize({0, 10});
                    });
                }
            } else if (!state.channels.is_empty()) {
                int style_color_count = 3;
                if (state.theme == Window::DARK) {
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImColor(154, 52, 18).Value);
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImColor(67, 20, 7).Value);
                    ImGui::PushStyleColor(ImGuiCol_Header, ImColor(154, 52, 18).Value);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImColor(255, 86, 30).Value);
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImColor(186, 56, 19).Value);
                    ImGui::PushStyleColor(ImGuiCol_Header, ImColor(255, 86, 30).Value);
                }
                state.channels.for_each([&](Channel &channel) {
                    if (!state.channel_search.empty() && (channel.name.find(state.channel_search) == std::string::npos && channel.id.find(state.channel_search) == std::string::npos)) {
                        return;
                    }
                    ImGui::BeginChild(("##channel" + channel.id).c_str(), {}, ImGuiChildFlags_AutoResizeY);
                    const char *name = channel.name.c_str();
                    const auto size = ImGui::CalcTextSize(name, nullptr, false, ImGui::GetWindowWidth() - 10);
                    if (ImGui::Selectable(("##selectable" + channel.id).c_str(), false, ImGuiSelectableFlags_AllowOverlap, {ImGui::GetWindowWidth(), size.y + 10})) {
                        state.selected_channel = std::make_unique<Channel>();
                        state.selected_channel->id = channel.id;
                        state.selected_channel->type = channel.type;
                        state.selected_channel->guildId = channel.guildId;
                        state.selected_channel->name = channel.name;
                        state.selected_channel->configured = channel.configured;
                        state.message_schedules = LinkedList<MessageSchedule>();
                        std::thread([&](){
                            auto res = window.api.schedules(state.selected_user->user_id, state.selected_guild->id, state.selected_channel->id, state.message_schedules);
                            if (!is_ok_status(res.status)) {
                                state.messages.push_front(res);
                                std::thread([&]() {
                                    std::this_thread::sleep_for(std::chrono::seconds(5));
                                    state.messages.pop_back();
                                }).detach();
                            }
                        }).detach();
                    }
                    ImGui::SameLine();
                    ImGui::SetCursorPos({5, 5});
                    ImGui::TextWrapped(name);
                    ImGui::EndChild();
                    if (channel.configured) {
                        auto p0 = ImGui::GetItemRectMin();
                        auto p1 = ImGui::GetItemRectMax();
                        p0.x -= 2;
                        p0.y -= 2;
                        p1.x += 2;
                        p1.y += 2;
                        ImGui::GetWindowDrawList()->AddRect(p0, p1, ImColor(59, 130, 246), 0, 0, 2.0f);
                    }
                    ImGui::ItemSize({0,5});
                });
                ImGui::PopStyleColor(style_color_count);
            }
            ImGui::EndChild();
            if (state.selected_channel != nullptr) {
                ImGui::SetNextWindowPos({20, vp->WorkPos.y + vp->WorkSize.y - 20}, ImGuiCond_Always, {0, 1.0f});
                ImGui::BeginChild("##message_scheduling", {}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
                {
                    static tm time;
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
                    ImGui::DateChooser("##date", time);
                    ImGui::PopStyleVar();
                    ImGui::SameLine();

                    static char const *hours[] = {
                            "00", "01", "02", "03", "04", "05", "06", "07",
                            "08", "09", "10", "11", "12", "13", "14", "15",
                            "16", "17", "18", "19", "20", "21", "22", "23"
                    };
                    static char const *minutes[] = {
                            "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
                            "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                            "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
                            "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
                            "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
                            "50", "51", "52", "53", "54", "55", "56", "57", "58", "59"
                    };
                    static int minutes_index = 0;
                    static int hours_index = 0;
                    char const *minutes_preview = minutes[minutes_index];
                    char const *hours_preview = hours[hours_index];
                    if (ImGui::BeginCombo("##hours", hours_preview, ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightSmall)) {
                        for (int i = 0; i < IM_ARRAYSIZE(hours); ++i) {
                            bool selected = i == hours_index;
                            if (ImGui::Selectable(hours[i], selected)) {
                                hours_index = i;
                            }
                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::SameLine();
                    ImGui::Text(":");
                    ImGui::SameLine();
                    if (ImGui::BeginCombo("##minutes", minutes_preview, ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightSmall)) {
                        for (int i = 0; i < IM_ARRAYSIZE(minutes); ++i) {
                            bool selected = i == minutes_index;
                            if (ImGui::Selectable(minutes[i], selected)) {
                                minutes_index = i;
                            }
                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::SameLine();
                    static std::string interval;
                    ImGui::PushItemWidth(-FLT_MIN);
                    ImGui::InputTextWithHint("##interval", "interval in secs", &interval, ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiInputTextCallbackData *data) -> int {
                        if (strchr("0123456789", (char)data->EventChar)) {
                            return 0;
                        } else return 1;
                    });

                    ImGui::Spacing();
                    ImGui::PushItemWidth(-45);
                    ImGui::InputTextMultilineWithHint("##message", "Enter a message to schedule", &state.message, {0, 30});
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    ImGui::BeginDisabled(state.message.empty() || interval.empty());
                    if (ImGui::ImageButton("##send", reinterpret_cast<ImTextureID>(Window::SendIcon), {20, 18})) {
                        std::thread([&](){
                            time.tm_min = minutes_index;
                            time.tm_hour = hours_index;
                            time.tm_sec = 0;
                            auto initiate_time = std::mktime(&time) * 1000;
                            std::istringstream iss(interval);
                            int _interval;
                            iss >> _interval;
                            MessageSchedule schedule {
                                    .guild_id = state.selected_guild->id,
                                    .channel_id = state.selected_channel->id,
                                    .selfbot_user_id = state.selected_user->user_id,
                                    .message_content = state.message,
                                    .initiate_time = std::to_string(initiate_time),
                                    .interval = _interval,
                                    .expired = false,
                            };
                            auto const res = window.api.post_schedule(schedule);
                            state.messages.push_front(res);
                            if (is_ok_status(res.status)) {
                                state.message_schedules.push_back(schedule);
                            }
                            std::thread([&]() {
                                std::this_thread::sleep_for(std::chrono::seconds(5));
                                state.messages.pop_back();
                            }).detach();
                        }).detach();
                    }
                    ImGui::EndDisabled();
                }
                ImGui::EndChild();
            }
        }
        /* Show Config */
        else if (state.selected_user != nullptr) {
            const ImVec2 &regionAvail = ImGui::GetContentRegionAvail();
            ImGui::BeginChild("##user", {0, 50});
            {
                Window::CircleImage(reinterpret_cast<ImTextureID>(state.selected_user->textureID), 40);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
                auto buttonHoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
                buttonHoverColor.w = 0.3;
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);
                if (state.selected_user->bot_running) {
                    if (ImGui::ImageButton("##stop", reinterpret_cast<ImTextureID>(Window::StopIcon), {30, 30})) {
                        auto res = window.api.stop_bot(state.selected_user->user_id);
                        state.messages.push_front(res);
                        std::thread([&]() {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            state.messages.pop_back();
                        }).detach();
                        if (is_ok_status(res.status)) {
                            state.selected_user->bot_running = false;
                        }
                    }
                } else {
                    if (ImGui::ImageButton("##start", reinterpret_cast<ImTextureID>(Window::PlayIcon), {30, 30})) {
                        auto res = window.api.start_bot(state.selected_user->user_id);
                        state.messages.push_front(res);
                        std::thread([&]() {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            state.messages.pop_back();
                        }).detach();
                        if (is_ok_status(res.status)) {
                            state.selected_user->bot_running = true;
                        }
                    }
                }
                ImGui::SameLine();
                auto textSize = ImGui::CalcTextSize("Guilds");
                auto window_width = ImGui::GetWindowWidth();
                ImGui::SetCursorPos({window_width * 0.5f - textSize.x * 0.5f, 10});
                ImGui::Text("Guilds");
                ImGui::SameLine();
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {5, 5});
                ImGui::SetCursorPosX(regionAvail.x - 30);
                if (ImGui::ImageButton("##close", reinterpret_cast<ImTextureID>(Window::CloseIcon), {20, 20})) {
                    state.selected_user.reset();
                    state.guilds.is_valid = false;
                    state.selected_guild.reset();
                    state.guild_search.clear();
                }
                ImGui::PopStyleVar(1);
                ImGui::PopStyleColor(2);
            }
            ImGui::EndChild();

            const auto search_box_size = ImGui::CalcTextSize("Search guild by name or ID").x + 20;
            ImGui::PushItemWidth(search_box_size);
            ImGui::SetCursorPosX((vp->WorkSize.x - search_box_size) * 0.5f);
            ImGui::InputTextWithHint("##search_guild", "Search guild by name or ID", &state.guild_search);
            ImGui::PopItemWidth();
            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::BeginChild("##guilds", {}, ImGuiChildFlags_Border);
            int style_color_count = 3;
            if (state.theme == Window::DARK) {
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImColor(22, 101, 52).Value);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImColor(13, 61, 32).Value);
                ImGui::PushStyleColor(ImGuiCol_Header, ImColor(23, 37, 101).Value);
            } else {
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImColor(60, 209, 117).Value);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImColor(40, 138, 77).Value);
                ImGui::PushStyleColor(ImGuiCol_Header, ImColor(76, 108, 255).Value);
            }
            if (!state.guilds.is_empty()) {
                state.guilds.for_each([&](Guild &guild) {
                    if (!state.guild_search.empty() && (guild.name.find(state.guild_search) == std::string::npos && guild.id.find(state.guild_search) == std::string::npos)) {
                        return;
                    }
                    if (guild.should_link_texture) {
                        auto textureID = Api::LoadTextureFromMemory(reinterpret_cast<const unsigned char *>(guild.icon_data.c_str()), guild.icon_data.size());
                        guild.iconTextureID = textureID;
                        window.cache.icon_cache[guild.id] = textureID;
                        guild.should_link_texture = false;
                    }
                    ImGui::SetNextWindowBgAlpha(0);
                    ImGui::BeginChild(guild.id.c_str(), {}, ImGuiChildFlags_AutoResizeY);
                    if (ImGui::Selectable(("##selectable" + guild.id).c_str(), false, ImGuiSelectableFlags_AllowOverlap, {0, ImGui::GetWindowHeight()})) {
                        state.selected_guild = std::make_unique<Guild>();
                        state.selected_guild->id = guild.id;
                        state.selected_guild->name = guild.name;
                        state.selected_guild->icon = guild.icon;
                        state.selected_guild->icon_data = guild.icon_data;
                        state.selected_guild->iconTextureID = guild.iconTextureID;
                        state.selected_guild->configured = guild.configured;
                        state.selected_guild->should_link_texture = guild.should_link_texture;
                        state.channels = LinkedList<Channel>();
                        std::thread([&]() {
                            Response res = window.api.load_channels(state.selected_user->user_id, state.selected_guild->id, state.channels);
                            if (!is_ok_status(res.status)) {
                                state.messages.push_front(res);
                                std::thread([&]() {
                                    std::this_thread::sleep_for(std::chrono::seconds(5));
                                    state.messages.pop_back();
                                }).detach();
                            }
                        }).detach();
                    }
                    ImGui::SetCursorPos({10, 10});
                    Window::CircleImage(reinterpret_cast<ImTextureID>(guild.iconTextureID), 60);
                    ImGui::SameLine();
                    ImGui::Spacing();
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::TextWrapped(guild.name.c_str());
                    ImGui::Spacing();
                    ImGui::Text(guild.id.c_str());
                    ImGui::EndGroup();
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::EndChild();
                    if (guild.configured) {
                        auto p0 = ImGui::GetItemRectMin();
                        auto p1 = ImGui::GetItemRectMax();
                        p0.x -= 2;
                        p0.y -= 2;
                        p1.x += 2;
                        p1.y += 2;
                        ImGui::GetWindowDrawList()->AddRect(p0, p1, ImColor(59, 130, 246), 0, 0, 2.0f);
                    }
                    ImGui::ItemSize({0,5});
                });
            }
            ImGui::PopStyleColor(style_color_count);
            ImGui::EndChild();
        }
        else {
            const ImVec2 &regionAvail = ImGui::GetContentRegionAvail();
            ImGui::Text("Users");
            ImGui::SameLine();
            ImGui::SetCursorPos({regionAvail.x - 20, 10});
            ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
            auto buttonHoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
            buttonHoverColor.w = 0.3;
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);
            if (ImGui::ImageButton("##theme", reinterpret_cast<ImTextureID>(window.get_theme_mode_icon()), {20, 20})) {
                window.toggle_theme();
            }
            ImGui::PopStyleColor(2);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::BeginChild("##users", {0, regionAvail.y - 120});
            {
                auto &users = window.users;
                if (!users.empty()) {
                    int i = 0;
                    std::vector<std::string> users_to_delete;
                    for (auto &[user_id, user]: users) {
                        ImGui::BeginChild(i + 1, {}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
                        {
                            Window::CircleImage(reinterpret_cast<ImTextureID>(user.textureID), 80);
                            ImGui::SameLine();
                            ImGui::Spacing();
                            ImGui::SameLine();

                            ImGui::BeginGroup();
                            {
                                ImGui::SetCursorPosY(40);
                                ImGui::Text(user.name.c_str());
                                ImGui::Spacing();
                                ImGui::Text(user.user_id.c_str());
                            }
                            ImGui::EndGroup();

                            /*Spacing*/ {
                                ImGui::SameLine();
                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Spacing();
                                ImGui::SameLine();
                            }

                            ImGui::BeginGroup();
                            {
                                ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
                                buttonHoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
                                buttonHoverColor.w = 0.3;
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {5, 5});
                                if (user.bot_running) {
                                    if (ImGui::ImageButton("##stop", reinterpret_cast<ImTextureID>(Window::StopIcon), {20, 20})) {
                                        auto res = window.api.stop_bot(user_id);
                                        state.messages.push_front(res);
                                        std::thread([&]() {
                                            std::this_thread::sleep_for(std::chrono::seconds(5));
                                            state.messages.pop_back();
                                        }).detach();
                                        if (is_ok_status(res.status)) {
                                            user.bot_running = false;
                                        }
                                    }
                                } else {
                                    if (ImGui::ImageButton("##start", reinterpret_cast<ImTextureID>(Window::PlayIcon), {20, 20})) {
                                        auto res = window.api.start_bot(user_id);
                                        state.messages.push_front(res);
                                        std::thread([&]() {
                                            std::this_thread::sleep_for(std::chrono::seconds(5));
                                            state.messages.pop_back();
                                        }).detach();
                                        if (is_ok_status(res.status)) {
                                            user.bot_running = true;
                                        }
                                    }
                                }
                                ImGui::SameLine();
                                if (ImGui::Button(" / ")) {
                                    state.show_tags = true;
                                    state.tags = LinkedList<Tag>();
                                    state.selected_user = std::make_unique<User>(user.user_id, user.name, user.bot_running, user.textureID);
                                    std::thread([&](){
                                        Response res = window.api.load_tags(state.selected_user->user_id, state.tags);
                                        if (!is_ok_status(res.status)) {
                                            state.messages.push_front(res);
                                            std::thread([&]() {
                                                std::this_thread::sleep_for(std::chrono::seconds(5));
                                                state.messages.pop_back();
                                            }).detach();
                                        }
                                    }).detach();
                                }
                                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8, 4 });
                                ImGui::SetItemTooltip("Tags");
                                ImGui::PopStyleVar();
                                ImGui::SameLine();
                                if (ImGui::ImageButton("##edit", reinterpret_cast<ImTextureID>(window.get_edit_icon()), {20, 20})) {
                                    state.guilds = LinkedList<Guild>();
                                    state.selected_user = std::make_unique<User>(user.user_id, user.name, user.bot_running, user.textureID);
                                    std::thread([&]() {
                                        Response res = window.api.load_guilds(state.selected_user->user_id, state.guilds);
                                        if (!is_ok_status(res.status)) {
                                            state.messages.push_front(res);
                                            std::thread([&]() {
                                                std::this_thread::sleep_for(std::chrono::seconds(5));
                                                state.messages.pop_back();
                                            }).detach();
                                        }
                                        std::stringstream error_stream;
                                        state.guilds.for_each([&](Guild &guild) {
                                            if (window.cache.icon_cache.find(guild.id) == window.cache.icon_cache.end()) {
                                                auto textureIDResponse = window.api.DownloadGuildIcon(guild.id, guild.icon);
                                                if (!is_ok_status(textureIDResponse.status)) {
                                                    error_stream << textureIDResponse.data << '\n';
                                                } else {
                                                    guild.icon_data = textureIDResponse.data;
                                                    guild.should_link_texture = true;
                                                }
                                            } else {
                                                guild.iconTextureID = window.cache.icon_cache[guild.id];
                                            }
                                        });

                                        const auto &err_str = error_stream.str();
                                        if (!err_str.empty()) {
                                            state.messages.push_front({err_str, 400});
                                            std::thread([&]() {
                                                std::this_thread::sleep_for(std::chrono::seconds(5));
                                                state.messages.pop_back();
                                            }).detach();
                                        }
                                    }).detach();
                                }
                                ImGui::SameLine();
                                if (ImGui::ImageButton("##delete", reinterpret_cast<ImTextureID>(Window::DeleteIcon), {20, 20})) {
                                    ImGui::OpenPopup("Delete?");
                                }
                                ImGui::PopStyleVar(1);
                                ImGui::PopStyleColor(2);

                                ImGui::Spacing();
                                ImGui::Spacing();
                                ImGui::Spacing();
                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Spacing();
                                ImGui::Spacing();
                                ImGui::Spacing();
                                ImGui::Spacing();
                                ImGui::SameLine();
                                ImGui::Image(reinterpret_cast<ImTextureID>(user.bot_running ? Window::OnlineStatusIcon : Window::OfflineStatusIcon), {20, 20});
                                ImGui::SameLine();
                                ImGui::Text(user.bot_running ? "online" : "offline");

                                ImVec2 center = vp->GetCenter();
                                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                                if (ImGui::BeginPopupModal("Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                                    ImGui::Text("Do you want to delete this bot?");
                                    ImGui::Spacing();
                                    ImGui::Spacing();
                                    ImGui::Spacing();

                                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                                    ImGui::PushStyleColor(ImGuiCol_Border, ImColor{34, 197, 94}.Value);

                                    int style_color_var_count = 1;
                                    switch (state.theme) {
                                        case Window::Theme::DARK: {
                                            ImGui::PushStyleColor(ImGuiCol_Button, ImColor{5, 46, 22}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor{22, 101, 52}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor{5, 46, 22}.Value);
                                            style_color_var_count += 3;
                                        }
                                            break;
                                        case Window::Theme::LIGHT: {
                                            ImGui::PushStyleColor(ImGuiCol_Button, ImColor{22, 101, 52}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor{5, 46, 22}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor{22, 101, 52}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImColor{255, 255, 255}.Value);
                                            style_color_var_count += 4;
                                        }
                                    }

                                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                                        auto response = window.api.delete_user(user.user_id);
                                        state.messages.push_front(response);
                                        std::thread([&]() {
                                            std::this_thread::sleep_for(std::chrono::seconds(5));
                                            state.messages.pop_back();
                                        }).detach();
                                        if (is_ok_status(response.status) || response.status == StatusInternalServerError) {
                                            users_to_delete.push_back(user_id);
                                        }
                                        ImGui::CloseCurrentPopup();
                                    }
                                    ImGui::PopStyleColor(style_color_var_count);
                                    ImGui::SetItemDefaultFocus();
                                    ImGui::SameLine();

                                    ImGui::PushStyleColor(ImGuiCol_Border, ImColor{239, 68, 68}.Value);
                                    style_color_var_count = 1;
                                    switch (state.theme) {
                                        case Window::Theme::DARK: {
                                            ImGui::PushStyleColor(ImGuiCol_Button, ImColor{69, 10, 10}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor{153, 27, 27}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor{69, 10, 10}.Value);
                                            style_color_var_count += 3;
                                        }
                                            break;
                                        case Window::Theme::LIGHT: {
                                            ImGui::PushStyleColor(ImGuiCol_Button, ImColor{153, 27, 27}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor{69, 10, 10}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor{153, 27, 27}.Value);
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImColor{255, 255, 255}.Value);
                                            style_color_var_count += 4;
                                        }
                                    }

                                    if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                                    ImGui::PopStyleColor(style_color_var_count);
                                    ImGui::PopStyleVar(1);
                                    ImGui::EndPopup();
                                }
                            }
                            ImGui::EndGroup();
                        }
                        ImGui::EndChild();
                        ImGui::Spacing();
                        ImGui::Spacing();
                        i++;
                    }
                    for (const auto &user_id: users_to_delete) {
                        users.erase(user_id);
                    }
                }
            }
            ImGui::EndChild();
            ImGui::SetNextWindowPos({20, vp->WorkPos.y + vp->WorkSize.y - 20}, ImGuiCond_Always, {0, 1.0f});
            if (ImGui::BeginChild("##token", {0, 0}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) {
                ImGui::PushItemWidth(-80);
                ImGui::InputTextWithHint("##token", "Enter bot token", &state.token, ImGuiInputTextFlags_CharsNoBlank);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::BeginDisabled(state.token.empty());
                if (ImGui::Button("add", {-FLT_MIN, 0})) {
                    Response response = window.api.add_user(state.token, window.users);
                    state.messages.push_front(response);
                    std::thread([&]() {
                        std::this_thread::sleep_for(std::chrono::seconds(5));
                        state.messages.pop_back();
                    }).detach();
                    state.token = "";
                }
                ImGui::EndDisabled();
            }
            ImGui::EndChild();
        }
        if (!state.messages.is_empty()) {
            ImGui::SetNextWindowPos({20, 10}, ImGuiCond_Always);
            ImGui::BeginChild("##error", {}, ImGuiChildFlags_AutoResizeY);
            int i = 0;
            state.messages.for_each([&](Response const &message) {
                bool is_error = !is_ok_status(message.status);
                ImGui::SetNextWindowBgAlpha(1);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {30.0f, 25.0f});
                ImGui::PushStyleColor(ImGuiCol_FrameBg, is_error ? ImColor{69, 10, 10}.Value : ImColor{5, 46, 22}.Value);
                ImGui::PushStyleColor(ImGuiCol_Border, is_error ? ImColor{239, 68, 68}.Value : ImColor{34, 197, 94}.Value);
                ImGui::BeginChild((std::string("##error").append(std::to_string(i + 1))).c_str(), {}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_FrameStyle);
                ImGui::TextWrapped(message.data.c_str());
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);
                ImGui::Spacing();
                ImGui::Spacing();
                i++;
            });
            ImGui::EndChild();
        }
        ImGui::End();
    });
    return 0;
}
