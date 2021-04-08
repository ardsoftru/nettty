/*-------------------------------------------------------------------------*/
/**
   @file    tcpserver.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <unordered_map>
#include <queue>
#include <vector>
#include <memory>

#include "stdio.h"

using SEND_DATA_LIST = std::queue<std::vector<uint8_t>>;

/*
  Экземпляр описывающий TCP клиента
*/
class CTCPServerClient
{
	friend class CTCPServer;

private:
	//Сокет TCP клиента
	int sock;
	//Межбайтовый интервал
	uint64_t interval;
	//Список для отправки паектов
	SEND_DATA_LIST sendDataList;
	//Флаг разрешающий запись
	bool enableSend = false;
	
	epoll_event connevnt;

	/*
	Отправка данных
	*/
	bool sendData();

protected:
	//Приемный буфер
	std::vector<uint8_t> receiveBuff;
	//Время посднего приема байт
	uint64_t rec_stamp;
	//Флаг ожидания
	bool waiting;

	/*
	Прием данных
	*/
	virtual bool receiveData();


public: 
	CTCPServerClient(int sock);
	virtual ~CTCPServerClient();
	
	/*
	Возвращает сокет клиента
	*/
	int GetSocket();
	
	/*
	Закрытие сокета
	*/
	void Close();
	
	template <typename T>
	void Send(T&& buff)
	{
		this->sendDataList.push(std::forward<T>(buff));
		this->sendData();
	}
	
	/*
	Возвращает в указанный буфер данные
	*/
	std::vector<uint8_t> GetReceivedData();
	
	epoll_event & GetConnEvnt();
};

using TCPSERVER_CLIENTS_LIST = std::unordered_map<int, std::unique_ptr<CTCPServerClient>>;

/*
  Класс отвечающий за функционал TCP сервера
*/
class CTCPServer
{
private:
	//Максимальное количество одновременно подключенных клиентов
	uint32_t maxConnections;
	//сокет TCP сервера
	int sock;
	//Hadle возвращаемый функцией epoll_create1
	int efd;
	epoll_event listenevnt;
	
	//Наборы для работы
	fd_set readSet;
	fd_set writeSet;
	fd_set exceptSet;	

	/*
	Сокету выставляется неблокирующий режим работы
	*/
	void setnonblocking(int sock);

	/*
	Функция закрывает файловые дескрипторы сокета и epoll
	*/
	void close_descriptors();
	
protected:
	//Порт который прослушивает TCP сервер
	uint16_t port;
	//Межбайтовый интервал
	uint64_t interval;

	//Список TCP клиентов
	TCPSERVER_CLIENTS_LIST clients;

	/*
	Функция создаёт экземпляр класса клиента установившего соединение
	*/
	virtual std::unique_ptr<CTCPServerClient> createClient(int sock);

	/*
	Функция удаления клиента из списка
	*/
	virtual void deleteClient(TCPSERVER_CLIENTS_LIST::iterator& client);
	
public:
	CTCPServer() = delete;
	CTCPServer(uint64_t interval, uint32_t maxConnections = 0);
	virtual ~CTCPServer(void);
	
	/*
	Запуск TCP сервера
	*/
	void Open(const std::string & interface, uint16_t port);
	
	/*
	Останов TCP сервера
	*/
	virtual void Close();
	
	/*
	флаг успешного открытия сокета
	*/
	bool Opened();
	
	/*
	Функция работает с клиентами
	*/
	bool Accept(int timeout = 0);

	inline uint64_t Interval()
	{
		return this->interval;
	}

	inline void Interval(uint64_t value)
	{
		this->interval = value;
	}

	/*
	Список TCP клиентов
	*/
	TCPSERVER_CLIENTS_LIST & GetClients();
};