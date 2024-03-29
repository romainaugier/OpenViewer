// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerApp/timeline.h"
#include "OpenViewerUtils/random.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "imgui.h"

#if !defined(LOVA_TIMELINE_DEBUG)
#define LOVA_TIMELINE_DEBUG
#endif // !defined(LOVA_TIMELINE_DEBUG)

LOVA_NAMESPACE_BEGIN

enum TimelineWidgetMode_
{
    TimelineWidgetMode_Nav = 0,
    TimelinewidgetMode_Cut = 1
};

enum TimelineWidgetDragSource_ 
{
    TimelineWidgetDragSource_Head = 0,
    TimelineWidgetDragSource_Body = 1,
    TimelineWidgetDragSource_Tail = 2
};

TimelineWidget::TimelineWidget()
{

}

TimelineWidget::~TimelineWidget()
{

}

void TimelineWidget::draw() noexcept
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | 
                             ImGuiWindowFlags_NoCollapse;

    constexpr float frames_width = 5.0f;
    constexpr float items_height = 20.0f;
    constexpr float handles_width = 5.0f;

    static int8_t drag_source = -1;
    static lov::TimelineItem* drag_source_item = nullptr;
    static ImVec2 drag_source_pos = ImVec2(0.0f, 0.0f);
    static uint32_t drag_orig_start_frame = 0;
    static uint32_t drag_orig_end_frame = 0;

    if(this->m_show)
    {
        ImGui::Begin("Timeline", &this->m_show, flags);
        {
            if(ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Media");

                if(payload != nullptr)
                {
                    lov::Media* media = *(lov::Media**)(payload->Data);
                    this->internal_timeline.add_item(media);
                }

                ImGui::EndDragDropTarget();
            }

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            const ImVec2 canvas_size = ImGui::GetContentRegionAvail();

            const ImVec2 canvas_x0 = ImVec2(canvas_pos.x, canvas_pos.y);
            const ImVec2 canvas_x1 = ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y);
            const ImVec2 canvas_x2 = ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y);
            const ImVec2 canvas_x3 = ImVec2(canvas_pos.x, canvas_pos.y + canvas_size.y);

#if defined(LOVA_TIMELINE_DEBUG)
            const uint32_t timeline_canvas_debug_col = 0xFF8800FF;
            draw_list->AddQuad(canvas_x0, canvas_x1, canvas_x2, canvas_x3, timeline_canvas_debug_col);
            draw_list->AddText(canvas_x0, timeline_canvas_debug_col, "Timeline");
#endif

            // Frames
            const float frames_canvas_height = 20.0f;

            const ImVec2 frames_canvas_x0 = canvas_x0;
            const ImVec2 frames_canvas_x1 = ImVec2(canvas_x1.x, canvas_x1.y + frames_canvas_height);

#if defined(LOVA_TIMELINE_DEBUG)
            const uint32_t frames_canvas_debug_col = 0xFF00FF00;
            draw_list->AddRect(frames_canvas_x0, frames_canvas_x1, frames_canvas_debug_col);
            draw_list->AddText(frames_canvas_x0, frames_canvas_debug_col, "Frames");
#endif

            draw_list->PushClipRect(frames_canvas_x0, frames_canvas_x1);

            uint32_t global_start, global_end;
            this->internal_timeline.get_global_range(global_start, global_end);

            uint32_t focus_start, focus_end;
            this->internal_timeline.get_focus_range(focus_start, focus_end);

            for(uint32_t i = 0; i < (global_end - global_start) + 1; i++)
            {
                const bool special_line = i % 24 == 0;
                const float line_height = special_line ? 0.0f : (4 * (frames_canvas_height / 5.0)) ;

                const ImVec2 line_p0 = ImVec2(frames_canvas_x0.x + i * frames_width,
                                              frames_canvas_x0.y + line_height);
                const ImVec2 line_p1 = ImVec2(frames_canvas_x0.x + i * frames_width,
                                              frames_canvas_x0.y + frames_canvas_height);

                draw_list->AddLine(line_p0, line_p1, 0xFFFFFF00, 0.5f);

                if(special_line)
                {
                    const ImVec2 text_pos = ImVec2(line_p0.x + 3, line_p0.y + 1);
                    draw_list->AddText(text_pos, 0xFF0088FF, fmt::format("{}", i).c_str());
                }
            }

            draw_list->PopClipRect();

            // Items
            const ImVec2 items_canvas_x0 = ImVec2(canvas_x0.x, frames_canvas_x1.y);
            const ImVec2 items_canvas_x1 = ImVec2(canvas_x1.x, canvas_x3.y);

            draw_list->PushClipRect(items_canvas_x0, items_canvas_x1);

#if defined(LOVA_TIMELINE_DEBUG)
            constexpr uint32_t items_canvas_debug_col = 0xFF88FF00;
            draw_list->AddRect(items_canvas_x0, items_canvas_x1, items_canvas_debug_col);
            draw_list->AddText(items_canvas_x0, items_canvas_debug_col, "Items");
