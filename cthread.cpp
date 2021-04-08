/*-------------------------------------------------------------------------*/
/**
   @file    cthread.cpp
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include "cthread.h"

/*
------------------------------------------------------------------------------
Реализация методов класса CTCPServer
------------------------------------------------------------------------------
*/

void CThread::Terminate()
{
	this->terminated = true;
}

CThread::CThread()
{
}

CThread::CThread(const std::string & thread_name)
	: name{ thread_name }
{
}

CThread::~CThread()
{
	this->Stop();
}

bool CThread::Start()
{
	//Блокируется _thread
	std::lock_guard<std::mutex> lock(this->_mutex);
	if (this->_thread.joinable() )
		return false;

	//Запуск потока
	this->terminated = false;
	this->_thread = std::thread([this]() 
		{
			this->Execute();
		});

	//Если указано имя то его надо присвоить потоку
	if (this->name != "")
		pthread_setname_np(this->_thread.native_handle(), this->name.c_str());

	return true;
}

void CThread::Stop()
{
	//Блокируется _thread
	std::lock_guard<std::mutex> lock(this->_mutex);
	if (!this->_thread.joinable())
		return;

	if (!this->IsTerminated())
		this->Terminate();
	this->_thread.join();
}

bool CThread::Running()
{
	return this->_thread.joinable();
}

bool CThread::IsTerminated()
{
	return this->terminated;
}

