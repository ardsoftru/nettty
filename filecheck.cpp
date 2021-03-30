/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include "filecheck.h"

#include <sys/inotify.h>
#include "auxil.h"

#define INOTIFY_ERROR	-1

CFileCheck::CFileCheck(const std::string & path)
{
	this->inotify = INOTIFY_ERROR;
	this->path = path;
}

CFileCheck::~CFileCheck()
{
	if (this->inotify != INOTIFY_ERROR)
		close(this->inotify);
}

/*
Флаг того что произошло обновление
*/
bool CFileCheck::Updated()
{
	if (this->inotify == INOTIFY_ERROR)
	{
		//Инициализация системы отслеживающей изменения в файле serial.config
		this->inotify = inotify_init();
		if (this->inotify != -1)
		{
			uint32_t mask = IN_CREATE | IN_MOVED_TO | IN_MODIFY | IN_DELETE | IN_DELETE_SELF;
			if (inotify_add_watch(this->inotify, this->path.c_str(), mask) == INOTIFY_ERROR)
			{
				close(this->inotify);
				this->inotify = INOTIFY_ERROR;
			}
		}
		return false;
	}
	
	bool changed = false;
	
	fd_set change_set;
	FD_ZERO(&change_set);
	FD_SET(this->inotify, &change_set);
	
	struct timeval tv = { 0, 0 };
	int res = select(this->inotify + 1, &change_set, NULL, NULL, &tv);
	switch (res)
	{
		//Нет ничего, ждём
		case 0 :
			break;
		
		//Ошибка, закрываем дескриптор
		case INOTIFY_ERROR:
			close(this->inotify);
			this->inotify = INOTIFY_ERROR;
			break;
		
	default:
		//Есть изменения
		size_t size = sizeof(struct inotify_event) + NAME_MAX + 1;
		char* buff = static_cast<char*>(malloc(size));
		memset(buff, 0, size);
		size = read(this->inotify, buff, size);
		struct inotify_event* i_event = (inotify_event*)buff;
		if ((i_event->mask & (IN_DELETE | IN_DELETE_SELF)) != 0)
		{
			close(this->inotify);
			this->inotify = INOTIFY_ERROR;
		}
		free(buff);
		
		changed = true;
		break;
	}
	
	return changed;
}
