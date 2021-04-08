/*-------------------------------------------------------------------------*/
/**
   @file    settings.cpp
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include "settings.h"

#include "auxil.h"

/*
------------------------------------------------------------------------------
Реализация методов класса CSettings
------------------------------------------------------------------------------
*/
CSettings::CSettings()
{
}

CSettings::~CSettings()
{
}

bool CSettings::LoadSettings(const std::string & path)
{
	dictionary* ini = iniparser_load(path.c_str());
	if (ini == NULL)
		return false;

	this->loadSettings(ini);
	iniparser_freedict(ini);

	return true;
}

void CSettings::SaveSettings(const std::string & path)
{
	if (!IsFileExists(path))
	{
		FILE * f;
		f = fopen(path.c_str(), "w");
		if (f != NULL)
			fclose(f);
	}
	dictionary* ini = iniparser_load(path.c_str());
	if (ini == NULL)
		return;

	this->saveSettings(ini);

	//Сохранение в файл
	FILE * f;
	f = fopen(path.c_str(), "w");
	if (f != NULL)
	{
		iniparser_dump_ini(ini, f);
		fclose(f);
	}
	iniparser_freedict(ini);
}
