#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>

//需要连接的库
#pragma comment (lib, "Ws2_32.lib")

//定义默认接受数据缓冲区大小及侦听的端口
#define DEFAULT_BUFFLEN 1024
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
//	int iSendResult;
	char recvFileName[DEFAULT_BUFFLEN];
	char recvFile[DEFAULT_BUFFLEN];
	std::string fileName;
	
	//初始化WINSOCK
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error:" <<  iResult << std::endl;
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
		std::cout << "WSAStartup failed with error:" <<  iResult << std::endl;
		WSACleanup();
		return 1;
	}

	//创建一个socket用于侦听服务器
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "socket failed with error:" <<  WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	//将侦听socket绑定到系统以用于侦听服务器
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)  {
		std::cout << "bind failed with error:" <<  WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	//侦听端口
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		std::cout << "listen failed with error:" << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	while (true)
	{
		std::cout << std::endl;
		std::cout << "等待客户端连接..." << std::endl;
		//接收客户端的连接，即让客户端socket与侦听socket建立连接
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "accept failed with error:" << WSAGetLastError() << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}


		//接收文件名
		std::ofstream out;
		iResult = recv(ClientSocket, recvFileName, DEFAULT_BUFFLEN, 0);
		if (iResult > 0) {
			recvFileName[iResult] = '\0';
			fileName = recvFileName;
			std::cout << "接收文件名为： " << fileName << std::endl;
			out.open(fileName, std::ofstream::out);
		}
		else {
			std::cout << "Receive file name error!" << std::endl;
		}


		int count = 0;
		//接收文件内容直到连接关闭
		do {
			iResult = recv(ClientSocket, recvFile, DEFAULT_BUFFLEN, 0);
			if (iResult == DEFAULT_BUFFLEN) {
				for (int i = 0; i < DEFAULT_BUFFLEN; ++i) {
					out.put(recvFile[i]);
					count++;
				}
			}
			else if (iResult > 0)
			{
				for (int i = 0; i < iResult; ++i) {
					out.put(recvFile[i]);
					count++;
				}
			}
			else if (iResult == 0)
				std::cout << "接收文件完成" << std::endl;
			else {
				std::cout << "recv failed with error:" << WSAGetLastError() << std::endl;
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
		} while (iResult > 0);

		out.close();
		std::cout << "接收文件大小: " << count << std::endl;

		//接收完成后关闭连接
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			std::cout << "shutdown failed with error:" << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	}
		//清空连接并释放资源
		closesocket(ClientSocket);
		WSACleanup();


	return 0;
}