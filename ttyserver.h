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
����� ���������� � ��������� �� ���� Ethernet �������
*/
class CPacket
{
private:
	//����� � ������� ��� ��������
	std::vector<uint8_t> send_data;
	//����� � ��������� �������
	std::vector<uint8_t> rec_data;
	//���������� ������������� TCP �������
	int client_id;
	//����� �������� ������ � ���������������� ���������
	uint64_t send_time;

public:
	explicit CPacket(int id)
		: client_id{id}, send_time{0}
	{

	}

	/*
	"����������" ����� � ������� ��� �������� � ���������������� ����
	*/
	template<typename T>
	void SetSendBuff(T&& buff)
	{
		this->send_data = std::forward<T>(buff);
	}

	/*
	"����������" ����� � ������� ��� �������� �� TCP ����������
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
����� � ������� �������� ������ TCP ���� <--> TTY
*/
class CTTYServerThread : public CThread
{
private:
	//������� ���������������� ������ � ������ � ���������
	std::mutex _mutex;
	//������� ������� ��� ���������
	std::queue<CPacket> data_queue;

	//���� ������������� �������������������� TCP ������
	bool reconnectTCP;
	//���� ������������� �������������������� ���������������� ���������
	bool reconnectTTY;

	//��������� ������ ���������� ��� TCP ������
	CTCPServer server;
	//��������� ������ ����������� � ���������������� �����������
	CSerial serial;
	//��������� ������
	TTYSettings settings;

	/*
	������� �������������� 
	*/
	void checkSettingsChange();

	/*
	������ TCP �������
	*/
	void processTCPServer();

	/*
	������ ����������������� ����������
	*/
	void processTTY();

protected:
	void Execute() override; 

public:
	/*
	����������� ��� �������� ������������ ������
	*/
	template<typename T>
	CTTYServerThread(const std::string& name, T&& sett)
		: CThread(name), server{ 0, 0 }, serial{ name }, settings{ std::forward<T>(sett) }
	{

	}

	/*
	������� ������� ������� �������
	*/
	void ClearQueue();

	/*
	������������ ����� ���������
	*/
	void SetSettings(TTYSettings & settings);
};

using TTY_SERVER_LIST = std::unordered_map<std::string, std::unique_ptr<CTTYServerThread>>;

/*
����� ������ ��� ������ � �������� CTTYServerThread
*/
class CTTYServerThreadList
{
private:
	//������ �������� ��� TTY - TCP ����
	TTY_SERVER_LIST list;

public:
	CTTYServerThreadList();

	/*
	������� ������
	*/
	void Clear();

	/*
	��������� �� ����������
	*/
	void UpdateConfig(CTTYSettings & settings);
};