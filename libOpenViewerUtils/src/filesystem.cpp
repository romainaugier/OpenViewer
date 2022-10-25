// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/filesystem.h"

#include <filesystem>

LOVU_NAMESPACE_BEGIN

FS_NAMESPACE_BEGIN

LOVU_FORCEINLINE size_t file_count_in_directory(const std::string& directory_path) noexcept
{
    return static_cast<size_t>(std::distance(std::filesystem::directory_iterator(directory_path), std::filesystem::directory_iterator{}));
}

LOVU_FORCEINLINE size_t file_count_in_directory(const std::string_view& directory_path) noexcept
{
    return static_cast<size_t>(std::distance(std::filesystem::directory_iterator(directory_path), std::filesystem::directory_iterator{}));
}

LOVU_DLL void get_file_sequence_from_file(file_sequence& file_seq, const std::string& file_path) noexcept
{
    const std::filesystem::path parent_dir = std::filesystem::path(file_path).parent_path();
    const std::string base_file_name = std::filesystem::path(file_path).filename().string();
    const std::string base_file_stem = std::filesystem::path(file_path).stem().string();
    const std::string base_file_extension = std::filesystem::path(file_path).extension().string();
    
    file_sequence tmp_file_seq;
    tmp_file_seq.reserve(file_count_in_directory(parent_dir.string()));

    const std::regex num_pattern("\\d+");

    auto base_num_begin = std::sregex_iterator(base_file_stem.begin(), base_file_stem.end(), num_pattern);
    auto base_num_end = std::sregex_iterator();

    uint32_t base_file_frame_num = UINT32_MAX;
    
    for (const auto& file : std::filesystem::directory_iterator(parent_dir))
    {
        const std::string it_file_name = std::filesystem::path(file).filename().string();

        if (it_file_name == base_file_name) continue;

        const std::string it_file_stem = std::filesystem::path(file).stem().string();
        const std::string it_file_extension = std::filesystem::path(file).extension().string();

        auto it_num_begin = std::sregex_iterator(it_file_stem.begin(), it_file_stem.end(), num_pattern);
        auto it_num_end = std::sregex_iterator();

        bool found_another_file = false;

        for (std::sregex_iterator it = it_num_begin; it != it_num_end; ++it)
        {
            const std::smatch it_match = *it;
            const std::string it_match_str = it_match.str();
            const std::regex it_match_pattern_regex(it_match_str);

            std::string it_match_sub = it_file_stem;
            it_match_sub.replace(it_match.position(), it_match.length(), "#");

            for (std::sregex_iterator base = base_num_begin; base != base_num_end; ++base)
            {
                const std::smatch base_match = *base;
                const std::string base_match_str = base_match.str();
                const std::regex base_match_pattern_regex(base_match_str);


                std::string base_match_sub = base_file_stem;
                base_match_sub.replace(base_match.position(), base_match.length(), "#");

                if (base_match_sub == it_match_sub)
                {
                    const uint32_t it_frame_num = std::stoi(it_match_str);

                    tmp_file_seq.emplace_back(std::make_pair(str::clean_os_path(file.path().string()), it_frame_num));

                    found_another_file = true;

                    if (base_file_frame_num == UINT32_MAX) 
                    {
                        base_file_frame_num = std::stoi(base_match_str);
                        tmp_file_seq.emplace_back(std::make_pair(str::clean_os_path(file_path), base_file_frame_num));
                    }

                    break;
                }
            }

            if (found_another_file) break;
        }
    }

    tmp_file_seq.shrink_to_fit();

    std::sort(tmp_file_seq.begin(), tmp_file_seq.end(), [&](const file_sequence_item& a, const file_sequence_item& b){ return a.second < b.second; });

    file_seq = std::move(tmp_file_seq);
}

LOVU_DLL bool is_image(const std::string& path) noexcept
{
    for (uint8_t i = 0; i < LOVUARRAYSIZE(image_extensions); i++)
    {
        if (str::ends_with(path, image_extensions[i])) return true;
    }

    return false;
}

LOVU_DLL bool is_image(const std::string_view& path) noexcept
{
    for (uint8_t i = 0; i < LOVUARRAYSIZE(image_extensions); i++)
    {
        if (str::ends_with(path, image_extensions[i])) return true;
    }

    return false;
}

LOVU_DLL bool is_video(const std::string& path) noexcept
{
    for (uint8_t i = 0; i < LOVUARRAYSIZE(video_extensions); i++)
    {
        if (str::ends_with(path, video_extensions[i])) return true;
    }

    return false;
}

LOVU_DLL bool is_video(const std::string_view& path) noexcept
{
    for (uint8_t i = 0; i < LOVUARRAYSIZE(video_extensions); i++)
    {
        if (str::ends_with(path, video_extensions[i])) return true;
    }

    return false;
}

LOVU_DLL std::string expand_from_executable_dir(const std::string& path_to_expand) noexcept
{
#ifdef LOVU_WIN
    wchar_t sz_path[MAX_PATH];
    GetModuleFileNameW(nullptr, sz_path, MAX_PATH);
#else if LOVU_LINUX
    char sz_path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", sz_path, PATH_MAX);
    if (count < 0 || count >= PATH_MAX) return "";
    sz_path[count] = "\0";
#endif
    std::string cwd = std::filesystem::path(sz_path).parent_path().string();
    str::clean_os_path(cwd);
    return cwd + path_to_expand;
}

LOVU_DLL std::string get_documents_folder() noexcept
{
#ifdef LOVU_WIN
    PWSTR ppsz_path;

    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &ppsz_path);

    std::wstring my_path;

    if (SUCCEEDED(hr)) 
    {
        my_path = ppsz_path;
        
        CoTaskMemFree(ppsz_path); 
        return std::string(my_path.begin(), my_path.end());
    }
#else if LOVU_LINUX
// https://stackoverflow.com/questions/2910377/get-home-directory-in-linux
    const char* homedir;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    std::string doc_dir = homedir;
    doc_dir += "/Documents";

    return doc_dir;
#endif
}

FS_NAMESPACE_END

LOVU_NAMESPACE_END