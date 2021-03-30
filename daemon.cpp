/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include <string>
#include <cstdio>
#include <unistd.h>

#include "daemon.h"

/*
������� ��������� ���������� ��� ����� ��������� fork
*/
void StartAsDaemon(const std::string & daemon_name)
{
	pid_t parid, sid;
	
	parid = fork();
	if (parid < 0)
	{
		exit(EXIT_FAILURE);
	}
	else if (parid != 0)
	{
		exit(EXIT_SUCCESS);
	}
	sid = setsid(); // ���������� ���������� ������ ��������
	if (sid < 0) 
	{
		exit(EXIT_FAILURE);
	}
}