/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include "ttyserver.h"

#include <algorithm>
#include <cstring>
#include <list>

#ifdef DEBUG
#include "logs.h"
#include "auxil.h"
#endif // DEBUG



/*
------------------------------------------------------------------------------
���������� ������� ������ CTTYServerThreads
------------------------------------------------------------------------------
*/

void CTTYServerThread::checkSettingsChange()
{
	std::lock_guard<std::mutex> lock(this->_mutex);
	if (this->reconnectTCP)
	{
		//���������� ������������� TCP �����
		this->reconnectTCP = false;
		this->ClearQueue();
		this->server.Close();
	}
	if (this->reconnectTTY)
	{
		//���������� ������������� TTY �����
		this->reconnectTTY = false;
		this->serial.Close();
		this->serial.Baud(this->settings.tty.baud);
		this->serial.StopBits(this->settings.tty.stopbits);
		this->serial.Parity(this->settings.tty.parity);
		this->serial.CTSRTS(this->settings.tty.cts_rts);
	}
	//��� ��������� �� ������� �����������������
	this->serial.Interval(this->settings.interval);
}

void CTTYServerThread::processTCPServer()
{
	{
		std::lock_guard<std::mutex> lock(this->_mutex);
		this->server.Open("", this->settings.tcp_port);
	}
	if (!this->server.Opened())
		return;

	if (!this->server.Accept())
	{
		this->ClearQueue();
		return;
	}

	for (const auto& it : this->server.GetClients())
	{
		auto& client = it.second;
		std::vector<uint8_t> buff = client->GetReceivedData();
		if (buff.empty())
			continue;

		if (this->serial.Opened())
		{
			//������������ ������ � ���������� ��� � �������
			CPacket packet{ client->GetSocket() };
			packet.SetSendBuff(std::move(buff));
			this->data_queue.push(std::move(packet));
		}
	}
}

void CTTYServerThread::processTTY()
{
	{
		std::lock_guard<std::mutex> lock(this->_mutex);
		if (!this->serial.Open())
		{
			this->ClearQueue();
			return;
		}
	}
	if (!this->serial.Receive())
	{
		//��������� ������, �������� COM ���� ���������
		this->serial.Close();
		//�.�. ���� �� ��������������� ������ ������ ������� ������� ���
		this->ClearQueue();
		return;
	}

	//��� ��������� ������
	std::vector<uint8_t>buff = this->serial.GetReceivedData();
	if (buff.empty())
		return;

#ifdef DEBUG
	AddLog(Format("Data from tty '%s', bytes count is %u", this->Name().c_str(), buff.size()));
#endif // DEBUG

	if (!this->data_queue.empty())
	{
#ifdef DEBUG
		AddLog(Format("Data from tty '%s', set to ethernet client", this->Name().c_str()));
#endif // DEBUG

		auto& packet = this->data_queue.front();
		packet.SetRecBuff(std::move(buff));
	}
}

void CTTYServerThread::Execute()
{
	while (!this->IsTerminated())
	{
		//��������� ���������� ��������� ��������
		this->checkSettingsChange();
		this->processTCPServer();
		this->processTTY();

		//��������� ������� �������
		if (!this->data_queue.empty())
		{
			auto& packet = this->data_queue.front();
			//��������, ����� ����� ������� ��� ���
			auto client = this->server.GetClients().find(packet.ClientID());
			if (client == this->server.GetClients().end())
			{
				this->data_queue.pop();
				continue;
			}
			if (packet.SendTime() == 0)
			{
				//���� ����� �������� �� ����������, ������ ���� ����� ���� ��������� � ���������������� ���������
				std::vector<uint8_t> in_buff = packet.GetSendBuff();
#ifdef DEBUG
				AddLog(Format("%s data from eth to tty: %s", this->Name().c_str(), HexToStr(in_buff).c_str()));
#endif
				if (!this->serial.Send(std::move(in_buff)))
				{
					//����� ��� �������� ������, ������ ������ ���� ��������
					this->ClearQueue();
					this->serial.Close();
					continue;
				}

				//����� ����������, ������ ���������� ����� �������� � ��������� �����
				packet.SendTime(TimeTicks());
			}
			else
			{
				//����� ��� ���������
				std::vector<uint8_t> in_buff = packet.GetRecBuff();
				if (in_buff.empty())
				{
					//���, ��� ������, �������� �� �������
					if (IsTimeExpired(packet.SendTime(), TimeTicks(), this->settings.timeout))
					{
						//������ ���, ��������� ����� �� �������
						this->data_queue.pop();
					}
				}
				else
				{
#ifdef DEBUG
					AddLog(Format("%s data from tty to eth: %s", this->Name().c_str(), HexToStr(in_buff).c_str()));
#endif
					//������ ����, �������� �� �� Ethernet
					client->second->Send(std::move(in_buff));
					this->data_queue.pop();
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void CTTYServerThread::ClearQueue()
{
	while (!this->data_queue.empty())
		this->data_queue.pop();
}

void CTTYServerThread::SetSettings(TTYSettings & settings)
{
	std::lock_guard<std::mutex> lock(this->_mutex);
	if (this->settings != settings)
	{
		//��������� ��������� � ����������
		this->reconnectTCP = this->settings.reconnectTCP(settings);
		this->reconnectTTY = this->settings.reconnectTTY(settings);
		this->settings = settings;
	}
}

/*
------------------------------------------------------------------------------
���������� ������� ������ CTTYServerThreadList
------------------------------------------------------------------------------
*/

CTTYServerThreadList::CTTYServerThreadList()
{
}

void CTTYServerThreadList::Clear()
{
	this->list.clear();
}

void CTTYServerThreadList::UpdateConfig(CTTYSettings & settings)
{
	//��������� ������ �������, ���� ���������� � ���� ����
	//�� ��� ��������� � pairs_list ����� ���������, ����� �������
	TTY_SERVER_LIST pairs_list{std::move(this->list)};

	//������ ������������������ ��� TTY - TCP ����
	for (auto & sett : settings.List())
	{
		//����� ������������� ������
		auto it = pairs_list.find(sett.first);
		if (it == pairs_list.end())
		{
			//� ����� ������ ��� ���, ����������� � ������
			auto sthread = std::make_unique<CTTYServerThread>(sett.first, sett.second);
			sthread->Start();
			this->list.emplace(sett.first, std::move(sthread));

			continue;
		}

		//�����������������
		it->second->SetSettings(sett.second);
		this->list.emplace(sett.first, std::move(it->second));
	}
}
