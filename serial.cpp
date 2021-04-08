/*-------------------------------------------------------------------------*/
/**
   @file    serial.cpp
   @author  Airat Rakhmatullin
*/
/*--------------------------------------------------------------------------*/

#include "serial.h"

#ifdef DEBUG
#include "logs.h"
#include "auxil.h"
#endif

/*
------------------------------------------------------------------------------
Реализация методов класса CSerial
------------------------------------------------------------------------------
*/
CSerial::CSerial(const std::string & fname)
{
	this->fd = -1;
	this->fname = fname;
	this->full_path = fname;
	
	this->baud = 9600;
	this->stopbits = STOPBITS_ONE;
	this->parity = PARITY_NONE;
	this->interval = 2;
	this->cts_rts = false;
}

CSerial::~CSerial()
{
	this->Close();
}

/*
Функция открывает COM порт
*/
bool CSerial::Open()
{
	if (this->Opened())
		return true;

	this->waiting = false;
	this->rec_stamp = 0;
	
	//Открывается COM порт
	this->fd = open(this->full_path.c_str(), O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY);
	if (this->fd == -1)
		return false;

	struct termios term;
	memset(&term, 0, sizeof(term));
	
	//Устанавливается скорость обмена
	int speed;
	switch (baud)
	{	
	case 1200: 
		speed = B1200;
		break;
	case 2400: 
		speed = B2400;
		break;
	case 4800: 
		speed = B4800;
		break;
	case 9600: 
		speed = B9600;
		break;
	case 19200: 
		speed = B19200;
		break;
	case 38400:
		speed = B38400;
		break;
	case 57600:
		speed = B57600;
		break;
	case 115200: 
		speed = B115200;
		break;	
	default:   
		speed = B9600;
		break;
	}	
	if ((cfsetospeed(&term, speed) < 0) || (cfsetispeed(&term, speed) < 0))
	{
		this->Close();
		return false;
	}
	
	/*
	CREAD - включить прием
	CLOCAL - игнорировать управление линиями с помошью
	*/
	term.c_cflag |= (CREAD | CLOCAL);
	//CSIZE - маска размера символа
	term.c_cflag &= ~CSIZE;
	//CS8 - 8 битные символы
	term.c_cflag |= CS8;	
	//CSTOPB - при 1 - два стоп бита, при 0 - один
	if (this->stopbits == STOPBITS_ONE)
		term.c_cflag &= ~CSTOPB;
	else
		term.c_cflag |= CSTOPB;
	//Аппаратное управление потоком
	if (this->cts_rts)
		term.c_cflag |= CRTSCTS;
	else
		term.c_cflag &= ~CRTSCTS;

	//Бит паритета
	switch (this->parity)
	{
	//Нет бита паритета
	case PARITY_NONE:
		//Выключается проверка четности
		term.c_iflag &= ~INPCK;
		term.c_cflag &= ~PARODD;
		term.c_cflag &= ~PARENB;
		break;
		
	//Нечётный
	case PARITY_ODD:
		term.c_iflag |= INPCK;
		term.c_cflag |= PARODD;
		term.c_cflag |= PARENB;
		break;
		
	//Чётный
	case PARITY_EVEN:
		term.c_iflag |= INPCK;
		term.c_cflag &= ~PARODD;
		term.c_cflag |= PARENB;
		break;
	}
	
	//Применение настроек
	if (tcsetattr(this->fd, TCSANOW, &term) < 0)
	{
		this->Close();
		return false;
	}	
	
	//очищаем входной и выходной буфер
	tcflush(fd, TCIOFLUSH);	

	return true;
}

/*
Функция закрывает COM порт
*/
void CSerial::Close()
{
	if (this->Opened())
	{
		close(this->fd);
		this->fd = -1;
	}
}

/*
Прием данных
*/
bool CSerial::Receive()
{
	//Наборы для работы
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(this->fd, &readSet);
	struct timeval tv = { 0, 0 };

	int res = select(this->fd + 1, &readSet, NULL, NULL, &tv);
	if (res < 0)
		return false;

	//Вроде что то есть
	std::array<uint8_t, 255> buff;
	auto retval = read(this->fd, buff.data(), buff.size());
	if (retval > 0)
	{
		//Данные пришли
		std::copy(buff.begin(), std::next(buff.begin(), retval), std::back_inserter(this->data));
#ifdef DEBUG
		AddLog(Format("Data in internal buffer of '%s' %u bytes", this->fname.c_str(), this->data.size()));
#endif
		this->rec_stamp = TimeTicks();
		this->waiting = true;
	}
	else if (retval < 0)
	{
		if (errno == EAGAIN)
		{
			//Всё нормально, ждём...
			if (this->data.empty())
				this->rec_stamp = TimeTicks();
		}
		else
			return false; //Возникла ошибка
	}

	return true;
}

bool CSerial::send()
{
	tcflush(this->fd, TCIOFLUSH);
	ssize_t sent = write(this->fd, this->send_data.data(), this->send_data.size());
	if (sent == -1)
	{
		if (errno != EAGAIN)
			return false;
	}
	return true;
}
	
/*
Возвращает в указанный буфер данные
*/
std::vector<uint8_t> CSerial::GetReceivedData()
{
	std::vector<uint8_t> result;
	if (this->waiting && IsTimeExpired(this->rec_stamp, TimeTicks(), this->interval))
	{
		//Истекло время после последнего прихода байта
		this->waiting = false;
		result.swap(this->data);
		this->data.clear();
	}

	//Возвращается пустой вектор
	return result;
}

