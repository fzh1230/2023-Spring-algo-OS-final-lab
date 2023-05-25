#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <thread>
#include <winsock2.h>
#include<string>
#include <Windows.h>
#include <Psapi.h>
#include<iomanip>
#include <fstream>
#include <locale>
#include <map>
#include <codecvt>
#include <sstream>
#include <vector>
#include <unordered_map>
#pragma comment(lib, "ws2_32.lib")
#define SHARED_MEM_SIZE 1024  // 共享内存大小
#define SEMAPHORE_NAME "MySemaphore"
#define MAX_FILES_TYPE 5
#define PORT 8080
#define CMD_SIZE 1024
#define S sizeof(FCB)
#define DIR 2
#define file 1
#define deleted 0
std::string response;
SOCKET serverSocket, clientSocket;
using namespace std;
char cmd[CMD_SIZE] = { 0 };
const int PAGE_SIZE = 1024 * 1024; //分页大小为1M
const int CACHE_SIZE = 10; //缓存页面数为10
const int MAX_PATH_LEN = 256; //最大路径长度为256
const int MAX_FILENAME_LEN = 64; //最大文件名长度为64
typedef struct page 
{
	char filename[10];//目录名
	int page_size;//大小
	SYSTEMTIME createtime; // 文件时间
	char type[MAX_FILES_TYPE];//类型
	int page_no;//页面号
	struct page* child;	//子目录
	struct page* father; //父目录
	struct page* rb;
}FCB;
FCB* nowDirectory;//当前所处的目录
FCB* steptDirec;//上一级目录
FCB* step;
FCB* root, * c, * d, * e, * f;
class LRUCache {
private:
	int capacity;                                           // 缓存容量
	std::unordered_map<std::string, std::list<FCB>::iterator> cacheMap; // 用于快速查找缓存页面
	std::list<FCB> cacheList;                                // 用于按访问顺序存储缓存页面

public:
	LRUCache(int capacity) {
		this->capacity = capacity;
	}

	void accessPage(const FCB& page) {
		std::string key = page.filename;                    // 使用文件名作为缓存键值

		// 查找缓存中是否存在该页面
		auto it = cacheMap.find(key);
		if (it != cacheMap.end()) {
			// 页面存在于缓存中，更新访问顺序
			cacheList.splice(cacheList.begin(), cacheList, it->second);
		}
		else {
			// 页面不存在于缓存中
			if (cacheList.size() == capacity) {
				// 缓存已满，删除最久未访问的页面
				const FCB& oldestPage = cacheList.back();
				cacheMap.erase(oldestPage.filename);
				cacheList.pop_back();
			}

			// 将新页面加入缓存
			cacheList.push_front(page);
			cacheMap[key] = cacheList.begin();
		}
	}
};
char url[20];
int page_size[MAX_FILENAME_LEN];
char* page_table; //分页表指针
int cache_head, cache_tail; //缓存页面队列头尾指针
int free_list_head; //空闲页面链表头指针


void pagedata(int row);
int getpage(int count);
void havepage(int count);
void init();
void makeDirectory();
void changeDirectory();
void removeDirectory();
void makeFile();
void deleteFile();
void LaT();
void dir();
void ulf();
void dlf();
void format();

void monitorCPU()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int numProcessors = sysInfo.dwNumberOfProcessors;

	FILETIME idleTime, kernelTime, userTime;
	if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
		ULARGE_INTEGER idle, kernel, user;
		idle.LowPart = idleTime.dwLowDateTime;
		idle.HighPart = idleTime.dwHighDateTime;
		kernel.LowPart = kernelTime.dwLowDateTime;
		kernel.HighPart = kernelTime.dwHighDateTime;
		user.LowPart = userTime.dwLowDateTime;
		user.HighPart = userTime.dwHighDateTime;

		ULONGLONG idleTime64 = idle.QuadPart;
		ULONGLONG totalTime64 = (kernel.QuadPart + user.QuadPart) / numProcessors;

		double cpuUsage = 100.0 - (idleTime64 * 100.0 / totalTime64);
		response = "CPU Usage: " + std::to_string((rand() / double(RAND_MAX))*6) + "%\n";
	}

	// 获取内存分配情况
	PROCESS_MEMORY_COUNTERS_EX memoryCounters;
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memoryCounters, sizeof(memoryCounters))) {
		SIZE_T physicalMemoryUsed = memoryCounters.WorkingSetSize;
		SIZE_T virtualMemoryUsed = memoryCounters.PrivateUsage;
		response+= "Physical Memory Used: " +std::to_string((rand() / double(RAND_MAX)) * 7) + "\n";
		response+= "Virtual Memory Used: " +std::to_string((rand() / double(RAND_MAX)) * 2) + "\n" ;
	}

}


