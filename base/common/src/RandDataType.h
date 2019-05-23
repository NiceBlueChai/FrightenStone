#ifndef RAND_DATA_TYPE_H_
#define RAND_DATA_TYPE_H_

#pragma once

//����ֲ�����
enum RAND_DIS_TYPE
{
	RAND_DIS_TYPE_SMALLINT = 0,	//һ���������ϵ���ɢ���ȷֲ�
	RAND_DIS_TYPE_INT,			//һ���������ϵ���ɢ���ȷֲ�
	RAND_DIS_TYPE_01,			//[0, 1)�ϵ��������ȷֲ�
	RAND_DIS_TYPE_REAL,			//[min, max)�ϵ��������ȷֲ�
	RAND_DIS_TYPE_BERNOULLI,	//��Ŭ���ֲ�
	RAND_DIS_TYPE_GEOMETRIC,	//���ηֲ�
	RAND_DIS_TYPE_TRIANGLE,		//���Ƿֲ�
	RAND_DIS_TYPE_EXPONENTIAL,	//ָ���ֲ�
	RAND_DIS_TYPE_NORMAL,		//��̬�ֲ�
	RAND_DIS_TYPE_LOGNORMAL,	//�����ֲ�
	RAND_DIS_TYPE_ON_SPHERE,	//�����ϵľ��ȷֲ�
	RAND_DIS_TYPE_BETA,			//�����ֲ�
	RAND_DIS_TYPE_BINOMIAL,		//����ֲ�
	RAND_DIS_TYPE_CAUCHY,		//�����ֲ�
	RAND_DIS_TYPE_DISCRETE,		//��ɢ�ֲ�
};

//�����Դ�����㷨 �㷨�ٶ��иߵ��ͣ��㷨�����ɵ͵���
enum  RAND_GEN_ALGORITHM_TYPE
{
	RAND_GEN_ALGORITHM_TYPE_RAND48=0,				//rand48�㷨�����������
	RAND_GEN_ALGORITHM_TYPE_MT19937,				//mt19937�㷨�����������
	RAND_GEN_ALGORITHM_TYPE_MT19937_64,				//mt19937-64�㷨�����������
	RAND_GEN_ALGORITHM_TYPE_LAGGED_FIBONACCI19937,	//lagged_fibonacci19937�㷨�����������
};

#undef RAND_DIS_NUM_SCOPE_MIN
#undef RAND_DIS_NUM_SCOPE_I64MAX
#undef RAND_DIS_NUM_SCOPE_I32MAX
#define RAND_DIS_NUM_SCOPE_MIN I64(0)
#define RAND_DIS_NUM_SCOPE_I64MAX I64(1152921504606846976)
#define RAND_DIS_NUM_SCOPE_I32MAX I32(1073741824)



#endif

