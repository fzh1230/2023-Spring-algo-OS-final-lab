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
#define SHARED_MEM_SIZE 1024  // �����ڴ��С
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
    int job_id;  // ��ҵID
    int pid;  // ����ID
    int status;  // ����״̬��0Ϊδ������1Ϊ�����У�2Ϊ����ͣ��3Ϊ����ֹ��4Ϊ����ɣ�
    double progress;  // ��ɱ��ʣ�0��1֮�䣩
    double jobwork;
    time_t start_time;  // ��ʼʱ��
    time_t end_time;  // ����ʱ��
};
struct JobLog {
    int job_id;  // ��ҵID
    int status;  // ����״̬��0Ϊδ������1Ϊ�����У�2Ϊ����ͣ��3Ϊ����ֹ��4Ϊ����ɣ�
    double progress;  // ��ɱ��ʣ�0��1֮�䣩
    time_t timestamp;  // ʱ���
};
std::vector<Job> jobList;
std::vector<JobLog> jobLog;
#define SHARED_MEM_SIZE 1024  // �����ڴ��С

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
	HANDLE hSemaphore;  // �ź������
	HANDLE hMapFile;    // �����ڴ���
	LPCTSTR pBuf;       // �����ڴ�ָ��

	// �����ź���
	hSemaphore = CreateSemaphore(NULL, 0, 1, L"Semaphore");
	if (hSemaphore == NULL)
	{
		std::cout << "�����ź���ʧ�ܣ��������: " << GetLastError() << std::endl;
		return 1;
	}

	// ���������ڴ�
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHARED_MEM_SIZE, L"SharedMemory");
	if (hMapFile == NULL)
	{
		std::cout << "���������ڴ�ʧ�ܣ��������: " << GetLastError() << std::endl;
		CloseHandle(hSemaphore);
		return 1;
	}

	// ӳ�乲���ڴ�
	pBuf = (LPTSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
	if (pBuf == NULL)
	{
		std::cout << "ӳ�乲���ڴ�ʧ�ܣ��������: " << GetLastError() << std::endl;
		CloseHandle(hMapFile);
		CloseHandle(hSemaphore);
		return 1;
	}

	while (true)
	{
		// �ȴ��ͻ�������
		std::cout << "��ҵ����ϵͳ�ȴ��ͻ��������У�" << std::endl;
		WaitForSingleObject(hSemaphore, INFINITE);

		// ��ȡ�ͻ�������
		std::string request((char*)pBuf);
		std::cout << "�ӿͻ��˽��յ�����: " << request << std::endl;
		std::string requestType;
		int jobProcessID;
		std::istringstream iss(request);
		iss >> requestType >> jobProcessID;
		if (request == "createJob")
		{
			Job newJob;
			newJob.job_id = GenerateUniqueJobID();  // ����Ψһ����ҵID
			newJob.pid = StartNewProcess();  // �����µĽ���
			newJob.status = 1;  // ��������״̬Ϊ����
			newJob.progress = 0.0f;  // ������ɱ���Ϊ0
			newJob.start_time = time(NULL);  // ��¼��ʼʱ��
			newJob.end_time = 0;  // ����ʱ����δ֪
			newJob.jobwork = 0;

			// ������ҵ��ӵ���ҵ�б�
			jobList.push_back(newJob);

			// �����������ӦΪ "job created"
			response = "Job Created Successfully\n";
			response += "Job ID: " + std::to_string(newJob.job_id) + "\n";
			response += "Process ID: " + std::to_string(newJob.pid) + "\n";
			response += "Status: " + std::to_string(newJob.status) + "\n";
			response += "Progress: " + std::to_string(newJob.progress) + "\n";
		}
		else if (request == "ListJob")
		{
			std::string jobInfo;

			// ������ҵ�б���ȡÿ����ҵ����Ϣ
			for (const Job& job : jobList)
			{
				jobInfo += "Job ID: " + std::to_string(job.job_id) + "\n";
				jobInfo += "Process ID: " + std::to_string(job.pid) + "\n";
				jobInfo += "Status: " + std::to_string(job.status) + "\n";
				jobInfo += "Progress: " + std::to_string(job.progress) + "\n";
				jobInfo += "Start Time: " + std::to_string(job.start_time) + "\n";
				jobInfo += "End Time: " + std::to_string(job.end_time) + "\n";
			}

			// �����������ӦΪ��ҵ��Ϣ
			response = jobInfo;
		}
		else if (requestType == "stopJob")
		{
			jobList[jobProcessID - 1].status = 2;
			response = "��ҵ���̺�Ϊ " + std::to_string(jobProcessID) + " ����ҵ��ֹͣ";
		}
		else if (requestType == "startJob")
		{
			jobList[jobProcessID - 1].status = 1;
			response = "��ҵ���̺�Ϊ " + std::to_string(jobProcessID) + " ����ҵ�ѿ�ʼ";
		}
		else if (requestType == "removeJob")
		{
			jobList[jobProcessID - 1].status = 3;
			response = "��ҵ���̺�Ϊ " + std::to_string(jobProcessID) + " ����ҵ����ֹ";
		}
		else 
		{
			response = "�����ڲ����ⲿ���Ҳ���ǿ����еĳ���";
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

		// ����Ӧд�빲���ڴ�
		CopyMemory((PVOID)pBuf, response.c_str(), (response.length() + 1) * sizeof(char));

		// ֪ͨ�ͻ�����Ӧ��д��
		ReleaseSemaphore(hSemaphore, 1, NULL);
	}

	// ������Դ
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	CloseHandle(hSemaphore);

	return 0;

}