#include "crystalflask_network.h"
#include "win32_network.cpp"

struct network_packet
{
    
};

struct network_message
{
    network_packet Packets[256];
    
};

global_variable b32 GlobalNetworkIsRunning = true;

DWORD WINAPI
NetworkThreadStartProc(LPVOID lpParam)
{
    StartWSA();
    
    /*
    while (GlobalNetworkIsRunning)
    {
        if (true)
        {
        }
        else
        {
            Sleep(33);
        }
    }
    */
    
    WSACleanup();
    
    return 0;
}

internal void
NetworkStart()
{
    //DWORD ThreadID;
    //HANDLE ThreadHandle = CreateThread(0, 0, NetworkThreadStartProc, 0, 0, &ThreadID);
}

internal network_message*
NetworkGetMessage()
{
    //return message from queue.
    return NULL;
}