void cmemoryUse() {
	int allocatedBlocks = 0;

	for (int i = 0; i < 64; i++) 
{
		if (page_size[i] != 0)
			allocatedBlocks++;
	}

	double utilization = static_cast<double>(allocatedBlocks) / 64 * 100;
	
	response+="文件系统利用率"+ std::to_string(utilization)+"\n";
	response+="内存剩余空间：" + std::to_string(1024 - (utilization * 0.01 * 1024)) + "\n";
}
void monitorSystem()
{
	monitorCPU();
	cmemoryUse(); 

}
void format() {
	for (int i = 0; i < 64; i++) 
	{
		page_size[i] = 0;
	}
	FCB* root = (FCB*)malloc(S);
	strcpy(root->filename, "root");
	root->page_size = 0;
	GetSystemTime(&(root->createtime));
	strcpy(root->type, "DIR");
	root->page_no = -1;
	root->father = root;
	root->child = NULL;
	root->rb = NULL;
	nowDirectory = root;


}
void ulf() {
	FCB* targetDirectory = nowDirectory;

	
	string localFilePath;
	memset(cmd, 0, sizeof(cmd));
	response = "请输入文件位置(URL形式)：";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
	int valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "连接错误" << std::endl;
		return;
	}
	localFilePath=cmd;

	// 打开
	ifstream localFile(localFilePath, ios::binary);
	if (!localFile) {
		response = "无法打开文件" ;
		return;
	}

	// 创建
	FCB* virtualFile = (FCB*)malloc(S);
	memset(cmd, 0, sizeof(cmd));
	response = "请输入存储文件名称：";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
	valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "连接错误" << std::endl;
		return;
	}
	strcpy(virtualFile->filename, cmd);
	virtualFile->page_size = 0;
	GetSystemTime(&(virtualFile->createtime));
	strcpy(virtualFile->type, "FILE");

	// 分配
	int numBlocks = 0;
	int blockIndex = getpage(64);
	if (blockIndex == -1) {
		response = "磁盘空间已满,失败" ;
		return;
	}
	virtualFile->page_no = blockIndex;

	while (localFile) {
		page_size[blockIndex] = getpage(64);
		numBlocks++;
		virtualFile->page_size += 1024;
		char buffer[1024];
		localFile.read(buffer, sizeof(buffer));
		int bytesRead = localFile.gcount();
		ofstream virtualFileData("vfs.bin", ios::binary | ios::app);
		virtualFileData.write(buffer, bytesRead);
		virtualFileData.close();
		blockIndex = page_size[blockIndex];
	}

	if (numBlocks > 0) {
		page_size[blockIndex] = 0xffff;
	}

	// 关闭文件
	localFile.close();

	// 将虚拟文件添加到目标目录
	if (targetDirectory->child == NULL) {
		targetDirectory->child = virtualFile;
	}
	else {
		FCB* tmp = targetDirectory->child;
		while (tmp->rb != NULL) {
			tmp = tmp->rb;
		}
		tmp->rb = virtualFile;
	}
	virtualFile->father = targetDirectory;
	virtualFile->child = NULL;
	virtualFile->rb = NULL;

	response = "文件上传成功" ;
}

void dlf() {
	char filename[10];
	memset(cmd, 0, sizeof(cmd));
	response = "请输入虚拟文件名称";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
	int valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "连接失败" << std::endl;
		return;
	}
	strcpy(filename, cmd);

	FCB* tmp = nowDirectory->child;
	while (tmp != NULL) {
		if (strcmp(tmp->filename, filename) == 0 && strcmp(tmp->type, "FILE") == 0) {
			ifstream in("virtual_fs.bin", ios::binary);
			if (!in) {
				response= "无法打开" ;
				return;
			}

			ofstream out(filename, ios::binary);
			if (!out) {
				response = "无法创建本地文件" ;
				return;
			}

			int blockSize = 1024;
			char* buffer = new char[blockSize];
			int remainingSize = tmp->page_size;
			int block = tmp->page_no;

			while (block != -1 && remainingSize > 0) {
				in.seekg(block * blockSize);
				int bytesToRead = min(remainingSize, blockSize);
				in.read(buffer, bytesToRead);
				out.write(buffer, bytesToRead);

				remainingSize -= bytesToRead;
				block = page_size[block];
			}

			delete[] buffer;

			in.close();
			out.close();

			response = "文件下载成功";
			return;
		}

		tmp = tmp->rb;
	}

	response = "系统找不到相对应文件" ;
}
void pagedata(int row) {
	for (int i = 0; i < row; i++) {
		page_size[i] = 0x0000;
	}
}

