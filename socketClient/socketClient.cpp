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

//��Ҫ���ӵĿ�
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

//Զ�̶˿�
#define DEFAULT_PORT  "27015"
//Ĭ�ϻ�������С
#define DEFAULT_BUFLEN 1024



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
	char sendBuff[DEFAULT_BUFLEN];
	std::string fileName = "a.txt";
	//���ý������ݵĻ�������С
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	std::string ip;
	//ȷ�������Ƿ���Ч
	if (argc == 2) {
		ip = argv[1];
  std::cout << "Զ�̷�����IP:  " << ip << std::endl;
	}
	else if (argc == 3) {
		ip = argv[1];
  std::cout << "Զ�̷�����IP:  " << ip << std::endl;
		fileName = argv[2];
	}
	else {
		ip = "127.0.0.1";
  std::cout << "Զ�̷�����IP:  " << ip << std::endl;
	}

	// ��ʼ��winsock 
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 1;
	}

	//Ϊhints����ռ�
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//ָ����������ַ�Ͷ˿�,argv[1]ΪIPV4����IPV6��ַ
	iResult = getaddrinfo(ip.c_str(), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	//�������ӵ�������ֱ�����ӳɹ�
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		//��ʼ�����ӵ���������socket����
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		//����ʧ�����ͷ���Դ
		if (ConnectSocket == INVALID_SOCKET) {
			std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
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
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return 1;
	}

	//�����ļ���
	iResult = send(ConnectSocket, fileName.c_str(), (int)fileName.length(), 0);
	//����ʧ����ر�����
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "�ļ�����ռ�ֽ���: " << iResult << std::endl;
	std::cout << "�����ļ���: " << fileName << std::endl;

	//�����ļ�����
	std::ifstream in(fileName, std::ifstream::in);
	int i = 0;
	while (in.get(sendBuff[i % DEFAULT_BUFLEN])) {
		i++;
		if (i % DEFAULT_BUFLEN == 0) {
			iResult = send(ConnectSocket, sendBuff, sizeof(sendBuff), 0);
			//����ʧ����ر�����
			if (iResult == SOCKET_ERROR) {
				std::cout << "send failed: " << WSAGetLastError() << std::endl;
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
		}
	}
	iResult = send(ConnectSocket, sendBuff, i % DEFAULT_BUFLEN, 0);
	//����ʧ����ر�����
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "�����ļ���С: " << i << std::endl;
	in.close();

	//��û��������Ҫ����ʱ�����ȹرյ�ǰ���ڷ������ݵ�socket,�Ա������ͷ���Դ��������socket���ڽ�������
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//�ӷ���˽�������ֱ���������ر�����
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "Bytes received: " << iResult << std::endl;
			recvbuf[iResult] = '\0';
			std::cout << "Bytes received: " << recvbuf << std::endl;
		}
		else if (iResult == 0)
			std::cout << "�Ͽ�����..." << std::endl;
		else
			std::cout << "recv failed: " << WSAGetLastError() << std::endl;
	} while (iResult > 0);

	//������������֮��ر�socket���ͷ�������Դ
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
