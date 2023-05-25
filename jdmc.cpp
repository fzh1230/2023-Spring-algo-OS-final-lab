#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <string>
using namespace std;
#define SHARED_MEM_SIZE 1024  // �����ڴ��С
#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024
void help() {
    cout << "makedir                  �ڸ�ҳ����Ŀ¼" << endl;
    cout << "changedir                �����Ӧ��Ŀ¼" << endl;
    cout << "removedir              ɾ����Ӧ����Ŀ¼" << endl;
    cout << "dir        �г���ǰĿ¼�ڵ��ļ�����Ŀ¼" << endl;
    cout << "listAsTree           ��������ʽ��ʾĿ¼" << endl;
    cout << "make                           �����ļ�" << endl;
    cout << "deleteFile                     ɾ���ļ�" << endl;
    cout << "ulf        �ϴ������ļ����������ļ�ϵͳ" << endl;
    cout << "dlf        ���ظ������ļ�ϵͳ�ļ�������" << endl;
    cout << "createJob                      ������ҵ" << endl;
    cout << "ListJob                    չʾ��ҵ��־" << endl;
    cout << "stopJob jobID          ֹͣ��Ӧ��ҵ����" << endl;
    cout << "startJob jobID         ��ʼ��Ӧ��ҵ����" << endl;
    cout << "removeJob jobID          ɾ����Ӧ����ҵ" << endl;
    cout << "help                       �ṩ������Ϣ" << endl;
    cout << "exit                         �˳��ó���" << endl;
}
int main() 
{
   
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "��ʼ��ʧ��" << std::endl;
        return -1;
    }

    SOCKET clientSocket;
    struct sockaddr_in serverAddress {};
    char buffer[BUFFER_SIZE] = { 0 };

    // �����׽���
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "�����׽���ʧ��" << std::endl;
        WSACleanup();
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(PORT);

    // ���ӵ�������
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "�������ӵ�������" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    while (true) {
        // ���������������
        std::string request;
        std::cout << "�뷢���������: ";
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
            std::cerr << "��������ʧ��" << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return -1;
        }

        // ����Ƿ����˳��ź�
        if (request == "exit") 
        {
            std::cout << "�˳���..." << std::endl;
            Sleep(2000);
            std::cout << "Shutdown server...ok";
            exit(0);

           
        }

        // ���շ�������Ӧ
        memset(buffer, 0, sizeof(buffer));
        int valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (valread <= 0) {
            std::cerr << "ʧ��" << std::endl;
        }
        else {
            std::cout << "�ͻ��˻�Ӧ: \n" << buffer << std::endl;
        }
    }


    return 0;
}
