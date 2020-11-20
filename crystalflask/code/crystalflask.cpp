#include "crystalflask.h"

#include "gl3w.h"
#include "gl3w.c"

//this is the game layer
global_variable game_state *GlobalGameState = NULL;
global_variable b32 GlobalGameInitialized = false;

internal void
StartupGame(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    if (!GlobalGameInitialized)
    {
        
        //init opengl if not already opened
        if (glViewport == NULL)
        {
            gl3wInit();
        }
        
        GlobalGameInitialized = true;
    }
    
    
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    
    GlobalGameState = (game_state *)Memory->PermanentStorage;
    
    if(!Memory->IsInitialized)
    {
        
        InitializeArena(&GlobalGameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
                        (u8 *)Memory->PermanentStorage + sizeof(game_state));
        
        GlobalGameState->World = PushStruct(&GlobalGameState->WorldArena, world);
        
        
        Memory->IsInitialized = true;
    }
    
    
}

internal void
GameInput(game_input* Input)
{
    
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog)
        {
        }
        else
        {
            if(Controller->MoveUp.EndedDown)
            {
            }
            if(Controller->MoveDown.EndedDown)
            {
            }
            if(Controller->MoveLeft.EndedDown)
            {
            }
            if(Controller->MoveRight.EndedDown)
            {
            }
            
        }
    }
    
    
}



extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    StartupGame(Memory, Input, Buffer);
    GameInput(Input);
    
    
    /*
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    */
    
}



extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    //GameOutputSound(GameState, SoundBuffer, 400);
}
