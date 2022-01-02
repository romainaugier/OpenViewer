// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <filesystem>
#include <regex>
#include <vector>

#include "decl.h" 
#include "string_utils.h"

namespace Utils
{
    OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE size_t FileCountInDirectory(const std::string& directoryPath)
    {
        return static_cast<size_t>(std::distance(std::filesystem::directory_iterator(directoryPath), std::filesystem::directory_iterator{}));
    }

    using FileSequenceItem = std::pair<std::string, uint32_t>;
    using FileSequence = std::vector<FileSequenceItem>;

    OPENVIEWER_STATIC_FUNC void GetFileSequenceFromFile(FileSequence& fileSequence, const std::string& filePath)
    {
        const std::filesystem::path parentDir = std::filesystem::path(filePath).parent_path();
        const std::string baseFileName = std::filesystem::path(filePath).filename().string();
        const std::string baseFileStem = std::filesystem::path(filePath).stem().string();
        const std::string baseFileExtension = std::filesystem::path(filePath).extension().string();
        
        FileSequence fileSeq;
        fileSeq.reserve(FileCountInDirectory(parentDir.string()));

        const std::regex numPattern("\\d+");
    
        auto baseNumBegin = std::sregex_iterator(baseFileStem.begin(), baseFileStem.end(), numPattern);
        auto baseNumEnd = std::sregex_iterator();

        uint32_t baseFileFrameNum = UINT32_MAX;
        
        for (const auto& file : std::filesystem::directory_iterator(parentDir))
        {
            const std::string itFileName = std::filesystem::path(file).filename().string();

            if (itFileName == baseFileName) continue;

            const std::string itFileStem = std::filesystem::path(file).stem().string();
            const std::string itFileExtension = std::filesystem::path(file).extension().string();

            auto itNumBegin = std::sregex_iterator(itFileStem.begin(), itFileStem.end(), numPattern);
            auto itNumEnd = std::sregex_iterator();

            bool foundAnotherFile = false;

            for (std::sregex_iterator it = itNumBegin; it != itNumEnd; ++it)
            {
                const std::smatch itMatch = *it;
                const std::string itMatchStr = itMatch.str();
                std::regex itMatchPatternRegex(itMatchStr);

                std::string itMatchSub = itFileStem;
                itMatchSub = std::regex_replace(itMatchSub, itMatchPatternRegex, "#");

                for (std::sregex_iterator base = baseNumBegin; base != baseNumEnd; ++base)
                {
                    const std::smatch baseMatch = *base;
                    const std::string baseMatchStr = baseMatch.str();
                    std::regex baseMatchPatternRegex(baseMatchStr);

                    std::string baseMatchSub = baseFileStem;
                    baseMatchSub = std::regex_replace(baseMatchSub, baseMatchPatternRegex, "#");

                    // std::cout << baseMatchStr << " " << baseMatchSub << " // " << itMatchStr << " " << itMatchSub << "\n";

                    if (baseMatchSub == itMatchSub)
                    {
                        const uint32_t itFrameNum = std::stoi(itMatchStr);

                        fileSeq.emplace_back(std::make_pair(CleanOSPath(file.path().string()), itFrameNum));

                        foundAnotherFile = true;

                        if (baseFileFrameNum == UINT32_MAX) 
                        {
                            baseFileFrameNum = std::stoi(baseMatchStr);
                            fileSeq.emplace_back(std::make_pair(CleanOSPath(filePath), baseFileFrameNum));
                        }

                        break;
                    }
                }

                if (foundAnotherFile) break;
            }
        }

        fileSeq.shrink_to_fit();

        std::sort(fileSeq.begin(), fileSeq.end(), [&](const FileSequenceItem& a, const FileSequenceItem& b){ return a.second < b.second; });

        // for (const auto& file : fileSeq)
        // {
        //     std::cout << file.first << " / Frame : " << file.second << "\n";
        // }

        fileSequence = std::move(fileSeq);
    }
} // End namespace Utils