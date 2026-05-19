#include "flpch.h"
#include "ContentBrowser.h"

#include <imgui.h>

namespace Flux {

    static const ImVec4 s_AccentColor = ImVec4(0.910f, 0.627f, 0.125f, 1.00f);
    static const ImVec4 s_AccentColorD = ImVec4(0.784f, 0.502f, 0.063f, 1.00f);

    ContentBrowser::ContentBrowser()
    {
        SetRootPath(std::filesystem::current_path() / "assets");
    }

    void ContentBrowser::SetRootPath(const std::filesystem::path& root)
    {
        m_RootPath = root;
        m_CurrentPath = root;
        RefreshDirectory();
    }

    void ContentBrowser::OnImGuiRender()
    {
        ImGui::Begin("##ContentBrowserPanel", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

        ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.11f, 0.11f, 0.11f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.14f, 0.14f, 0.14f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabSelectedOverline, s_AccentColor);

        if (ImGui::BeginTabBar("##CBTabs"))
        {
            ImGui::PushStyleColor(ImGuiCol_Text,
                m_ActiveTab == 0 ? s_AccentColor : ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            if (ImGui::BeginTabItem("Content Browser"))
            {
                ImGui::PopStyleColor();
                m_ActiveTab = 0;

                DrawToolbar();
                ImGui::Separator();

                float sidebarWidth = 140.0f;

                ImGui::BeginChild("##CBSidebar", ImVec2(sidebarWidth, 0), true);
                DrawSidebar();
                ImGui::EndChild();

                ImGui::SameLine();

                ImGui::BeginChild("##CBGrid", ImVec2(0, 0), false);
                DrawBreadcrumb();
                DrawFileGrid();
                ImGui::EndChild();

                ImGui::EndTabItem();
            }
            else
            {
                ImGui::PopStyleColor();
            }

            ImGui::PushStyleColor(ImGuiCol_Text,
                m_ActiveTab == 1 ? s_AccentColor : ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            if (ImGui::BeginTabItem("Console"))
            {
                ImGui::PopStyleColor();
                m_ActiveTab = 1;
                ImGui::TextDisabled("No messages");
                ImGui::EndTabItem();
            }
            else
            {
                ImGui::PopStyleColor();
            }

            ImGui::EndTabBar();
        }

        ImGui::PopStyleColor(4);
        ImGui::End();

        if (!m_PendingNavigate.empty())
        {
            NavigateTo(m_PendingNavigate);
            m_PendingNavigate.clear();
        }
    }

    void ContentBrowser::DrawToolbar()
    {
        ImGui::SetNextItemWidth(200.0f);
        char searchBuf[128];
        std::strncpy(searchBuf, m_SearchBuffer.c_str(), sizeof(searchBuf));
        searchBuf[sizeof(searchBuf) - 1] = '\0';
        if (ImGui::InputTextWithHint("##CBSearch", "Search...", searchBuf, sizeof(searchBuf)))
            m_SearchBuffer = searchBuf;

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70.0f + ImGui::GetCursorPosX());

        ImGui::PushStyleColor(ImGuiCol_Button, s_AccentColorD);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_AccentColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.40f, 0.03f, 1.0f));
        if (ImGui::Button("  Import  "))
        {
            if (m_ImportCallback)
                m_ImportCallback("");
        }
        ImGui::PopStyleColor(3);
    }

    void ContentBrowser::DrawSidebar()
    {
        if (!std::filesystem::exists(m_RootPath)) return;

        ImGui::PushStyleColor(ImGuiCol_Text, s_AccentColor);
        ImGui::Text("ASSETS");
        ImGui::PopStyleColor();
        ImGui::Separator();

        for (auto& entry : std::filesystem::directory_iterator(m_RootPath))
        {
            if (!entry.is_directory()) continue;

            std::string folderName = entry.path().filename().string();
            bool selected = (m_CurrentPath == entry.path());

            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(s_AccentColor.x, s_AccentColor.y, s_AccentColor.z, 0.25f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(s_AccentColor.x, s_AccentColor.y, s_AccentColor.z, 0.15f));
            if (ImGui::Selectable(("  " + folderName).c_str(), selected))
                m_PendingNavigate = entry.path(); 
            ImGui::PopStyleColor(2);
        }
    }

    void ContentBrowser::DrawBreadcrumb()
    {
        std::filesystem::path rel = std::filesystem::relative(m_CurrentPath, m_RootPath.parent_path());
        std::string pathStr = rel.string();
        for (char& c : pathStr) if (c == '\\') c = '/';

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Text("%s", pathStr.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    void ContentBrowser::DrawFileGrid()
    {
        float cellSize = m_ThumbnailSize + m_Padding;
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int   columns = std::max(1, (int)(panelWidth / cellSize));

        ImGui::Columns(columns, nullptr, false);

        for (auto& entry : m_Entries)
        {
            if (!m_SearchBuffer.empty())
            {
                std::string nameLower = entry.Name;
                std::string filterLower = m_SearchBuffer;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
                if (nameLower.find(filterLower) == std::string::npos)
                {
                    ImGui::NextColumn();
                    continue;
                }
            }

            ImGui::PushID(entry.Name.c_str());

            const char* icon = GetFileIcon(entry.Type);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(s_AccentColor.x, s_AccentColor.y, s_AccentColor.z, 0.3f));

            if (entry.Type == FileType::Folder)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.75f, 0.20f, 1.0f));
            else if (entry.Type == FileType::Model)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.75f, 0.95f, 1.0f));
            else if (entry.Type == FileType::Texture)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.90f, 0.40f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.70f, 1.0f));

            ImGui::Button(icon, ImVec2(m_ThumbnailSize, m_ThumbnailSize));
            ImGui::PopStyleColor(4);

            if (!entry.IsDirectory && entry.Type == FileType::Model)
            {
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    std::string pathStr = entry.Path.string();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM",
                        pathStr.c_str(), pathStr.size() + 1);
                    ImGui::Text("  %s", entry.Name.c_str());
                    ImGui::EndDragDropSource();
                }
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (entry.IsDirectory)
                    m_PendingNavigate = entry.Path;
                else if (m_ImportCallback)
                    m_ImportCallback(entry.Path.string());
            }

            ImGui::TextWrapped("%s", entry.Name.c_str());

            ImGui::PopID();
            ImGui::NextColumn();
        }

        ImGui::Columns(1);

        if (m_CurrentPath != m_RootPath)
        {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            if (ImGui::SmallButton("< Back"))
                m_PendingNavigate = m_CurrentPath.parent_path();
            ImGui::PopStyleColor();
        }
    }

    void ContentBrowser::NavigateTo(const std::filesystem::path& path)
    {
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
        {
            m_CurrentPath = path;
            RefreshDirectory();
        }
    }

    void ContentBrowser::RefreshDirectory()
    {
        m_Entries.clear();
        if (!std::filesystem::exists(m_CurrentPath)) return;

        std::vector<FileEntry> folders, files;

        for (auto& entry : std::filesystem::directory_iterator(m_CurrentPath))
        {
            FileEntry fe;
            fe.Path = entry.path();
            fe.Name = entry.path().filename().string();
            fe.IsDirectory = entry.is_directory();
            fe.Type = fe.IsDirectory ? FileType::Folder : GetFileType(entry.path());

            if (fe.IsDirectory) folders.push_back(fe);
            else                files.push_back(fe);
        }

        for (auto& f : folders) m_Entries.push_back(f);
        for (auto& f : files)   m_Entries.push_back(f);
    }

    ContentBrowser::FileType ContentBrowser::GetFileType(const std::filesystem::path& path) const
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".obj" || ext == ".gltf" || ext == ".glb" || ext == ".fbx")
            return FileType::Model;
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".dds")
            return FileType::Texture;
        if (ext == ".flux" || ext == ".scene" || ext == ".flscene")
            return FileType::Scene;

        return FileType::Unknown;
    }

    const char* ContentBrowser::GetFileIcon(FileType type) const
    {
        switch (type)
        {
        case FileType::Folder:  return "[DIR]";
        case FileType::Model:   return "[OBJ]";
        case FileType::Texture: return "[IMG]";
        case FileType::Scene:   return "[SCN]";
        default:                return "[...]";
        }
    }

} // namespace Flux