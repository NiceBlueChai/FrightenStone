#ifndef WIN_DEPEND_LIB_H_
#define WIN_DEPEND_LIB_H_
#pragma once
#if defined(NEED_WINDOWS_COMMON_API)

#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "user32.lib" )
#pragma comment (lib,"Psapi.lib")	//��ȡ����·��������

//windows ������

#endif // defined(_WIN32)|defined(_WIN64)

#endif // !WIN_DEPEND_LIB_H_


