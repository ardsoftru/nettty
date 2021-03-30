/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <unordered_map>

#include <termios.h>
#include <sys/select.h>
#include <vector>

#include "auxil.h"

/*
��������� �������� COM �����
*/

//���� ��������
#define PARITY_NONE		0
#define PARITY_ODD		1
#define PARITY_EVEN		2
	
//���� ����
#define STOPBITS_ONE	0
#define STOPBITS_TWO	1

/*
������ ������ COM �����
*/
#define COM_BUFF_SIZE 0xFFFF
	
/*
����� �������������� �� COM �����
*/
class CSerial
{
private:
	//������ �� �������� ����
	int fd;
	//������ ���� � �����
	std::string full_path;
	
	//���� �������� ���������� �������
	bool waiting;
	
	//����� ���������� ������ ����
	uint64_t rec_stamp;
	//�������� �����
	std::vector<uint8_t> data;
	//����� ��� �������� ������
	std::vector<uint8_t> send_data;

	/*
	���������� ������� �������� ������
	*/
	bool send();
	
protected:	
	//��� �����
	std::string fname;
	//��������� �����
	uint32_t baud;
	uint16_t stopbits;
	uint16_t parity;
	bool cts_rts;
	//����������� �������� � ��
	uint32_t interval;

	
public:
	CSerial() = delete;
	CSerial(const std::string & fname);
	virtual ~CSerial();
	
	/*
	������� ��������� COM ����
	*/
	virtual bool Open();
	
	/*
	������� ��������� COM ����
	*/
	void Close();

	/*
	����� ������
	*/
	bool Receive();
	
	/*
	�������� ������
	� �������� T ������������ std::vector<uint8_t>
	*/
	template <typename T>
	bool Send(T&& buff)
	{
		this->send_data = std::forward<T>(buff);
		return this->send();
	}
	
	/*
	���������� � ��������� ����� ������
	*/
	virtual std::vector<uint8_t> GetReceivedData();
	
	/*
	���� �������� ������ �����
	*/
	inline bool Waiting()
	{
		return this->waiting;
	}
	
	/*
	����� Get � Set ������� ������
	*/
	inline uint32_t Interval()
	{
		return this->interval;
	}

	inline void Interval(uint16_t interval)
	{
		this->interval = interval;
	}

	inline uint32_t Baud()
	{
		return this->baud;
	}

	inline void Baud(uint32_t baud)
	{
		this->baud = baud;
	}

	inline uint16_t StopBits()
	{
		return this->stopbits;
	}

	inline void StopBits(uint16_t stopbits)
	{
		this->stopbits = stopbits;
	}

	inline uint16_t Parity()
	{
		return this->parity;
	}

	inline void Parity(uint16_t parity)
	{
		this->parity = parity;
	}

	inline bool CTSRTS()
	{
		return this->cts_rts;
	}

	inline void CTSRTS(bool value)
	{
		this->cts_rts = value;
	}
	
	/*
	���� ��������� ������
	*/
	inline bool Opened()
	{
		return this->fd != -1;
	}
};
