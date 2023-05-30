// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/timeline.h"
#include "OpenViewer/media_pool.h"

void on_frame_changed(const uint32_t frame, lov::TimelineItem* timeline_item)
{
    if(timeline_item == nullptr)
    {
        spdlog::debug("Current item is null, doing nothing");
        return;
    }

    spdlog::debug("Frame : {}, Item path : {}", frame, timeline_item->get_media()->make_path_at_frame(frame));
}

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Timeline Test");

    try
    {
        lov::MediaPool media_pool;
        lov::Timeline timeline;

        timeline.push_event_callback(lov::TimelineEventType_FrameChanged,
                                     on_frame_changed);

        media_pool.add_media(fmt::format("{}/exr/sequence/compo_0500.0100.exr",
                                         TEST_DATA_DIR));

        media_pool.add_media(fmt::format("{}/exr/exr_multilayers.exr",
                                         TEST_DATA_DIR));

        lov::TimelineItem tl_item1(media_pool.get_media(0));
        timeline.add_item(tl_item1);

        lov::TimelineItem tl_item2(media_pool.get_media(1));
        tl_item2.set_start_frame(tl_item1.get_end_frame() + 1);
        tl_item2.set_end_frame(tl_item2.get_start_frame() + 30);
        timeline.add_item(tl_item2);

        lov::TimelineItem* tl_item_get1 = timeline.get_item_at_frame(58);

        if(tl_item_get1 != nullptr)
        {
            spdlog::debug("Item at frame 58 is : {}", tl_item_get1->get_media()->make_path_at_frame(58));
        }
        else
        {
            spdlog::debug("Cannot find any TimelineItem at frame 58");
        }
        
        tl_item_get1 = timeline.get_item_at_frame(160);

        if(tl_item_get1 != nullptr)
        {
            spdlog::debug("Item at frame 160 is : {}", tl_item_get1->get_media()->make_path_at_frame(160));
        }
        else
        {
            spdlog::debug("Cannot find any TimelineItem at frame 160");
        }

        timeline.fit_ranges_to_items();

        // timeline.play();

        // std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, caught exception : \n  {}", err.what());
        
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("Test Passed");

    return EXIT_SUCCESS;
}