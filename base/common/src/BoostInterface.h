#ifndef BOOST_INTERFACE_H_
#define BOOST_INTERFACE_H_

#pragma once

#include <boost/format.hpp>					//��ʽ�ַ�������
#include <boost/type_traits/is_class.hpp>	//bosst�ж��Ƿ��ࣨ�޷����ֽṹ����ࣩ�ķ���
#include <boost/atomic.hpp>					//boostԭ�Ӳ������������߳����� ��Ҫ���Ӿ�̬����߶�̬��

// #define BOOST_LIB_NAME "boost_atomic"
// 
// #include <boost/atomic/detail/atomic_template.hpp>	//ԭ�Ӳ���ģ��
//#include <boost/config/auto_link.hpp>		//boost�Զ����ӿ�
//#  pragma comment(lib, BOOST_LIB_PREFIX BOOST_STRINGIZE(atomic) "-" BOOST_LIB_TOOLSET BOOST_LIB_THREAD_OPT BOOST_LIB_RT_OPT "-" BOOST_LIB_VERSION ".lib")


//ԭ�ӿ�
#ifndef BASECODE_INC_BOOST_ATOMIC_HPP_LIB
#define BASECODE_INC_BOOST_ATOMIC_HPP_LIB

	#ifndef _DEBUG
		#pragma comment(lib, "libboost_atomic-vc141-mt-x64-1_66.lib")
	#else
		#pragma comment(lib, "libboost_atomic-vc141-mt-gd-x64-1_66.lib")
	#endif

#endif

#endif

