#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <string>
using namespace std;
#define SHARED_MEM_SIZE 1024  // 共享内存大小
#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024
void help() {
    cout << "makedir                  在该页创建目录" << endl;
    cout << "changedir                进入对应子目录" << endl;
    cout << "removedir              删除对应空子目录" << endl;
    cout << "dir        列出当前目录内的文件或子目录" << endl;
    cout << "listAsTree           以树的形式显示目录" << endl;
    cout << "make                           创建文件" << endl;
    cout << "deleteFile                     删除文件" << endl;
    cout << "ulf        上传本机文件到该虚拟文件系统" << endl;
    cout << "dlf        下载该虚拟文件系统文件到本机" << endl;
    cout << "createJob                      创建作业" << endl;
    cout << "ListJob                    展示作业日志" << endl;
    cout << "stopJob jobID          停止对应作业运行" << endl;
    cout << "startJob jobID         开始对应作业运行" << endl;
    cout << "removeJob jobID          删除对应的作业" << endl;
    cout << "help                       提供帮助信息" << endl;
    cout << "exit                         退出该程序" << endl;
}
int main() 
{
   
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "初始化失败" << std::endl;
        return -1;
    }

    SOCKET clientSocket;
    struct sockaddr_in serverAddress {};
    char buffer[BUFFER_SIZE] = { 0 };

    // 创建套接字
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "创建套接字失败" << std::endl;
        WSACleanup();
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(PORT);

    // 连接到服务器
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "不能连接到服务器" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    while (true) {
        // 发送请求给服务器
        std::string request;
        std::cout << "请发送你的请求: ";
        std::getline(std::cin, request);


      
        if (request == "help")
        {
            help();
        }
        if (request == "init")
        {
            cout << "System initalization..." << endl;
            Sleep(1000);
            cout << "Preparing Job manager....ok" << endl;
            Sleep(1000);
            cout << "Server is running..." << endl;
        }
     
        if (send(clientSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
            std::cerr << "发送数据失败" << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return -1;
        }

        // 检查是否发送退出信号
        if (request == "exit") 
        {
            std::cout << "退出中..." << std::endl;
            Sleep(2000);
            std::cout << "Shutdown server...ok";
            exit(0);

           
        }

        // 接收服务器响应
        memset(buffer, 0, sizeof(buffer));
        int valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (valread <= 0) {
            std::cerr << "失败" << std::endl;
        }
        else {
            std::cout << "客户端回应: \n" << buffer << std::endl;
        }
    }


    return 0;
}
