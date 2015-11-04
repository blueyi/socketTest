#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>

//��Ҫ���ӵĿ�
#pragma comment (lib, "Ws2_32.lib")

//����Ĭ�Ͻ������ݻ�������С�������Ķ˿�
#define DEFAULT_BUFFLEN 1024
#define DEFAULT_PORT "27015"

int __cdecl main(void)
{
	//����WSADATA����WSADATA�а���windows sockets ��ʵ����Ϣ
	WSADATA wsaData;
	//������Ӧ�����ķ���ֵ
	int iResult;
	//��������socket�����ӷ���˵�socket
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	//�������ip��ַ��Ϣ�Ľṹ��
	struct addrinfo *result = NULL;
	struct addrinfo hints;

	//��Ž��յ�������
//	int iSendResult;
	char recvFileName[DEFAULT_BUFFLEN];
	char recvFile[DEFAULT_BUFFLEN];
	std::string fileName;
	
	//��ʼ��WINSOCK
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error:" <<  iResult << std::endl;
		return 1;
	}

	//����IP��ַ��ŵĽṹ��ռ�
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //ָ��ʹ��IPV4��ַ��ַ
	hints.ai_socktype = SOCK_STREAM;  //ָ��socket��
	hints.ai_protocol = IPPROTO_TCP;  //ָ��ʹ��TCPЭ��
	hints.ai_flags = AI_PASSIVE;  //ָ��һ����־λ

	//��ȡ����IP��ַ��Ϣ
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error:" <<  iResult << std::endl;
		WSACleanup();
		return 1;
	}

	//����һ��socket��������������
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "socket failed with error:" <<  WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	//������socket�󶨵�ϵͳ����������������
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)  {
		std::cout << "bind failed with error:" <<  WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	//�����˿�
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
		std::cout << "�ȴ��ͻ�������..." << std::endl;
		//���տͻ��˵����ӣ����ÿͻ���socket������socket��������
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "accept failed with error:" << WSAGetLastError() << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}


		//�����ļ���
		std::ofstream out;
		iResult = recv(ClientSocket, recvFileName, DEFAULT_BUFFLEN, 0);
		if (iResult > 0) {
			recvFileName[iResult] = '\0';
			fileName = recvFileName;
			std::cout << "�����ļ���Ϊ�� " << fileName << std::endl;
			out.open(fileName, std::ofstream::out);
		}
		else {
			std::cout << "Receive file name error!" << std::endl;
		}


		int count = 0;
		//�����ļ�����ֱ�����ӹر�
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
				std::cout << "�����ļ����" << std::endl;
			else {
				std::cout << "recv failed with error:" << WSAGetLastError() << std::endl;
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
		} while (iResult > 0);

		out.close();
		std::cout << "�����ļ���С: " << count << std::endl;

		//������ɺ�ر�����
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			std::cout << "shutdown failed with error:" << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	}
		//������Ӳ��ͷ���Դ
		closesocket(ClientSocket);
		WSACleanup();


	return 0;
}