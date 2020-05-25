#include <winsock2.h>
#include <mstcpip.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <rpc.h>
#include <ntdsapi.h>
#include <stdio.h>
#include <tchar.h>

#ifndef UNICODE
#define UNICODE
#endif

#define RECV_DATA_BUF_SIZE 256

global_variable WSADATA GlobalWSAData;


// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
// link with fwpuclnt.lib for Winsock secure socket extensions
#pragma comment(lib, "fwpuclnt.lib")
// link with ntdsapi.lib for DsMakeSpn function
#pragma comment(lib, "ntdsapi.lib")



/// Server
//1. Initialize Winsock.
//2. Create a socket.
//3. Bind the socket.
//4. Listen on the socket for a client.
//5. Accept a connection from a client.
//6. Receive and send data.
//7. Disconnect.

/// Client
//1. Initialize Winsock.
//2. Create a socket.
//3. Connect to the server.
//4. Send and receive data.
//5. Disconnect.


internal void
StartWSA()
{
    int Result = WSAStartup(MAKEWORD(2,2), &GlobalWSAData);
    if (Result != 0)
    {
        char ErrorBuffer[60];
        sprintf_s(ErrorBuffer, 60, "WSAStartup failed: %d\n", Result);
        MessageBoxA(NULL, ErrorBuffer,
                    NULL, MB_OK | MB_ICONEXCLAMATION);
    }
}


// The following function assumes that Winsock
// has already been initialized
int
SecureTcpConnect(IN const struct sockaddr *serverAddr,
                 IN ULONG serverAddrLen,
                 IN const wchar_t * serverSPN,
                 IN const SOCKET_SECURITY_SETTINGS * securitySettings,
                 IN ULONG settingsLen)
/**
Routine Description:

    This routine creates a TCP client socket, securely connects to the
    specified server, sends & receives data from the server, and then closes
    the socket

Arguments:

    serverAddr - a pointer to the sockaddr structure for the server.

    serverAddrLen - length of serverAddr in bytes

    serverSPN - a NULL-terminated string representing the SPN
               (service principal name) of the server host computer

    securitySettings - pointer to the socket security settings that should be
                       applied to the connection

    serverAddrLen - length of securitySettings in bytes

Return Value:

    Winsock error code indicating the status of the operation, or NO_ERROR if
    the operation succeeded.

--*/
{
    int iResult = 0;
    int sockErr = 0;
    SOCKET sock = INVALID_SOCKET;
    
    WSABUF wsaBuf = { 0 };
    char *dataBuf = "12345678";
    DWORD bytesSent = 0;
    char recvBuf[RECV_DATA_BUF_SIZE] = { 0 };
    
    DWORD bytesRecvd = 0;
    DWORD flags = 0;
    SOCKET_PEER_TARGET_NAME *peerTargetName = NULL;
    DWORD serverSpnStringLen = (DWORD) wcslen(serverSPN);
    DWORD peerTargetNameLen = sizeof (SOCKET_PEER_TARGET_NAME) +
        (serverSpnStringLen * sizeof (wchar_t));
    
    //-----------------------------------------
    // Create a TCP socket
    sock = WSASocket(serverAddr->sa_family,
                     SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sock == INVALID_SOCKET) {
        iResult = WSAGetLastError();
        wprintf(L"WSASocket returned error %ld\n", iResult);
        goto cleanup;
    }
    //-----------------------------------------
    // Turn on security for the socket.
    sockErr = WSASetSocketSecurity(sock,
                                   securitySettings, settingsLen, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSASetSocketSecurity returned error %ld\n", iResult);
        goto cleanup;
    }
    
    //-----------------------------------------
    // Specify the server SPN
    peerTargetName = (SOCKET_PEER_TARGET_NAME *) HeapAlloc(GetProcessHeap(),
                                                           HEAP_ZERO_MEMORY, peerTargetNameLen);
    if (!peerTargetName) {
        iResult = ERROR_NOT_ENOUGH_MEMORY;
        wprintf(L"Out of memory\n");
        goto cleanup;
    }
    // Use the security protocol as specified by the settings
    peerTargetName->SecurityProtocol = securitySettings->SecurityProtocol;
    // Specify the server SPN
    peerTargetName->PeerTargetNameStringLen = serverSpnStringLen;
    RtlCopyMemory((BYTE *) peerTargetName->AllStrings,
                  (BYTE *) serverSPN, serverSpnStringLen * sizeof (wchar_t)
                  );
    
    sockErr = WSASetSocketPeerTargetName(sock,
                                         peerTargetName,
                                         peerTargetNameLen, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSASetSocketPeerTargetName returned error %ld\n", iResult);
        goto cleanup;
    }
    
    
    //-----------------------------------------
    // Connect to the server
    sockErr = WSAConnect(sock,
                         serverAddr, serverAddrLen, NULL, NULL, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSAConnect returned error %ld\n", iResult);
        goto cleanup;
    }
    // At this point a secure connection must have been established.
    wprintf(L"Secure connection established to the server\n");
    
    //-----------------------------------------
    // Send some data securely
    wsaBuf.len = (ULONG) strlen(dataBuf);
    wsaBuf.buf = dataBuf;
    sockErr = WSASend(sock, &wsaBuf, 1, &bytesSent, 0, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSASend returned error %ld\n", iResult);
        goto cleanup;
    }
    wprintf(L"Sent %d bytes of data to the server\n", bytesSent);
    
    //-----------------------------------------
    // Receive server's response securely
    wsaBuf.len = RECV_DATA_BUF_SIZE;
    wsaBuf.buf = recvBuf;
    sockErr = WSARecv(sock, &wsaBuf, 1, &bytesRecvd, &flags, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSARecv returned error %ld\n", iResult);
        goto cleanup;
    }
    wprintf(L"Received %d bytes of data from the server\n", bytesRecvd);
    
    cleanup:
    if (sock != INVALID_SOCKET) {
        //This will trigger the cleanup of all IPsec filters and policies that
        //were added for this socket. The cleanup will happen only after all
        //outstanding data has been sent out on the wire.
        closesocket(sock);
    }
    if (peerTargetName) {
        HeapFree(GetProcessHeap(), 0, peerTargetName);
    }
    return iResult;
}

