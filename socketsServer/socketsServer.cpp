#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

//需要连接的库
#pragma comment (lib, "Ws2_32.lib")

//定义默认接受数据缓冲区大小及侦听的端口
#define DEFAULT_BUFFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(void)
{
	//创建WSADATA对象，WSADATA中包含windows sockets 的实现信息
	WSADATA wsaData;
	//接收相应函数的返回值
	int iResult;
	//声明侦听socket和连接服务端的socket
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	//声明存放ip地址信息的结构体
	struct addrinfo *result = NULL;
	struct addrinfo hints;

	//存放接收到的内容
	int iSendResult;
	char recvbuf[DEFAULT_BUFFLEN];
	int recvbuflen = DEFAULT_BUFFLEN;
	
	//初始化WINSOCK
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	//分配IP地址存放的结构体空间
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //指定使用IPV4网址地址
	hints.ai_socktype = SOCK_STREAM;  //指定socket流
	hints.ai_protocol = IPPROTO_TCP;  //指定使用TCP协议
	hints.ai_flags = AI_PASSIVE;  //指定一个标志位

	//获取本地IP地址信息
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	//创建一个socket用于侦听服务器
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	//将侦听socket绑定到系统以用于侦听服务器
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)  {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	//侦听端口
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//接受客户端的连接，即让客户端socket与侦听socket建议连接
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accpet failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// 接受到服务端连接的SOCKET后服务端socket可以进行释放
	closesocket(ListenSocket);

	//接收数据直到连接关闭
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			//将接到到的数据回显给客户端
			char *sendbuf = "Hello Client";
			iSendResult = send(ClientSocket, sendbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			recvbuf[iSendResult] = '\0';
			printf("Bytes sent: %d\n", iSendResult);
			printf("Sent content: %s\n", recvbuf);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	//接收完成后关闭连接
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	//清空连接并释放资源
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}