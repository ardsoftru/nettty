/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <chrono>
#include <semaphore.h>
#include <unistd.h>

#include <thread>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/mman.h> //for shm
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <fstream>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <queue>

#include "time.h"

using namespace std;
using namespace chrono;

//Константа несуществующего кода
#define INVALID_PID	-1

using ByteArray = std::vector<uint8_t>;
using DATA_LIST = std::queue<ByteArray>;

/*
	Функция проверяет наличие файла
*/
bool IsFileExists(const std::string & filename);

/*
Функция проверяет не истекло ли заданное время
timestamp - отмеченное время, относительно которого идет отсчет
curr - текущее время
period - период на который идет проверка
*/
bool IsTimeExpired(const uint64_t & timestamp, const uint64_t & curr, const uint64_t & period);

/*
Время в мс прошедшее от старта системы
*/
uint64_t TimeTicks(void);

/*
Функция форматирования строки
*/
std::string Format(const char * fmt, ...);

/*
По номеру ошибки возвращает строку с её описанием
*/
std::string ErrorToStr(int errnum);

/*
Функция генерирует уникальный неповторяющийся идентификатор
*/
uint64_t UniqueID();

/*
Функция снимает ограничение для процесса на количество открытых фалов
*/
bool SetNoFilesLimit();

/*
Возвращает буфер в виде строки байтов
*/
std::string HexToStr(const void* buff, size_t size);
std::string HexToStr(const std::vector<uint8_t>& buff);