/*!
 * MIT License
 *
 * Copyright (c) 2019 ericyonng<120453674@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file  : AesHandle.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief : not export this class.use it as a inner class
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_AesHandle_H__
#define __Base_Common_Component_Impl_AesHandle_H__

#pragma once
#define SBOX_TO_TABLES
#include "base/common/basedefs/DataType/DataType.h"
#include "base/common/basedefs/Macro/MacroDefs.h"
#include "base/common/component/Defs/FS_AesDefs.h"

FS_NAMESPACE_BEGIN

class FS_String;

class AesHandle
{
    // ���ܽ���
public:
    bool Encrypt(FS_AesDefs::Aes256Context key, const FS_String &plaintext, FS_String &cyphertext);
    bool Decrypt(FS_AesDefs::Aes256Context key, const FS_String &cyphertext, FS_String &plaintext);

private:
    // ��չ������Կ
    bool _ExpandEncKey(U8 *k, U8 *rc);
    // ����Կ�ӿ���
    void _AddRoundKey_cpy(FS_String &context, U8 *key, U8 *cpk);
    // �ֽڴ���
    void _SubBytes(FS_String &context);
    // ��������λ
    void _ShiftRows(FS_String &context);
    // �����л���
    void _MixColumns(FS_String &context);
    // �������
    void _AddRoundKey(FS_String &context, U8 *key);

    // ��������λ
    void _ShiftRows_Inv(FS_String &context);
    // �����ֽڴ���
    void _SubBytes_Inv(FS_String &context);
    // ������Կ��չ
    void _ExpandDecKey(U8 *k, U8 *rc);
    // �����л���
    void _MixColumns_Inv(FS_String &context);

#pragma region calculate on Galois field
    // s�м���
#ifndef SBOX_TO_TABLES
    // s����ȡ��Ӧ��ֵ
    U8 _RJ_SBox(U8 x);
    // ������٤�������ϵĳ���
    U8 _GF_MulInv(U8 x); // calculate multiplicative inverse
    // ��GF���ϼ������
    U8 _GF_Log(U8 x); // calculate logarithm gen 3
    // ��GF���ϼ�������ķ�����
    U8 _GF_ALog(U8 x); // calculate anti-logarithm gen 3
    // s�е�����ȡ��Ӧ��ֵ
    U8 _RJ_SBox_Inv(U8 x);
#endif
    // ��ָ��
    U8 _RJ_xTime(U8 x);
#pragma endregion
};

FS_NAMESPACE_END

#endif
