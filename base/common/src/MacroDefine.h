#ifndef MACRO_DEFINE_H_
#define MACRO_DEFINE_H_

#pragma once

// Ĭ��IP��ַ
#ifndef DEFAULT_IP
#define DEFAULT_IP            "127.0.0.1"
#endif

//�ڴ��������������
#define MEM_POOL_GC_PERIOD		1000		//1s����һ��

//�ڴ��
#define		MEMBLOCK_MINIUM_SIZE		msize_t(8)						//�鵥λ8BYTE�����С��Ϊ�鵥λ��С��������
#define		ALIGN_SIZE					msize_t(8)						//8�ֽڶ���
#define		MEMPAGE_MINIUM_SIZE			msize_t(64*1024*1024)			//64MB��С�ڴ�ҳ���ڴ�ҳ��С��ҪΪ256MB��������
#define		MEMPAGE_INIT_NUM			msize_t(4)						//��ʼ���ڴ�ҳ��
#define		THREAD_WORK_PERIOD			msize_t(5*1000)					//ά���߳�ִ������
#define		PYSICAL_MEM_REMAIN_BYTE		msize_t(MEMPAGE_MINIUM_SIZE)	//�����ڴ汣������������ϵͳ����
#define		MEM_POOL_SUC_MERGE_SIZE		msize_t(64*1024)				//�ɹ��ں���С������
#define		MEM_POOL_MERGE_TIME_OUT		msize_t(10)						//10ms �ں�����ʱʱ��Ƭ
#define		MEM_POOL_CPU_IDLE_VAL		MYDOUBLE(60)					//cpu���е�ʹ������ֵ

//������
#undef THIS_OBJ_NAME_MACRO
#define THIS_OBJ_NAME_MACRO	(typeid(*this).name())

#undef OBJ_NAME_MACRO
#define OBJ_NAME_MACRO(obj)	(#obj)

#undef UNUSED_ARG
#define UNUSED_ARG(x)	(x)

#undef ZERO_CLEAN_OBJ
#define ZERO_CLEAN_OBJ(x)	memset(&(x), 0, sizeof(x));

#undef ZERO_CLEAN_PTR
#define ZERO_CLEAN_PTR(x, sz) memset((void *)(x), 0, (sz));

#undef COPY_OBJ
#define COPY_OBJ(dest, src) memmove(&(dest),&(src), gtool::Min(sizeof(dest), sizeof(src)));

#undef CONST_TO_NONE
#define  CONST_TO_NONE(TYPE, VAR) ( ASSERT(VAR), *(const_cast<TYPE *>(&(VAR))) )	//var����ǿ�

#undef STRARRAY_ADD_ZERO_TAIL
#define STRARRAY_ADD_ZERO_TAIL(strobj, pos) \
{ \
	auto zero_pos_##__LINE__ = gtool::Max(pos, I32(0));\
	strobj[gtool::Min(zero_pos_##__LINE__, I32(sizeof(strobj) - 1))] = 0;\
}

#undef STRPTR_ADD_ZERO_TAIL
#define STRPTR_ADD_ZERO_TAIL(strptr, strsize, pos)\
{ \
auto zero_pos_##__LINE__ = gtool::Max(pos, I32(0)); \
	strptr[gtool::Min(zero_pos_##__LINE__, I32(strsize - 1))] = 0;\
}

#undef LOOP_MAX_DEF
#define LOOP_MAX_DEF 1000

//�������Ƚ�
#undef FLOAT_COMPARE_A_EQUAL_B
#define FLOAT_COMPARE_A_EQUAL_B(a, b, epsilon)\
( ((double(a)-double(b))<(epsilon)) && (-(epsilon)<(double(a)-double(b))) )

#undef FLOAT_COMPARE_A_BIGGER_B
#define FLOAT_COMPARE_A_BIGGER_B(a, b, epsilon)\
((double(a)-double(b))>(epsilon))

#undef FLOAT_COMPARE_A_LOWWER_B
#define FLOAT_COMPARE_A_LOWWER_B(a, b, epsilon)\
((double(a)-double(b))<(-(epsilon)))

#undef FLOAT_COMPARE_A_BIGGER_EQUAL_B
#define  FLOAT_COMPARE_A_BIGGER_EQUAL_B(a, b, epsilon)\
(FLOAT_COMPARE_A_BIGGER_B(a, b, epsilon)||FLOAT_COMPARE_A_EQUAL_B(a, b, epsilon))

#undef FLOAT_COMPARE_A_LOWWER_EQUAL_B
#define  FLOAT_COMPARE_A_LOWWER_EQUAL_B(a, b, epsilon)\
(FLOAT_COMPARE_A_LOWWER_B(a, b, epsilon)||FLOAT_COMPARE_A_EQUAL_B(a, b, epsilon))

#endif

