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

#include "base/common/component/Defs/FS_AesDefs.h"

class FS_String;

class AesHandle
{
public:
    static bool GenerateKey(FS_String &key);

    // ���ܽ���
public:
    void Encrypt_ecb(const FS_String &key, const FS_String &plaintext, FS_String &cyphertext);
    void Decrypt_ecb(const FS_String &key, const FS_String &cyphertext, FS_String &plaintext);

private:
    // ��չ������Կ
    bool _ExpandEncKey(uint8_t *k, uint8_t *rc);
    // ����Կ�ӿ���
    void _AddRoundKey_cpy(uint8_t *buf, uint8_t *key, uint8_t *cpk);

    // �ֽڴ���
    void _SubBytes(uint8_t *buf);
    // ��������λ
    void _ShiftRows(uint8_t *buf);
    // �����л���
    void _MixColumns(uint8_t *buf);
    // �������
    void _AddRoundKey(uint8_t *buf, uint8_t *key);

    // ��������λ
    void _ShiftRows_Inv(uint8_t *buf);
    // �����ֽڴ���
    void _SubBytes_Inv(uint8_t *buf);
    // ������Կ��չ
    void _ExpandDecKey(uint8_t *k, uint8_t *rc);
    // �����л���
    void _MixColumns_Inv(uint8_t *buf);

    // s�м���
#ifndef BACK_TO_TABLES
    // s����ȡ��Ӧ��ֵ
    uint8_t _RJ_SBox(uint8_t x);
    // ������٤�������ϵĳ���
    uint8_t _GF_MulInv(uint8_t x); // calculate multiplicative inverse
    // ��GF���ϼ������
    uint8_t _GF_Log(uint8_t x); // calculate logarithm gen 3
    // ��GF���ϼ�������ķ�����
    uint8_t _GF_ALog(uint8_t x); // calculate anti-logarithm gen 3
    // s�е�����ȡ��Ӧ��ֵ
    uint8_t _RJ_SBox_Inv(uint8_t x);
#endif
    // ��ָ��
    uint8_t _RJ_xTime(uint8_t x);
};

#endif
