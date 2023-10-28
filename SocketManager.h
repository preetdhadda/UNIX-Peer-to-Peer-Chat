#ifndef _SOCKET_MANAGER_H_
#define _SOCKET_MANAGER_H_

void SocketManager_Init(int LocalPort, char* RemoteMachine, char* RemotePort); 

int GetSocket(); 

struct addrinfo* GetRemoteAddressInfo();

void SocketManager_Shutdown();

#endif