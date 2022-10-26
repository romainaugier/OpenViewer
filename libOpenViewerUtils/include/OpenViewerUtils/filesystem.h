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

// Finds if the given file is part of a filesequence, and if so returns a string formatted with necessary infos,
// like this : seq?D:/path/to/image_sequence_#.exr 100-150
LOVU_DLL std::string get_file_sequence_from_file(const std::string& file_path) noexcept;

// Finds all available filenames inside a directory, looking for file sequences which will be formatted
// like this : seq?D:/path/to/image_sequence_#.exr 100-150,
// and single files
LOVU_DLL void get_filenames_from_dir(std::vector<std::string>& file_names, 
                                     const std::string& directory_path) noexcept;

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

// Returns true if the given path exists, whether it is a directory or a file
LOVU_DLL bool exists(const std::string& path) noexcept;

FS_NAMESPACE_END

LOVU_NAMESPACE_END