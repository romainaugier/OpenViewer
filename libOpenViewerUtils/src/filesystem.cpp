// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/filesystem.h"

#include <locale>
#include <codecvt>

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

LOVU_DLL std::string get_file_sequence_from_file(const std::string& file_path) noexcept
{
    const std::regex num_pattern("\\d+");

    const std::filesystem::path parent_directory = std::filesystem::path(file_path).parent_path();
    const std::string base_file_name = std::filesystem::path(file_path).filename().string();
    
    const std::string filename = std::filesystem::path(file_path).filename().string();


    const auto matches_begin = std::sregex_iterator(filename.begin(), filename.end(), num_pattern);
    const auto matches_end = std::sregex_iterator();

    std::vector<std::string> entries_name_transformed;

    bool found_match = false;
    int32_t match_transformed_entry_index = -1;

    for(std::sregex_iterator it = matches_begin; it != matches_end; ++it)
    {
        const std::smatch match = *it;
        const uint8_t padding = match.length();

        std::string dash = "";

        for(uint8_t i = 0; i < padding; i++) dash += "#";

        entries_name_transformed.emplace_back(fmt::format("{}{}{}",
                                                        filename.substr(0, match.position()),
                                                        dash,
                                                        filename.substr(match.position() + match.length())));
    }

    for(const auto& new_entry : std::filesystem::directory_iterator(parent_directory))
    {
        if(new_entry.is_directory()) continue;

        const std::string new_filename = new_entry.path().filename().string();

        if(new_filename == filename) continue;

        uint8_t j = 0;

        const auto new_matches_begin = std::sregex_iterator(new_filename.begin(), 
                                                            new_filename.end(), 
                                                            num_pattern);
        const auto new_matches_end = std::sregex_iterator();

        for(std::sregex_iterator it = new_matches_begin; it != new_matches_end; ++it)
        {
            const std::smatch new_match = *it;
            const uint8_t padding = new_match.length();

            std::string dash = "";

            for(uint8_t i = 0; i < padding; i++) dash += "#";

            const std::string new_entry_name_transformed = fmt::format("{}{}{}",
                                                                    new_filename.substr(0, new_match.position()),
                                                                    dash,
                                                                    new_filename.substr(new_match.position() + new_match.length()));

            if(std::find(entries_name_transformed.begin(), 
                        entries_name_transformed.end(), 
                        new_entry_name_transformed) != entries_name_transformed.end())
            {
                found_match = true;
                match_transformed_entry_index = j;
                break;
            }

            j++;
        }

        if(found_match)
        {
            break;
        }
    }

    if(found_match)
    {
        const std::string_view file_sequence_entry = entries_name_transformed[match_transformed_entry_index];

        int32_t fileseq_count = 0;
        int32_t fileseq_start = INT32_MAX;
        int32_t fileseq_end = INT32_MIN;

        for(const auto& new_entry : std::filesystem::directory_iterator(parent_directory))
        {
            if(new_entry.is_directory()) continue;

            const std::string new_filename = new_entry.path().filename().string();

            const auto new_matches_begin = std::sregex_iterator(new_filename.begin(), 
                                                                new_filename.end(), 
                                                                num_pattern);
            const auto new_matches_end = std::sregex_iterator();

            for(std::sregex_iterator it = new_matches_begin; it != new_matches_end; ++it)
            {
                const std::smatch new_match = *it;
                const int32_t match_int = std::stoi(new_match.str());
                const uint8_t padding = new_match.length();

                std::string dash = "";

                for(uint8_t i = 0; i < padding; i++) dash += "#";

                const std::string new_entry_name_transformed = fmt::format("{}{}{}",
                                                                        new_filename.substr(0, new_match.position()),
                                                                        dash,
                                                                        new_filename.substr(new_match.position() + new_match.length()));

                if(new_entry_name_transformed == file_sequence_entry)
                {
                    if(match_int > fileseq_end)
                    {
                        fileseq_end = match_int;
                    }

                    if(match_int < fileseq_start)
                    {
                        fileseq_start = match_int;
                    }

                    fileseq_count++;

                    break;
                }
            }
        }

        return fmt::format("seq?{}/{} [{}-{}]", parent_directory.string(),
                                                file_sequence_entry, 
                                                fileseq_start, 
                                                fileseq_end);
    }

    return "";
}

