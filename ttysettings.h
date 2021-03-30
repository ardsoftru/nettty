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
Структура описывает настройки пары TTY - TCP порт
*/
struct TTYSettings
{
	//Название tty порта для которого эти
	std::string name;
	struct TTY
	{
		//Скорость обмена
		uint32_t baud;
		//Количество стоп бит
		uint16_t stopbits;
		//Бит паритета
		uint16_t parity;
		//CTS-RTS flow control
		bool cts_rts;
	}tty;
	//Максимальное время ожидания ответа по последовательному порту
	uint16_t timeout;
	//Межбайтовый интервал в мс
	uint8_t interval;
	//прослушиваемый TCP порт
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

//Список настроек
typedef std::unordered_map<std::string, TTYSettings> TTY_SETTINGS_LIST;

class CTTYSettings : public CSettings
{
private:
	//Путь к файлу с конфигурацией
	std::string path;
	//Список настроек по каждому порту
	TTY_SETTINGS_LIST list;
	//Флаг изменения настроек
	bool changed = false;
	//Класс для проверки изменений в файле с конфигурацией
	CFileCheck file_check;

protected:
	/*
	Функция сохраняет настройки в ini файл
	*/
	virtual void saveSettings(dictionary* ini) override;

	/*
	Функция читает настройки из ini файла
	*/
	virtual void loadSettings(dictionary* ini) override;

public:
	CTTYSettings(const std::string & path);

	/*
	Отработка по текущей конфигурации
	*/
	void Process();

	/*
	Набор Get и Set функций
	*/
	bool GetChanged();
	void SetChanged(bool value);

	TTY_SETTINGS_LIST & List();
};