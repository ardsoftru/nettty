/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <unordered_map>
#include <mutex>
#include <unordered_map>

#include "cthread.h"
#include "tcpserver.h"
#include "serial.h"
#include "ttysettings.h"

/*
Класс работающий с пришедшим по сети Ethernet пакетом
*/
class CPacket
{
private:
	//Буфер с данными для отправки
	std::vector<uint8_t> send_data;
	//Буфер с принятыми данными
	std::vector<uint8_t> rec_data;
	//Уникальный идентификатор TCP клиента
	int client_id;
	//Время отправки данных в последовательный интерфейс
	uint64_t send_time;

public:
	explicit CPacket(int id)
		: client_id{id}, send_time{0}
	{

	}

	/*
	"Запоминает" пакет с данными для отправки в последовательный порт
	*/
	template<typename T>
	void SetSendBuff(T&& buff)
	{
		this->send_data = std::forward<T>(buff);
	}

	/*
	"Запоминает" пакет с данными для отправки по TCP соединению
	*/
	template<typename T>
	void SetRecBuff(T&& buff)
	{
		this->rec_data = std::forward<T>(buff);
	}

	std::vector<uint8_t> GetSendBuff()
	{
		return std::move(this->send_data);
	}

	std::vector<uint8_t> GetRecBuff()
	{
		return std::move(this->rec_data);
	}

	void SendTime(uint64_t timestamp)
	{
		this->send_time = timestamp;
	}

	uint64_t SendTime()
	{
		return this->send_time;
	}

	int ClientID()
	{
		return this->client_id;
	}
};

/*
Поток в котором крутится связка TCP порт <--> TTY
*/
class CTTYServerThread : public CThread
{
private:
	//Мьютекс разграничивающий доступ к списку с командами
	std::mutex _mutex;
	//Очередь пакетов для обработки
	std::queue<CPacket> data_queue;

	//Флаг необходимости переинициализировать TCP сервер
	bool reconnectTCP;
	//Флаг необходимости переинициализировать последовательный интерфейс
	bool reconnectTTY;

	//Экземпляр класса работающий как TCP сервер
	CTCPServer server;
	//Экземпляр класса работающего с последовательным интерфейсом
	CSerial serial;
	//Настройки потока
	TTYSettings settings;

	/*
	Функция отрабатывающая 
	*/
	void checkSettingsChange();

	/*
	Работа TCP сервера
	*/
	void processTCPServer();

	/*
	Работа последовательного интерфейса
	*/
	void processTTY();

protected:
	void Execute() override; 

public:
	/*
	Конструктор для создания именованного потока
	*/
	template<typename T>
	CTTYServerThread(const std::string& name, T&& sett)
		: CThread(name), server{ 0, 0 }, serial{ name }, settings{ std::forward<T>(sett) }
	{

	}

	/*
	Функция очистки очереди пакетов
	*/
	void ClearQueue();

	/*
	Выставляются новые настройки
	*/
	void SetSettings(TTYSettings & settings);
};

using TTY_SERVER_LIST = std::unordered_map<std::string, std::unique_ptr<CTTYServerThread>>;

/*
Класс список для работы с классами CTTYServerThread
*/
class CTTYServerThreadList
{
private:
	//Список активных пар TTY - TCP порт
	TTY_SERVER_LIST list;

public:
	CTTYServerThreadList();

	/*
	Очистка списка
	*/
	void Clear();

	/*
	Отработка по настройкам
	*/
	void UpdateConfig(CTTYSettings & settings);
};