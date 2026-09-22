// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

// Calls below LOV_LOG_ACTIVE_LEVEL are removed at compile time, whatever the
// runtime level says. Its own binary since the macro applies to a whole TU.
#define LOV_LOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include "lov_test.hpp"

#include "OpenViewer/log.hpp"

using namespace lov;

LOV_TEST(calls_below_the_active_level_are_compiled_out)
{
    log::set_level(spdlog::level::trace);

    log_trace(LogCategory::Media, "compiled_out_trace_marker");
    log_debug(LogCategory::Media, "compiled_out_debug_marker");
    log_info(LogCategory::Media, "kept_info_marker");

    const std::string log = lov_test::read_log_file();

    LOV_CHECK(log.find("compiled_out_trace_marker") == std::string::npos);
    LOV_CHECK(log.find("compiled_out_debug_marker") == std::string::npos);
    LOV_CHECK(lov_test::log_has_line(log, "[ov::media]", "kept_info_marker"));
}

LOV_TEST_MAIN()
