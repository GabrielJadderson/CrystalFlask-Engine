// TODO(Gabriel): add command buffer and move this entire function into crystalflask_imgui, the only reason it's here is because we can't call engine functions or entity functions before they've been resolved.

void
crystalflask_console::ExecCommand(const char* command_line)
{
    AddLog("# %s\n", command_line);
    
    // Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
    HistoryPos = -1;
    for (int i = History.Size-1; i >= 0; i--)
        if (Stricmp(History[i], command_line) == 0)
    {
        free(History[i]);
        History.erase(History.begin() + i);
        break;
    }
    History.push_back(Strdup(command_line));
    
    
    
    
    // Process command
    if (Stricmp(command_line, "CLEAR") == 0)
    {
        ClearLog();
    }
    else if (Stricmp(command_line, "HELP") == 0)
    {
        AddLog("Commands:");
        for (int i = 0; i < Commands.Size; i++)
            AddLog("- %s", Commands[i]);
    }
    
    else if (Stricmp(command_line, "HISTORY") == 0)
    {
        int first = History.Size - 10;
        for (int i = first > 0 ? first : 0; i < History.Size; i++)
            AddLog("%3d: %s\n", i, History[i]);
    }
    
    else if (Stricmp(command_line, "monkeytest") == 0)
    {
        for (u32 Z = 10;  Z < 15; Z+=2)
        {
            for (u32 Y = 10;  Y < 100; Y+=2)
            {
                for (u32 X = 10;  X < 100; X+=2)
                {
                    char NameBuffer[25];
                    sprintf_s(NameBuffer, 25, "Monkey [X:%d, Y:%d, Z%d]", X, Y, Z);
                    entity* en = CreateEntityPrimitive(GlobalScene, NameBuffer, PRIMITIVE_MONKEY);
                    en->Position = glm::vec3{X, Z, Y};
                }
            }
        }
    }
    
    else if (Stricmp(command_line, "delete_all_entities") == 0)
    {
        DeleteAllEntitiesInScene(GlobalScene);
    }
    
    else if (Stricmp(command_line, "entitycount") == 0)
    {
        if (GlobalScene)
        {
            AddLog("%d\n", GlobalScene->EntityCount);
        }
    }
    
    
    else if (Stricmp(command_line, "EXIT") == 0)
    {
        // TODO(Gabriel): clean exit.
        exit(0);
    }
    
    else
    {
        AddLog("Unknown command: '%s'\n", command_line);
    }
    
    
    
    // On commad input, we scroll to bottom even if AutoScroll==false
    ScrollToBottom = true;
}

