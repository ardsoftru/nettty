/*-------------------------------------------------------------------------*/
/**
   @file    auxil.h
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#pragma once

#include <string>
#include "iniparser.h"

/*
����� � ������� �������� ��� ��������� ����������
*/
class CSettings
{
protected:
	/*
	������� ��������� ��������� � ini ����
	*/
	virtual void saveSettings(dictionary* ini) = 0;

	/*
	������� ������ ��������� �� ini �����
	*/
	virtual void loadSettings(dictionary* ini) = 0;

public:
	CSettings();
	virtual ~CSettings();

	/*
	�������� �������� ���������� �� ini ����� ������� ������ ������������� 
	��� �� ��� � ����������� ���� (�� ������� ���� ����)
	*/
	bool LoadSettings(const std::string & path);

	/*
	���������� �������� � ��������� ����
	*/
	void SaveSettings(const std::string & path);
};
