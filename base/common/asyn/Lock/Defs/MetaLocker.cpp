/**
* @file MetaLocker.cpp
* @auther Huiya Song <120453674@qq.com>
* @date 2019/04/25
* @brief ʹ��windows�ٽ���ʵ�֣���������
*/
#include "stdafx.h"
#include "base/common/asyn/Lock/Defs/MetaLocker.h"

MetaLocker::MetaLocker()
{
    memset(&_handle, 0, sizeof(_handle));
}

MetaLocker::~MetaLocker()
{

}
