#pragma once

#include <imgui.h>

namespace Flux {

    class EditorTheme
    {
    public:
        static void Apply()
        {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            // ── Geometry ──────────────────────────────────────────────────────
            style.WindowPadding = ImVec2(8.0f, 8.0f);
            style.FramePadding = ImVec2(6.0f, 4.0f);
            style.CellPadding = ImVec2(6.0f, 3.0f);
            style.ItemSpacing = ImVec2(8.0f, 5.0f);
            style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
            style.IndentSpacing = 18.0f;
            style.ScrollbarSize = 10.0f;
            style.GrabMinSize = 10.0f;

            style.WindowBorderSize = 0.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupBorderSize = 1.0f;
            style.FrameBorderSize = 0.0f;
            style.TabBorderSize = 0.0f;

            style.WindowRounding = 4.0f;
            style.ChildRounding = 4.0f;
            style.FrameRounding = 3.0f;
            style.PopupRounding = 4.0f;
            style.ScrollbarRounding = 3.0f;
            style.GrabRounding = 3.0f;
            style.TabRounding = 4.0f;

            style.WindowMenuButtonPosition = ImGuiDir_None;

            // ── Palette ───────────────────────────────────────────────────────
            //  bg0  darkest background      #141414
            //  bg1  panel background        #1c1c1c
            //  bg2  widget / hover          #252525
            //  bg3  active / border         #2e2e2e
            //  acc  yellow accent           #e8a020
            //  acc2 accent hover            #f0b840
            //  acc3 accent active           #c88010
            //  txt  primary text            #e8e8e8
            //  txt2 secondary / disabled    #707070

            const ImVec4 bg0 = ImVec4(0.078f, 0.078f, 0.078f, 1.00f); // #141414
            const ImVec4 bg1 = ImVec4(0.110f, 0.110f, 0.110f, 1.00f); // #1c1c1c
            const ImVec4 bg2 = ImVec4(0.145f, 0.145f, 0.145f, 1.00f); // #252525
            const ImVec4 bg3 = ImVec4(0.180f, 0.180f, 0.180f, 1.00f); // #2e2e2e
            const ImVec4 acc = ImVec4(0.910f, 0.627f, 0.125f, 1.00f); // #e8a020
            const ImVec4 acc2 = ImVec4(0.941f, 0.722f, 0.251f, 1.00f); // #f0b840
            const ImVec4 acc3 = ImVec4(0.784f, 0.502f, 0.063f, 1.00f); // #c88010
            const ImVec4 txt = ImVec4(0.910f, 0.910f, 0.910f, 1.00f); // #e8e8e8
            const ImVec4 txt2 = ImVec4(0.439f, 0.439f, 0.439f, 1.00f); // #707070
            const ImVec4 none = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);

            // ── Colors ────────────────────────────────────────────────────────
            colors[ImGuiCol_Text] = txt;
            colors[ImGuiCol_TextDisabled] = txt2;

            colors[ImGuiCol_WindowBg] = bg1;
            colors[ImGuiCol_ChildBg] = bg1;
            colors[ImGuiCol_PopupBg] = bg0;

            colors[ImGuiCol_Border] = bg3;
            colors[ImGuiCol_BorderShadow] = none;

            colors[ImGuiCol_FrameBg] = bg2;
            colors[ImGuiCol_FrameBgHovered] = bg3;
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.220f, 0.220f, 0.220f, 1.00f);

            colors[ImGuiCol_TitleBg] = bg0;
            colors[ImGuiCol_TitleBgActive] = bg0;
            colors[ImGuiCol_TitleBgCollapsed] = bg0;

            colors[ImGuiCol_MenuBarBg] = bg0;

            colors[ImGuiCol_ScrollbarBg] = bg1;
            colors[ImGuiCol_ScrollbarGrab] = bg3;
            colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.230f, 0.230f, 0.230f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive] = acc3;

            colors[ImGuiCol_CheckMark] = acc;
            colors[ImGuiCol_SliderGrab] = acc;
            colors[ImGuiCol_SliderGrabActive] = acc2;

            colors[ImGuiCol_Button] = bg3;
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.230f, 0.230f, 0.230f, 1.00f);
            colors[ImGuiCol_ButtonActive] = acc3;

            colors[ImGuiCol_Header] = ImVec4(acc.x, acc.y, acc.z, 0.20f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(acc.x, acc.y, acc.z, 0.35f);
            colors[ImGuiCol_HeaderActive] = ImVec4(acc.x, acc.y, acc.z, 0.50f);

            colors[ImGuiCol_Separator] = bg3;
            colors[ImGuiCol_SeparatorHovered] = acc3;
            colors[ImGuiCol_SeparatorActive] = acc;

            colors[ImGuiCol_ResizeGrip] = ImVec4(acc.x, acc.y, acc.z, 0.20f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(acc.x, acc.y, acc.z, 0.60f);
            colors[ImGuiCol_ResizeGripActive] = acc;

            colors[ImGuiCol_Tab] = bg2;
            colors[ImGuiCol_TabHovered] = bg3;
            colors[ImGuiCol_TabActive] = ImVec4(0.200f, 0.200f, 0.200f, 1.00f);
            colors[ImGuiCol_TabUnfocused] = bg1;
            colors[ImGuiCol_TabUnfocusedActive] = bg2;

            colors[ImGuiCol_DockingPreview] = ImVec4(acc.x, acc.y, acc.z, 0.50f);
            colors[ImGuiCol_DockingEmptyBg] = bg0;

            colors[ImGuiCol_PlotLines] = acc;
            colors[ImGuiCol_PlotLinesHovered] = acc2;
            colors[ImGuiCol_PlotHistogram] = acc;
            colors[ImGuiCol_PlotHistogramHovered] = acc2;

            colors[ImGuiCol_TableHeaderBg] = bg2;
            colors[ImGuiCol_TableBorderStrong] = bg3;
            colors[ImGuiCol_TableBorderLight] = bg2;
            colors[ImGuiCol_TableRowBg] = none;
            colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

            colors[ImGuiCol_TextSelectedBg] = ImVec4(acc.x, acc.y, acc.z, 0.30f);
            colors[ImGuiCol_DragDropTarget] = acc;
            colors[ImGuiCol_NavHighlight] = acc;
            colors[ImGuiCol_NavWindowingHighlight] = acc;
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
        }

        static void PushSectionHeader()
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.910f, 0.627f, 0.125f, 1.00f));
        }

        static void PopSectionHeader()
        {
            ImGui::PopStyleColor();
        }
    };

} // namespace Flux