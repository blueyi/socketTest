#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

//需要连接的库
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")



//远程端口
#define DEFAULT_PORT  "27015"
//默认缓冲区大小
#define DEFAULT_BUFLEN 512

int __cdecl main(int argc, char **argv)
{
	//创建WSADATA对象，WSADATA中包含windows sockets 的实现信息
	WSADATA wsaData;
	// 声明连接到服务端的socket对象
	SOCKET ConnectSocket = INVALID_SOCKET;
	//声明addrinfo对象，其中包括sockaddr结构体值
	//result和ptr将指向相应的主机地址信息,hints存放socket属性信息
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	//向服务器发送的数据内容
	char *sendbuf = "Hello Server";
	//设置接收数据的缓冲区大小
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	//确定参数是否有效
	if (argc != 2) {
		printf("usage: %s sever-name\n", argv[0]);
		return 1;
	}

	// 初始化winsock 
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	//为hints分配空间
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//指定服务器地址和端口,argv[1]为IPV4或者IPV6地址
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	//尝试连接到服务器直到连接成功
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		//初始化连接到服务器的socket对象
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		//创建失败则释放资源
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}
		//连接到服务器
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	//连接完成后释放地址信息占用的内存
	freeaddrinfo(result);
	//如果连接失败则返回
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	//发送数据
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	//发送失败则关闭连接
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	printf("Byte Sent: %ld\n", iResult);

	//当没有数据需要发送时，首先关闭当前用于发送数据的socket,以便服务端释放资源，并将该socket用于接收数据
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//从服务端接收数据直接服务器关闭连接
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			recvbuf[iResult] = '\0';
			printf("Bytes received: %s\n", recvbuf);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);

	//当接收完数据之后关闭socket并释放连接资源
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
