/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <string>

/*
Класс для проверки изменения файла 
*/
class CFileCheck
{
private:
	//Handle для отслеживания изменения
	int inotify;
	//Полный путь к файлу
	std::string path;
	//Флаг обновления файла
	bool updated;
	
public:
	CFileCheck(const std::string & path);
	~CFileCheck();

	/*
	Флаг того что произошло обновление
	*/
	bool Updated();
};
