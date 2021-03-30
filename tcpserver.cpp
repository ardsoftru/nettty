/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include "tcpserver.h"

#include <stack>
#include <array>

#include "auxil.h"
#include "logs.h"


/*
------------------------------------------------------------------------------
���������� ������� ������ CTCPServer
------------------------------------------------------------------------------
*/
std::unique_ptr<CTCPServerClient> CTCPServer::createClient(int sock)
{
	return std::make_unique<CTCPServerClient>(sock);
}

void CTCPServer::deleteClient(TCPSERVER_CLIENTS_LIST::iterator& client)
{
	epoll_ctl(this->efd, EPOLL_CTL_DEL, client->second->GetSocket(), &client->second->GetConnEvnt());
	this->clients.erase(client->second->GetSocket());
}

CTCPServer::CTCPServer(uint64_t interval, uint32_t maxConnections)
{
	this->sock = -1;
	this->efd = -1;
	this->port = 0;
	this->maxConnections = maxConnections;
	this->interval = interval;
}

CTCPServer::~CTCPServer(void)
{
	this->Close();
}

/*
������ TCP �������
*/
void CTCPServer::Open(const std::string & interface, uint16_t port)
{
	if (this->Opened())
		return;

	this->port = port;
	this->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (this->sock < 0)
		return;

	//��� ��� �� ����� ���������� ���������� ������� ����� ��������� ������� ����
	int one = 1;
	if ((setsockopt(this->sock,	SOL_SOCKET,	SO_REUSEADDR, &one, sizeof(one))) == -1)
	{
		close(this->sock);
		this->sock = -1;
		return;
	}

	//������������� �����
	this->setnonblocking(this->sock);

	this->efd = epoll_create1(0);
	if (this->efd < 0)
	{
		close(this->sock);
		this->sock = -1;
		return;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->port);
	addr.sin_addr.s_addr = 0L;			
	if (bind(this->sock, (const struct sockaddr *)&addr, (socklen_t)sizeof(addr)) < 0)
	{
		AddLog(Format("error in bind %s", strerror(errno)));

		this->close_descriptors();
		return;
	}

	if (listen(this->sock, SOMAXCONN))
	{
		this->close_descriptors();
		return;
	}

	//this->listenevnt.events = EPOLLIN | EPOLLPRI | EPOLLET;
	this->listenevnt.events = EPOLLIN;
	this->listenevnt.data.fd = this->sock;
	if (epoll_ctl(this->efd, EPOLL_CTL_ADD, this->sock, &this->listenevnt) < 0)
	{
		this->close_descriptors();
		return;
	}
}

void CTCPServer::setnonblocking(int sock)
{
	int code = fcntl(sock, F_GETFD, 0);
	fcntl(sock, F_SETFL, code | O_NONBLOCK);
}

void CTCPServer::close_descriptors()
{
	close(this->sock);
	this->sock = -1;
	close(this->efd);
	this->efd = -1;
}

