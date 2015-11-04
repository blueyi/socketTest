#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <string>
#include <fstream>

//需要连接的库
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

//远程端口
#define DEFAULT_PORT  "27015"
//默认缓冲区大小
#define DEFAULT_BUFLEN 1024



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
	char sendBuff[DEFAULT_BUFLEN];
	std::string fileName = "a.txt";
	//设置接收数据的缓冲区大小
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	std::string ip;
	//确定参数是否有效
	if (argc == 2) {
		ip = argv[1];
  std::cout << "远程服务器IP:  " << ip << std::endl;
	}
	else if (argc == 3) {
		ip = argv[1];
  std::cout << "远程服务器IP:  " << ip << std::endl;
		fileName = argv[2];
	}
	else {
		ip = "127.0.0.1";
  std::cout << "远程服务器IP:  " << ip << std::endl;
	}

	// 初始化winsock 
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 1;
	}

	//为hints分配空间
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//指定服务器地址和端口,argv[1]为IPV4或者IPV6地址
	iResult = getaddrinfo(ip.c_str(), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	//尝试连接到服务器直到连接成功
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		//初始化连接到服务器的socket对象
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		//创建失败则释放资源
		if (ConnectSocket == INVALID_SOCKET) {
			std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
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
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return 1;
	}

	//发送文件名
	iResult = send(ConnectSocket, fileName.c_str(), (int)fileName.length(), 0);
	//发送失败则关闭连接
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "文件名所占字节数: " << iResult << std::endl;
	std::cout << "发送文件名: " << fileName << std::endl;

	//发送文件内容
	std::ifstream in(fileName, std::ifstream::in);
	int i = 0;
	while (in.get(sendBuff[i % DEFAULT_BUFLEN])) {
		i++;
		if (i % DEFAULT_BUFLEN == 0) {
			iResult = send(ConnectSocket, sendBuff, sizeof(sendBuff), 0);
			//发送失败则关闭连接
			if (iResult == SOCKET_ERROR) {
				std::cout << "send failed: " << WSAGetLastError() << std::endl;
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
		}
	}
	iResult = send(ConnectSocket, sendBuff, i % DEFAULT_BUFLEN, 0);
	//发送失败则关闭连接
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "发送文件大小: " << i << std::endl;
	in.close();

	//当没有数据需要发送时，首先关闭当前用于发送数据的socket,以便服务端释放资源，并将该socket用于接收数据
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//从服务端接收数据直到服务器关闭连接
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "Bytes received: " << iResult << std::endl;
			recvbuf[iResult] = '\0';
			std::cout << "Bytes received: " << recvbuf << std::endl;
		}
		else if (iResult == 0)
			std::cout << "断开连接..." << std::endl;
		else
			std::cout << "recv failed: " << WSAGetLastError() << std::endl;
	} while (iResult > 0);

	//当接收完数据之后关闭socket并释放连接资源
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
