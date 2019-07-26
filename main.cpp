//非Unix系统
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#define close closesocket

class WinSockInit
{
	WSADATA _wsa;
public:
	WinSockInit()
	{  //分配套接字版本信息2.0，WSADATA变量地址
		WSAStartup(MAKEWORD(2, 0), &_wsa);

	}
	~WinSockInit()
	{
		WSACleanup();//功能是终止Winsock 2 DLL (Ws2_32.dll) 的使用
	}
};

//Unix系统
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <iostream>
#include <string>
#define BUFF_SIZE 1024
using namespace std;

//处理URL
void UrlRouter(int clientSock, string const& url)
{
	string hint;
	if (url == "/")
	{
		cout << url << " 收到信息\n";
		hint = "haha, this is home page!";
		send(clientSock, hint.c_str(), hint.length(), 0);
	}
	else if (url == "/hello")
	{
		cout << url << " 收到信息\n";
		hint = "你好!";
		send(clientSock, hint.c_str(), hint.length(), 0);
	}
	else
	{
		cout << url << " 收到信息\n";
		hint = "未定义URL!";
		send(clientSock, hint.c_str(), hint.length(), 0);
	}

}

int main()
{
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(WIN32)
	WinSockInit socklibInit;//如果为Windows系统，进行WSAStartup
#endif

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//建立套接字，失败返回-1
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET; //指定地址族
	addr.sin_addr.s_addr = INADDR_ANY;//IP初始化
	addr.sin_port = htons(8090);//端口号初始化

	int rc;
	rc = bind(sock, (sockaddr*)& addr, sizeof(addr));//分配IP和端口

	rc = listen(sock, 0);//设置监听

	//设置客户端
	sockaddr_in clientAddr;
	int clientAddrSize = sizeof(clientAddr);
	int clientSock;
	//接受客户端请求
	while (-1 != (clientSock = accept(sock, (sockaddr*)& clientAddr, (socklen_t*)& clientAddrSize)))
	{
		//HTTP请求的数据报文
		string requestStr = string();
		//数据总长度
		int total_length = 0;
		//数据缓冲区
		char buff[BUFF_SIZE] ;
		//从recv缓冲区读取出的实际数据长度
		int recv_length;
		do {
			recv_length = recv(clientSock, buff, sizeof(buff), 0);
			if (-1 == recv_length) {//连接失败时返回SOCKET_ERROR
				cout << "连接失败" << endl;
			}
			else if (0 == recv_length) {//连接结束时返回0
				cout << "连接结束" << endl;
			}
			else {//接收到的数据长度
				requestStr.append(buff, 0, recv_length);
				total_length += recv_length;
			}
		} while (recv_length >= BUFF_SIZE);//接收到的数据长度小于数据缓冲区的大小说明recv缓冲区里的数据已经复制完了
		cout << requestStr << endl;
		
		//取得第一行
		string firstLine = requestStr.substr(0, requestStr.find("\r\n"));
		//取得URL
		firstLine = firstLine.substr(firstLine.find(" ") + 1);//substr，复制函数，参数为起始位置（默认0），复制的字符数目
		string url = firstLine.substr(0, firstLine.find(" "));//find返回找到的第一个匹配字符串的位置，而不管其后是否还有相匹配的字符串。

		//发送响应头
		string response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html; charset=utf8\r\n"
			"Connection: close\r\n"
			"\r\n";
		send(clientSock, response.c_str(), response.length(), 0);
		//处理URL
		UrlRouter(clientSock, url);

		close(clientSock);//关闭客户端套接字
	}

	close(sock);//关闭服务器套接字

	return 0;
}
