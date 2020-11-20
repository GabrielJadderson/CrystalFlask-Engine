#pragma once


#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
    
    
#ifndef COMPILER_MSVC
#define COMPILER_MSVC 0
#endif
    
#ifndef COMPILER_LLVM
#define COMPILER_LLVM 0
#endif
    
#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
    
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif
    
#if COMPILER_MSVC
#include <intrin.h>
#endif
    
    
    typedef int8_t s8;
    typedef int16_t s16;
    typedef int32_t s32;
    typedef int64_t s64;
    
    typedef int32_t b32;
    
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    
    typedef size_t memory_index;
    
    typedef float r32;
    typedef double r64;
    
    typedef uintptr_t umm;
    typedef intptr_t smm;
    
#define internal static
#define local_persist static
#define global_variable static
    
#define Pi32 3.14159265359f
    
    
    
    
#if CRYSTALFLASK_DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif
    
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)
    
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
    
    //~ BEGIN Memory
    /*
    typedef struct {
        memory_index Used;
        memory_index Size;
        void *Base;
    } memory;
    */
    struct memory_arena
    {
        memory_index Size;
        u8 *Base;
        memory_index Used;
    };
    
    internal void InitializeArena(memory_arena *Arena, memory_index Size, u8 *Base)
    {
        Arena->Size = Size;
        Arena->Base = Base;
        Arena->Used = 0;
    }
    
    
    inline void CopyMemory(void *Destination, void* Source, u64 Length)
    {
        u8 *Dest = (u8*)Destination;
        u8 *Src = (u8*)Source;
        while (Length--)
            *Dest++ = *Src++;
    }
    
    inline void ZeroMemory(void *Source, u64 Length)
    {
        u8 *Src = (u8*)Source;
        while (Length--)
            *Src++ = 0;
    }
    
    
    internal void *PushMemory(memory_arena *Memory, memory_index Amount) {
        void *Result = 0;
        if ((Memory->Used + Amount) < Memory->Size) {
            Result = (u8 *)Memory->Base + Memory->Used;
            Memory->Used += Amount;
        }
        return Result;
    }
    
#define PopStruct(Arena, type) (void)PopSize_(Arena, sizeof(type))
#define PopArray(Arena, Count, type) (void)PopSize_(Arena, (Count)*sizeof(type))
#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
    void *
        PushSize_(memory_arena *Arena, memory_index Size)
    {
        Assert((Arena->Used + Size) <= Arena->Size);
        void *Result = Arena->Base + Arena->Used;
        Arena->Used += Size;
        
        return Result;
    }
    
    void PopSize_(memory_arena *Arena, memory_index Size)
    {
        Assert((Arena->Used - Size) >= 0);
        Arena->Used -= Size;
    }
    
    
    //~ END Memory
    
    
    inline b32 IsEndOfLine(char C)
    {
        b32 Result = ((C == '\n') ||
                      (C == '\r'));
        
        return Result;
    }
    
    inline b32 IsWhitespace(char C)
    {
        b32 Result = ((C == ' ') ||
                      (C == '\t') ||
                      (C == '\v') ||
                      (C == '\f') ||
                      IsEndOfLine(C));
        
        return Result;
    }
    
    inline b32 StringCMP(u64 Length, const char *A, const char *B)
    {
        b32 Result = false;
        for (u64 Index = 0; Index < Length; ++Index)
            if (A[Index] != B[Index])
            return true;
        return Result;
    }
    
    inline b32 StringsAreEqual(u64 ALength, const char *A, const  char *B)
    {
        b32 Result = false;
        
        if(B)
        {
            const char *At = B;
            for(u64 Index = 0;
                Index < ALength;
                ++Index, ++At)
            {
                if((*At == 0) ||
                   (A[Index] != *At))
                {
                    return(false);
                }
            }
            
            Result = (*At == 0);
        }
        else
        {
            Result = (ALength == 0);
        }
        
        return Result;
    }
    
    
    internal void
        CatStrings(size_t SourceACount, char *SourceA,
                   size_t SourceBCount, char *SourceB,
                   size_t DestCount, char *Dest)
    {
        
        for(int Index = 0;
            Index < SourceACount;
            ++Index)
        {
            *Dest++ = *SourceA++;
        }
        
        for(int Index = 0;
            Index < SourceBCount;
            ++Index)
        {
            *Dest++ = *SourceB++;
        }
        
        *Dest++ = 0;
    }
    
