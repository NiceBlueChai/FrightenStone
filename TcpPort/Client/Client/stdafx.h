// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#pragma region windows网络支持
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
// #include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif // _WIN32
#pragma endregion


// TODO: 在此处引用程序需要的其他头文件
