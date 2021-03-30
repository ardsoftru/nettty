/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <mutex>
#include <sys/time.h>
#include <queue>
#include "time.h"
#include "string.h"

#include "cthread.h"

using namespace std;

const std::string DATEFMT = "DD.MM.YYYY hh:mm:ss";

/*
������� ���������� �������� ���� � ������ �� ���������
*/

void SetDefLogFile(const std::string & deflog);

/*
������� � ��������� ���� ��������� ���������
*/
int WriteLog(const std::string & msg, const std::string & filename);
int WriteLog(const std::string & msg);

class CLogs : public CThread
{
	class CLog
	{
	private:
		struct timeval tv;
		struct timezone tz;
		struct tm tm;
		std::string message;
		
	public:
		CLog(const std::string & message)
		{
			this->message = message;
			gettimeofday(&this->tv, &this->tz);
			localtime_r(&this->tv.tv_sec, &this->tm);
		}
		
		std::string Message()
		{
			char str[DATEFMT.length() + 1];
			
			memset(str, 0, sizeof(str));
			strftime(str, sizeof(str), "%d.%m.%Y %H:%M:%S", &this->tm);
			
			return std::string(str) + "." + std::to_string(this->tv.tv_usec / 1000) + " -- " + this->message;
		}
	};
	
private:
	//������ ���������
	std::queue<CLog*> messages;
	//���� � ������� ���� ������
	std::string logfile;
	//������� ���������������� ������ � ������ � ���������
	std::mutex _mutex;

	//��������� �� ������ ������ ������� �� ����������
	std::string getFirst();
	//���������� ������ ��������� 
	void saveMessages();

protected:
	virtual void Execute();
	
public:
	
	CLogs(const std::string & logfile);
	~CLogs();
	
	//������� ���������� sql ������� � ������
	void Add(const string & message);
	//������� ������ ������
	void Clear();
};

//������ ����� ���������� �� ������� ������
void CreateLogs(const std::string & logfile);
//�������� ������ ������
void ClearLogs();
//���������� ������ 
void AddLog(const std::string & message);
