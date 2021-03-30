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
‘ункци¤ запускает приложение как демон использу¤ fork
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
	sid = setsid(); // генерируем уникальный индекс процесса
	if (sid < 0) 
	{
		exit(EXIT_FAILURE);
	}
}