#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void errors(int type)
{
	switch (type)
	{
	case 1:
		cerr << "Error starting winsock!" << endl;
		break;
	case 2:
		cerr << "Error creating socket!, " << endl;
		break;
	case 3:
		cerr << "Error connect to server!, " << endl;
		break;
	case 4:
		cerr << "Can't open file or file not found!" << endl;
		break;
	};
}

int main() 
{
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (WSAStartup(ver, &wsData) != 0) 
	{
		errors(1);
		return -1;
	}

	SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clientSock == INVALID_SOCKET) 
	{
		errors(2);
		cerr << " " << WSAGetLastError() << endl;
		return -1;
	}

	char serverAddress[NI_MAXHOST];
	memset(serverAddress, 0, NI_MAXHOST);

	cout << "Enter server address (for default put 127.0.0.1): ";
	cin.getline(serverAddress, NI_MAXHOST);

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(55000);
	inet_pton(AF_INET, serverAddress, &hint.sin_addr);

	char welcomeMsg[255];
	const int BUFFER_SIZE = 1024;
	char bufferFile[BUFFER_SIZE];
	char fileRequested[FILENAME_MAX];
	ofstream file;


	if (connect(clientSock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) 
	{
		errors(3);
		cerr << " " << WSAGetLastError() << endl;
		closesocket(clientSock);
		WSACleanup();
		return -1;
	}

	int byRecv = recv(clientSock, welcomeMsg, 255, 0);
	if (byRecv == 0 || byRecv == -1) 
	{
		closesocket(clientSock);
		WSACleanup();
		return -1;
	}

	bool clientClose = false;
	int codeAvailable = 404;
	const int fileAvailable = 200;
	const int fileNotfound = 404;
	long fileRequestedsize = 0;
	do 
	{
		int fileDownloaded = 0;
		memset(fileRequested, 0, FILENAME_MAX);
		cout << "Enter file name: " << endl;
		cin.getline(fileRequested, FILENAME_MAX);

		byRecv = send(clientSock, fileRequested, FILENAME_MAX, 0);
		if (byRecv == 0 || byRecv == -1) 
		{
			clientClose = true;
			break;
		}

		byRecv = recv(clientSock, (char*)&codeAvailable, sizeof(int), 0);
		if (byRecv == 0 || byRecv == -1) 
		{
			clientClose = true;
			break;
		}
		if (codeAvailable == 200) {
			byRecv = recv(clientSock, (char*)&fileRequestedsize, sizeof(long), 0);
			if (byRecv == 0 || byRecv == -1) 
			{
				clientClose = true;
				break;
			}

			file.open(fileRequested, ios::binary | ios::trunc);

			do 
			{
				memset(bufferFile, 0, BUFFER_SIZE);
				byRecv = recv(clientSock, bufferFile, BUFFER_SIZE, 0);

				if (byRecv == 0 || byRecv == -1) 
				{
					clientClose = true;
					break;
				}

				file.write(bufferFile, byRecv);
				fileDownloaded += byRecv;
			} while (fileDownloaded < fileRequestedsize);
			file.close();
		}
		else if (codeAvailable == 404) 
		{
			errors(4);
		}
	} while (!clientClose);
	closesocket(clientSock);
	WSACleanup();
	return 0;
}