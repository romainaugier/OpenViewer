// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/settings.h"

#include "pybind11/pybind11.h"
#include "pybind11/detail/common.h"

namespace py = pybind11;

LOV_NAMESPACE_BEGIN

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
    throw std::runtime_error("to_json not implemented for this type of object: " + obj.cast<std::string>());
}


PYBIND11_MODULE(openviewer, m)
{
    m.doc() = "OpenViewer Python Module";

    py::class_<Settings>(m, "Settings")
        .def(py::init(&Settings::get_instance))
        .def("save", &Settings::save)
        .def("load", &Settings::load)
        .def("__getitem__", [](Settings &self, const std::string& key) { return from_json(self.data()[key]); })
        .def("__setitem__", [](Settings &self, const std::string& key, py::handle value) {self.data()[key] = to_json(value); });
}

LOV_NAMESPACE_END