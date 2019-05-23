#include <BaseCode/ComAlgorithm.h>

NAMESPACE_BEGIN(com_algorithm)

//�����ʽ�����ַ���ռλ��������%��%���ټ��һ���ַ����磺%s%l��%%�����һ��%������ǰ��
U64 CalcFmtNum(const std::string& strFmt)
{
	//�ҳ������ڵ�%����
	U64 nPosStart = 0, nPosSec = 0, nCount = 0;

	for (nPosStart = 0; 
		nPosStart = strFmt.find_first_of('%', nPosStart),
		( (nPosStart != std::string::npos)?( (nPosSec = strFmt.find_first_of('%', nPosStart + 1)), true):false ); )
	{
		if (nPosSec != std::string::npos)
		{
			if (max(I64(nPosSec - nPosStart), I64(0)) > 1)
			{
				//������
				nCount += 2;
				nPosStart = nPosSec + 1;
			}
			else
			{
				//���ڻ��ص�
				//����������λ�ü�����
				nPosStart = nPosSec + 1;
				continue;
			}
		}
		else
		{
			++nCount;
			break;
		}
	}	

	return nCount;
}

//��ȡ��ʽ�����ַ�����һ����Ч%λ��
bool GetFirstValidFmtString(const std::string strFmt, U64& u64FirIndex, std::string& strOut)
{
	if (strFmt.length() <= 0)
		return false;

	u64FirIndex = 0;
	while ((u64FirIndex = strFmt.find_first_of('%', u64FirIndex), true))
	{
		if (u64FirIndex == std::string::npos)
		{
			strOut += strFmt.substr(0);
			return true;
		}
		else
		{
			if (u64FirIndex == 0)
			{
				return true;
			}
			else
			{
				strOut += strFmt.substr(0, u64FirIndex);
				return true;
			}
		}
	}

	u64FirIndex = std::string::npos;

	return false;
}

//u64StartIndex ����Ϊ��ǰҪ��ʽ����%����λ�� NextFmtPosOut�����һ��δ����ʽ����λ�����std::string::npos��ʾ����
const char * NextFmtString(const std::string& strFmt, const U64& u64StartIndex, U64& NextFmtPosOut)
{
	if ((strFmt.length() <= 0) || (u64StartIndex == std::string::npos))
	{
		NextFmtPosOut = std::string::npos;
		return NULL;
	}		

	U64 u64CacheIndex = u64StartIndex;
	while ((NextFmtPosOut = strFmt.find_first_of('%', u64CacheIndex), true))
	{
		if (NextFmtPosOut == std::string::npos)
		{
			return strFmt.substr(u64StartIndex).c_str();
		}
		else
		{
			if (max(I64(NextFmtPosOut - u64CacheIndex), I64(0)) > 1)
			{
				//������ȡ���ַ���Ƭ��
				return strFmt.substr(u64StartIndex, NextFmtPosOut - u64StartIndex).c_str();
			}
			else
			{
				//���ڻ��ص�
				//����������λ�ü�����
				++u64CacheIndex;
				continue;
			}
		}
	}

	//��������
	NextFmtPosOut = std::string::npos;

	return NULL;
}

NAMESPACE_END(com_algorithm)