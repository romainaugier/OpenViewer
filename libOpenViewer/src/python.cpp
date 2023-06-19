// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/settings.h"

#include "pybind11/pybind11.h"
#include "pybind11/detail/common.h"

namespace py = pybind11;

// Convert json to pyobject and vice versa
// https://github.com/pybind/pybind11/issues/1627

py::object from_json(const json& j)
{
    if (j.is_null())
    {
        return py::none();
    }
    if (j.is_boolean())
    {
        return py::bool_(j.get<bool>());
    }
    if (j.is_number())
    {
        double number = j.get<double>();
        if (number == std::floor(number))
        {
            return py::int_(j.get<int>());
        }
        else
        {
            return py::float_(number);
        }
    }
    if (j.is_string())
    {
        return py::str(j.get<std::string>());
    }
    if (j.is_array())
    {
        py::list obj;
        for (const auto& el: j)
        {
            obj.attr("append")(from_json(el));
        }
        return obj;
    }
    if (j.is_object())
    {
        py::dict obj;
        for (json::const_iterator it = j.cbegin(); it != j.cend(); ++it)
        {
            obj[py::str(it.key())] = from_json(it.value());
        }
        return obj;
    }

    return py::none();
}

json to_json(py::handle obj)
{
    if (obj.is_none())
    {
        return nullptr;
    }
    if (py::isinstance<py::bool_>(obj))
    {
        return obj.cast<bool>();
    }
    if (py::isinstance<py::int_>(obj))
    {
        return obj.cast<long>();
    }
    if (py::isinstance<py::float_>(obj))
    {
        return obj.cast<double>();
    }
    if (py::isinstance<py::str>(obj))
    {
        return obj.cast<std::string>();
    }
    if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj))
    {
        json out;
        for (py::handle value: obj)
        {
            out.push_back(to_json(value));
        }
        return out;
    }
    if (py::isinstance<py::dict>(obj))
    {
        json out;
        for (py::handle key: obj)
        {
            out[key.cast<std::string>()] = to_json(obj[key]);
        }
        return out;
    }

    return nullptr;
}


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

    py::class_<lov::Settings, std::unique_ptr<lov::Settings, BlankDeleter<lov::Settings>>>(m, "Settings")
        .def_static("get_instance", &lov::Settings::get_instance, pybind11::return_value_policy::reference)
        .def("save", &lov::Settings::save)
        .def("load", &lov::Settings::load)
        .def("__getitem__", [](lov::Settings* self, const std::string& key) { return from_json(self->operator[](key)); })
        .def("__setitem__", [](lov::Settings* self, const std::string& key, py::handle value) { self->operator[](key) = to_json(value); });
}