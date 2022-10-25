// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "string.h"

#ifdef LOVU_WIN
#include "windows.h"
#include "ShlObj_core.h"
#else if LOVU_LINUX
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#define FS_NAMESPACE_BEGIN namespace fs {
#define FS_NAMESPACE_END }

LOVU_NAMESPACE_BEGIN

FS_NAMESPACE_BEGIN

using file_sequence_item = std::pair<std::string, uint32_t>;
using file_sequence = std::vector<file_sequence_item>;

constexpr char* image_extensions[] = { "exr", "jpg", "jpeg", "bmp", "tif", "tiff", "png",
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

constexpr char* video_extensions[] = { "mp4", "m4p", "m4v", "mov", "qt", "avi", "yuv", "mkv",
                                       "MP4", "M4P", "M4V", "MOV", "QT", "AVI", "YUV", "MKV" };

// Returns the number of files in a directory
LOVU_FORCEINLINE size_t file_count_in_directory(const std::string& directory_path) noexcept;
LOVU_FORCEINLINE size_t file_count_in_directory(const std::string_view& directory_path) noexcept;

// Fills a file sequence with file sequence items
LOVU_DLL void get_file_sequence_from_file(file_sequence& file_sequence, const std::string& file_path) noexcept;

// Checks if the given filepath is an image file
LOVU_DLL bool is_image(const std::string& path) noexcept;
LOVU_DLL bool is_image(const std::string_view& path) noexcept;

// Checks if the given filepath if a video file
LOVU_DLL bool is_video(const std::string& path) noexcept;
LOVU_DLL bool is_video(const std::string_view& path) noexcept;

// Expands a path that starts in the executable folder
LOVU_DLL std::string expand_from_executable_dir(const std::string& path_to_expand) noexcept;

// Returns the path to the documents folder
LOVU_DLL std::string get_documents_folder_path() noexcept;

FS_NAMESPACE_END

LOVU_NAMESPACE_END