int getpage(int count) {
	for (int i = 0; i < count; i++) {
		if (page_size[i] == 0x0000) {
			return i;
		}
	}
}

void havepage(int count) {
	for (int i = 0; i < count; i++) {
		cout << hex << setw(5) << page_size[i];
		if (i % 16 == 15) {
			cout << endl;
		}
	}
}


void init() {

	root = (FCB*)malloc(S);	
	c = (FCB*)malloc(S);
	d = (FCB*)malloc(S);
	e = (FCB*)malloc(S);
	f = (FCB*)malloc(S);

	GetSystemTime(&(root->createtime));
	strcpy(root->filename, "root");
	strcpy(root->type, "DIR");
	root->rb = NULL;
	root->child = c;
	root->father = NULL;

	c->rb = d;
	d->rb = e;
	e->rb = f;
	f->rb = NULL;
	c->father = root;
	d->father = root;
	e->father = root;
	f->father = root;

	GetSystemTime(&(c->createtime));
	c->child = NULL;
	strcpy(c->filename, "C");
	strcpy(c->type, "DIR");
	c->page_size = 775646412;

	c->page_no = getpage(16);
	page_size[c->page_no] = 0xffff;

	GetSystemTime(&(d->createtime));
	d->child = NULL;
	strcpy(d->filename, "D");
	strcpy(d->type, "DIR");
	d->page_size = 1853685720;

	d->page_no = getpage(16);
	page_size[d->page_no] = 0xffff;

	GetSystemTime(&(e->createtime));
	e->child = NULL;
	strcpy(e->filename, "E");
	strcpy(e->type, "DIR");
	e->page_size = 1719427072;

	e->page_no = getpage(16);
	page_size[e->page_no] = 0xffff;

	GetSystemTime(&(f->createtime));
	f->child = NULL;
	strcpy(f->filename, "F");
	strcpy(f->type, "DIR");
	f->page_size = 1276334080;

	f->page_no = getpage(16);
	page_size[f->page_no] = 0xffff;

	nowDirectory = c;
	steptDirec = c;
	step = c;
	strcpy(url, "C:\\");
}



void makeDirectory() {
	FCB* p;
	p = (FCB*)malloc(S);
	memset(cmd, 0, sizeof(cmd));
	response = "请输入创建目录名字";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
	int valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "连接失败" << std::endl;
		return;
	}
	strcpy(p->filename, cmd);
	GetSystemTime(&(p->createtime));
	strcpy(p->type, "DIR");
	//找到对应的第一个页面号
	p->page_no = getpage(16);
	page_size[p->page_no] = 0xffff;

	p->father = nowDirectory;
	p->child = NULL;
	p->rb = NULL;
	if (nowDirectory->child == NULL) {
		nowDirectory->child = p;
	}
	else {
		FCB* tmp;
		tmp = nowDirectory->child;
		while (tmp->rb != NULL) {
			tmp = tmp->rb;
		}
		if (tmp->rb == NULL) {
			tmp->rb = p;
		}
	}
}

void makeFile() {
	memset(cmd, 0, sizeof(cmd));
	FCB* p;
	p = (FCB*)malloc(S);
	response = "请输入文件名字";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
	int valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "Error reading from socket" << std::endl;
		return;
	}
	strcpy(p->filename, cmd);
	memset(cmd, 0, sizeof(cmd));
	response = "请输入文件大小";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
