
internal bool
EditorFloatEdit4(const char* label, float col[4], ImGuiColorEditFlags flags, float dragSpeed)
{
    using namespace ImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float square_sz = GetFrameHeight();
    const float w_full = CalcItemWidth();
    const float w_button = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
    const float w_inputs = w_full - w_button;
    const char* label_display_end = FindRenderedTextEnd(label);
    g.NextItemData.ClearFlags();
    
    BeginGroup();
    PushID(label);
    
    // If we're not showing any slider there's no point in doing any HSV conversions
    const ImGuiColorEditFlags flags_untouched = flags;
    if (flags & ImGuiColorEditFlags_NoInputs)
        flags = (flags & (~ImGuiColorEditFlags__DisplayMask)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;
    
    // Context menu: display and modify options (before defaults are applied)
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        ColorEditOptionsPopup(col, flags);
    
    // Read stored options
    if (!(flags & ImGuiColorEditFlags__DisplayMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DisplayMask);
    if (!(flags & ImGuiColorEditFlags__DataTypeMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DataTypeMask);
    if (!(flags & ImGuiColorEditFlags__PickerMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__PickerMask);
    if (!(flags & ImGuiColorEditFlags__InputMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__InputMask);
    flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags__DisplayMask | ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags__InputMask));
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__DisplayMask)); // Check that only 1 is selected
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__InputMask));   // Check that only 1 is selected
    
    const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
    const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
    const int components = alpha ? 4 : 3;
    
    // Convert to the formats we need
    float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
    if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
        ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
    else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
        ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
    //int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };
    int i[4] = { (int)f[0], (int)f[1], (int)f[2],(int)f[3] };
    
    bool value_changed = false;
    bool value_changed_as_float = false;
    
    const ImVec2 pos = window->DC.CursorPos;
    const float inputs_offset_x = (style.ColorButtonPosition == ImGuiDir_Left) ? w_button : 0.0f;
    window->DC.CursorPos.x = pos.x + inputs_offset_x;
    
    if ((flags & (ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        // RGB/HSV 0..255 Sliders
        const float w_item_one = ImMax(1.0f, (float)(int)((w_inputs - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
        const float w_item_last = ImMax(1.0f, (float)(int)(w_inputs - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));
        
        const bool hide_prefix = (w_item_one <= CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
        static const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
        static const char* fmt_table_int[3][4] =
        {
            {   "%3d",   "%3d",   "%3d",   "%3d" }, // Short display
            { "X:%3d", "Y:%3d", "Z:%3d", "W:%3d" }, // Long display for RGBA
            { "X:%3d", "Y:%3d", "Z:%3d", "W:%3d" }  // Long display for HSVA
        };
        static const char* fmt_table_float[3][4] =
        {
            {   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
            { "X:%0.3f", "Y:%0.3f", "Z:%0.3f", "W:%0.3f" }, // Long display for RGBA
            { "X:%0.3f", "Y:%0.3f", "Z:%0.3f", "W:%0.3f" }  // Long display for HSVA
        };
        const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_DisplayHSV) ? 2 : 1;
        
        for (int n = 0; n < components; n++)
        {
            if (n > 0)
                SameLine(0, style.ItemInnerSpacing.x);
            SetNextItemWidth((n + 1 < components) ? w_item_one : w_item_last);
            if (flags & ImGuiColorEditFlags_Float)
            {
                value_changed |= DragFloat(ids[n], &f[n], dragSpeed / 255.0f, 0.0f, 0, fmt_table_float[fmt_idx][n]);
                value_changed_as_float |= value_changed;
            } else
            {
                value_changed |= DragInt(ids[n], &i[n], dragSpeed, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
            }
            if (!(flags & ImGuiColorEditFlags_NoOptions))
                OpenPopupOnItemClick("context");
        }
    } else if ((flags & ImGuiColorEditFlags_DisplayHex) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        // RGB Hexadecimal Input
        char buf[64];
        if (alpha)
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", i[0], i[1], i[2], i[3]);
        else
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", i[0], i[1], i[2]);
        SetNextItemWidth(w_inputs);
        if (InputText("##Text", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
        {
            value_changed = true;
            char* p = buf;
            while (*p == '#' || ImCharIsBlankA(*p))
                p++;
            i[0] = i[1] = i[2] = i[3] = 0;
            if (alpha)
                sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]); // Treat at unsigned (%X is unsigned)
            else
                sscanf(p, "%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2]);
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context");
    }
    
    ImGuiWindow* picker_active_window = NULL;
    
    
    if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
    {
        window->DC.CursorPos = ImVec2(pos.x + w_full + style.ItemInnerSpacing.x, pos.y + style.FramePadding.y);
        TextEx(label, label_display_end);
    }
    
    // Convert back
    if (value_changed && picker_active_window == NULL)
    {
        if (!value_changed_as_float)
            for (int n = 0; n < 4; n++)
            f[n] = i[n] / 255.0f;
        if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
            ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
        if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
            ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
        
        col[0] = f[0];
        col[1] = f[1];
        col[2] = f[2];
        if (alpha)
            col[3] = f[3];
    }
    
    PopID();
    EndGroup();
    
    // Drag and Drop Target
    // NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the same test.
    if ((window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_HoveredRect) && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
    {
        bool accepted_drag_drop = false;
        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
        {
            memcpy((float*)col, payload->Data, sizeof(float) * 3); // Preserve alpha if any //-V512
            value_changed = accepted_drag_drop = true;
        }
        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
        {
            memcpy((float*)col, payload->Data, sizeof(float) * components);
            value_changed = accepted_drag_drop = true;
        }
        
        // Drag-drop payloads are always RGB
        if (accepted_drag_drop && (flags & ImGuiColorEditFlags_InputHSV))
            ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
        EndDragDropTarget();
    }
    
    // When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
    if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
        window->DC.LastItemId = g.ActiveId;
    
    if (value_changed)
        MarkItemEdited(window->DC.LastItemId);
    
    return value_changed;
}

internal bool
EditorFloatEdit3(const char* label, float col[3], ImGuiColorEditFlags flags, float dragSpeed)
{
    return EditorFloatEdit4(label, col, flags | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_Float, dragSpeed);
}


//~ Utilities

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
struct crystalflask_console
{
    char                  InputBuf[256];
    ImVector<char*>       Items;
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;
    
    crystalflask_console()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;
        
        Commands.push_back("HELP");
        Commands.push_back("monkeytest");
        Commands.push_back("entitycount");
        Commands.push_back("delete_all_entities");
        
        AutoScroll = true;
        ScrollToBottom = true;
        
        AddLog("crystalflask Started.");
    }
    ~crystalflask_console()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            free(History[i]);
    }
    
    // Portable helpers
    static int   Stricmp(const char* str1, const char* str2)         { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
    static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
    static char* Strdup(const char *str)                             { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
    static void  Strtrim(char* str)                                  { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--;
        *str_end = 0; }
    static int
        TextEditCallbackStub(ImGuiInputTextCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
    {
        crystalflask_console* console = (crystalflask_console*)data->UserData;
        return console->TextEditCallback(data);
    }
    
    int
        crystalflask_console::TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
            case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION
                
                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }
                
                // Build a list of candidates
                ImVector<const char*> candidates;
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end-word_start)) == 0)
                    candidates.push_back(Commands[i]);
                
                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end-word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
                    data->DeleteChars((int)(word_start-data->Buf), (int)(word_end-word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }
                    
                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end-word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }
                    
                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }
                
                break;
            }
            case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= History.Size)
                        HistoryPos = -1;
                }
                
                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }
    
    void ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            free(Items[i]);
        Items.clear();
    }
    
    void AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf)-1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
    }
    
    
    void Draw(const char* title, bool* p_open)
    {
        //ImGui::SetNextWindowSize(ImVec2((r32)GlobalWidth, 600), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2((r32)GlobalWidth, 150), ImGuiCond_Appearing);
        //ImGui::SetNextWindowSize(ImVec2((r32)GlobalWidth, 600), ImGuiCond_Once);
        
        const float DISTANCE = 0.0f;
        static int corner = 0;
        ImGuiIO& io = ImGui::GetIO();
        
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        
        ImGui::SetNextWindowBgAlpha(0.2f);
        
        if (!ImGui::Begin(title, p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::End();
            return;
        }
        
        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;
            ImGui::EndPopup();
        }
        
        // TODO: display items starting from the bottom
        bool copy_to_clipboard = false;
        //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }
        
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
        
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear")) ClearLog();
            if (ImGui::Selectable("Copy all to clipboard")) copy_to_clipboard = true;
            ImGui::EndPopup();
        }
        
        // Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
        // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
        // You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
        // To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
        //     ImGuiListClipper clipper(Items.Size);
        //     while (clipper.Step())
        //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        // However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
        // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
        // and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
        // If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        for (int i = 0; i < Items.Size; i++)
        {
            const char* item = Items[i];
            if (!Filter.PassFilter(item))
                continue;
            
            // Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
            bool pop_color = false;
            if (strstr(item, "[error]") || strstr(item, "[ERROR]"))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true;
            }
            else if (strstr(item, "[warning]") || strstr(item, "[WARNING]"))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7568f, 0.0274f, 1.0f)); pop_color = true;
            }
            else if (strstr(item, "[info]") || strstr(item, "[INFO]"))
            {
                /*c
                91/255
                192/255
                222/255
                */
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.75f, 0.87f, 1.0f)); pop_color = true;
            }
            else if (strstr(item, "[success]") || strstr(item, "[SUCCESS]"))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); pop_color = true;
            }
            else if (strstr(item, "[critical]") || strstr(item, "[CRITICAL]"))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); pop_color = true;
            }
            else if (strncmp(item, "# ", 2) == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true;
            }
            
            
            ImGui::TextUnformatted(item);
            if (pop_color)
                ImGui::PopStyleColor();
        }
        if (copy_to_clipboard) {
            ImGui::LogFinish();
            copy_to_clipboard = false;
        }
        if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;
        
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();
        
        // Command-line
        bool reclaim_focus = true;
        if (ImGui::InputText("", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue |
                             ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
                             &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }
        
        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
        
        ImGui::End();
    }
    
    void ExecCommand(const char* command_line);
};

