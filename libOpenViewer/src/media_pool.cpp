// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media_pool.hpp"

LOV_NAMESPACE_BEGIN

std::string get_file_sequence_from_file(const std::string& file_path) noexcept
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

LOV_NAMESPACE_END
