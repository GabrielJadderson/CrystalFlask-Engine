#ifndef CRYSTALFLASK_STRING_H
#define CRYSTALFLASK_STRING_H



struct string
{
    char* StringBuffer;
    u64 StringBufferLength;
    
    string(char* InputString)
    {
        StringBufferLength = StringLength(InputString);
        StringBuffer = (char*) PushArray(&GlobalResourceArena, StringBufferLength, char);
        CopyMemory(StringBuffer, InputString, StringBufferLength);
    }
    ~string()
    {
        ZeroMemory(StringBuffer, StringBufferLength);
        //PopArray(this.StringBuffer, this.StringBufferLength);
    }
};



#endif //CRYSTALFLASK_STRING_H
