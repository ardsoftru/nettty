/*-------------------------------------------------------------------------*/
/**
   @file    ttyserver.cpp
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
Реализация методов класса CTTYServerThreads
------------------------------------------------------------------------------
*/

void CTTYServerThread::checkSettingsChange()
{
	std::lock_guard<std::mutex> lock(this->_mutex);
	if (this->reconnectTCP)
	{
		//Необходимо перезагрузить TCP часть
		this->reconnectTCP = false;
		this->ClearQueue();
		this->server.Close();
	}
	if (this->reconnectTTY)
	{
		//Необходимо перезагрузить TTY часть
		this->reconnectTTY = false;
		this->serial.Close();
		this->serial.Baud(this->settings.tty.baud);
		this->serial.StopBits(this->settings.tty.stopbits);
		this->serial.Parity(this->settings.tty.parity);
		this->serial.CTSRTS(this->settings.tty.cts_rts);
	}
	//Эта настройка не требует переинициализации
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
			//Формирование пакета и размещение его в очереди
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
		//Произошла ошибка, возможно COM порт выдернули
		this->serial.Close();
		//т.к. порт не работоспособный смысла список пакетов держать нет
		this->ClearQueue();
		return;
	}

	//Идёт получение данных
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
		//Отработка алгоритмов изменения настроек
		this->checkSettingsChange();
		this->processTCPServer();
		this->processTTY();

		//Отработка очереди пакетов
		if (!this->data_queue.empty())
		{
			auto& packet = this->data_queue.front();
			//Проверка, вдруг этого клиента нет уже
			auto client = this->server.GetClients().find(packet.ClientID());
			if (client == this->server.GetClients().end())
			{
				this->data_queue.pop();
				continue;
			}
			if (packet.SendTime() == 0)
			{
				//Если время отправки не выставлено, значит этот пакет надо отправить в последовательный интерфейс
				std::vector<uint8_t> in_buff = packet.GetSendBuff();
#ifdef DEBUG
				AddLog(Format("%s data from eth to tty: %s", this->Name().c_str(), HexToStr(in_buff).c_str()));
#endif
				if (!this->serial.Send(std::move(in_buff)))
				{
					//Отказ при отправке пакета, значит список надо очистить
					this->ClearQueue();
					this->serial.Close();
					continue;
				}

				//Пакет отправился, значит отмечается время отправки и ожидается ответ
				packet.SendTime(TimeTicks());
			}
			else
			{
				//Пакет был отправлен
				std::vector<uint8_t> in_buff = packet.GetRecBuff();
				if (in_buff.empty())
				{
					//Нет, ещё данных, проверка на таймаут
					if (IsTimeExpired(packet.SendTime(), TimeTicks(), this->settings.timeout))
					{
						//Ответа нет, удаляется пакет из очереди
						this->data_queue.pop();
					}
				}
				else
				{
#ifdef DEBUG
					AddLog(Format("%s data from tty to eth: %s", this->Name().c_str(), HexToStr(in_buff).c_str()));
#endif
					//Данные есть, отправка их по Ethernet
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
		//Произошли изменения в настройках
		this->reconnectTCP = this->settings.reconnectTCP(settings);
		this->reconnectTTY = this->settings.reconnectTTY(settings);
		this->settings = settings;
	}
}

/*
------------------------------------------------------------------------------
Реализация методов класса CTTYServerThreadList
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
	//Временный список потоков, пока перемещаем в него всех
	//Те кто останутся в pairs_list после обработки, будут удалены
	TTY_SERVER_LIST pairs_list{std::move(this->list)};

	//Список сконфигурированных пар TTY - TCP порт
	for (auto & sett : settings.List())
	{
		//Поиск существующего потока
		auto it = pairs_list.find(sett.first);
		if (it == pairs_list.end())
		{
			//С таким именем нет ещё, добавляется в список
			auto sthread = std::make_unique<CTTYServerThread>(sett.first, sett.second);
			sthread->Start();
			this->list.emplace(sett.first, std::move(sthread));

			continue;
		}

		//Переинициализация
		it->second->SetSettings(sett.second);
		this->list.emplace(sett.first, std::move(it->second));
	}
}
