/*-------------------------------------------------------------------------*/
/**
   @file    cthread.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <thread>
#include <atomic>
#include <mutex>

/*
Абстрактный класс обертка над стандартным thread реализующий работу с 
*/
class CThread
{
private:
	//Экземпляр потока
	std::thread _thread;
	//Имя потока
	std::string name;
	//Мьютекс
	std::mutex _mutex;
	//Флаг останова потока
	std::atomic_bool terminated;

protected:
	/*
	Команда на останов
	*/
	void Terminate();

	/*
	Возвращает true если поступила команда на останов потока
	*/
	bool IsTerminated();

	/*
	Абстрактная функция в которой выполняется работа потока
	*/
	virtual void Execute() = 0;


public:
	/*
	Обычный конструктор
	*/
	CThread();

	/*
	Конструктор для создания именованного потока
	*/
	CThread(const std::string & thread_name);

	virtual ~CThread();

	/*
	Функция запуска потока
	*/
	bool Start();

	/*
	Функция останова потока
	*/
	void Stop();

	/*
	Возвращает true если поток запущен
	*/
	bool Running();

	const std::string& Name()
	{
		return this->name;
	}
};

