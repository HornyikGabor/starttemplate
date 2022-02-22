#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

/*
Ind�t�s: parancssorb�l: C:\Users\Hornyik G�bor\source\repos\Lab01\Debug>Lab01.exe 152.66.188.5 80 / fajl.html
*/
int main(int argc, char* argv[])
{
	// Ellen�rz�s
	if (argc < 5)
	{
		printf("Haszn�lat: %s IP port oldal f�jl\n", argv[0]);
		return 1;
	}

	// Winsock inicializ�l�s
	WSADATA	wsd;
	if (WSAStartup(0x0202, &wsd) != 0)
	{
		perror("WSAStartup");
		return 1;
	}

	//1. SOCKET l�trehoz�sa
	SOCKET sk;
	if ((sk = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket: %d\r\n", WSAGetLastError());
		return 1;
	}

	//2. SOCKET c�mez�se
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(atoi(argv[2]));

	//3. Kapcsol�d�s
	if (connect(sk, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("connect: %d\r\n", WSAGetLastError());
		return 1;
	}

	//4. K�r�s
	char request[256];
	int reqlen;
	reqlen = sprintf(request, "GET %s HTTP/1.0\r\n\r\n", argv[3]);

	//5. K�r�s elk�ld�se
	send(sk, request, reqlen, 0);

	//6. V�lasz fogad�sa
	char received[1024];
	int recvlen;
	recvlen = recv(sk, received, sizeof(received) - 1, MSG_WAITALL);
	if (recvlen < 0) {
		printf("recv: %d\r\n", WSAGetLastError());
		return 1;
	}
	received[recvlen] = 0;
	//printf("Tartalom: %s\n", received);

	//7. V�lasz feldogoz�s
	char version[16];
	int status;
	char error[256];
	if (sscanf(received, "HTTP/%16s %d %256[^\r\n]\r\n", version, &status, error) == 3) {
		printf("A szerver st�tusz jelz�se: %d %s\n", status, error);
	}
	char* pdata;
	if ((pdata = strstr(received, "\r\n\r\n")) == NULL) {
		printf("Hiba!");
		return 1;
	}

	//8. Adatment�s �llom�nyba
	FILE* f;
	if ((f = fopen(argv[4], "w")) == NULL) {
		perror("fopen");
		return 1;
	}
	fwrite(pdata + 4, 1, recvlen - (pdata - received) - 4, f);
	while ((recvlen = recv(sk, received, sizeof(received), 0)) > 0) {
		fwrite(received, 1, recvlen, f);
	}
	fclose(f);

	//9. SOCKET bez�r�sa
	closesocket(sk);

	WSACleanup();

	return 0;
}