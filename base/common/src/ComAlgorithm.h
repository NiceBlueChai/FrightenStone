#ifndef COM_ALGORITHM_H_
#define COM_ALGORITHM_H_

#pragma once

NAMESPACE_BEGIN(com_algorithm)

//�����ʽ�����ַ���ռλ��������%��%���ټ��һ���ַ����磺%s%l��%%�����һ��%������ǰ��
U64 CalcFmtNum(const std::string& strFmt);

//��ȡ��ʽ�����ַ�����һ����Ч%λ��
bool GetFirstValidFmtString(const std::string strFmt, U64& u64FirIndex, std::string& strOut);


//��һ����ʽ�����ַ���Ƭ��
const char * NextFmtString(const std::string& strFmt, const U64& u64StartIndex, U64& NextFmtPosOut);

//�����㷨

NAMESPACE_END(com_algorithm)

#endif

