/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
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
  ��������� ����������� TCP �������
*/
class CTCPServerClient
{
	friend class CTCPServer;

private:
	//����� TCP �������
	int sock;
	//����������� ��������
	uint64_t interval;
	//������ ��� �������� �������
	SEND_DATA_LIST sendDataList;
	//���� ����������� ������
	bool enableSend = false;
	
	epoll_event connevnt;

	/*
	�������� ������
	*/
	bool sendData();

protected:
	//�������� �����
	std::vector<uint8_t> receiveBuff;
	//����� �������� ������ ����
	uint64_t rec_stamp;
	//���� ��������
	bool waiting;

	/*
	����� ������
	*/
	virtual bool receiveData();


public: 
	CTCPServerClient(int sock);
	virtual ~CTCPServerClient();
	
	/*
	���������� ����� �������
	*/
	int GetSocket();
	
	/*
	�������� ������
	*/
	void Close();
	
	template <typename T>
	void Send(T&& buff)
	{
		this->sendDataList.push(std::forward<T>(buff));
		this->sendData();
	}
	
	/*
	���������� � ��������� ����� ������
	*/
	std::vector<uint8_t> GetReceivedData();
	
	epoll_event & GetConnEvnt();
};

using TCPSERVER_CLIENTS_LIST = std::unordered_map<int, std::unique_ptr<CTCPServerClient>>;

/*
  ����� ���������� �� ���������� TCP �������
*/
class CTCPServer
{
private:
	//������������ ���������� ������������ ������������ ��������
	uint32_t maxConnections;
	//����� TCP �������
	int sock;
	//Hadle ������������ �������� epoll_create1
	int efd;
	epoll_event listenevnt;
	
	//������ ��� ������
	fd_set readSet;
	fd_set writeSet;
	fd_set exceptSet;	

	/*
	������ ������������ ������������� ����� ������
	*/
	void setnonblocking(int sock);

	/*
	������� ��������� �������� ����������� ������ � epoll
	*/
	void close_descriptors();
	
protected:
	//���� ������� ������������ TCP ������
	uint16_t port;
	//����������� ��������
	uint64_t interval;

	//������ TCP ��������
	TCPSERVER_CLIENTS_LIST clients;

	/*
	������� ������ ��������� ������ ������� ������������� ����������
	*/
	virtual std::unique_ptr<CTCPServerClient> createClient(int sock);

	/*
	������� �������� ������� �� ������
	*/
	virtual void deleteClient(TCPSERVER_CLIENTS_LIST::iterator& client);
	
public:
	CTCPServer() = delete;
	CTCPServer(uint64_t interval, uint32_t maxConnections = 0);
	virtual ~CTCPServer(void);
	
	/*
	������ TCP �������
	*/
	void Open(const std::string & interface, uint16_t port);
	
	/*
	������� TCP �������
	*/
	virtual void Close();
	
	/*
	���� ��������� �������� ������
	*/
	bool Opened();
	
	/*
	������� �������� � ���������
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
	������ TCP ��������
	*/
	TCPSERVER_CLIENTS_LIST & GetClients();
};