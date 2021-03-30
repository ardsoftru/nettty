/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include "auxil.h"

#include <cstdio>
#include <mutex>
#include <spawn.h>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sys/time.h>
#include <sys/resource.h>

/*
	������� ��������� ������� �����
*/
bool IsFileExists(const std::string & filename)
{
	if (access(filename.c_str(), F_OK) != 0)
		return false;
	
	return true;
}

/*
������� ��������� �� ������� �����
*/
bool IsTimeExpired(const uint64_t & timestamp, const uint64_t & curr, const uint64_t & period)
{
	uint64_t diff = curr - timestamp;
	return (diff >= period);
}

/*
����� � �� ��������� �� ������ �������
*/
uint64_t TimeTicks(void)
{	
	struct timespec spec;
	
	clock_gettime(CLOCK_MONOTONIC, &spec);	
	uint64_t t = static_cast<uint64_t>(spec.tv_sec);
	t *= 1000;	
	//����� ���������� �� ��������� � ������� ������� �������
	t = static_cast<uint64_t>(round(static_cast<double>(spec.tv_nsec) / 1.0e6)) + t;
	
	return t;
}

/*
������� �������������� ������
*/
std::string Format(const char * fmt, ...)
{
	char *buffer = NULL;
	va_list ap;

	va_start(ap, fmt);
	int size = vasprintf(&buffer, fmt, ap);
	va_end(ap);

	if (size == -1)
		return "";

	std::string result = std::string(buffer);
	free(buffer);

	return result;
}

/*
�� ������ ������ ���������� ������ � � ���������
*/
std::string ErrorToStr(int errnum) 
{
	char* e = strerror(errnum);

	if (e != nullptr)
		return std::string(e);
	else
		return "";
}

/*
������� ���������� ���������� ��������������� �������������
*/
uint64_t uniqueID = 0;
std::mutex uniqueID_mutex;

uint64_t UniqueID()
{
	std::lock_guard<std::mutex> lock(uniqueID_mutex);
	uniqueID++;

	return uniqueID;
}

/*
������� ������� ����������� ��� �������� �� ���������� �������� �����
*/
bool SetNoFilesLimit()
{
	rlimit limit = { 25000 , 25000 };
	return setrlimit(RLIMIT_NOFILE, &limit) != -1;
}

/*
���������� ����� � ���� ������ ������
*/
std::string HexToStr(const std::vector<uint8_t>& buff)
{
	std::ostringstream ss;

	ss << std::setfill('0') << std::hex;

	std::for_each(buff.begin(), buff.end(), [&](int c) {ss << std::setw(2) << c; });

	return ss.str();
}

std::string HexToStr(const void* buff, size_t size)
{
	const uint8_t* data = static_cast<const uint8_t*>(buff);
	std::vector<uint8_t> dest(data, data + size);

	return HexToStr(dest);
}
