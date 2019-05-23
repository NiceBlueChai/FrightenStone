#ifndef WIN_GTOOL_HPP_
#define WIN_GTOOL_HPP_

#if defined(NEED_WINDOWS_COMMON_API)



NAMESPACE_BEGIN(gtool)

//windowsƽ̨���뼶
time_t InsertCurrentTime(gtool::CFmtIOString& fos);


//��ȡԶ��ip�˿�
template<NETWORK_CARD_TYPE NetCardType = NETWORK_CARD_TYPE_0>
static I32 GetLocalIp(char *&ip, bool bUseToBind = true , bool bSockStream = true, bool bIpv4 = true)
{
	// ��ñ���������+
	BUFFER128 szBuffer = { 0 };
	char hostname[MAX_PATH] = { 0 };
	CHECKF_NL(gethostname(hostname, MAX_PATH) == 0);
	char *pBuffer = szBuffer;

	//��ȡip
	const char *pHost = hostname;
	const char *pService = NULL;
	CHECKF_NL(GetIPByDomain(pHost, pService
		, pBuffer
		, sizeof(szBuffer)
		, NetCardType
		, bUseToBind ? NET_SYS::AI_FLAGS_TYPE_AI_PASSIVE : NET_SYS::AI_FLAGS_TYPE_NONE
		, bSockStream ? NET_SYS::SOCK_TYPE_SOCK_STREAM : NET_SYS::SOCK_TYPE_SOCK_DGRAM
		, bIpv4 ? NET_SYS::FAMILY_TYPE_AF_INET : NET_SYS::FAMILY_TYPE_AF_INET6
		, NET_SYS::PROTOCOL_TYPE_IPPROTO_IP		
	) );

	strcpy(ip, szBuffer);

	return strlen(ip) > 0;
}
// 
// template<NETWORK_CARD_TYPE NetCardType = NETWORK_CARD_TYPE_0>
// static I32 GetLocalIp(const SOCKET_IDD sSocket, char *&ip,U16& nPort, const I32 ipaddrsize)
// {
// 	if (!ip || ipaddrsize <= 0) return -1;
// 
// 	int ret = 0;
// 	struct sockaddr_in dstadd_in;
// 	socklen_t  len = sizeof(struct sockaddr_in);
// 	memset(&dstadd_in, 0, sizeof(struct sockaddr_in));
// 	if (ret = getsockname(sSocket, (struct sockaddr*)&dstadd_in, &len) < 0)
// 	{
// 		ASSERT(false);
// 		return WSAGetLastError();
// 	}
// 	else
// 	{
// 		if (ipaddrsize < strlen(inet_ntoa(dstadd_in.sin_addr))) return -1;
// 		sprintf(ip, "%s", inet_ntoa(dstadd_in.sin_addr));
// 		nPort = ntohs(dstadd_in.sin_port);
// 	}
// 
// 	return ret;
// }


//��ȡ�ڴ���Ϣ
void GetMemoryInfo(MEMORYSTATUS& MemInfo);

template<TIMER_ACCURACY_TYPE eTimerType = TIMER_ACCURACY_TYPE_SEC >
static double GetCurHighPreciTimestamp()	//����cpu�񵴵ĸ߾���ʱ���
{
	LARGE_INTEGER nLargeNumber;
	ZERO_CLEAN_OBJ(nLargeNumber);
	IF_NOT_NL(QueryPerformanceFrequency(&nLargeNumber))
		return -1;

	double lfFrequancy = double(nLargeNumber.QuadPart);
	IF_NOT_NL(lfFrequancy > (std::numeric_limits<double>::min)())
		return -1;

	IF_NOT_NL(QueryPerformanceCounter(&nLargeNumber))
		return -1;

	double lfCpuCount = nLargeNumber.QuadPart;
	switch (eTimerType)
	{
	case TIMER_ACCURACY_TYPE_SEC:
		return double(lfCpuCount / lfFrequancy);
	case TIMER_ACCURACY_TYPE_MILL_SEC:
		return double(lfCpuCount / lfFrequancy * 1000);
	case TIMER_ACCURACY_TYPE_MICRO_SEC:
		return double(lfCpuCount / lfFrequancy * 1000 * 1000);
	case TIMER_ACCURACY_TYPE_NANO_SEC:
		return double(lfCpuCount / lfFrequancy * 1000 * 1000 * 1000);
	default:
		ASSERT(false);
		break;
	}

	return -1;
}

//����ʱ���
static double GetMilliSecTimestamp()
{
	return GetCurHighPreciTimestamp<TIMER_ACCURACY_TYPE_MILL_SEC>();
}

static double SecTimestamp()
{
	return GetCurHighPreciTimestamp<TIMER_ACCURACY_TYPE_SEC>();
}

msize_t GetMemPoolAlignByte(const msize_t&nByte, msize_t nPoolRestByte);	//nPoolRestByte<0��ȡϵͳʣ���ڴ�

msize_t GetSysAlignByte(const msize_t&nByte);

msize_t GetAvailProcessSize();	//ʣ����̿ռ�

msize_t GetTotalProcessSize();	//�ܽ��̿ռ�

//��ȡ����·��
bool GetProcPath(
	__in	bool	bCurrentProc,
	__in	MYULONG	ulPid,
	__inout LPTSTR	lpProcPath,
	__in	MYULONG	ulProcPathBufLen
);

//�����������
HANDLE CreateProcessSnapshot();

I32 GetFirstProcessPid(HANDLE& hSnapshot);

I32 GetNextProcessPid(HANDLE& hSnapshot);


///< ö�ٴ��ڻص�����
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

///< ͨ������ID��ȡ���ھ��
HWND GetWindowHwndByPID(DWORD dwProcessID);


NAMESPACE_END(gtool)

#endif

#endif // !WIN_GTOOL_HPP_

#pragma once
