/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <string>

/*
����� ��� �������� ��������� ����� 
*/
class CFileCheck
{
private:
	//Handle ��� ������������ ���������
	int inotify;
	//������ ���� � �����
	std::string path;
	//���� ���������� �����
	bool updated;
	
public:
	CFileCheck(const std::string & path);
	~CFileCheck();

	/*
	���� ���� ��� ��������� ����������
	*/
	bool Updated();
};