LOVU_DLL void get_filenames_from_dir(std::vector<std::string>& file_names, 
                                     const std::string& directory_path) noexcept
{
    const std::regex num_pattern("\\d+");

    const uint32_t num_files = file_count_in_directory(directory_path);

    std::vector<std::string> files;
    files.reserve(num_files);
    std::vector<std::string> processed_files;
    processed_files.reserve(num_files);

    if(!exists(directory_path)) return;

    for(const auto& entry : std::filesystem::directory_iterator(directory_path))
    {
        if(entry.is_directory()) 
        {
            continue;
        }

        const std::string filename = entry.path().filename().string();

        if(std::find(processed_files.begin(), 
                     processed_files.end(), 
                     filename) != processed_files.end())
        {
            continue;
        }

        const auto matches_begin = std::sregex_iterator(filename.begin(), filename.end(), num_pattern);
        const auto matches_end = std::sregex_iterator();

        std::vector<std::string> entries_name_transformed;

        bool found_match = false;
        int32_t match_transformed_entry_index = -1;

        for(std::sregex_iterator it = matches_begin; it != matches_end; ++it)
        {
            const std::smatch match = *it;
            const uint8_t padding = match.length();

            std::string dash = "";

            for(uint8_t i = 0; i < padding; i++) dash += "#";

            entries_name_transformed.emplace_back(fmt::format("{}{}{}",
                                                              filename.substr(0, match.position()),
                                                              dash,
                                                              filename.substr(match.position() + match.length())));
        }

        for(const auto& new_entry : std::filesystem::directory_iterator(directory_path))
        {
            if(new_entry.is_directory()) continue;

            const std::string new_filename = new_entry.path().filename().string();

            if(new_filename == filename) continue;

            uint8_t j = 0;

            const auto new_matches_begin = std::sregex_iterator(new_filename.begin(), 
                                                                new_filename.end(), 
                                                                num_pattern);
            const auto new_matches_end = std::sregex_iterator();

            for(std::sregex_iterator it = new_matches_begin; it != new_matches_end; ++it)
            {
                const std::smatch new_match = *it;
                const uint8_t padding = new_match.length();

                std::string dash = "";

                for(uint8_t i = 0; i < padding; i++) dash += "#";

                const std::string new_entry_name_transformed = fmt::format("{}{}{}",
                                                                           new_filename.substr(0, new_match.position()),
                                                                           dash,
                                                                           new_filename.substr(new_match.position() + new_match.length()));

                if(std::find(entries_name_transformed.begin(), 
                             entries_name_transformed.end(), 
                             new_entry_name_transformed) != entries_name_transformed.end())
                {
                    found_match = true;
                    match_transformed_entry_index = j;
                    break;
                }

                j++;
            }

            if(found_match)
            {
                break;
            }
        }

        if(found_match)
        {
            const std::string_view file_sequence_entry = entries_name_transformed[match_transformed_entry_index];

            int32_t fileseq_count = 0;
            int32_t fileseq_start = INT32_MAX;
            int32_t fileseq_end = INT32_MIN;

            for(const auto& new_entry : std::filesystem::directory_iterator(directory_path))
            {
                if(new_entry.is_directory()) continue;

                const std::string new_filename = new_entry.path().filename().string();

                if(std::find(processed_files.begin(), 
                             processed_files.end(), 
                             new_filename) != processed_files.end())
                    {
                        continue;
                    }

                const auto new_matches_begin = std::sregex_iterator(new_filename.begin(), 
                                                                    new_filename.end(), 
                                                                    num_pattern);
                const auto new_matches_end = std::sregex_iterator();

                for(std::sregex_iterator it = new_matches_begin; it != new_matches_end; ++it)
                {
                    const std::smatch new_match = *it;
                    const int32_t match_int = std::stoi(new_match.str());
                    const uint8_t padding = new_match.length();

                    std::string dash = "";

                    for(uint8_t i = 0; i < padding; i++) dash += "#";

                    const std::string new_entry_name_transformed = fmt::format("{}{}{}",
                                                                               new_filename.substr(0, new_match.position()),
                                                                               dash,
                                                                               new_filename.substr(new_match.position() + new_match.length()));

                    if(new_entry_name_transformed == file_sequence_entry)
                    {
                        if(match_int > fileseq_end)
                        {
                            fileseq_end = match_int;
                        }

                        if(match_int < fileseq_start)
                        {
                            fileseq_start = match_int;
                        }

                        fileseq_count++;

                        if(std::find(processed_files.begin(),
                                     processed_files.end(),
                                     new_filename) == processed_files.end())
                        {
                            processed_files.emplace_back(std::move(new_filename));
                        }

                        break;
                    }
                }
            }

            files.emplace_back(fmt::format("seq?{}/{} [{}-{}]", directory_path,
                                                                file_sequence_entry, 
                                                                fileseq_start, 
                                                                fileseq_end));
        }
        else
        {
            files.emplace_back(fmt::format("{}/{}", directory_path, filename));
        }

        if(std::find(processed_files.begin(),
                     processed_files.end(),
                     filename) == processed_files.end())
        {
            processed_files.emplace_back(filename);
        }
    }

    files.shrink_to_fit();
    file_names = std::move(files);
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

LOVU_DLL std::string get_documents_folder_path() noexcept
{
#ifdef LOVU_WIN
    PWSTR ppsz_path;

    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &ppsz_path);

    std::wstring my_path;

    if (SUCCEEDED(hr)) 
    {
        my_path = ppsz_path;
        
        CoTaskMemFree(ppsz_path); 

        // https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
        return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(my_path);
    }
    else
    {
        return "";
    }
#else if LOVU_LINUX
// https://stackoverflow.com/questions/2910377/get-home-directory-in-linux
    const char* homedir;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    return fmt::format("{}/Documents", homedir);
#endif
}

LOVU_DLL bool exists(const std::string& path) noexcept
{
    return std::filesystem::exists(std::filesystem::path(path));
}

FS_NAMESPACE_END

LOVU_NAMESPACE_END