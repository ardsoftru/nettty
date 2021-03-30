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

//��������� ��������������� ����
#define INVALID_PID	-1

using ByteArray = std::vector<uint8_t>;
using DATA_LIST = std::queue<ByteArray>;

/*
	������� ��������� ������� �����
*/
bool IsFileExists(const std::string & filename);

/*
������� ��������� �� ������� �� �������� �����
timestamp - ���������� �����, ������������ �������� ���� ������
curr - ������� �����
period - ������ �� ������� ���� ��������
*/
bool IsTimeExpired(const uint64_t & timestamp, const uint64_t & curr, const uint64_t & period);

/*
����� � �� ��������� �� ������ �������
*/
uint64_t TimeTicks(void);

/*
������� �������������� ������
*/
std::string Format(const char * fmt, ...);

/*
�� ������ ������ ���������� ������ � � ���������
*/
std::string ErrorToStr(int errnum);

/*
������� ���������� ���������� ��������������� �������������
*/
uint64_t UniqueID();

/*
������� ������� ����������� ��� �������� �� ���������� �������� �����
*/
bool SetNoFilesLimit();

/*
���������� ����� � ���� ������ ������
*/
std::string HexToStr(const void* buff, size_t size);
std::string HexToStr(const std::vector<uint8_t>& buff);