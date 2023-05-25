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
struct Job {
    int job_id;  // 作业ID
    int pid;  // 进程ID
    int status;  // 运行状态（0为未启动，1为运行中，2为已暂停，3为已终止，4为已完成）
    double progress;  // 完成比率（0到1之间）
    double jobwork;
    time_t start_time;  // 开始时间
    time_t end_time;  // 结束时间
};
struct JobLog {
    int job_id;  // 作业ID
    int status;  // 运行状态（0为未启动，1为运行中，2为已暂停，3为已终止，4为已完成）
    double progress;  // 完成比率（0到1之间）
    time_t timestamp;  // 时间戳
};
std::vector<Job> jobList;
std::vector<JobLog> jobLog;
#define SHARED_MEM_SIZE 1024  // 共享内存大小

int StartNewProcess() {
	return static_cast<int>(std::time(nullptr));
}
int GenerateUniqueJobID() {
	static int jobID = 0;
	return ++jobID;
}
void monitorJobStatus()
{
	int totalJobs = jobList.size();
	int runningJobs = 0;
	int successJobs = 0;
	int canceledJobs = 0;
	int pausedJobs = 0;

	for (const Job& job : jobList)
	{
		if (job.status == 1)
			runningJobs++;
		else if (job.status == 2)
			canceledJobs++;
		else if (job.status == 3)
			pausedJobs++;
		else if (job.status == 4)
			successJobs++;
	}
	std::string totalJobsStr = std::to_string(jobList.size());
	std::string runningJobsStr = std::to_string(runningJobs);
	std::string successJobsStr = std::to_string(successJobs);
	std::string canceledJobsStr = std::to_string(canceledJobs);
	std::string pausedJobsStr = std::to_string(pausedJobs);


	response += "Total Jobs: " + totalJobsStr + "\n";
	response += "Running Jobs: " + runningJobsStr + "\n";
	response += "Success Jobs: " + successJobsStr + "\n";
	response += "Canceled Jobs: " + canceledJobsStr + "\n";
	response += "Paused Jobs: " + pausedJobsStr + "\n";
}
int main()
{
	HANDLE hSemaphore;  // 信号量句柄
	HANDLE hMapFile;    // 共享内存句柄
	LPCTSTR pBuf;       // 共享内存指针

	// 创建信号量
	hSemaphore = CreateSemaphore(NULL, 0, 1, L"Semaphore");
	if (hSemaphore == NULL)
	{
		std::cout << "创建信号量失败，错误代码: " << GetLastError() << std::endl;
		return 1;
	}

	// 创建共享内存
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHARED_MEM_SIZE, L"SharedMemory");
	if (hMapFile == NULL)
	{
		std::cout << "创建共享内存失败，错误代码: " << GetLastError() << std::endl;
		CloseHandle(hSemaphore);
		return 1;
	}

	// 映射共享内存
	pBuf = (LPTSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
	if (pBuf == NULL)
	{
		std::cout << "映射共享内存失败，错误代码: " << GetLastError() << std::endl;
		CloseHandle(hMapFile);
		CloseHandle(hSemaphore);
		return 1;
	}

	while (true)
	{
		// 等待客户端请求
		std::cout << "作业管理系统等待客户端请求中：" << std::endl;
		WaitForSingleObject(hSemaphore, INFINITE);

		// 读取客户端请求
		std::string request((char*)pBuf);
		std::cout << "从客户端接收到请求: " << request << std::endl;
		std::string requestType;
		int jobProcessID;
		std::istringstream iss(request);
		iss >> requestType >> jobProcessID;
		if (request == "createJob")
		{
			Job newJob;
			newJob.job_id = GenerateUniqueJobID();  // 生成唯一的作业ID
			newJob.pid = StartNewProcess();  // 启动新的进程
			newJob.status = 1;  // 设置运行状态为启动
			newJob.progress = 0.0f;  // 设置完成比率为0
			newJob.start_time = time(NULL);  // 记录开始时间
			newJob.end_time = 0;  // 结束时间暂未知
			newJob.jobwork = 0;

			// 将新作业添加到作业列表
			jobList.push_back(newJob);

			// 假设服务器响应为 "job created"
			response = "Job Created Successfully\n";
			response += "Job ID: " + std::to_string(newJob.job_id) + "\n";
			response += "Process ID: " + std::to_string(newJob.pid) + "\n";
			response += "Status: " + std::to_string(newJob.status) + "\n";
			response += "Progress: " + std::to_string(newJob.progress) + "\n";
		}
		else if (request == "ListJob")
		{
			std::string jobInfo;

			// 遍历作业列表，获取每个作业的信息
			for (const Job& job : jobList)
			{
				jobInfo += "Job ID: " + std::to_string(job.job_id) + "\n";
				jobInfo += "Process ID: " + std::to_string(job.pid) + "\n";
				jobInfo += "Status: " + std::to_string(job.status) + "\n";
				jobInfo += "Progress: " + std::to_string(job.progress) + "\n";
				jobInfo += "Start Time: " + std::to_string(job.start_time) + "\n";
				jobInfo += "End Time: " + std::to_string(job.end_time) + "\n";
			}

			// 假设服务器响应为作业信息
			response = jobInfo;
		}
		else if (requestType == "stopJob")
		{
			jobList[jobProcessID - 1].status = 2;
			response = "作业进程号为 " + std::to_string(jobProcessID) + " 的作业已停止";
		}
		else if (requestType == "startJob")
		{
			jobList[jobProcessID - 1].status = 1;
			response = "作业进程号为 " + std::to_string(jobProcessID) + " 的作业已开始";
		}
		else if (requestType == "removeJob")
		{
			jobList[jobProcessID - 1].status = 3;
			response = "作业进程号为 " + std::to_string(jobProcessID) + " 的作业已终止";
		}
		else 
		{
			response = "不是内部或外部命令，也不是可运行的程序";
		}
		if (!jobList.empty())
		{
			for (int i = 0; i < jobList.size(); i++)
			{
				if (jobList[i].status == 1 && jobList[i].jobwork != 10)
				{
					jobList[i].jobwork++;
					jobList[i].progress = jobList[i].jobwork / 10;
				}
				if (jobList[i].jobwork == 10)
				{
					jobList[i].status = 4;
				}
			}
		}

		// 将响应写入共享内存
		CopyMemory((PVOID)pBuf, response.c_str(), (response.length() + 1) * sizeof(char));

		// 通知客户端响应已写入
		ReleaseSemaphore(hSemaphore, 1, NULL);
	}

	// 清理资源
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	CloseHandle(hSemaphore);

	return 0;

}