/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include <string>
#include <signal.h>
#include <sys/wait.h>

#include "auxil.h"
#include "logs.h"
#include "daemon.h"
#include "ttysettings.h"
#include "ttyserver.h"

#ifdef DEBUG
//���� � ����� � �����
const std::string log_file = "/tmp/nettty.log";
//���� � ����� � �������������
const std::string config_path = "/home/pi/nettty/config";
#else
//���� � ����� � �����
const std::string log_file = "/tmp/nettty.log";
//���� � ����� � �������������
const std::string config_path = "/opt/nettty/config";
#endif // DEBUG

//���� ������
bool running = true;

//������ ��
#define version "0.4"

//����� ��� ������ � �����������
CTTYSettings tty_settings(config_path);
//����� ��� ������ �� ������� ��� TTY - TCP ����
CTTYServerThreadList tty_list;

/*
������� ����������� ���������� ����������
*/
void stopApp()
{
	AddLog("Stopping NetTTY...");

	AddLog("NetTTY successfully stopped.");

	//������� ������� �����������
	ClearLogs();
}

void signals_handler(int sig)
{
	switch (sig)
	{
	case SIGTERM:
		//������������ ���� ����������� ������
		running = false;
		break;
	}
}

/*
������� ������������� ����������
*/
void initApp()
{
	//������ ������� �����������
	CreateLogs(log_file);

	AddLog("Starting NetTTY...");

	//������������� ��������� ��������� �������
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signals_handler;  // ��������� ����������
	sigset_t   set;
	sigemptyset(&set);
	sigaddset(&set, SIGTERM); //�������� ������� Terminate
	sa.sa_mask = set;
	sigaction(SIGTERM, &sa, NULL);

	if (!SetNoFilesLimit())
		AddLog(Format("Failed to set files limit: %s", strerror(errno)));

	AddLog(Format("NetTTY successfully started, version %s.", version));
}

int main(int argc, char* arcgv[])
{
#ifndef DEBUG
	//������ ���������� ��� ������
	StartAsDaemon("nettty");
#endif

	//�������������
	initApp();
	//�������� ������������
	tty_settings.LoadSettings(config_path);
	tty_list.UpdateConfig(tty_settings);

	while (running)
	{
		//��������� �� ��������� ��������
		tty_settings.Process();
		//��������
		if (!running)
			break;

		if (tty_settings.GetChanged())
		{
			tty_list.UpdateConfig(tty_settings);
			tty_settings.SetChanged(false);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	//������� �������
	stopApp();

	return EXIT_SUCCESS;
}
