// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <filesystem>
#include <regex>
#include <vector>

#include "decl.h" 
#include "string_utils.h"

#ifdef OV_WIN
#include "windows.h"
#include "ShlObj_core.h"
#else if OV_LINUX
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

namespace Utils
{
    namespace Fs
    {
        OV_STATIC_FUNC OV_FORCEINLINE size_t FileCountInDirectory(const std::string& directoryPath)
        {
            return static_cast<size_t>(std::distance(std::filesystem::directory_iterator(directoryPath), std::filesystem::directory_iterator{}));
        }

        using FileSequenceItem = std::pair<std::string, uint32_t>;
        using FileSequence = std::vector<FileSequenceItem>;

        OV_STATIC_FUNC void GetFileSequenceFromFile(FileSequence& fileSequence, const std::string& filePath)
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
                    const std::regex itMatchPatternRegex(itMatchStr);

                    std::string itMatchSub = itFileStem;
                    itMatchSub.replace(itMatch.position(), itMatch.length(), "#");

                    for (std::sregex_iterator base = baseNumBegin; base != baseNumEnd; ++base)
                    {
                        const std::smatch baseMatch = *base;
                        const std::string baseMatchStr = baseMatch.str();
                        const std::regex baseMatchPatternRegex(baseMatchStr);


                        std::string baseMatchSub = baseFileStem;
                        baseMatchSub.replace(baseMatch.position(), baseMatch.length(), "#");

                        if (baseMatchSub == itMatchSub)
                        {
                            const uint32_t itFrameNum = std::stoi(itMatchStr);

                            fileSeq.emplace_back(std::make_pair(Str::CleanOSPath(file.path().string()), itFrameNum));

                            foundAnotherFile = true;

                            if (baseFileFrameNum == UINT32_MAX) 
                            {
                                baseFileFrameNum = std::stoi(baseMatchStr);
                                fileSeq.emplace_back(std::make_pair(Str::CleanOSPath(filePath), baseFileFrameNum));
                            }

                            break;
                        }
                    }

                    if (foundAnotherFile) break;
                }
            }

            fileSeq.shrink_to_fit();

            std::sort(fileSeq.begin(), fileSeq.end(), [&](const FileSequenceItem& a, const FileSequenceItem& b){ return a.second < b.second; });

            fileSequence = std::move(fileSeq);
        }

        OV_STATIC_FUNC bool IsImage(const std::string& path) noexcept
        {
            constexpr char* imageExtensions[] = { "exr", "jpg", "jpeg", "bmp", "tif", "tiff", "png",
                                                  "raw", "cr2", "arw", "sr2", "nef", "orf", "psd",
                                                  "bmp", "ppm", "cin", "dds", "dcm", "dpx", "fits",
                                                  "hdr", "heic", "avif", "ico", "iff", "jp2", "j2k",
                                                  "pbm", "pgm", "ptex", "rla", "pic", "tga", "tpic",
                                                  "zfile", "tex",
                                                  "EXR", "JPG", "JPEG", "BMP", "TIF", "TIFF", "PNG",
                                                  "RAW", "CR2", "ARW", "SR2", "NEF", "ORF", "PSD",
                                                  "BMP", "PPM", "CIN", "DDS", "DCM", "DPX", "FITS",
                                                  "HDR", "HEIC", "AVIF", "ICO", "IFF", "JP2", "J2K",
                                                  "PBM", "PGM", "PTEX", "RLA", "PIC", "TGA", "TPIC",
                                                  "ZFILE", "TEX" };

            for (uint8_t i = 0; i < OVARRAYSIZE(imageExtensions); i++)
            {
                if (Str::EndsWith(path, imageExtensions[i])) return true;
            }

            return false;
        }

        OV_STATIC_FUNC bool IsVideo(const std::string& path) noexcept
        {
            constexpr char* videoExtensions[] = { "mp4", "m4p", "m4v", "mov", "qt", "avi", "yuv", "mkv",
                                                  "MP4", "M4P", "M4V", "MOV", "QT", "AVI", "YUV", "MKV" };

            for (uint8_t i = 0; i < OVARRAYSIZE(videoExtensions); i++)
            {
                if (Str::EndsWith(path, videoExtensions[i])) return true;
            }

            return false;
        }

        OV_STATIC_FUNC bool IsOpenEXR(const std::string& path) noexcept
        {
            return Str::EndsWith(path, "exr");
        }

        OV_STATIC_FUNC std::string ExpandCwd(const std::string& pathToExpand) noexcept
        {
            std::string cwd = std::filesystem::current_path().string();
            Str::CleanOSPath(cwd);
            return cwd + pathToExpand;
        }

        OV_STATIC_FUNC std::string GetDocumentFolder() noexcept
        {
#ifdef OV_WIN
            PWSTR ppszPath;

            HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &ppszPath);

            std::wstring myPath;

            if (SUCCEEDED(hr)) 
            {
                myPath = ppszPath;
                
                CoTaskMemFree(ppszPath); 
                return std::string(myPath.begin(), myPath.end());
            }
#else if OV_LINUX
// https://stackoverflow.com/questions/2910377/get-home-directory-in-linux
            const char* homedir;

            if ((homedir = getenv("HOME")) == NULL) {
                homedir = getpwuid(getuid())->pw_dir;
            }

            std::string docDir = homedir;
            docDir += "/Documents";

            return docDir;
#endif
        }
    } // End namespace Fs
} // End namespace Utils