#endif

            for(size_t i = 0; i < this->internal_timeline.get_items()->size(); i++)
            {
                const lov::TimelineItem timeline_item = this->internal_timeline.get_items()->operator[](i);

                const ImVec2 timeline_item_x0 = ImVec2(items_canvas_x0.x + timeline_item.get_start_frame() * frames_width,
                                                       items_canvas_x0.y);
                const ImVec2 timeline_item_x1 = ImVec2(items_canvas_x0.x + timeline_item.get_end_frame() * frames_width,
                                                       items_canvas_x0.y + items_height);

#if defined(LOVA_TIMELINE_DEBUG)
                constexpr uint32_t timeline_items_debug_col = 0xFF88FF00;
                draw_list->AddRect(timeline_item_x0, timeline_item_x1, timeline_items_debug_col);
#endif

                const uint32_t timeline_item_color = 0xFF000000 | lovu::wang_hash_sampler_32(i + 1);

                draw_list->AddRectFilled(timeline_item_x0, timeline_item_x1, timeline_item_color);

                const ImVec2 timeline_item_head_x0 = timeline_item_x0;
                const ImVec2 timeline_item_head_x1 = ImVec2(timeline_item_x0.x + handles_width,
                                                            timeline_item_x1.y);

                if(ImGui::IsMouseHoveringRect(timeline_item_head_x0, timeline_item_head_x1))
                {
                    draw_list->AddRectFilled(timeline_item_head_x0,
                                             timeline_item_head_x1,
                                             0x44FFFFFF);

                    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        drag_source = TimelineWidgetDragSource_Head;
                        drag_source_item = &this->internal_timeline.get_items()->at(i);
                        drag_source_pos = ImGui::GetMousePos();
                        drag_orig_start_frame = drag_source_item->get_start_frame();
                        drag_orig_end_frame = drag_source_item->get_end_frame();
                    }
                }

                const ImVec2 timeline_item_body_x0 = ImVec2(timeline_item_head_x1.x,
                                                            timeline_item_head_x0.y);
                const ImVec2 timeline_item_body_x1 = ImVec2(timeline_item_x1.x - handles_width,
                                                            timeline_item_x1.y);

                if(ImGui::IsMouseHoveringRect(timeline_item_body_x0, timeline_item_body_x1))
                {
                    draw_list->AddRectFilled(timeline_item_body_x0,
                                             timeline_item_body_x1,
                                             0x44FFFFFF);

                    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        drag_source = TimelineWidgetDragSource_Body;
                        drag_source_item = &this->internal_timeline.get_items()->at(i);
                        drag_source_pos = ImGui::GetMousePos();
                        drag_orig_start_frame = drag_source_item->get_start_frame();
                        drag_orig_end_frame = drag_source_item->get_end_frame();
                    }
                }

                ImGui::IsMouseDragging(ImGuiMouseButton_Left);

                const ImVec2 timeline_item_tail_x0 = ImVec2(timeline_item_body_x1.x,
                                                            timeline_item_body_x0.y);
                const ImVec2 timeline_item_tail_x1 = ImVec2(timeline_item_x1);

                if(ImGui::IsMouseHoveringRect(timeline_item_tail_x0, timeline_item_tail_x1))
                {
                    draw_list->AddRectFilled(timeline_item_tail_x0,
                                             timeline_item_tail_x1,
                                             0x44FFFFFF);

                    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        drag_source = TimelineWidgetDragSource_Tail;
                        drag_source_item = &this->internal_timeline.get_items()->at(i);
                        drag_source_pos = ImGui::GetMousePos();
                        drag_orig_start_frame = drag_source_item->get_start_frame();
                        drag_orig_end_frame = drag_source_item->get_end_frame();
                    }
                }
            }

            draw_list->PopClipRect();
        }
        ImGui::End();
    }

    if(ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
       drag_source > -1 &&
       drag_source_item != nullptr)
    {
        const float delta = ImGui::GetMousePos().x - drag_source_pos.x;
    
        const uint32_t translate = PIX_TO_FRAME(delta, frames_width);

        switch (drag_source)
        {
        case TimelineWidgetDragSource_Body:
            drag_source_item->set_start_frame(drag_orig_start_frame + translate);
            drag_source_item->set_end_frame(drag_orig_end_frame + translate);
            break;
        
        case TimelineWidgetDragSource_Head:
            drag_source_item->set_start_frame(drag_orig_start_frame + translate);
            break;

        case TimelineWidgetDragSource_Tail:
            drag_source_item->set_end_frame(drag_orig_end_frame + translate);
            break;

        default:
            break;
        }
    }

    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        drag_source = -1;
        drag_source_item = nullptr;
        drag_source_pos = ImVec2(0.0f, 0.0f);
    }
}

LOVA_NAMESPACE_END
