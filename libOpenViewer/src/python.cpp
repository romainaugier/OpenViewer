// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/settings.h"
#include "OpenViewer/media_pool.h"
#include "OpenViewer/cache.h"
#include "OpenViewer/display.h"

#include "pybind11/pybind11.h"
#include "pybind11/detail/common.h"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;

// For classes with a private destructor
// See : https://github.com/pybind/pybind11/issues/114
template < typename T>
struct BlankDeleter
{
    void operator()(T * inst) const {}
};


// Class with pure virtual functions
class PyMedia : public lov::Media
{
public:
    using lov::Media::Media;

    std::string make_path_at_frame(const uint32_t frame) const noexcept 
    {
        PYBIND11_OVERRIDE_PURE(std::string, lov::Media, make_path_at_frame, frame);
    }

    uint32_t get_hash_at_frame(const uint32_t frame) const noexcept 
    {
        PYBIND11_OVERRIDE_PURE(uint32_t, lov::Media, get_hash_at_frame, frame);
    }

    void load_frame_to_memory(void* cache_address, const uint32_t frame) const noexcept 
    {
        PYBIND11_OVERRIDE_PURE(void, lov::Media, load_frame_to_memory, cache_address, frame);
    }

    bool is_cached_at_frame(const uint32_t frame) const noexcept 
    {
        PYBIND11_OVERRIDE_PURE(bool, lov::Media, is_cached_at_frame, frame);
    }

    void set_cached_at_frame(const uint32_t frame, const bool cached = true) noexcept 
    {
        PYBIND11_OVERRIDE_PURE(void, lov::Media, set_cached_at_frame, frame, cached);
    }

    void debug() const noexcept 
    {
        PYBIND11_OVERRIDE_PURE(void, lov::Media, debug);
    }
};


PYBIND11_MODULE(PyOpenViewer, m)
{
    m.doc() = "OpenViewer Python Module";

    m.def("log_debug", [](const std::string& message) { spdlog::debug(message); });
    m.def("log_info", [](const std::string& message) { spdlog::info(message); });
    m.def("log_warning", [](const std::string& message) { spdlog::warn(message); });
    m.def("log_error", [](const std::string& message) { spdlog::error(message); });
    m.def("log_critical", [](const std::string& message) { spdlog::critical(message); });

    py::class_<lov::Settings, std::unique_ptr<lov::Settings, BlankDeleter<lov::Settings>>>(m, "Settings")
        .def_static("get_instance", &lov::Settings::get_instance, pybind11::return_value_policy::reference)
        .def("save", &lov::Settings::save)
        .def("load", &lov::Settings::load)
        .def("__getitem__", [](lov::Settings* self, const std::string& key) { return self->operator[](key); })
        .def("__setitem__", [](lov::Settings* self, const std::string& key, py::handle value) { self->operator[](key) = value; });

    py::class_<lov::Media, PyMedia>(m, "Media")
        .def(py::init<>())
        .def_property("width", &lov::Media::get_width, &lov::Media::set_width)
        .def_property("height", &lov::Media::get_height, &lov::Media::set_height)
        .def_property("nchannels", &lov::Media::get_nchannels, &lov::Media::set_nchannels)
        .def_property("type", &lov::Media::get_type, &lov::Media::set_type)
        .def_property_readonly("path", &lov::Media::get_path)
        .def_property_readonly("length", &lov::Media::get_length)
        .def_property_readonly("start_frame", &lov::Media::get_start_frame)
        .def_property_readonly("end_frame", &lov::Media::get_end_frame)
        .def("has_layers", &lov::Media::has_layers)
        .def("make_path_at_frame", &lov::Media::make_path_at_frame)
        .def("is_cached_at_frame", &lov::Media::is_cached_at_frame);

    py::class_<lov::Media, lov::Image>(m, "Image")
        .def(py::init<const std::string>());

    py::class_<lov::Media, lov::ImageSequence>(m, "ImageSequence")
        .def(py::init<const std::string>());

    py::class_<lov::MediaPool>(m, "MediaPool")
        .def(py::init<>())
        .def("add_media", &lov::MediaPool::add_media)
        .def("remove_media", &lov::MediaPool::remove_media)
        .def("get_media",
             static_cast<lov::Media* (lov::MediaPool::*)(const std::string&)>(&lov::MediaPool::get_media),
             py::return_value_policy::reference)
        .def("get_media",
             static_cast<lov::Media* (lov::MediaPool::*)(const uint32_t)>(&lov::MediaPool::get_media),
             py::return_value_policy::reference);

    py::class_<lov::Cache>(m, "Cache")
        .def(py::init<>())
        .def("add", &lov::Cache::add)
        .def("resize", &lov::Cache::resize)
        .def("get_data_ptr", &lov::Cache::get_data_ptr)
        .def("flush", &lov::Cache::flush);

    py::class_<lov::Display>(m, "Display")
        .def(py::init<>())
        .def("set_data", &lov::Display::set_data)
        .def("get_gl_texture", &lov::Display::get_gl_texture);
}