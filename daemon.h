/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <string.h>

using namespace std;

//������� ��������� ���������� ��� ����� ��������� fork
void StartAsDaemon(const std::string & daemon_name);