/*
������� �������� � ���������
*/
bool CTCPServer::Accept(int timeout)
{
	//�������� ����������� �������
	std::vector<epoll_event> events(this->clients.size() + 1);
	int nfds = epoll_wait(this->efd, events.data(), this->clients.size() + 1, timeout);
	if (nfds < 0)
	{
		this->Close();
		return false;
	}

	for (int i = 0; i < nfds; i++)
	{
		if (events[i].data.fd == this->sock)
		{
			//����������� ������ �������
			struct sockaddr_in addr { 0 };

			addr.sin_family = AF_INET;
			addr.sin_port = 0;
			addr.sin_addr.s_addr = 0L;
			socklen_t size = sizeof(addr);
			int Socket = accept(this->sock, (struct sockaddr*)&addr, (socklen_t*)&size);
			if (Socket > 0)
			{
				std::unique_ptr<CTCPServerClient> client = this->createClient(Socket);
				epoll_event & connevnt = client->GetConnEvnt();
				if (epoll_ctl(this->efd, EPOLL_CTL_ADD, Socket, &connevnt) < 0)
					continue;

				//this->setnonblocking(Socket);
				client->interval = this->interval;
				this->clients.emplace(client->GetSocket(), std::move(client));

#ifdef DEBUG
				AddLog(Format("TCP Client %d connected, client count is %u", Socket, this->clients.size()));
#endif

				continue;
			}
			else
			{
				AddLog(Format("Error due TCP client connection: %s", strerror(errno)));
			}
		}
		else
		{
			auto client = this->clients.find(events[i].data.fd);
			if (client == this->clients.end())
				continue;

			//������� �� ������������� �������
			if (events[i].events & EPOLLIN)
			{
				//��� �� ������
				if (!client->second->receiveData())
				{
					//������, ������ ��������� � ������ �� ��������
					this->deleteClient(client);
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				//�������� ������
				if (!client->second->sendData())
				{
					//������, ������ ��������� � ������ �� ��������
					//AddLog(Format("Error on send, client: %u", client->GetSocket()));
					this->deleteClient(client);
				}
			}
			else if (events[i].events & EPOLLRDHUP)
			{
				//������, ������ ��������� � ������ �� ��������
				AddLog(Format("Error on socket, client: %u", client->second->GetSocket()));
				this->deleteClient(client);
			}
		}
	}

	return true;
}

TCPSERVER_CLIENTS_LIST & CTCPServer::GetClients()
{
	return this->clients;
}

/*
������� TCP �������
*/
void CTCPServer::Close()
{
	if (!this->Opened())
		return;
	
	for (const auto &it : this->clients)
		epoll_ctl(this->efd, EPOLL_CTL_DEL, it.second->GetSocket(), &it.second->GetConnEvnt());
	this->clients.clear();
	
	shutdown(this->sock, 2);
	this->close_descriptors();
}

/*
���� ��������� �������� ������
*/
bool CTCPServer::Opened()
{
	if (this->sock == -1)
		return false;
	
	return true;
}

/*
------------------------------------------------------------------------------
���������� ������� ������ CTCPServerClient
------------------------------------------------------------------------------
*/
CTCPServerClient::CTCPServerClient(int sock)
{
	this->sock = sock;
	this->rec_stamp = TimeTicks();
	this->interval = 0;
	this->waiting = false;

	//connevnt.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP;
	connevnt.events = EPOLLIN | EPOLLHUP;
	connevnt.data.fd = sock;

}

CTCPServerClient::~CTCPServerClient(void)
{
	this->Close();
}

/*
���������� ����� �������
*/
int CTCPServerClient::GetSocket()
{
	return this->sock;
}

/*
����� ������
*/
bool CTCPServerClient::receiveData()
{
	//����� ������

	//����� ��� �� ����
	std::array<uint8_t, 255> buff;
	auto retval = read(this->sock, buff.data(), buff.size());
	if (retval > 0)
	{
		//������ ������
		std::copy(buff.begin(), std::next(buff.begin(), retval), std::back_inserter(this->receiveBuff));
		this->rec_stamp = TimeTicks();
		this->waiting = true;
	}
	else if (retval == 0)
	{
		//������ ������ ����������
		AddLog(Format("Error on receive, client: %u, recv error: %s, retval = %d", this->GetSocket(), ErrorToStr(errno).c_str(), retval));
		return false;
	}
	else if (retval < 0)
	{
		if (errno == EWOULDBLOCK)
		{
			//�� ���������, ���...
			if (this->receiveBuff.empty())
				this->rec_stamp = TimeTicks();
		}
		else
		{
			AddLog(Format("Error on receive, client: %u, recv error: %s, retval = %d", this->GetSocket(), ErrorToStr(errno).c_str(), retval));
			return false; //�������� ������
		}
	}

	return true;
}

/*
���������� � ��������� ����� ������
*/
std::vector<uint8_t> CTCPServerClient::GetReceivedData()
{
	std::vector<uint8_t> result;
	if (this->waiting && IsTimeExpired(this->rec_stamp, TimeTicks(), this->interval))
	{
		//������� ����� ����� ���������� ������� �����
		result.swap(this->receiveBuff);
		this->receiveBuff.clear();
		this->waiting = false;
	}

	return result;
}

bool CTCPServerClient::sendData()
{
	while (!this->sendDataList.empty())
	{
		const auto& data = this->sendDataList.front();
		//������� ������ � �������
		ssize_t sent = send(this->sock, data.data(), data.size(), 0);

		if (static_cast<ssize_t>(data.size()) != sent)
			AddLog(Format("CTCPServerClient Sending %d bytes, sent %d bytes", data.size(), sent));

		if (sent < 0)
		{
			if (errno == EWOULDBLOCK)
			{
				//���� ����� ��������� �� ���������� �� ���� �������������� ��������
				this->sendDataList.pop();
				break;
			}
			return false;
		}
		this->sendDataList.pop();
	}

	return true;
}

epoll_event & CTCPServerClient::GetConnEvnt()
{
	return this->connevnt;
}

/*
�������� ������
*/
void CTCPServerClient::Close()
{
	if (this->sock != -1)
	{
		close(this->sock);
		this->sock = -1;
	}
}