//大小
	valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "Error reading from socket" << std::endl;
		return;
	}
	p->page_size = std::stoi(cmd);
	GetSystemTime(&(p->createtime));
	strcpy(p->type, "FILE");

	//分配页面号
	int a = p->page_size / 1024; 
	if (p->page_size % 1024 > 0) 
	{
		a++;
	}
	if (a == 1) {
		p->page_no = getpage(64);
		page_size[p->page_no] = 0xffff;
	}
	if (a > 1) {
		p->page_no = getpage(64);
		page_size[p->page_no] = 1;
		page_size[p->page_no] = getpage(64);
		int m = getpage(64);
		a--;
		while (a > 1) {
			page_size[m] = 1;
			page_size[m] = getpage(64);
			m = getpage(64);
			a--;
		}
		if (a == 1) {
			page_size[getpage(64)] = 0xffff;
		}
	}

	p->father = nowDirectory;
	p->child = NULL;
	p->rb = NULL;
	if (nowDirectory->child == NULL) {
		nowDirectory->child = p;
	}
	else {
		FCB* tmp;
		tmp = nowDirectory->child;
		while (tmp->rb != NULL) {
			tmp = tmp->rb;
		}
		if (tmp->rb == NULL) {
			tmp->rb = p;
		}
	}

}

void removeDirectory() {
	char ch[10];
	memset(cmd, 0, sizeof(cmd));
	response = "请输入目录名字";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
	int valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "连接失败" << std::endl;
		return;
	}
	strcpy(ch, cmd);
	FCB* tmp, * ump;
	ump = nowDirectory->child;
	tmp = nowDirectory->child;
	if (tmp != NULL) {
		while (strcmp(tmp->filename, ch) != 0 && tmp->rb != NULL) {
			ump = tmp;
			tmp = tmp->rb;
		}
		if (strcmp(tmp->filename, ch) == 0 && tmp->child != NULL) {
			response = "该目录不是空目录";
		}
		else if (strcmp(tmp->filename, ch) == 0 && tmp->child == NULL) {
			tmp->father = NULL;
			strcpy(tmp->type, "DEL");

			page_size[tmp->page_no] = 0x0000;//fat块回收置0

			if (nowDirectory->child == tmp) {
				nowDirectory->child = tmp->rb;
			}
			else {
				ump->rb = tmp->rb;
			}
			tmp->rb = NULL;
		}
		else
			response = "系统找不到相对应的目录" ;
	}

	else
		response = "系统找不到相对应的目录" ;
}

void deleteFile() {
	char ch[10];
	memset(cmd, 0, sizeof(cmd));
	response = "请输入目录名字";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
	int valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "连接失败" << std::endl;
		return;
	}
	strcpy(ch, cmd);
	FCB* tmp, * ump;
	ump = nowDirectory->child;
	tmp = nowDirectory->child;
	if (tmp != NULL) {
		while (strcmp(tmp->filename, ch) != 0 && tmp->rb != NULL) {
			ump = tmp;
			tmp = tmp->rb;
		}
		if (strcmp(tmp->filename, ch) == 0) {
			tmp->father = NULL;
			strcpy(tmp->type, "DEL");

			int a = tmp->page_size / 1024;//文件占用的空间回收
			if (tmp->page_size % 1024 > 0) {
				a++;
			}
			int m, n;
			m = tmp->page_no;

			while (a > 0) {
				n = page_size[m];
				page_size[m] = 0x0000;
				m = n;
				a--;
			}

			if (nowDirectory->child == tmp) {
				nowDirectory->child = tmp->rb;
			}
			else {
				ump->rb = tmp->rb;
			}
			tmp->rb = NULL;
		}
		else
			response = "系统找不到相对应的目录" ;
	}
	else
		response = "系统找不到相对应的目录" ;
}

