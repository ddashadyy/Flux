#pragma once

#include "Flux/Core/Base.h"
#include <functional>
#include <filesystem>
#include <string>
#include <vector>

namespace Flux {

    class ContentBrowser
    {
    public:
        using ImportCallback = std::function<void(const std::string& path)>;

        ContentBrowser();

        void SetRootPath(const std::filesystem::path& root);
        void SetImportCallback(ImportCallback cb) { m_ImportCallback = std::move(cb); }

        void OnImGuiRender();

    private:
        void DrawToolbar();
        void DrawSidebar();
        void DrawFileGrid();
        void DrawBreadcrumb();

        void NavigateTo(const std::filesystem::path& path);
        void RefreshDirectory();

        enum class FileType { Folder, Model, Texture, Scene, Unknown };
        FileType    GetFileType(const std::filesystem::path& path) const;
        const char* GetFileIcon(FileType type) const;

    private:
        std::filesystem::path m_RootPath;
        std::filesystem::path m_CurrentPath;
        std::filesystem::path m_PendingNavigate; 

        struct FileEntry {
            std::filesystem::path Path;
            std::string           Name;
            FileType              Type;
            bool                  IsDirectory;
        };

        std::vector<FileEntry> m_Entries;
        std::string            m_SearchBuffer;

        ImportCallback m_ImportCallback;

        float m_ThumbnailSize = 80.0f;
        float m_Padding = 12.0f;
        int   m_ActiveTab = 0;
    };

} // namespace Flux