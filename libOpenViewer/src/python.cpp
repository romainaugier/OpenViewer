// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/settings.h"
#include "OpenViewer/media_pool.h"
#include "OpenViewer/cache.h"

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

    py::class_<lov::Media>(m, "Media")
        .def(py::init<>())
        .def_property("width", &lov::Media::get_width, &lov::Media::set_width)
        .def_property("height", &lov::Media::get_height, &lov::Media::set_height)
        .def_property("nchannels", &lov::Media::get_nchannels, &lov::Media::set_nchannels)
        .def_property("type", &lov::Media::get_type, &lov::Media::set_type)
        .def_property_readonly("path", &lov::Media::get_path)
        .def_property_readonly("length", &lov::Media::get_length)
        .def_property_readonly("start_frame", &lov::Media::get_start_frame)
        .def_property_readonly("end_frame", &lov::Media::get_end_frame)
        .def("has_layers", &lov::Media::has_layers);

    py::class_<lov::Image, lov::Media>(m, "Image")
        .def(py::init<const std::string>())
        .def("make_path_at_frame", &lov::Image::make_path_at_frame)
        .def("is_cached_at_frame", &lov::Image::is_cached_at_frame)
        .def("get_hash_at_frame", &lov::Image::get_hash_at_frame)
        .def("load_frame_to_cache", &lov::Image::load_frame_to_cache);

    py::class_<lov::ImageSequence, lov::Media>(m, "ImageSequence")
        .def(py::init<const std::string>())
        .def("make_path_at_frame", &lov::ImageSequence::make_path_at_frame)
        .def("is_cached_at_frame", &lov::ImageSequence::is_cached_at_frame);

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
        .def("resize", &lov::Cache::resize);
}