#define CharIsDigit(C) (C >= '0' && C <= '9')
    
    inline internal r32 StringToFloat(char *String) {
        return (r32)atof(String);
    }
    
    
    inline internal u64 StringLength(char *String)
    {
        u64 Count = 0;
        while(*String++)
        {
            ++Count;
        }
        return(Count);
    }
    
    
    inline internal u32 StringToUInt(char *String) {
        u32 Value = 0;
        if (StringLength(String))
            Value = (u32)strtoul(String, 0, 0);
        return Value;
    }
    
    
    inline internal u64 StringLengthConst(const char *String)
    {
        const char *S;
        for (S = String; *S; ++S)
            ;
        return (S - String);
    }
    
    
    inline void ZeroMem(s8 *Data, u64 ALength)
    {
        for(u64 Index = 0; Index < ALength; Index++)
        {
            Data[Index] = 0;
        }
    }
    
    
    internal s32 S32FromZInternal(char **AtInit)
    {
        s32 Result = 0;
        
        char *At = *AtInit;
        while((*At >= '0') &&
              (*At <= '9'))
        {
            Result *= 10;
            Result += (*At - '0');
            ++At;
        }
        
        *AtInit = At;
        
        return Result;
    }
    
    internal s32 S32FromZ(char *At)
    {
        char *Ignored = At;
        s32 Result = S32FromZInternal(&At);
        return Result;
    }
    
    
    inline u32 SafeTruncateUInt64(u64 Value)
    {
        Assert(Value <= 0xFFFFFFFF);
        u32 Result = (u32)Value;
        return Result;
    }
    
    
    
    typedef struct thread_context
    {
        int Placeholder;
    } thread_context;
    
    
#if CRYSTALFLASK_INTERNAL
    
    typedef struct debug_read_file_result
    {
        u32 ContentsSize;
        void *Contents;
    } debug_read_file_result;
    
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
    typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);
    
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, char *Filename)
    typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
    
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(thread_context *Thread, char *Filename, u32 MemorySize, void *Memory)
    typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
    
#endif
    
    
    typedef struct game_offscreen_buffer
    {
        void *Memory;
        s32 Width;
        s32 Height;
        s32 Pitch;
        s32 BytesPerPixel;
    } game_offscreen_buffer;
    
    typedef struct game_sound_output_buffer
    {
        s32 SamplesPerSecond;
        s32 SampleCount;
        s16 *Samples;
    } game_sound_output_buffer;
    
    typedef struct game_button_state
    {
        s32 HalfTransitionCount;
        b32 EndedDown;
    } game_button_state;
    
    typedef struct game_keyboard_state
    {
        s32 HalfTransitionCount;
        b32 EndedDown;
        u32 VK;
    } game_keyboard_state;
    
    typedef struct game_controller_input
    {
        b32 IsConnected;
        b32 IsAnalog;
        r32 StickAverageX;
        r32 StickAverageY;
        
        game_keyboard_state Key;
        
        union
        {
            game_button_state Buttons[12];
            struct
            {
                
                game_button_state MoveUp;
                game_button_state MoveDown;
                game_button_state MoveLeft;
                game_button_state MoveRight;
                
                game_button_state ActionUp;
                game_button_state ActionDown;
                game_button_state ActionLeft;
                game_button_state ActionRight;
                
                game_button_state LeftShoulder;
                game_button_state RightShoulder;
                
                game_button_state Back;
                game_button_state Start;
                
                //
                
                game_button_state Terminator;
            };
        };
    } game_controller_input;
    
    
    
    typedef struct game_input
    {
        float MouseWheel;
        float MouseWheelH;
        game_button_state MouseButtons[5];
        s32 MouseX, MouseY, MouseZ;
        
        r32 dtForFrame;
        
        game_controller_input Controllers[5];
    } game_input;
    
    typedef struct game_memory
    {
        b32 IsInitialized;
        
        u64 PermanentStorageSize;
        void *PermanentStorage;
        
        u64 TransientStorageSize;
        void *TransientStorage;
        
        u64 TempStorageSize;
        void *TempStorage;
        
        u64 StringStorageSize;
        void *StringStorage;
        
        debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
        debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
        debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
    } game_memory;
    
    
    
#define GAME_UPDATE_AND_RENDER(name) void name(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
    
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
    
#define GAME_GET_SOUND_SAMPLES(name) void name(thread_context *Thread, game_memory *Memory, game_sound_output_buffer *SoundBuffer)
    typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
    
    //~ Game.dll
    
    inline game_controller_input*
        GetController(game_input *Input, int unsigned ControllerIndex)
    {
        Assert(ControllerIndex < ArrayCount(Input->Controllers));
        
        game_controller_input *Result = &Input->Controllers[ControllerIndex];
        return(Result);
    }
    
#ifdef __cplusplus
}
#endif