struct win32_network_instance
{
    SOCKET Socket = INVALID_SOCKET;
    WSABUF wsaBuf = {0};
    char recvBuf[RECV_DATA_BUF_SIZE] = {0};
    DWORD bytesSent = 0;
    DWORD bytesRecvd = 0;
};

internal win32_network_instance
Win32CreateSecureSocket(IN const struct sockaddr *serverAddr,
                        IN ULONG serverAddrLen,
                        IN const wchar_t * serverSPN,
                        IN const SOCKET_SECURITY_SETTINGS * securitySettings,
                        IN ULONG settingsLen)
{
    int Result = 0;
    int sockErr = 0;
    
    char *dataBuf = "12345678";
    
    win32_network_instance NetworkInstance = {};
    
    DWORD flags = 0;
    SOCKET_PEER_TARGET_NAME *peerTargetName = NULL;
    DWORD serverSpnStringLen = (DWORD) wcslen(serverSPN);
    DWORD peerTargetNameLen = sizeof (SOCKET_PEER_TARGET_NAME) +
        (serverSpnStringLen * sizeof (wchar_t));
    
    //-----------------------------------------
    // Create a TCP socket
    NetworkInstance.Socket = WSASocket(serverAddr->sa_family,
                                       SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (NetworkInstance.Socket == INVALID_SOCKET) {
        Result = WSAGetLastError();
        Win32MessageBoxError("WSASocket returned error %ld\n", Result);
        goto cleanup;
    }
    //-----------------------------------------
    // Turn on security for the socket.
    sockErr = WSASetSocketSecurity(NetworkInstance.Socket,
                                   securitySettings,settingsLen, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        Result = WSAGetLastError();
        Win32MessageBoxError("WSASetSocketSecurity returned error %ld\n", Result);
        goto cleanup;
    }
    
    //-----------------------------------------
    // Specify the server SPN
    peerTargetName = (SOCKET_PEER_TARGET_NAME *) HeapAlloc(GetProcessHeap(),
                                                           HEAP_ZERO_MEMORY, peerTargetNameLen);
    if (!peerTargetName) {
        Result = ERROR_NOT_ENOUGH_MEMORY;
        Win32MessageBoxError("Out of memory");
        goto cleanup;
    }
    // Use the security protocol as specified by the settings
    peerTargetName->SecurityProtocol = securitySettings->SecurityProtocol;
    // Specify the server SPN
    peerTargetName->PeerTargetNameStringLen = serverSpnStringLen;
    RtlCopyMemory((BYTE *) peerTargetName->AllStrings,
                  (BYTE *) serverSPN, serverSpnStringLen * sizeof (wchar_t)
                  );
    
    sockErr = WSASetSocketPeerTargetName(NetworkInstance.Socket,
                                         peerTargetName,
                                         peerTargetNameLen, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        Result = WSAGetLastError();
        Win32MessageBoxError("WSASetSocketPeerTargetName returned error %ld\n", Result);
        goto cleanup;
    }
    
    
    //-----------------------------------------
    // Connect to the server
    sockErr = WSAConnect(NetworkInstance.Socket,
                         serverAddr,
                         serverAddrLen,
                         NULL,
                         NULL,
                         NULL,
                         NULL);
    if (sockErr == SOCKET_ERROR) {
        Result = WSAGetLastError();
        Win32MessageBoxError("WSAConnect returned error %ld\n", Result);
        goto cleanup;
    }
    // At this point a secure connection must have been established.
    wprintf(L"Secure connection established to the server\n");
    
    return NetworkInstance;
    
    cleanup:
    if (NetworkInstance.Socket != INVALID_SOCKET) {
        //This will trigger the cleanup of all IPsec filters and policies that
        //were added for this socket. The cleanup will happen only after all
        //outstanding data has been sent out on the wire.
        closesocket(NetworkInstance.Socket);
    }
    if (peerTargetName) {
        HeapFree(GetProcessHeap(), 0, peerTargetName);
    }
}