/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include "ttysettings.h"

#include "auxil.h"
#include "iniparser.h"
#include "serial.h"
#include "logs.h"

/*
------------------------------------------------------------------------------
Реализация методов класса CTTYSettings
------------------------------------------------------------------------------
*/

void CTTYSettings::saveSettings(dictionary * ini)
{
}

void CTTYSettings::loadSettings(dictionary * ini)
{
	//Подчистка существующего списка
	this->list.clear();

	//Перебор секций в файле с конфигурацией
	int count = iniparser_getnsec(ini);
	if (count < 1)
		return;

	for (int i = 0; i < count; i++)
	{
		//Имя секции
		const char* name = iniparser_getsecname(ini, i);
		if (name == NULL)
			continue;

		/*
		Настройки представляют собой ini файл
		Каждая секция содержит в себе настройки пары TTY - TCP
		Секция и описания настроек:
		[COM1]
		tty=/dev/ttyUSB0
		baud=9600
		stopbits=0
		parity=0
		timeout=1000
		interval=2
		tcpport=54000

		[COM1] - название секции, не несёт особой нагрузки можно назвать как угодно, что бы было понятно для чего она, самое главное имя не должно повторяться
		таких секций может быть несколько
		tty - содержит полный путь к файл TTY порта, уникально и не может повторяться
		baud - скорость передачи по COM порту, возможные варианты: 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200, 128000, 256000.
		stopbits - количество стоп бит: 0 (STOPBITS_ONE), 1 (STOPBITS_TWO).
		parity - бит паритета: 0 (PARITY_NONE) - нет, 1 (PARITY_ODD) - нечётный, 2 (PARITY_EVEN) - чётный
		ctsrts - флаг управления 
		timeout - максимальное  время ожидания ответа по последовательному порту в мс;
		interval - межбайтовый интервал в мс, если после приёма последнего байт проходит это время, считается что посылка по COM порту принята полностью
		tcpport - номер TCP порта который открывается для последовательного интерфейса tty, должен быть уникальным
		*/

		//Формирование структуры с настройками и добавление её в список
		try
		{
			TTYSettings sett = {};

			//Настройки TCP сервера
			sett.tcp_port = static_cast<uint16_t>(iniparser_getint(ini, Format("%s:tcpport", name).c_str(), 0));
			if (sett.tcp_port == 0)
			{
				AddLog(Format("Invalid TCP port settings for TTY interface '%s'", name));
				continue;
			}

			//Настройки последовательного интерфейса
			sett.name = std::string(iniparser_getstring(ini, Format("%s:tty", name).c_str(), ""));
			if (sett.name == "")
			{
				AddLog(Format("Undefined terminal name for TTY interface '%s'", name));
				continue;
			}
			sett.tty.baud = static_cast<uint32_t>(iniparser_getlongint(ini, Format("%s:baud", name).c_str(), 9600));
			sett.tty.stopbits = static_cast<uint16_t>(iniparser_getint(ini, Format("%s:stopbits", name).c_str(), STOPBITS_ONE));
			sett.tty.parity = static_cast<uint16_t>(iniparser_getint(ini, Format("%s:parity", name).c_str(), PARITY_NONE));
			sett.tty.cts_rts = iniparser_getboolean(ini, Format("%s:ctsrts", name).c_str(), 0) != 0;
			sett.interval = static_cast<uint8_t>(iniparser_getint(ini, Format("%s:interval", name).c_str(), 2));
			sett.timeout = static_cast<uint16_t>(iniparser_getint(ini, Format("%s:timeout", name).c_str(), 1000));

			//Дополнительная проверка параметров последовательного интерфейса
			switch (sett.tty.baud)
			{
			case 110: 
			case 300:
			case 600:
			case 1200:
			case 2400:
			case 4800:
			case 9600:
			case 14400:
			case 19200:
			case 38400:
			case 56000:
			case 57600:
			case 115200:
			case 128000:
			case 256000:
				break;

			default:
				AddLog(Format("Invalid baud rate settings for TTY interface '%s', set to default value: 9600", name));
				sett.tty.baud = 9600;
				break;
			}

			std::string stopbits_string;
			switch (sett.tty.stopbits)
			{
			case STOPBITS_ONE:
				stopbits_string = "STOPBITS_ONE";
				break;

			case STOPBITS_TWO:
				stopbits_string = "STOPBITS_TWO";
				break;

			default:
				AddLog(Format("Invalid stopbit settings for TTY interface '%s', set to default value: STOPBITS_ONE", name));
				sett.tty.baud = STOPBITS_ONE;
				stopbits_string = "STOPBITS_ONE";
				break;
			}

			std::string parity_string;
			switch (sett.tty.parity)
			{
			case PARITY_NONE:
				parity_string = "PARITY_NONE";
				break;

			case PARITY_ODD:
				parity_string = "PARITY_ODD";
				break;

			case PARITY_EVEN:
				parity_string = "PARITY_EVEN";
				break;

			default:
				AddLog(Format("Invalid parity settings for TTY interface '%s', set to default value: PARITY_NONE", name));
				sett.tty.baud = PARITY_NONE;
				parity_string = "PARITY_NONE";
				break;
			}

			std::string mess = "Settings for section '%s' accepted: terminal = %s, baud = %u, stopbits = %s, parity = %s, timeout = %u, interval = %u, tpc port = %u";
			mess = Format(mess.c_str(), name, sett.name.c_str(), sett.tty.baud, stopbits_string.c_str(), parity_string.c_str(), sett.timeout, sett.interval, sett.tcp_port);
			AddLog(mess);

			this->list.emplace(sett.name, std::move(sett));
		}
		catch (...)
		{
		}
	}
}

CTTYSettings::CTTYSettings(const std::string & path)
	: CSettings(), file_check{ path }
{
	this->path = path;
}

void CTTYSettings::Process()
{
	if (!IsFileExists(this->path))
	{
		if (!this->list.empty())
		{
			//Файл с настройками удалили, а конфигурация есть
			//Значит считается что произошли изменения
			this->list.clear();
			this->changed = true;
		}
		return;
	}

	//Файл есть, значит проверяется на изменение
	if (!this->file_check.Updated())
		return;

	//Произошли изменения в конфигурации, конфигурация перезагружается
	this->changed = this->LoadSettings(this->path);
}

bool CTTYSettings::GetChanged()
{
	return this->changed;
}

void CTTYSettings::SetChanged(bool value)
{
	this->changed = value;
}

TTY_SETTINGS_LIST & CTTYSettings::List()
{
	return this->list;
}
