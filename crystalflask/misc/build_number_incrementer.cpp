#include <stdio.h>
#include <iostream>

int main(int argc, char** argv) 
{
    FILE* file = fopen(argv[1], "r+b");
    //printf("%s\n", argv[1]);
    unsigned long long ReadCount = 0;
    int BuildNumber = 0;
    ReadCount = fread(&BuildNumber, sizeof(int), 1, file);
    //printf("read %llu bytes.\nBuildNumber:%d\n", ReadCount, BuildNumber);
    
    BuildNumber++;
    fseek(file, 0, SEEK_SET);
    fwrite(&BuildNumber, sizeof(int), 1, file);
    //printf("Wrote to file sucessfully\n");
    
    fclose(file);
}