global_variable crystalflask_console CrystalFlaskConsole;
//~ console



internal void
ImGuiInit(HWND Window)
{
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    io.ConfigDockingWithShift = 0;
    io.ConfigDockingAlwaysTabBar = 1;
    io.ConfigDockingTransparentPayload = 1;
    io.ConfigWindowsMoveFromTitleBarOnly = 1;
    
    
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    style.FrameBorderSize = 0;
    style.WindowBorderSize = 0;
    style.ScrollbarSize = 4;
    style.ItemSpacing[0] = 7.0f;
    style.ItemSpacing[1] = 7.0f;
    style.ItemInnerSpacing[0] = 3.0f;
    style.ItemInnerSpacing[1] = 3.0f;
    style.GrabMinSize = 8.0f;
    style.WindowPadding[0] = 7.0f;
    style.WindowPadding[1] = 7.0f;
    style.FramePadding[0] = 4.0f;
    style.FramePadding[1] = 6.0f;
    
    
    style.WindowRounding = 3.0f;
    style.ChildRounding = 3.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 1.0f;
    style.TabRounding = 2.0f;
    
    style.WindowMenuButtonPosition = 1;
    
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.157f, 0.157f, 0.157f, 0.50f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.157f, 0.157f, 0.157f, 0.50f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.157f, 0.157f, 0.157f, 0.50f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.187f, 0.187f, 0.187f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.280f, 0.280f, 0.280f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.280f, 0.280f, 0.280f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    
    style.Colors[ImGuiCol_Button] = ImVec4(0.373f, 0.373f, 0.373f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    
    style.Colors[ImGuiCol_Header] = ImVec4(0.190f, 0.190f, 0.190f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    
    style.Colors[ImGuiCol_Separator] = ImVec4(0.230f, 0.230f, 0.230f, 0.5f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.230f, 0.230f, 0.230f, 0.5f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.230f, 0.230f, 0.230f, 0.5f);
    
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.250f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    
    
    style.Colors[ImGuiCol_Tab] = ImVec4(0.8, 0.8f, 0.8f, 0.676f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.551f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.200f, 0.200f, 0.200f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.0, 0.0f, 1.0f);
    
    
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.980f, 0.680f, 0.260f, 0.350f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.980f, 0.680f, 0.260f, 0.350f);
    
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.243f, 0.545f, 0.350f);
    
    
    io.Fonts->AddFontFromFileTTF("resources/editor/imgui/fonts/Inter-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("resources/imgui/fonts/Inter-Black.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("resources/imgui/fonts/Inter-Bold.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("resources/imgui/fonts/Inter-Medium.ttf", 15.0f);
    
    
    //io.Fonts->AddFontFromFileTTF("resources/editor/imgui/Menlo-Regular.ttf", 14.0f);
    //io.Fonts->AddFontFromFileTTF("resources/editor/imgui/routed-gothic.ttf", 20.0f);
    
    
    
    
    
    ImGui_ImplWin32_Init(Window);
    ImGui_ImplOpenGL3_Init("#version 460");
}

internal void
ImGuiNewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    if (GlobalIsEditorEnabled) //only render imgui if the editor is enabled.
    {
        
    }
}

internal void
ImGuiFinish(ImGuiIO& io)
{
    
    //if (GlobalIsEditorEnabled) //only render imgui if the editor is enabled.
    //{
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    
    //}
}


static void ShowInfoOverlay(bool* Enable)
{
    if (GlobalIsInfoOverlayVisible)
    {
        const float DISTANCE = 5.0f;
        static int corner = 2;
        ImGuiIO& io = ImGui::GetIO();
        if (corner != -1)
        {
            ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
            ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("Example: Simple overlay", Enable, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
        {
            ImGui::Text("OpenGL %s\n%s", OpenGLInfo.Renderer, OpenGLInfo.OpenGLVersion);
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            
            if (ImGui::IsMousePosValid())
                ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
            else
                ImGui::Text("Mouse Position: <invalid>");
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Top-left",     NULL, corner == 0)) corner = 0;
                if (ImGui::MenuItem("Top-right",    NULL, corner == 1)) corner = 1;
                if (ImGui::MenuItem("Bottom-left",  NULL, corner == 2)) corner = 2;
                if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
                if (Enable && ImGui::MenuItem("Close")) *Enable = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
}
//~ overlay


//~ BEGIN Dockspace


void
RenderImGuiMenu()
{
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    
    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    
    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;
    
    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", NULL, window_flags);
    ImGui::PopStyleVar();
    
    if (opt_fullscreen)
        ImGui::PopStyleVar(2);
    
    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    else
    {
        //ShowDockingDisabledMessage();
    }
    
    
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "") != 0)
            {
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            if (ImGui::MenuItem("Open Scene", "") != 0)
            {
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            if (ImGui::MenuItem("Save Scene", "") != 0)
            {
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Settings", "") != 0)
            {
                GlobalPopupSettings = true;
            }
            
            ImGui::Separator();
            
            
            if (ImGui::MenuItem("Restart", "") != 0)
            {
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            
            
            if (ImGui::MenuItem("Exit", "") != 0)
            {
                GlobalRunning = false;
            }
            
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Assets"))
        {
            
            if (ImGui::MenuItem("Create", "") != 0)
            {
                
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            
            
            
            if (ImGui::MenuItem("Show in Explorer", "") != 0)
            {
                
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            if (ImGui::MenuItem("Open", "") != 0)
            {
                
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            
            if (ImGui::MenuItem("Delete", "") != 0)
            {
                
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            if (ImGui::MenuItem("Rename", "") != 0)
            {
                
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            
            if (ImGui::MenuItem("Import New Asset...", "") != 0)
            {
                Win32MessageBoxError("TODO(Gabriel): IMPLEMENT ME: line %d, file %s, func %s", __LINE__, __FILE__, __func__);
            }
            
            
            
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Entities"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Component"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
    
    
    ImGui::End();
}

//~ END Dockspace


internal void
RenderSceneHierarchy()
{
    static ImGuiWindowFlags winflags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("Scene Hierarchy", NULL, winflags))
    {
        if (!GlobalScene)
        {
            ImGui::Text("NO SCENE LOADED");
            ImGui::End();
            return;
        }
        
        
        static bool OpenSceneRenamePopup = false;
        static ImGuiTreeNodeFlags SceneTreeFlags = ImGuiTreeNodeFlags_Leaf;
        
        ImGui::Unindent(20);
        
        if (ImGui::TreeNodeEx(GlobalScene->SceneName, SceneTreeFlags))
        {
            
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
            //static bool align_label_with_current_x_position = false;
            static bool align_label_with_current_x_position = true;
            static bool drag_and_drop = true;
            
            
            if (align_label_with_current_x_position)
            {
                //ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
            }
            
            
            static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
            int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3); // Increase spacing to differentiate leaves from expanded contents.
            
            
            for (s32 EntityIndex = 0; EntityIndex < GlobalScene->EntityCacheCount; EntityIndex++)
            {
                entity Entity = GlobalScene->EntityCache[EntityIndex];
                
                if (!Entity.IsDeleted)
                {
                    
                    // Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
                    ImGuiTreeNodeFlags node_flags = base_flags;
                    const bool is_selected = (selection_mask & (1 << EntityIndex)) != 0;
                    if (is_selected)
                    {
                        node_flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    
                    
                    if (Entity.HasChild)
                    {
                        // TODO(Gabriel): MAKE SURE THE ENTITY INDEX IS the CHILDS index!? AND NOT THE PARENT???
                        
                        
                        ImGui::Image(0, ImVec2(15, 15), ImVec2(0,0), ImVec2(1,1), ImVec4(1.0f,1.0f,1.0f,1.0f), ImVec4(1.0f,1.0f,1.0f,0.5f));
                        
                        ImGui::SameLine();
                        bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)EntityIndex, node_flags, Entity.Child->Name->String, EntityIndex);
                        
                        
                        if (ImGui::IsItemClicked())
                        {
                            node_clicked = EntityIndex;
                            GlobalEditorSelectedEntity = &GlobalScene->EntityCache[EntityIndex];
                            GlobalEditorSelectedEntityWasSelected = true;
                            GlobalEditorSelectedEntityWasSelectedTriggerOnce = true;
                        }
                        
                        
                        if (drag_and_drop && ImGui::BeginDragDropSource())
                        {
                            ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
                            ImGui::Text(Entity.Child->Name->String);
                            ImGui::EndDragDropSource();
                        }
                        
                        
                        //when an entity is uncollapsed
                        if (node_open)
                        {
                            ImGui::TreePop(); //tree pop first so the indentation is reset.
                            ImGui::Text("Entity Nesting: FIX ME");
                        }
                        
                    } else
                    {
                        // Items 3..5 are Tree Leaves
                        // The only reason we use TreeNode at all is to allow selection of the leaf.
                        // Otherwise we can use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().
                        
                        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
                        
                        static ImTextureID TextureID = (void*)1;
                        ImGui::Image(TextureID, ImVec2(15, 15), ImVec2(0,0), ImVec2(1,1), ImVec4(1.0f,1.0f,1.0f,1.0f), ImVec4(1.0f,1.0f,1.0f,0.5f));
                        
                        ImGui::SameLine(30.0f);
                        ImGui::TreeNodeEx((void*)(intptr_t)EntityIndex, node_flags, Entity.Name->String, EntityIndex);
                        
                        static int fas = 2;
                        if (fas > 0)
                        {
                            //ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing() + 15);
                            fas--;
                        }
                        
                        
                        if (ImGui::IsItemClicked())
                        {
                            node_clicked = EntityIndex;
                            
                            GlobalEditorSelectedEntity = &GlobalScene->EntityCache[EntityIndex];
                            GlobalEditorSelectedEntityWasSelected = true;
                            GlobalEditorSelectedEntityWasSelectedTriggerOnce = true;
                        }
                        
                        if (drag_and_drop && ImGui::BeginDragDropSource())
                        {
                            ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
                            ImGui::Text(Entity.Name->String);
                            ImGui::EndDragDropSource();
                        }
                    }
                    
                }
                
            }
            
            
            
            //right click popup menu over a selected entity
            static int Selection = 0;
            if (ImGui::BeginPopupContextWindow())
            {
                
                if (ImGui::MenuItem("Rename Scene"))
                {
                    //delete entity
                    OpenSceneRenamePopup = true;
                }
                
                ImGui::Separator();
                
                
                if (ImGui::MenuItem("Create empty Entity", NULL))
                {
                    CreateEntity(GlobalScene, NULL);
                }
                if (ImGui::MenuItem("Delete Entity", NULL))
                {
                    DeleteEntity(GlobalScene, GlobalEditorSelectedEntity);
                    GlobalEditorSelectedEntity = NULL;
                }
                if (ImGui::MenuItem("Import Entity...", NULL))
                {
                    char* Path = Win32OpenFileDialog(file_type_Entity, NULL);
                    CreateEntityFromFile(GlobalScene, Path);
                    
                    if (Path)
                    {
                        free(Path);
                    }
                }
                if (ImGui::MenuItem("Export Entity...", NULL))
                {
                    char*Path = Win32SaveFileDialog(file_type_Entity, NULL);
                    SerializeEntityToFile(GlobalEditorSelectedEntity, Path);
                    
                    if (Path != NULL)
                    {
                        free(Path);
                    }
                }
                
                if (ImGui::BeginMenu("Add Primitives"))
                {
                    if (ImGui::MenuItem("Sphere", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Sphere", PRIMITIVE_SPHERE);
                    }
                    if (ImGui::MenuItem("Plane", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Plane", PRIMITIVE_PLANE);
                    }
                    if (ImGui::MenuItem("Cube", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Cube", PRIMITIVE_CUBE);
                    }
                    if (ImGui::MenuItem("Cube Blender", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Cube Blender", PRIMITIVE_CUBE_BLENDER);
                    }
                    if (ImGui::MenuItem("Cylinder", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Cylinder", PRIMITIVE_CYLINDER);
                    }
                    if (ImGui::MenuItem("Cone", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Cone", PRIMITIVE_CONE);
                    }
                    if (ImGui::MenuItem("Icosphere", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Icosphere", PRIMITIVE_ICOSPHERE);
                    }
                    if (ImGui::MenuItem("Monkey", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Monkey", PRIMITIVE_MONKEY);
                    }
                    if (ImGui::MenuItem("Water", NULL))
                    {
                        CreateEntityPrimitive(GlobalScene, "Water", PRIMITIVE_WATER);
                    }
                    
                    ImGui::EndMenu();
                }
                
                //if (p_open && ImGui::MenuItem("Close")) p_open = false;
                ImGui::EndPopup();
            }
            
            
            //ignore this
            if (node_clicked != -1)
            {
                // Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
                if (ImGui::GetIO().KeyCtrl)
                    selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
                else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, this commented bit preserve selection when clicking on item that is part of the selection
                    selection_mask = (1 << node_clicked);           // Click to single-select
                
            }
            
            ImGui::PopStyleVar();
            
            
            //if (align_label_with_current_x_position)
            //ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
            
            //Scene Renaming
            if (OpenSceneRenamePopup)
            {
                ImGui::OpenPopup("Rename?");
                OpenSceneRenamePopup = false;
            }
            if (ImGui::BeginPopupModal("Rename?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                
                static bool DidChange = false;
                static char InputBuf[256];
                ImGui::InputText("New name", InputBuf, IM_ARRAYSIZE(InputBuf));
                
                
                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    sprintf_s(GlobalScene->SceneName, 256, "%s", InputBuf);
                    ImGui::CloseCurrentPopup();
                }
                
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            
            
            
            
            
            ImGui::TreePop();
        }
        
        
        
        
    }
    
    ImGui::Columns(1);
    ImGui::End();
    
    
}

//~ Scene Hiearchy

int
EntityNameChangeCallbackStub(ImGuiInputTextCallbackData* data)
{
    switch (data->EventFlag)
    {
        case ImGuiInputTextFlags_CallbackCompletion:
        {
            break;
        }
        case ImGuiInputTextFlags_CallbackHistory:
        {
            break;
        }
    }
    return 0;
}


internal void
RenderProperties()
{
    static ImGuiWindowFlags winflags = ImGuiWindowFlags_NoCollapse;
    if (GlobalPropertiesEnabled)
    {
        if (ImGui::Begin("Properties", &GlobalPropertiesEnabled, winflags))
        {
            
            //Entities
            if (GlobalEditorSelectedEntity && GlobalEditorSelectedEntityWasSelected)
            {
                //Header
                bool EntityEnabled = (bool)GlobalEditorSelectedEntity->Enabled;
                ImGui::Checkbox("Enabled", &EntityEnabled);
                GlobalEditorSelectedEntity->Enabled = EntityEnabled;
                ImGui::SameLine();
                ImGui::Text("\t\t\tUnique EntityID: %d",GlobalEditorSelectedEntity->EntityIndex);
                
                if (GlobalEditorSelectedEntity->Enabled)
                {
                    //Name
                    static char NameBuffer[64];
                    if (GlobalEditorSelectedEntityWasSelectedTriggerOnce)
                    {
                        sprintf_s(NameBuffer, 64, "%s", GlobalEditorSelectedEntity->Name->String);
                        GlobalEditorSelectedEntityWasSelectedTriggerOnce = false;
                    }
                    
                    
                    if (ImGui::InputText("EntityName", NameBuffer, 64))
                    {
                        sprintf_s(GlobalEditorSelectedEntity->Name->String, 64, "%s", NameBuffer);
                    }
                    
                    
                    
                    //Transform
                    if (ImGui::CollapsingHeader("Transform"))
                    {
                        glm::vec3* pos = &GlobalEditorSelectedEntity->Position;
                        glm::vec3* rot = &GlobalEditorSelectedEntity->Rotation;
                        glm::vec3* scale = &GlobalEditorSelectedEntity->Scale;
                        
                        float input_pos[3] = { pos->x, pos->y, pos->z };
                        if (EditorFloatEdit3("Position", input_pos, 0, 10.0f))
                        {
                            GlobalEditorSelectedEntity->Position = glm::vec3(input_pos[0], input_pos[1], input_pos[2]);
                        }
                        
                        float input_rot[3] = { rot->x, rot->y, rot->z };
                        if (EditorFloatEdit3("Rotation", input_rot, 0, 10.0f))
                        {
                            GlobalEditorSelectedEntity->Rotation = glm::vec3(input_rot[0], input_rot[1], input_rot[2]);
                        }
                        
                        float input_scale[3] = { scale->x, scale->y, scale->z };
                        if (EditorFloatEdit3("Scale", input_scale, 0, 10.0f))
                        {
                            GlobalEditorSelectedEntity->Scale = glm::vec3(input_scale[0], input_scale[1], input_scale[2]);
                        }
                        
                    }
                    
                    /*
                    //Components
                    size_t size = SelectedGameObject->Components.Get().size();
                    for (size_t i = 0; i < size; ++i)
                    {
                        auto& component = SelectedGameObject->Components[i];
                        if (ImGui::CollapsingHeader(component.ComponentName.c_str(), &component.Alive))
                        {
                            component.OnImGuiRender(*SelectedGameObject);
                        }
                        
                        if (!component.Alive)
                        {
                            SelectedGameObject->RemoveComponent(&component);
                        }
                        
                        if (size != SelectedGameObject->Components.Get().size())
                        {
                            --i;
                            size = SelectedGameObject->Components.Get().size();
                        }
                    }
                    */
                    
                    
                    /*
                    if (ImGui::Button("Add Component"))
                    {
                        ImGui::OpenPopup("Components");
                    }
                    */
                    
                    /*
                    //open menu
                    if (ImGui::BeginMenu("Components"))
                    {
                        static bool n_menuitem_script_selected = false;
                        if (ImGui::MenuItem("Script", NULL, n_menuitem_script_selected))
                        {
                            //SelectedGameObject->AddComponent<ScriptComponent>();
                        }
        
                        static bool n_menuitem_shader_selected = false;
                        if (ImGui::MenuItem("Shader", NULL, n_menuitem_shader_selected))
                        {
                            //SelectedGameObject->AddComponent<ShaderComponent>();
                        }
        
                        ImGui::EndMenu();
                    }
                    */
                    
                    // Simple selection popup
                    // (If you want to show the current selection inside the Button itself, you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
                    //
                    
                    /*
                    ImGui::SameLine();
                    static CrystalFlask::GameObjectComponent::GameObjectComponentType n_selected_game_object_component;
                    if (ImGui::BeginPopup("Components"))
                    {
                        ImGui::Text("Available Components");
                        ImGui::Separator();
                        for (unsigned int i = 0; i < CrystalFlask::GameObjectComponent::AvailableGameObjectComponentsNameList.size(); i++)
                            if (ImGui::Selectable(CrystalFlask::GameObjectComponent::AvailableGameObjectComponentsNameList[i]))
                        {
                            n_selected_game_object_component = CrystalFlask::GameObjectComponent::AvailableGameObjectComponentsList[i];
                            SelectedGameObject->AddComponentOfType(n_selected_game_object_component);
                            //CF_CORE_FATAL("Selected {0}", CrystalFlask::GameObjectComponent::AvailableGameObjectComponentsNameList[i]);
                        }
                        ImGui::EndPopup();
                    }
                    
                    */
                }
                
            }
            
        }
        ImGui::End();
    }
}

//~ Scene properties


internal void
RenderSettings()
{
    
    if (GlobalPopupSettings)
    {
        static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;
        
        if (ImGui::Begin("Settings", &GlobalPopupSettings, window_flags))
        {
            
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("SettingsTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Editor"))
                {
                    ImGui::SliderFloat("EditorGridPlaneScale", &EditorGridPlaneScale, 0.001f, 100.0f);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Camera"))
                {
                    {
                        // TODO(Gabriel): REMOVE THIS, I don't like having these in here. maybe they should be included in program settings instead of camera settings?
                        r32 Width = (float)GlobalWidth;
                        r32 Height = (float)GlobalHeight;
                        ImGui::SliderFloat("Width", &Width, 1.0f, 1920.0f);
                        ImGui::SliderFloat("Height", &Height, 1.0f, 1080.0f);
                    }
                    
                    ImGui::SliderFloat("Fov", &Fov, 0.001f, 179.9f);
                    ImGui::SliderFloat("ZNear", &ZNear, 0.001f, 10000.0f);
                    ImGui::SliderFloat("ZFar", &ZFar, 0.001f, 10000.0f);
                    r32 imguiPosition3[3] = {CameraPosition[0], CameraPosition[1], CameraPosition[2]};
                    ImGui::SliderFloat3("Position", imguiPosition3, 0.001f, 10.0f);
                    CameraPosition[0] = imguiPosition3[0];
                    CameraPosition[1] = imguiPosition3[1];
                    CameraPosition[2] = imguiPosition3[2];
                    r32 imgui_camera_rotation[3] = {CameraRotation[0], CameraRotation[1], CameraRotation[2]};
                    ImGui::SliderFloat3("Rotation", imgui_camera_rotation, 0.001f, 10.0f);
                    CameraRotation[0] = imgui_camera_rotation[0];
                    CameraRotation[1] = imgui_camera_rotation[1];
                    CameraRotation[2] = imgui_camera_rotation[2];
                    ImGui::SliderFloat("Pitch", &Pitch, 0.001f, 50.0f);
                    ImGui::SliderFloat("Yaw", &Yaw, 0.001f, 50.0f);
                    
                    ImGui::EndTabItem();
                }
                
                ImGui::EndTabBar();
            }
            
            
            
        }
        ImGui::End();
    }
    
}
//~ editor settings

internal void
SubmitImGuiComponents()
{
    if (GlobalIsEditorEnabled)
    {
        RenderImGuiMenu();
        
        if (GlobalIsConsoleVisible)
            CrystalFlaskConsole.Draw("Console", NULL);
        
        //ImGui::ShowDemoWindow(NULL);
        ShowInfoOverlay(&GlobalIsInfoOverlayVisible);
        RenderSceneHierarchy();
        RenderProperties();
        RenderSettings();
    }
}

