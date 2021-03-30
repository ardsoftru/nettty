/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include <stdarg.h>

#include "logs.h"
#include "auxil.h"

std::string def_log = "";

CLogs* logs = NULL;

/*
Функция выставляет значение файл с логами по умолчанию
*/
void SetDefLogFile(const std::string & deflog)
{
	def_log = deflog;
}

/*
Функция в указанный файл добавляет сообщение
*/
int WriteLog(const std::string & msg, const std::string & filename)
{
	// функция записи строки в лог
	std::stringstream ss;
	ss << msg << endl;

	cout << ss.str();

	if (filename == "")
		return 0;

	FILE * pLog;
	pLog = fopen(filename.c_str(), "a");
	if (pLog == NULL)
		return 1;

	fputs(ss.str().c_str(), pLog);
	fclose(pLog);

	return 0;
}

//Функция в указанный файл добавляет сообщение
int WriteLog(const std::string & msg)
{
	return WriteLog(msg, def_log);
}


void CreateLogs(const std::string & logfile)
{
	if (logs == NULL)
	{
		logs = new CLogs(logfile);
		logs->Start();
	}
}

//Удаление списка команд
void ClearLogs()
{
	if (logs != NULL)
	{
		logs->Stop();
		delete logs;
		logs = NULL;
	}
}

//Добавление записи
void AddLog(const std::string & message)
{
	if (logs == NULL)
		return;
	
	logs->Add(message);
}

void CLogs::Execute()
{
	while (!IsTerminated())
	{
		this->saveMessages();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	//Вдруг что осталось еще
	this->saveMessages();
}

CLogs::CLogs(const std::string & logfile)
	: CThread("logs_thread")
{
	this->logfile = logfile;
}

CLogs::~CLogs()
{
	this->Clear();
}

//Функция добавления sql команды в список
void CLogs::Add(const string & message)
{
	if (this->IsTerminated())
		return;

	std::lock_guard<std::mutex> lock(this->_mutex);
	
	CLog* log = new CLog(message);
	this->messages.push(log);
}

//Извлекает из списка первую команду на выполнение
std::string CLogs::getFirst()
{
	std::lock_guard<std::mutex> lock(this->_mutex);
	
	if (this->messages.empty())
		return "";
	
	CLog* log = this->messages.front();
	std::string res = log->Message();
	this->messages.pop();
	delete log;
	
	return res;
}

void CLogs::saveMessages()
{
	std::string message;
	while ((message = this->getFirst()) != "")
	{
		//До тех пор пока есть данные поток крутится в этом цикле
		WriteLog(message, this->logfile);
	}
}

//Очистка списка команд
void CLogs::Clear()
{
	std::lock_guard<std::mutex> lock(this->_mutex);
	
	while (!this->messages.empty())
	{
		CLog* log = this->messages.front();
		this->messages.pop();
		delete log;
	}
}
