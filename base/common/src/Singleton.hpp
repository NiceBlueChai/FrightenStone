#ifndef SINGLETON_H_
#define SINGLETON_H_

#pragma once

// #include <BaseCode/WinLocker.h>
// #include <BaseCode/MyBaseMacro.h>
//#include <memory>

class CCriticalSectionProtect;

NAMESPACE_BEGIN(gtool)

//���ɿ���������ת�Ƶ��� ���ɴ��� �̰߳�ȫ �����ջ������
template<typename T>
class CSingleton
{
public:
	static inline T* GetInstance();

private:
	CSingleton(void) {}
	~CSingleton(void) {}
	CSingleton(const CSingleton&) {}
	CSingleton(CSingleton&&) {}
	CSingleton& operator= (const CSingleton&) {}

private:
	static typename std::auto_ptr<T>		m_pObj;
	static CCriticalSectionProtect			m_SafeLocker;		//�����ͷŵ�����
//	static bool								m_bLockCreateSuc;
//	static volatile CCriticalSectionProtect* m_pSafeLocker;
};

template<typename T>
typename std::auto_ptr<T> CSingleton<T>::m_pObj(NULL);

// template<typename T>
// volatile CCriticalSectionProtect* CSingleton<T>::m_pSafeLocker = NULL;

template<typename T>
CCriticalSectionProtect CSingleton<T>::m_SafeLocker;

template<typename T>
inline T* CSingleton<T>::GetInstance()
{
	//�ж�ָ���Ƿ���ڴ����򷵻ز������򴴽�
	ASSERT(m_SafeLocker.Lock());
	T * pObj = m_pObj.get();
	if (NULL != pObj)
	{	
		ASSERT(m_SafeLocker.Unlock());
		return pObj;
	}	
	pObj = NULL;
	m_pObj = std::auto_ptr<T>(new T);
	pObj = m_pObj.get();
	ASSERT(m_SafeLocker.Unlock());

	return pObj;
}


NAMESPACE_END(gtool)

#endif
