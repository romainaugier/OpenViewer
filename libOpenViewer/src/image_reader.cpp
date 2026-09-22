// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/image_reader.hpp"
#include "OpenViewer/log.hpp"

#include "readers/readers.hpp"

LOV_NAMESPACE_BEGIN

DETAIL_NAMESPACE_BEGIN

bool check_dst_size(const char* reader_name,
                    const stdromano::StringD& path,
                    const MediaLayer& layer,
                    const void* dst,
                    std::size_t dst_size) noexcept
{
    if(dst == nullptr)
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Null destination buffer while reading \"{}\"",
                  reader_name,
                  path);

        return false;
    }

    if(!layer.is_valid())
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Invalid layer description while reading \"{}\" ({}x{}, {}, {})",
                  reader_name,
                  path,
                  layer.width(),
                  layer.height(),
                  format_to_string(layer.format()),
                  depth_to_string(layer.depth()));

        return false;
    }

    const std::size_t needed = layer.nbytes();

    if(dst_size < needed)
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Destination buffer too small while reading \"{}\": {} bytes given, "
                  "{} needed ({}x{} {} {})",
                  reader_name,
                  path,
                  dst_size,
                  needed,
                  layer.width(),
                  layer.height(),
                  format_to_string(layer.format()),
                  depth_to_string(layer.depth()));

        return false;
    }

    return true;
}

static stdromano::StringD normalize_ext(const stdromano::StringD& ext) noexcept
{
    stdromano::StringD normalized = ext.lower();

    if(normalized.size() > 0 && normalized[0] == '.')
        return normalized.substr(1).copy();

    return normalized;
}

DETAIL_NAMESPACE_END

/* ------------------------------------------------------------------------ */
/* Registry                                                                 */
/* ------------------------------------------------------------------------ */

ImageReaderRegistry::ImageReaderRegistry()
{
    this->register_reader(stdromano::StringD::make_ref("exr"), &detail::g_reader_exr);

    this->register_reader(stdromano::StringD::make_ref("tif"), &detail::g_reader_tiff);
    this->register_reader(stdromano::StringD::make_ref("tiff"), &detail::g_reader_tiff);

    this->register_reader(stdromano::StringD::make_ref("jpg"), &detail::g_reader_stb_ldr);
    this->register_reader(stdromano::StringD::make_ref("jpeg"), &detail::g_reader_stb_ldr);

    this->register_reader(stdromano::StringD::make_ref("png"), &detail::g_reader_stb_ldr);

    this->register_reader(stdromano::StringD::make_ref("bmp"), &detail::g_reader_stb_ldr);

    this->register_reader(stdromano::StringD::make_ref("tga"), &detail::g_reader_stb_ldr);

    this->register_reader(stdromano::StringD::make_ref("hdr"), &detail::g_reader_stb_hdr);
}

ImageReaderRegistry& ImageReaderRegistry::get_instance() noexcept
{
    static ImageReaderRegistry instance;

    return instance;
}

void ImageReaderRegistry::register_reader(const stdromano::StringD& ext,
                                          const ImageReader* reader) noexcept
{
    if(reader == nullptr || reader->read_info == nullptr || reader->read_layer == nullptr)
    {
        log_error(LogCategory::ImageReader,
                  "Refusing to register an incomplete reader for extension \"{}\"",
                  ext);
        return;
    }

    this->_readers[detail::normalize_ext(ext)] = reader;
}

const ImageReader* ImageReaderRegistry::find(const stdromano::StringD& ext) const noexcept
{
    auto it = this->_readers.find(detail::normalize_ext(ext));

    return it == this->_readers.end() ? nullptr : it->second;
}

const ImageReader* ImageReaderRegistry::find_for_path(const stdromano::StringD& path) const noexcept
{
    return this->find(path.rsplit(stdromano::StringD::make_ref(".")));
}

stdromano::Vector<stdromano::StringD> ImageReaderRegistry::supported_extensions() const noexcept
{
    stdromano::Vector<stdromano::StringD> extensions;
    extensions.reserve(this->_readers.size());

    for(const auto& entry : this->_readers)
        extensions.push_back(entry.first.copy());

    return extensions;
}

/* ------------------------------------------------------------------------ */
/* Front-ends                                                               */
/* ------------------------------------------------------------------------ */

bool image_read_info(const stdromano::StringD& path, MediaInfo& info) noexcept
{
    const ImageReader* reader = ImageReaderRegistry::get_instance().find_for_path(path);

    if(reader == nullptr)
    {
        log_error(LogCategory::ImageReader,
                  "No reader registered for \"{}\" (extension: \"{}\")",
                  path,
                  path.rsplit(stdromano::StringD::make_ref(".")));

        return false;
    }

    if(!reader->read_info(path, info))
        return false;

    if(!info.is_valid())
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Produced an invalid MediaInfo for \"{}\": no usable main layer",
                  reader->name,
                  path);

        return false;
    }

    return true;
}

bool image_read_layer(const stdromano::StringD& path,
                      const stdromano::StringD& layer_name,
                      const MediaLayer& layer,
                      void* dst,
                      std::size_t dst_size) noexcept
{
    const ImageReader* reader = ImageReaderRegistry::get_instance().find_for_path(path);

    if(reader == nullptr)
    {
        log_error(LogCategory::ImageReader,
                  "No reader registered for \"{}\" (extension: \"{}\")",
                  path,
                  path.rsplit(stdromano::StringD::make_ref(".")));

        return false;
    }

    return reader->read_layer(path, layer_name, layer, dst, dst_size);
}

bool image_is_supported(const stdromano::StringD& path) noexcept
{
    return ImageReaderRegistry::get_instance().find_for_path(path) != nullptr;
}

LOV_NAMESPACE_END
