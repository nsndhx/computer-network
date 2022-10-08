#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)
int main(int argc, char** argv) {
	// 声明和初始化变量	
	WSADATA wsaData;
	int iResult;	DWORD dwError;	int i = 0;
	struct hostent* remoteHost;
	char host_name[256];
	struct in_addr addr;
	char** pAlias;

	//初始化Windows Sockets	
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	//查找主机名		
	iResult = gethostname(host_name, sizeof(host_name));
	if (iResult != 0)
	{
		printf("gethostname failed: %d\n", iResult);
		return 1;
	}

	//根据主机名获得主机信息
	remoteHost = gethostbyname(host_name);
	printf("Calling gethostbyname with %s\n", host_name);

	// 对返回结果进行判断	
	if (remoteHost == NULL)
	{
		dwError = WSAGetLastError();
		if (dwError != 0)
		{
			if (dwError == WSAHOST_NOT_FOUND)
			{
				printf("Host not found\n");
				return 1;
			}
			else if (dwError == WSANO_DATA)
			{
				printf("No data record found\n");
				return 1;
			}
			else
			{
				printf("Function failed with error: %ld\n", dwError);
				return 1;
			}
		}
	}
	else
	{
		printf("Function returned:\n");
		printf("\tOfficial name: %s\n", remoteHost->h_name);
		for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++)
		{
			printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
		}
		printf("\tAddress type: ");
		switch (remoteHost->h_addrtype)
		{
		case AF_INET:
			printf("AF_INET\n");
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS\n");
			break;
		default:
			printf(" %d\n", remoteHost->h_addrtype);
			break;
		}
		printf("\tAddress length: %d\n", remoteHost->h_length);
		// 如果返回的是IPv4的地址， 则输出		i = 0;		
		if (remoteHost->h_addrtype == AF_INET)
		{
			while (remoteHost->h_addr_list[i] != 0)
			{
				addr.s_addr = *(u_long*)remoteHost->h_addr_list[i++];
				printf("\tIP Address #%d: %s\n", i, inet_ntoa(addr));
			}
		}
		else if (remoteHost->h_addrtype == AF_NETBIOS)
		{
			printf("NETBIOS address was returned\n");
		}
	}
	return 0;
}