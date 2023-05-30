// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "string.h"
#include <filesystem>

#define FS_NAMESPACE_BEGIN namespace fs {
#define FS_NAMESPACE_END }

LOVU_NAMESPACE_BEGIN

FS_NAMESPACE_BEGIN

static const char* const image_extensions[] = { "exr", "jpg", "jpeg", "bmp", "tif", "tiff", "png", "raw", "cr2", "arw", "sr2", "nef", "orf", "psd", "bmp", "ppm", "cin", "dds", "dcm", "dpx", "fits", "hdr", "heic", "avif", "ico", "iff", "jp2", "j2k", "pbm", "pgm", "ptex", "rla", "pic", "tga", "tpic", "zfile", "tex" };

static const char* const video_extensions[] = { "mp4", "m4p", "m4v", "mov", "qt", "avi", "yuv", "mkv" };

// Returns the number of files in a directory
LOVU_FORCEINLINE size_t file_count_in_directory(const std::string& directory_path) noexcept;
LOVU_FORCEINLINE size_t file_count_in_directory(const std::string_view& directory_path) noexcept;

// Finds if the given file is part of a filesequence, and if so returns a string formatted with necessary infos,
// like this : seq?D:/path/to/image_sequence_#.exr [100-150]
LOVU_API std::string get_file_sequence_from_file(const std::string& file_path) noexcept;

// Finds all available filenames inside a directory, looking for file sequences which will be formatted
// like this : seq?D:/path/to/image_sequence_#.exr [100-150],
// and single files
LOVU_API void get_filenames_from_dir(std::vector<std::string>& file_names, 
                                     const std::string& directory_path) noexcept;

// Checks if the given filepath is an image file
LOVU_API bool is_image(const std::string& path) noexcept;
LOVU_API bool is_image(const std::string_view& path) noexcept;

// Checks if the given filepath if a video file
LOVU_API bool is_video(const std::string& path) noexcept;
LOVU_API bool is_video(const std::string_view& path) noexcept;

// Expands a path that starts in the executable folder
LOVU_API std::string expand_from_executable_dir(const std::string& path_to_expand) noexcept;

// Returns the path to the documents folder
LOVU_API std::string get_documents_folder_path() noexcept;

// Returns true if the given path exists, whether it is a directory or a file
LOVU_API bool exists(const std::string& path) noexcept;

// Makes the needed directory of the path if they do not exist
LOVU_API void makedirs(const std::string& path) noexcept;

// Returns the extension of the given file path
LOVU_FORCEINLINE std::string get_extension(const std::string& path) noexcept { return &std::filesystem::path(path).extension().string()[1]; }

// Returns the parent dir of the given path
LOVU_FORCEINLINE std::string get_parent_dir(const std::string& path) noexcept { return std::filesystem::path(path).parent_path().string(); }

// Returns the filname without the extension
LOVU_FORCEINLINE std::string get_filename_no_ext(const std::string& path) noexcept { return std::filesystem::path(path).stem().string(); }

FS_NAMESPACE_END

LOVU_NAMESPACE_END