#ifndef BASE_INCLUDE_DEFINE_H_
#define BASE_INCLUDE_DEFINE_H_

#pragma once
#include <BaseCode/WarningControl.h>

//������Ҫ����stdafx.h
#include <BaseCode/myclassmacro.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <windows.h>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <deque>
#include <stdlib.h>
#include <math.h>
#include <algorithm>						//std���㷨
#include <memory>							//һЩ�����ڴ���㷨
#include <limits>							//���ݵķ�Χ
#include <thread>							//�߳�
#include <atomic>
//#include <BaseCode/BoostInterface.h>		//boost�ӿ�
#include <BaseCode/OpenSSLInterface.h>		//openssl�ӿ�
#include<chrono>


//������------------------------------------------------------------------end
#include <BaseCode/UnionPtrMacro.h>
#include <BaseCode/MyBaseMacro.h>			//������
//#include <BaseCode/PathMacro.h>				//·������
#include <BaseCode/MyDataType.h>			//��������
#include <BaseCode/InlineFun.hpp>			//��������
#include <BaseCode/ConstantValueDefine.h>	//��������
#include<BaseCode/LEN_MACRO.h>				//���Ⱥ궨��
#include <BaseCode/enumtype.h>				//ö�ٶ���
//#include <BaseCode/MemPoolObj.h>			//�ڴ�ش�����
#include<BaseCode/MyBaseClassTemplete.hpp>	//����������ģ��
#include <BaseCode/MacroDefine.h>			//ͨ�ú�
#include <BaseCode/CheckNoLog.h>			//��log�Ķ���
#include <BaseCode/Timer.h>					//ʱ��
#include <BaseCode/Win/Cpu.h>				//cpuinfo
#include <BaseCode/BufferAdapter.hpp>		//����������

#include <BaseCode/Win/WinDependInl.h>		//windows api������Ҫʹ�� NEED_WINDOWS_API ���غ�

//�����------------------------------------------------------------------end


#include <BaseCode/SmartPtrC.hpp>		//����ָ��֧�ְ��������ڵ�������������
#include <BaseCode/SmartPtr.hpp>		//ֻ��ָ��ֻ֧���ࣨ�ұ���� release��
#include <BaseCode/LitUnmanagedPtr.h>	//����ѹ���ָ��
#include <BaseCode/TPtrList.hpp>		//list����
#include <BaseCode/LitPtrMap.hpp>		//map����
#include <BaseCode/LitObjSet.hpp>		//set
#include <BaseCode/ComAlgorithm.h>		//ͨ���㷨
#include <BaseCode/TaskBase.h>			//�������
#include <BaseCode/Win/WinLocker.h>			//windows�µ���
#include <BaseCode/Singleton.hpp>		//����
#include <BaseCode/FileDirSys.h>		//�ļ�Ŀ¼��д
#include <BaseCode/FmtIOString.h>		//�ַ���Ӧ��
#include <BaseCode/FmtAnalysis.h>		//boost��ʽ�ַ�������
#include <BaseCode/File2.h>				//��ȫ���ļ���
#include <BaseCode/gtool.hpp>			//ͨ�ù���
#include <BaseCode/Win/WinGtool.h>	//windows����



#endif

