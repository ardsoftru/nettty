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
//Путь к файлу с логом
const std::string log_file = "/tmp/nettty.log";
//Путь к файлу с конфигурацией
const std::string config_path = "/home/pi/nettty/config";
#else
//Путь к файлу с логом
const std::string log_file = "/tmp/nettty.log";
//Путь к файлу с конфигурацией
const std::string config_path = "/opt/nettty/config";
#endif // DEBUG

//Флаг работы
bool running = true;

//Версия ПО
#define version "0.4"

//Класс для работы с настройками
CTTYSettings tty_settings(config_path);
//класс для работы со списком пар TTY - TCP порт
CTTYServerThreadList tty_list;

/*
Функция корректного завершения приложения
*/
void stopApp()
{
	AddLog("Stopping NetTTY...");

	AddLog("NetTTY successfully stopped.");

	//Останов системы логирования
	ClearLogs();
}

void signals_handler(int sig)
{
	switch (sig)
	{
	case SIGTERM:
		//Сбрасывается флаг разрешающий работу
		running = false;
		break;
	}
}

/*
Функция инициализации приложения
*/
void initApp()
{
	//Запуск системы логирования
	CreateLogs(log_file);

	AddLog("Starting NetTTY...");

	//Инициализация перехвата системных событий
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signals_handler;  // указываем обработчик
	sigset_t   set;
	sigemptyset(&set);
	sigaddset(&set, SIGTERM); //Перехват события Terminate
	sa.sa_mask = set;
	sigaction(SIGTERM, &sa, NULL);

	if (!SetNoFilesLimit())
		AddLog(Format("Failed to set files limit: %s", strerror(errno)));

	AddLog(Format("NetTTY successfully started, version %s.", version));
}

int main(int argc, char* arcgv[])
{
#ifndef DEBUG
	//Запуск приложения как демона
	StartAsDaemon("nettty");
#endif

	//Инициализация
	initApp();
	//Загрузка конфигурации
	tty_settings.LoadSettings(config_path);
	tty_list.UpdateConfig(tty_settings);

	while (running)
	{
		//Отработка по изменению настроек
		tty_settings.Process();
		//Проверка
		if (!running)
			break;

		if (tty_settings.GetChanged())
		{
			tty_list.UpdateConfig(tty_settings);
			tty_settings.SetChanged(false);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	//Останов системы
	stopApp();

	return EXIT_SUCCESS;
}
