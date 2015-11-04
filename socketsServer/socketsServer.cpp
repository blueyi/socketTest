#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

//��Ҫ���ӵĿ�
#pragma comment (lib, "Ws2_32.lib")

//����Ĭ�Ͻ������ݻ�������С�������Ķ˿�
#define DEFAULT_BUFFLEN 512
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
	int iSendResult;
	char recvbuf[DEFAULT_BUFFLEN];
	int recvbuflen = DEFAULT_BUFFLEN;
	
	//��ʼ��WINSOCK
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
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
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	//����һ��socket��������������
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	//������socket�󶨵�ϵͳ����������������
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)  {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	//�����˿�
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//���ܿͻ��˵����ӣ����ÿͻ���socket������socket��������
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accpet failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// ���ܵ���������ӵ�SOCKET������socket���Խ����ͷ�
	closesocket(ListenSocket);

	//��������ֱ�����ӹر�
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			//���ӵ��������ݻ��Ը��ͻ���
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

	//������ɺ�ر�����
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	//������Ӳ��ͷ���Դ
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}