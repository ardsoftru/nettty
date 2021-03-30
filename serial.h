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
Константы настроек COM порта
*/

//Биты паритета
#define PARITY_NONE		0
#define PARITY_ODD		1
#define PARITY_EVEN		2
	
//Стоп биты
#define STOPBITS_ONE	0
#define STOPBITS_TWO	1

/*
Размер буфера COM порта
*/
#define COM_BUFF_SIZE 0xFFFF
	
/*
Класс отрабатывающий по COM порту
*/
class CSerial
{
private:
	//Ссылка на открытый файл
	int fd;
	//Полный путь у файлу
	std::string full_path;
	
	//Флаг ожидания завершения посылки
	bool waiting;
	
	//Время последнего приема байт
	uint64_t rec_stamp;
	//Приемный буфер
	std::vector<uint8_t> data;
	//Буфер для отправки данных
	std::vector<uint8_t> send_data;

	/*
	Внутренняя функция отправки данных
	*/
	bool send();
	
protected:	
	//Имя файла
	std::string fname;
	//Настройки порта
	uint32_t baud;
	uint16_t stopbits;
	uint16_t parity;
	bool cts_rts;
	//Межбайтовый интервал в мс
	uint32_t interval;

	
public:
	CSerial() = delete;
	CSerial(const std::string & fname);
	virtual ~CSerial();
	
	/*
	Функция открывает COM порт
	*/
	virtual bool Open();
	
	/*
	Функция закрывает COM порт
	*/
	void Close();

	/*
	Прием данных
	*/
	bool Receive();
	
	/*
	Отправка данных
	В качестве T используется std::vector<uint8_t>
	*/
	template <typename T>
	bool Send(T&& buff)
	{
		this->send_data = std::forward<T>(buff);
		return this->send();
	}
	
	/*
	Возвращает в указанный буфер данные
	*/
	virtual std::vector<uint8_t> GetReceivedData();
	
	/*
	Флаг ожидания приема байта
	*/
	inline bool Waiting()
	{
		return this->waiting;
	}
	
	/*
	Набор Get и Set методов класса
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
	Флаг открытого канала
	*/
	inline bool Opened()
	{
		return this->fd != -1;
	}
};
