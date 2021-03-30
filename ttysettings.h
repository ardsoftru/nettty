/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <unordered_map>
#include <cstring>
#include "settings.h"
#include "filecheck.h"


/*
��������� ��������� ��������� ���� TTY - TCP ����
*/
struct TTYSettings
{
	//�������� tty ����� ��� �������� ���
	std::string name;
	struct TTY
	{
		//�������� ������
		uint32_t baud;
		//���������� ���� ���
		uint16_t stopbits;
		//��� ��������
		uint16_t parity;
		//CTS-RTS flow control
		bool cts_rts;
	}tty;
	//������������ ����� �������� ������ �� ����������������� �����
	uint16_t timeout;
	//����������� �������� � ��
	uint8_t interval;
	//�������������� TCP ����
	uint16_t tcp_port;

	bool operator == (const TTYSettings & sett) const
	{
		return (this->name == sett.name) && (std::memcmp(&this->tty, &sett.tty, sizeof(TTY)) == 0) && (this->timeout == sett.timeout) && (this->interval == sett.interval);
	}

	bool operator != (const TTYSettings & sett) const
	{
		return !(*this == sett);
	}

	bool reconnectTCP(const TTYSettings & sett)
	{
		return this->tcp_port != sett.tcp_port;
	}

	bool reconnectTTY(const TTYSettings & sett)
	{
		return std::memcmp(&this->tty, &sett.tty, sizeof(TTY)) != 0;
	}
};

//������ ��������
typedef std::unordered_map<std::string, TTYSettings> TTY_SETTINGS_LIST;

class CTTYSettings : public CSettings
{
private:
	//���� � ����� � �������������
	std::string path;
	//������ �������� �� ������� �����
	TTY_SETTINGS_LIST list;
	//���� ��������� ��������
	bool changed = false;
	//����� ��� �������� ��������� � ����� � �������������
	CFileCheck file_check;

protected:
	/*
	������� ��������� ��������� � ini ����
	*/
	virtual void saveSettings(dictionary* ini) override;

	/*
	������� ������ ��������� �� ini �����
	*/
	virtual void loadSettings(dictionary* ini) override;

public:
	CTTYSettings(const std::string & path);

	/*
	��������� �� ������� ������������
	*/
	void Process();

	/*
	����� Get � Set �������
	*/
	bool GetChanged();
	void SetChanged(bool value);

	TTY_SETTINGS_LIST & List();
};