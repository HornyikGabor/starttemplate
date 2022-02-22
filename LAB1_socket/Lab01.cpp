#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

/*
Indítás: parancssorból: C:\Users\Hornyik Gábor\source\repos\Lab01\Debug>Lab01.exe 152.66.188.5 80 / fajl.html
*/
int main(int argc, char* argv[])
{
	// Ellenõrzés
	if (argc < 5)
	{
		printf("Használat: %s IP port oldal fájl\n", argv[0]);
		return 1;
	}

	// Winsock inicializálás
	WSADATA	wsd;
	if (WSAStartup(0x0202, &wsd) != 0)
	{
		perror("WSAStartup");
		return 1;
	}

	//1. SOCKET létrehozása
	SOCKET sk;
	if ((sk = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket: %d\r\n", WSAGetLastError());
		return 1;
	}

	//2. SOCKET címezése
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(atoi(argv[2]));

	//3. Kapcsolódás
	if (connect(sk, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("connect: %d\r\n", WSAGetLastError());
		return 1;
	}

	//4. Kérés
	char request[256];
	int reqlen;
	reqlen = sprintf(request, "GET %s HTTP/1.0\r\n\r\n", argv[3]);

	//5. Kérés elküldése
	send(sk, request, reqlen, 0);

	//6. Válasz fogadása
	char received[1024];
	int recvlen;
	recvlen = recv(sk, received, sizeof(received) - 1, MSG_WAITALL);
	if (recvlen < 0) {
		printf("recv: %d\r\n", WSAGetLastError());
		return 1;
	}
	received[recvlen] = 0;
	//printf("Tartalom: %s\n", received);

	//7. Válasz feldogozás
	char version[16];
	int status;
	char error[256];
	if (sscanf(received, "HTTP/%16s %d %256[^\r\n]\r\n", version, &status, error) == 3) {
		printf("A szerver státusz jelzése: %d %s\n", status, error);
	}
	char* pdata;
	if ((pdata = strstr(received, "\r\n\r\n")) == NULL) {
		printf("Hiba!");
		return 1;
	}

	//8. Adatmentés állományba
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

	//9. SOCKET bezárása
	closesocket(sk);

	WSACleanup();

	return 0;
}