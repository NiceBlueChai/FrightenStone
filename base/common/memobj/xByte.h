#ifndef XBYTE_H_
#define XBYTE_H_

#pragma once

#include <BaseCode/MemPoolObjApi.h>

//�ڴ��char ������ʹ��ʱ��Ҫʹ�� DEL_WAY_DELETE_MULTI 
class MEM_POOL_OBJ_API CXByte
{
	MEMPOOL_CREATE(m_MemObj)
public: 
	CXByte();
		~CXByte();

public:
	CXByte(const CXByte& other);
	CXByte(CXByte&& other);
	CXByte(const char& other);
	CXByte(char&& other);
	CXByte(char& other);

	//����
public:
	char& GetInnerObj() { return data; }
	char *GetMePtr() { return (char *)this; }


	//����������
public:
	operator char();
	operator const char();
	operator CXByte();
	operator const CXByte();	
	bool operator ==(const CXByte& other);
	bool operator ==(CXByte& other);
	bool operator ==(const char& other);
	bool operator ==(char& other);
	bool operator !=(const CXByte& other);
	bool operator !=(CXByte& other);
	bool operator !=(const char& other);
	bool operator !=(char& other);
	CXByte& operator =(const CXByte& other);
	CXByte& operator =(CXByte& other);
	CXByte& operator =(const char& other);
	CXByte& operator =(char& other);

	CXByte operator +(const CXByte& other);
	CXByte operator +(CXByte& other);
	CXByte operator +(const char& other);
	CXByte operator +(char& other);

	CXByte operator -(const CXByte& other);
	CXByte operator -(CXByte& other);
	CXByte operator -(const char& other);
	CXByte operator -(char& other);

	CXByte operator *(const CXByte& other);
	CXByte operator *(CXByte& other);
	CXByte operator *(const char& other);
	CXByte operator *(char& other);

	CXByte operator /(const CXByte& other);
	CXByte operator /(CXByte& other);
	CXByte operator /(const char& other);
	CXByte operator /(char& other);

	CXByte operator %(const CXByte& other);
	CXByte operator %(CXByte& other);
	CXByte operator %(const char& other);
	CXByte operator %(char& other);

	CXByte operator &(const CXByte& other);
	CXByte operator &(CXByte& other);
	CXByte operator &(const char& other);
	CXByte operator &(char& other);

	CXByte operator |(const CXByte& other);
	CXByte operator |(CXByte& other);
	CXByte operator |(const char& other);
	CXByte operator |(char& other);

	CXByte operator ~();

	CXByte& operator ++();
	CXByte operator ++(int);
	CXByte& operator --();
	CXByte operator --(int);

	CXByte& operator +=(const CXByte& other);
	CXByte& operator +=(CXByte& other);
	CXByte& operator +=(const char& other);
	CXByte& operator +=(char& other);

	CXByte& operator -=(const CXByte& other);
	CXByte& operator -=(CXByte& other);
	CXByte& operator -=(const char& other);
	CXByte& operator -=(char& other);

	CXByte& operator *=(const CXByte& other);
	CXByte& operator *=(CXByte& other);
	CXByte& operator *=(const char& other);
	CXByte& operator *=(char& other);

	CXByte& operator /=(const CXByte& other);
	CXByte& operator /=(CXByte& other);
	CXByte& operator /=(const char& other);
	CXByte& operator /=(char& other);

	CXByte& operator %= (const CXByte& other);
	CXByte& operator %= (CXByte& other);
	CXByte& operator %= (const char& other);
	CXByte& operator %= (char& other);

	CXByte& operator &=(const CXByte& other);
	CXByte& operator &=(CXByte& other);
	CXByte& operator &=(const char& other);
	CXByte& operator &=(char& other);

	CXByte& operator |=(const CXByte& other);
	CXByte& operator |=(CXByte& other);
	CXByte& operator |=(const char& other);
	CXByte& operator |=(char& other);

private:
	char data = 0;
};

#endif

