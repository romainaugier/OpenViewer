// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_IMAGE_READER)
#define __LOV_IMAGE_READER

#include "OpenViewer/media_info.hpp"

#include "stdromano/vector.hpp"

LOV_NAMESPACE_BEGIN

// A reader turns a file on disk into pixels in memory a caller already owns.
//
// Two rules make the rest of the library possible:
//
//  1. A reader never allocates the destination. It fills the span it is given.
//     That is what lets the cache own every pixel buffer in the process and
//     lets the same reader serve a cache slot, a test buffer, or a staging
//     buffer for the GPU without knowing the difference.
//
//  2. A reader never knows about the cache, the media pool, or the playhead.
//     The only inputs are a path, a layer, and a destination.
//
// read_info() is called once when a media is opened; read_layer() is called on
// a worker thread for every frame, so it must be re-entrant. Neither may throw:
// the format libraries that do (OpenEXR) are wrapped by the reader.
struct ImageReader
{
    // Human-readable name for logs, e.g. "OpenEXR".
    const char* name;

    // Fills `info` with the windows, aspect ratio and layer descriptions of the
    // file. Returns false and logs on failure; `info` is then unspecified.
    bool (*read_info)(const stdromano::StringD& path, MediaInfo& info) noexcept;

    // Decodes one layer into `dst`, which holds at least `dst_size` bytes.
    // `layer` must be one obtained from read_info() on the same file: it
    // supplies the expected dimensions, format, depth and channel names.
    // Returns false and logs on failure; the contents of `dst` are then
    // unspecified.
    bool (*read_layer)(const stdromano::StringD& path,
                       const stdromano::StringD& layer_name,
                       const MediaLayer& layer,
                       void* dst,
                       std::size_t dst_size) noexcept;
};

// Maps file extensions to readers.
//
// Built-in readers are registered lazily by the singleton's constructor rather
// than by namespace-scope objects, which would put the table at the mercy of
// static initialization order across translation units. That ordering bug is
// invisible in a static build and shows up as an empty table in a shared one.
class LOV_API ImageReaderRegistry
{
private:
    stdromano::HashMap<stdromano::StringD, const ImageReader*> _readers;

    ImageReaderRegistry();

public:
    LOV_NON_COPYABLE(ImageReaderRegistry)

    ~ImageReaderRegistry() = default;

    static ImageReaderRegistry& get_instance() noexcept;

    // `ext` is matched case-insensitively and a leading dot is accepted, so
    // "EXR", "exr" and ".exr" all resolve to the same reader. Registering an
    // extension that already has a reader replaces it, which is how an
    // application swaps in its own implementation.
    void register_reader(const stdromano::StringD& ext, const ImageReader* reader) noexcept;

    // nullptr when no reader handles the extension.
    const ImageReader* find(const stdromano::StringD& ext) const noexcept;

    // Same, taking the extension from everything after the last dot in `path`.
    const ImageReader* find_for_path(const stdromano::StringD& path) const noexcept;

    // Every registered extension, for the file dialog filter and for logging
    // what the build actually supports.
    stdromano::Vector<stdromano::StringD> supported_extensions() const noexcept;
};

/* Convenience front-ends, which is what callers outside the library should use */

LOV_API bool image_read_info(const stdromano::StringD& path, MediaInfo& info) noexcept;

LOV_API bool image_read_layer(const stdromano::StringD& path,
                              const stdromano::StringD& layer_name,
                              const MediaLayer& layer,
                              void* dst,
                              std::size_t dst_size) noexcept;

// True when some registered reader claims the extension. Cheap enough to call
// while walking a directory.
LOV_API bool image_is_supported(const stdromano::StringD& path) noexcept;

LOV_NAMESPACE_END

#endif // !defined(__LOV_IMAGE_READER)
