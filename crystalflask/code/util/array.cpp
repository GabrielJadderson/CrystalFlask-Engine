
template<typename T>
struct array
{
    T *Data; //do not index into this directly!
    T *DataPointer; //use this to advance and index.
    
    u64 CurrentIndex;
    u64 Count;
    u64 SpaceLeft;
    u64 DataPointerAdvancement;
    u64 MaxSize;
    memory_arena *MemoryArenaPointer;
    
    // TODO(Gabriel): cleanup: too many u64's...
    array(u64 Size) //try to always use a power of two for the size, to speed things up.
    {
        CurrentIndex = 0;
        Count = 0;
        SpaceLeft = Size;
        MaxSize = Size;
        //temp for now // TODO(Gabriel): temporary for now make one big arena for arrays.
        MemoryArenaPointer = &GlobalOpenGLArena;
        Data = (T*)PushSize_(MemoryArenaPointer, MaxSize*sizeof(T));
        if (!Data)
        {
            Win32MessageBoxError("Failed to allocate array, Memory allocator was probably not initialized. %d, %s, %s", __LINE__, __FILE__, __func__);
        }
        DataPointer = Data;
        DataPointerAdvancement = 0;
    }
    
    array() : array(32)
    {
        
    }
    
    ~array()
    {
        ZeroOut();
    }
    
    
    void ZeroOut()
    {
        // TODO(Gabriel): SIMD the hell out of this bro.
        
        u64 TotalBytes = (MaxSize * sizeof(T));
        
        if (TotalBytes % 8 == 0) //take the fast lane -> 8 bytes at a time
        {
            u64 Iterations = TotalBytes / 8;
            
            for(u64 Index = 0; Index < Iterations; Index++)
            {
                u64* cdata = (u64*)Data;
                cdata[Index] = 0;
            }
        }
        else
        {
            TotalBytes = (Count * sizeof(T));
            
            for(u64 Index = 0; Index < TotalBytes;  Index++) //use the count! no need to go to MaxSize
            {
                char* cdata = (char*)Data;
                cdata[Index] = 0;
            }
        }
        
    }
    
    T& operator[](u64 Index)
    {
        return DataPointer[Index];
    }
    
    //O(1)
    T Get(u64 Index)
    {
        if (Index <= Count && Index >= 0)
        {
            return DataPointer[Index];
        }
        return NULL;
    }
    
    
    //O(1)
    T At(u64 Index)
    {
        if (Index <= Count && Index >= 0)
        {
            return DataPointer[Index];
        }
        return NULL;
    }
    
    
    //O(1)
    void Push(T Element)
    {
        //resize and move everything
        if ((DataPointerAdvancement - 1) >= MaxSize)
        {
            for (u64 Index = 0; Index < Count; Index++)
            {
                Data[Index] = DataPointer[Index];
            }
            DataPointer = Data;
            DataPointerAdvancement = 0;
        }
        
        
        //add
        if (SpaceLeft > 0)
        {
            Count++;
            SpaceLeft--;
            DataPointer[CurrentIndex++] = Element;
        }
        else
        {
            //resize?/
        }
    }
    
    //O(1)
    //pops from the back
    T& Pop()
    {
        if (Count > 0)
        {
            Count--;
            SpaceLeft++;
            return DataPointer[--CurrentIndex];
        }
        
        return DataPointer[0];
    }
    
    //O(n)
    void PushInFront()
    {
        
    }
    
    //O(1)
    T PopInFront()
    {
        //resize and move everything
        if ((DataPointerAdvancement - 1) >= MaxSize)
        {
            for (u64 Index = 0; Index < Count; Index++)
            {
                Data[Index] = DataPointer[Index];
            }
            DataPointer = Data;
            DataPointerAdvancement = 0;
        }
        
        
        //Get first element
        T Result = DataPointer[0];
        Count--;
        CurrentIndex--;
        
        //move reference
        DataPointer = DataPointer + 1;
        DataPointerAdvancement++;
        SpaceLeft++;
        
        return Result;
    }
    
};