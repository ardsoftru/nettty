/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <string.h>

using namespace std;

//‘ункци€ запускает приложение как демон использу€ fork
void StartAsDaemon(const std::string & daemon_name);