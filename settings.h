/*-------------------------------------------------------------------------*/
/**
   @file    settings.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <string>
#include "iniparser.h"

/*
Класс в котором хранятся все настройки приложения
*/
class CSettings
{
protected:
	/*
	Функция сохраняет настройки в ini файл
	*/
	virtual void saveSettings(dictionary* ini) = 0;

	/*
	Функция читает настройки из ini файла
	*/
	virtual void loadSettings(dictionary* ini) = 0;

public:
	CSettings();
	virtual ~CSettings();

	/*
	Загрузка настроек приложения из ini файла который должен располагаться 
	там же где и исполняемый файл (по крайней мере пока)
	*/
	bool LoadSettings(const std::string & path);

	/*
	Сохранение настроек в указанный файл
	*/
	void SaveSettings(const std::string & path);
};