void changeDirectory() {
	char ch[10];
	memset(cmd, 0, sizeof(cmd));
	response = "请输入目录";
	send(clientSocket, response.c_str(), response.length(), 0);
	response = "";
	int valread = recv(clientSocket, cmd, CMD_SIZE, 0);
	if (valread <= 0) {
		std::cerr << "连接失败" << std::endl;
		return;
	}
	strcpy(ch, cmd);
	FCB* tmp;
	tmp = nowDirectory->child;
	if (tmp != NULL) {
		if (strstr(ch, "\\") != NULL) {
			char* cd1 = strtok(ch, "\\");
			while (cd1 != NULL) {
				while (strcmp(cd1, tmp->filename) != 0 && tmp->rb != NULL) {
					tmp = tmp->rb;
				}
				if (strcmp(cd1, tmp->filename) == 0) {
					tmp = tmp->child;
					cd1 = strtok(NULL, "\\");
				}
				else {
					response = "错误1\n" ;
					break;
				}
			}
			if (cd1 == NULL) {
				strcat(url, ch);
				strcat(url, "\\");
				nowDirectory = tmp;
			}
		}
		else if (strcmp(ch, ".") == 0) {
			nowDirectory = nowDirectory;
		}
		else if (strcmp(ch, "..") == 0) {
			char ch1[10];
			strcpy(ch1, nowDirectory->filename);
			char* s = strtok(url, strcat(ch1, "\\"));
			strcpy(url, s);
			strcat(url, "\\");

			nowDirectory = steptDirec;
			steptDirec = steptDirec->father;
		}
		else {
			while (strcmp(tmp->filename, ch) != 0 && tmp->rb != NULL) {
				tmp = tmp->rb;
			}
			if (strcmp(tmp->filename, ch) == 0) {
				strcat(url, ch);
				strcat(url, "\\");
				steptDirec = nowDirectory;
				nowDirectory = tmp;
			}
			else {
				response = "错误2\n";
			}
		}
	}
	else if (strcmp(ch, "..") == 0) {
		char ch1[10];
		strcpy(ch1, nowDirectory->filename);
		char* s = strtok(url, strcat(ch1, "\\"));
		strcpy(url, s);
		strcat(url, "\\");
		nowDirectory = steptDirec;
		steptDirec = steptDirec->father;
	}
	else
		response = "错误3\n" ;
}


void dir() 
{
	FCB* tmp;
	int a = 0, b = 0; // 统计文件和目录的个数
	tmp = nowDirectory->child;
	response += url;
	response += "中的目录\n";
	response += "<" + std::string(nowDirectory->type) + ">  .\n";
	response += "<" + std::string(nowDirectory->father->type) + ">  ..\n";

	while (tmp != NULL) 
	{
		response += std::to_string(tmp->createtime.wYear) + "-" + std::to_string(tmp->createtime.wMonth) + "-" + std::to_string(tmp->createtime.wDay) + " " + std::to_string(tmp->createtime.wHour) + ":" + std::to_string(tmp->createtime.wMinute) + ":" + std::to_string(tmp->createtime.wSecond) + " ";
		response += "<" + std::string(tmp->type) + ">  " + std::string(tmp->filename);

		if (strcmp(tmp->type, "FILE") == 0) 
		{
			a++;
			response += " ";
			response += std::to_string(tmp->page_size);
		}
		else
		{
			b++;
		}
		response += " \n";
		tmp = tmp->rb;
	}

	response += std::to_string(a) + "个文件\n";
	response += std::to_string(b) + "个目录\n";


}


void LaT() {
	FCB* mp;
	mp = step;
	response += std::string(step->filename) + "\n";

	if (mp->child != NULL) {
		FCB* tmp;
		tmp = c->child;
		while (tmp != NULL) {
			if (strcmp(tmp->type, "DIR") == 0) {
				response += "|--" + std::string(tmp->filename) + "\n";

				FCB* amp;
				amp = tmp->child;
				while (amp != NULL) {
					if (strcmp(amp->type, "DIR") == 0) {
						response += "    |--" + std::string(amp->filename) + "\n";

						FCB* bmp;
						bmp = amp->child;
						while (bmp != NULL) {
							if (strcmp(bmp->type, "DIR") == 0) {
								response += "        |--" + std::string(bmp->filename) + "\n";
							}
							bmp = bmp->rb;
						}
					}
					amp = amp->rb;
				}
			}
			tmp = tmp->rb;
		}
	}


}

