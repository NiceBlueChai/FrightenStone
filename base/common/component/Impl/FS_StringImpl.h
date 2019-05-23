#ifdef __Base_Common_Component_Impl_FS_String__H__
/**
* @file    FS_StringImpl.h
* @author  Huiya Song<120453674@qq.com>
* @date    2019/03/01
* @brief
*/

#pragma once

#include "base/common/component/Impl/BufferAddapter.h"
#include "base/common/assist/assistobjs/assistobjs.h"
#include "base/common/component/Impl/FS_ArgsExpander.h"

FS_NAMESPACE_BEGIN
template<typename T>
inline void FS_String::_append_fmt_str(const Byte8 *fmtstr, const T &val)
{
    auto len = sprintf(_cache, fmtstr, val);
    _cache[len] = 0;
    _buffer += _cache;
}

template<typename T>
void FS_String::_append_fmt_str(const Byte8 *fmtstr, Int16 bufferSize, const T &val)
{
    Byte8 *bufferCache = new Byte8[bufferSize];
    bufferCache[0] = 0;
    auto len = sprintf(bufferCache, fmtstr, val);
    bufferCache[len] = 0;
    _buffer += static_cast<char *>(bufferCache);
    delete[]bufferCache;
}

// �󶨵Ĳ������ɳ����ṩ�Ŀɱ��������
template<typename... Args>
inline FS_String &FS_String::Format(const char *fmt, Args&&... rest)
{
    return FormatCStyle(fmt, std::forward<Args>(rest)...);

//     UInt64 firstIndex = 0, nextIndex = 0;
//     AppendFirstValidPlaceHolderString(fmt, firstIndex, _buffer);
//     NextPlaceHolderPos(fmt, firstIndex, nextIndex);
// 
//     //����
//     if(firstIndex != std::string::npos)
//     {
//         FS_String str;
//         _AppendFormat(std::move(fmt), firstIndex, nextIndex, str, std::forward<Args>(rest)...);
//         _buffer += str.c_str();
//     }
//     else
//     {
//         AppendNoFmtStr(std::forward<Args>(rest)...);
//     }

//    return *this;
}

template<typename... Args>
inline FS_String &FS_String::Append(Args&&... rest)
{
    char expandArray[] = {((*this) << rest, 0)...};    // ʹ��,���ʽ�Կɱ������������չ����չ��rest...�����ַ�ʽ����ʹ�õݹ�
    return *this;
}

//��������ģ����ֹ�ݹ�
template<typename T>
inline void FS_String::_AppendFormat(const std::string &strLeft, UInt64 &startPlaceHolderIndex, UInt64 &nextPlaceHolderIndex, FS_String &obj, T &&val)
{
    _DoAppendFormat(strLeft, startPlaceHolderIndex, nextPlaceHolderIndex, obj, val);
}

//Args&...�� ������Args������չ���õ�������������rest
template<typename T, typename... Args>
inline void FS_String::_AppendFormat(const std::string &strLeft, UInt64 &startPlaceHolderIndex, UInt64 &nextPlaceHolderIndex, FS_String &obj, T &&val, Args&&... rest)
{
    // ��restģ�������������չ
    _DoAppendFormat(strLeft, startPlaceHolderIndex, nextPlaceHolderIndex, obj, val);
    _AppendFormat(strLeft, startPlaceHolderIndex, nextPlaceHolderIndex, obj, std::forward<Args>(rest)...);
}

template<typename T>
inline void FS_String::_DoAppendFormat(const std::string &strLeft, UInt64 &startPlaceHolderIndex, UInt64 &nextPlaceHolderIndex, FS_String &obj, T &&val)
{
    // ʣ��Ĳ���
    if(startPlaceHolderIndex == std::string::npos)
    {
        obj << val;
        return;
    }

    // ���и�ʽ���Ʒ�
    const std::string &strCache = (nextPlaceHolderIndex != std::string::npos) ? \
        strLeft.substr(startPlaceHolderIndex, nextPlaceHolderIndex - startPlaceHolderIndex):\
        strLeft.substr(startPlaceHolderIndex);

    //����
    {
        //buffer����
        UInt64 bufferSize = strCache.length() + 1 + GetBufferAddapterSize<T>::GetBufferNeeded(std::forward<T>(val)) + 1;
        fs::SmartPtr<Byte8, AssistObjsDefs::MultiDelete> bufferCache = new char[bufferSize];

        //�ϳ��ַ���
        auto len = sprintf(static_cast<char *>(bufferCache), strCache.c_str(), val);
        bufferCache[len] = 0;
        _buffer += static_cast<char *>(bufferCache);
    }
     
    //ȡ��һ����ʽ�����ַ���Ƭ��
    startPlaceHolderIndex = nextPlaceHolderIndex;   //��һ������λ��
    NextPlaceHolderPos(strLeft, startPlaceHolderIndex, nextPlaceHolderIndex);
}

template< typename T>
inline void FS_String::_Append(T &&arg)
{
    (*this) << arg;
}
template<typename T, typename... Args>
inline void FS_String::_Append(T &&arg, Args&&... rest)
{
    (*this) << arg;
    _Append(std::forward<Args>(rest)...);
}

//��������ģ����ֹ�ݹ�
// template<typename T>
// inline void FS_String::_AppendNoFmt(T &&val)
// {
//     (*this) << val;
// }

FS_NAMESPACE_END

#endif
