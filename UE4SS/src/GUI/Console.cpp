#include <cctype>
#include <memory>

#include <DynamicOutput/DynamicOutput.hpp>
#include <GUI/Console.hpp>
#include <GUI/GUI.hpp>
#include <GUI/ImGuiUtility.hpp>
#include <UE4SSProgram.hpp>

#include <imgui_internal.h>

namespace RC::GUI
{
    // Wrapper for CalcTextSizeA.
    auto CalcTextSize(const char* text, float max_width, const char** remaining = nullptr) -> ImVec2
    {
        ImGuiContext& g = *GImGui;
        ImFont* font = g.Font;
        const float font_size = g.FontSize;
        ImVec2 text_size = font->CalcTextSizeA(font_size, max_width, -1.0f, text, nullptr, remaining);
        text_size.x = IM_FLOOR(text_size.x + 0.99999f);
        return text_size;
    }

    auto Console::GetLanguageDefinitionNone() -> const TextEditor::LanguageDefinition&
    {
        static bool inited = false;
        static TextEditor::LanguageDefinition langDef;
        if (!inited)
        {
            langDef.mName = "None";
            inited = true;
        }
        return langDef;
    }

    auto Console::GetPalette() const -> const TextEditor::Palette&
    {
        const static TextEditor::Palette p = {{
                ImGui::ColorConvertFloat4ToU32(ImVec4{0.800f, 0.800f, 0.800f, 1.0f}), // Default
                0xffd69c56,                                                           // Keyword
                0xff00ff00,                                                           // Number
                0xff7070e0,                                                           // String
                0xff70a0e0,                                                           // Char literal
                0xffffffff,                                                           // Punctuation
                0xff408080,                                                           // Preprocessor
                0xffaaaaaa,                                                           // Identifier
                0xff9bc64d,                                                           // Known identifier
                0xffc040a0,                                                           // Preproc identifier
                0xff206020,                                                           // Comment (single line)
                0xff406020,                                                           // Comment (multi line)
                ImGui::ColorConvertFloat4ToU32(ImVec4{0.156f, 0.156f, 0.156f, 1.0f}), // Background
                0xffe0e0e0,                                                           // Cursor
                ImGui::ColorConvertFloat4ToU32(ImVec4{0.65f, 0.24f, 0.57f, 0.38f}),   // Selection
                0x800020ff,                                                           // ErrorMarker
                0x40f08000,                                                           // Breakpoint
                0xff707000,                                                           // Line number
                0x40000000,                                                           // Current line fill
                0x40808080,                                                           // Current line fill (inactive)
                0x40a0a0a0,                                                           // Current line edge
        }};
        return p;
    }

    auto Console::render() -> void
    {
        std::lock_guard<std::mutex> guard(m_lines_mutex);
        m_text_editor.Render("TextEditor", {-16.0f, -31.0f + -8.0f});
        ImGui_AutoScroll("TextEditor", &m_previous_max_scroll_y);
    }

    auto Console::render_search_box() -> void
    {
        m_filter.Draw("Search log", 200);
    }

    static auto LogLevel_to_ImColor(Color::Color color) -> std::pair<ImColor, ImColor>
    {
        switch (color)
        {
        case Color::Default:
            return {g_imgui_text_editor_default_bg_color, g_imgui_text_editor_default_text_color};
        case Color::NoColor:
            return {g_imgui_text_editor_normal_bg_color, g_imgui_text_editor_normal_text_color};
        case Color::Cyan:
            return {g_imgui_text_editor_verbose_bg_color, g_imgui_text_editor_verbose_text_color};
        case Color::Yellow:
            return {g_imgui_text_editor_warning_bg_color, g_imgui_text_editor_warning_text_color};
        case Color::Red:
            return {g_imgui_text_editor_error_bg_color, g_imgui_text_editor_error_text_color};
        case Color::Green:
            return {g_imgui_text_editor_default_bg_color, g_imgui_text_green_color};
        case Color::Blue:
            return {g_imgui_text_editor_default_bg_color, g_imgui_text_blue_color};
        case Color::Purple:
            return {g_imgui_text_editor_default_bg_color, g_imgui_text_purple_color};
        }

        throw std::runtime_error{"[LogLevel_to_ImColor] Unhandled log_level"};
    }

    // 1) Overload taking std::string
    auto Console::add_line(const std::string& line, Color::Color color) -> void
    {
        std::lock_guard<std::mutex> guard(m_lines_mutex);

        // FILTER: only allow our mod’s “Current Profile:” lines
        if (line.find("Current Profile:") == std::string::npos)
            return;

        if (m_text_editor.GetTotalLines() < 0)
            throw std::runtime_error{"Negative line count in console"};

        if (static_cast<size_t>(m_text_editor.GetTotalLines()) >= m_maximum_num_lines)
            m_text_editor.ClearLines();

        if (m_lines.size() >= m_maximum_num_lines)
            m_lines.clear();

        if (color != Color::Default && color != Color::NoColor)
            m_text_editor.GetLineColorMarkers()
                .emplace(m_text_editor.GetTotalLines() + 1, LogLevel_to_ImColor(color));

        m_lines.emplace_back(line);
        m_text_editor.AddTextLine(line);
    }

    // 2) Overload taking StringType
    auto Console::add_line(const StringType& line, Color::Color color) -> void
    {
        auto utf8_string = to_string(line);
        std::lock_guard<std::mutex> guard(m_lines_mutex);

        // FILTER: only allow our mod’s “Current Profile:” lines
        if (utf8_string.find("Current Profile:") == std::string::npos)
            return;

        if (m_text_editor.GetTotalLines() < 0)
            throw std::runtime_error{"Negative line count in console"};

        if (static_cast<size_t>(m_text_editor.GetTotalLines()) >= m_maximum_num_lines)
            m_text_editor.ClearLines();

        if (m_lines.size() >= m_maximum_num_lines)
            m_lines.clear();

        if (color != Color::Default && color != Color::NoColor)
            m_text_editor.GetLineColorMarkers()
                .emplace(m_text_editor.GetTotalLines() + 1, LogLevel_to_ImColor(color));

        m_lines.emplace_back(utf8_string);
        m_text_editor.AddTextLine(utf8_string);
    }

} // namespace RC::GUI