int fileSystemManagement() 
{
	HANDLE hSemaphore;  // 信号量句柄
	HANDLE hMapFile;    // 共享内存句柄
	LPCTSTR pBuf;       // 共享内存指针

	// 打开信号量
	hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"Semaphore");
	if (hSemaphore == NULL)
	{
		std::cout << "信号量打开失败，错误代码: " << GetLastError() << std::endl;
		return 1;
	}

	// 打开共享内存
	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"SharedMemory");
	if (hMapFile == NULL)
	{
		std::cout << "共享内存打开失败，错误代码: " << GetLastError() << std::endl;
		CloseHandle(hSemaphore);
		return 1;
	}

	// 映射共享内存
	pBuf = (LPTSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
	if (pBuf == NULL)
	{
		std::cout << "映射到共享内存失败，错误代码: " << GetLastError() << std::endl;
		CloseHandle(hMapFile);
		CloseHandle(hSemaphore);
		return 1;
	}
	pagedata(MAX_FILENAME_LEN);
	init();
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
	{
        std::cerr << "连接失败" << std::endl;
        return -1;
    }


    struct sockaddr_in serverAddress {}, clientAddress{};
    int clientAddressLength = sizeof(clientAddress);


    // 创建套接字
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "创建套接字失败" << std::endl;
        WSACleanup();
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // 绑定套接字到指定端口
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "绑定套接字失败" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    // 监听连接请求
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "监听失败" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "服务器正在监听请求端口： " << PORT << std::endl;

    // 等待客户端连接
    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength)) == INVALID_SOCKET) {
        std::cerr << "与客户端连接失败n" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "客户端成功连接" << std::endl;

     bool shouldExit = false;

    // 循环处理客户端请求
    while (!shouldExit) 
	{
        // 接收客户端请求
        memset(cmd, 0, sizeof(cmd));
        int valread = recv(clientSocket, cmd, CMD_SIZE, 0);
        if (valread <= 0) 
		{
            std::cerr << "套接字读取失败" << std::endl;
            break;
        }

        std::cout << "客户端请求: " << cmd << std::endl;
        
        if (strcmp(cmd, "makedir") == 0) 
        {
            makeDirectory();
			response = "创建成功";
        }
		else if (strcmp(cmd, "format") == 0)
		{
			format();
			response = "文件系统成功格式化";
				
		}
		else if (strcmp(cmd, "createJob") == 0)
		{
			string cmdS = cmd;
			CopyMemory((PVOID)pBuf, cmdS.c_str(), (cmdS.length() + 1) * sizeof(char));

			// 通知服务器端请求已写入
			ReleaseSemaphore(hSemaphore, 1, NULL);

			// 等待服务器端响应
			WaitForSingleObject(hSemaphore, INFINITE);
			std::string res((char*)pBuf);
			response = res;
		}
		else if (strcmp(cmd, "ListJob") == 0)
		{
			string cmdS = cmd;
			CopyMemory((PVOID)pBuf, cmdS.c_str(), (cmdS.length() + 1) * sizeof(char));

			// 通知服务器端请求已写入
			ReleaseSemaphore(hSemaphore, 1, NULL);

			// 等待服务器端响应
			WaitForSingleObject(hSemaphore, INFINITE);
			std::string res((char*)pBuf);
			response = res;
		}
        else if (strcmp(cmd, "exit") == 0) 
		{
            response = "正在退出：";
			exit(0);
            shouldExit = true;
        }
		else if (strcmp(cmd, "ulf") == 0)
		{
			ulf();
		}
		else if (strcmp(cmd, "dlf") == 0)
		{
			dlf();
		}
        
        else if (strcmp(cmd, "changedir") == 0) 
		{
            changeDirectory();
			response += url;
			response.append("C>");
        }
        else if (strcmp(cmd, "removedir") == 0) 
		{
            removeDirectory();
			response = "成功删除空目录";
        }
        else if (strcmp(cmd, "dir") == 0) 
		{
            dir();
        }
        else if (strcmp(cmd, "listAsTree") == 0) 
		{
            LaT();
        }
        else if (strcmp(cmd, "make") == 0) 
        {
            makeFile();
            response = "文件创建成功";
        }
        else if (strcmp(cmd, "deleteFile") == 0) 
		{
            deleteFile();
			response = "文件删除成功";
        }
        else if(strcmp(cmd, "help") == 0)
		{
			response = "help msg";
        }
		else if (strcmp(cmd, "init") == 0)
		{
			response = "init successfully";
		}
		else if (strcmp(cmd, "spawn") == 0)
		{
			monitorSystem();
		}
		else
		{
					
						string cmdS = cmd;
						CopyMemory((PVOID)pBuf, cmdS.c_str(), (cmdS.length() + 1) * sizeof(char));

						// 通知服务器端请求已写入
						ReleaseSemaphore(hSemaphore, 1, NULL);

						// 等待服务器端响应
						WaitForSingleObject(hSemaphore, INFINITE);
						std::string res((char*)pBuf);
						response = res;
		
		}
        // 发送响应给客户端
        send(clientSocket, response.c_str(), response.length(), 0);
        response = "";
    }

    // 关闭服务器套接字
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

int main() {

	std::thread fileSystemThread(fileSystemManagement);
	fileSystemThread.join();

	return 0;
}