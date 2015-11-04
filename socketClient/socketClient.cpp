#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

//��Ҫ���ӵĿ�
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")



//Զ�̶˿�
#define DEFAULT_PORT  "27015"
//Ĭ�ϻ�������С
#define DEFAULT_BUFLEN 512

int __cdecl main(int argc, char **argv)
{
	//����WSADATA����WSADATA�а���windows sockets ��ʵ����Ϣ
	WSADATA wsaData;
	// �������ӵ�����˵�socket����
	SOCKET ConnectSocket = INVALID_SOCKET;
	//����addrinfo�������а���sockaddr�ṹ��ֵ
	//result��ptr��ָ����Ӧ��������ַ��Ϣ,hints���socket������Ϣ
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	//����������͵���������
	char *sendbuf = "Hello Server";
	//���ý������ݵĻ�������С
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	//ȷ�������Ƿ���Ч
	if (argc != 2) {
		printf("usage: %s sever-name\n", argv[0]);
		return 1;
	}

	// ��ʼ��winsock 
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	//Ϊhints����ռ�
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//ָ����������ַ�Ͷ˿�,argv[1]ΪIPV4����IPV6��ַ
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	//�������ӵ�������ֱ�����ӳɹ�
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		//��ʼ�����ӵ���������socket����
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		//����ʧ�����ͷ���Դ
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}
		//���ӵ�������
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	//������ɺ��ͷŵ�ַ��Ϣռ�õ��ڴ�
	freeaddrinfo(result);
	//�������ʧ���򷵻�
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	//��������
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	//����ʧ����ر�����
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	printf("Byte Sent: %ld\n", iResult);

	//��û��������Ҫ����ʱ�����ȹرյ�ǰ���ڷ������ݵ�socket,�Ա������ͷ���Դ��������socket���ڽ�������
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//�ӷ���˽�������ֱ�ӷ������ر�����
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

	//������������֮��ر�socket���ͷ�������Դ
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
