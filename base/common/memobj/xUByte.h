#ifndef XUBYTE_H_
#define XUBYTE_H_

#pragma once

#include <BaseCode/MemPoolObjApi.h>

//�ڴ��char ������ʹ��ʱ��Ҫʹ�� DEL_WAY_DELETE_MULTI 
class MEM_POOL_OBJ_API CUXByte
{
	MEMPOOL_CREATE(m_MemObj)
public: 
	CUXByte();
		~CUXByte();

public:
	CUXByte(const CUXByte& other);
	CUXByte(CUXByte&& other);
	CUXByte(const unsigned char& other);
	CUXByte(unsigned char&& other);
	CUXByte(unsigned char& other);

	//����
public:
	unsigned char& GetInnerObj() { return data; }
	unsigned char *GetMePtr() { return (unsigned char *)this; }


	//����������
public:
	operator unsigned char();
	operator const unsigned char();
	operator CUXByte();
	operator const CUXByte();	
	bool operator ==(const CUXByte& other);
	bool operator ==(CUXByte& other);
	bool operator ==(const unsigned char& other);
	bool operator ==(unsigned char& other);
	bool operator !=(const CUXByte& other);
	bool operator !=(CUXByte& other);
	bool operator !=(const unsigned char& other);
	bool operator !=(unsigned char& other);
	CUXByte& operator =(const CUXByte& other);
	CUXByte& operator =(CUXByte& other);
	CUXByte& operator =(const unsigned char& other);
	CUXByte& operator =(unsigned char& other);

	CUXByte operator +(const CUXByte& other);
	CUXByte operator +(CUXByte& other);
	CUXByte operator +(const unsigned char& other);
	CUXByte operator +(unsigned char& other);

	CUXByte operator -(const CUXByte& other);
	CUXByte operator -(CUXByte& other);
	CUXByte operator -(const unsigned char& other);
	CUXByte operator -(unsigned char& other);

	CUXByte operator *(const CUXByte& other);
	CUXByte operator *(CUXByte& other);
	CUXByte operator *(const unsigned char& other);
	CUXByte operator *(unsigned char& other);

	CUXByte operator /(const CUXByte& other);
	CUXByte operator /(CUXByte& other);
	CUXByte operator /(const unsigned char& other);
	CUXByte operator /(unsigned char& other);

	CUXByte operator %(const CUXByte& other);
	CUXByte operator %(CUXByte& other);
	CUXByte operator %(const unsigned char& other);
	CUXByte operator %(unsigned char& other);

	CUXByte operator &(const CUXByte& other);
	CUXByte operator &(CUXByte& other);
	CUXByte operator &(const unsigned char& other);
	CUXByte operator &(unsigned char& other);

	CUXByte operator |(const CUXByte& other);
	CUXByte operator |(CUXByte& other);
	CUXByte operator |(const unsigned char& other);
	CUXByte operator |(unsigned char& other);

	CUXByte operator ~();

	CUXByte& operator ++();
	CUXByte operator ++(int);
	CUXByte& operator --();
	CUXByte operator --(int);

	CUXByte& operator +=(const CUXByte& other);
	CUXByte& operator +=(CUXByte& other);
	CUXByte& operator +=(const unsigned char& other);
	CUXByte& operator +=(unsigned char& other);

	CUXByte& operator -=(const CUXByte& other);
	CUXByte& operator -=(CUXByte& other);
	CUXByte& operator -=(const unsigned char& other);
	CUXByte& operator -=(unsigned char& other);

	CUXByte& operator *=(const CUXByte& other);
	CUXByte& operator *=(CUXByte& other);
	CUXByte& operator *=(const unsigned char& other);
	CUXByte& operator *=(unsigned char& other);

	CUXByte& operator /=(const CUXByte& other);
	CUXByte& operator /=(CUXByte& other);
	CUXByte& operator /=(const unsigned char& other);
	CUXByte& operator /=(unsigned char& other);

	CUXByte& operator %= (const CUXByte& other);
	CUXByte& operator %= (CUXByte& other);
	CUXByte& operator %= (const unsigned char& other);
	CUXByte& operator %= (unsigned char& other);

	CUXByte& operator &=(const CUXByte& other);
	CUXByte& operator &=(CUXByte& other);
	CUXByte& operator &=(const unsigned char& other);
	CUXByte& operator &=(unsigned char& other);

	CUXByte& operator |=(const CUXByte& other);
	CUXByte& operator |=(CUXByte& other);
	CUXByte& operator |=(const unsigned char& other);
	CUXByte& operator |=(unsigned char& other);

private:
	unsigned char data = 0;
};

#endif

