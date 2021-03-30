/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <thread>
#include <atomic>
#include <mutex>

/*
����������� ����� ������� ��� ����������� thread ����������� ������ � 
*/
class CThread
{
private:
	//��������� ������
	std::thread _thread;
	//��� ������
	std::string name;
	//�������
	std::mutex _mutex;
	//���� �������� ������
	std::atomic_bool terminated;

protected:
	/*
	������� �� �������
	*/
	void Terminate();

	/*
	���������� true ���� ��������� ������� �� ������� ������
	*/
	bool IsTerminated();

	/*
	����������� ������� � ������� ����������� ������ ������
	*/
	virtual void Execute() = 0;


public:
	/*
	������� �����������
	*/
	CThread();

	/*
	����������� ��� �������� ������������ ������
	*/
	CThread(const std::string & thread_name);

	virtual ~CThread();

	/*
	������� ������� ������
	*/
	bool Start();

	/*
	������� �������� ������
	*/
	void Stop();

	/*
	���������� true ���� ����� �������
	*/
	bool Running();

	const std::string& Name()
	{
		return this->name;
	}
};

