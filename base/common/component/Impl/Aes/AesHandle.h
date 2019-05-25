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
// 
// class AesHandle
// {
// public:
//     const FS_AesDefs::Aes256Context &GetKey() const;
//     static bool GenerateKey(FS_String &key);
// 
//     // ���ܽ���
// public:
//     void Encrypt()
// 
// private:
//     FS_AesDefs::Aes256Context _key;
// public:
//     //����key
//     bool GenerateKey(uint8_t *szKey);
// 
//     //���ܽ���
// protected:
//     //����ģ�� ֻ����һ��128bit 16�ֽ����� ��buf��С������128λ��������
//     void Encrypt_ecb(uint8_t *buf);
//     //����ģ�� ֻ����һ��128bit���� ��buf��С������128λ��������
//     void Decrypt_ecb(uint8_t *buf);
// 
//     //��ʼ��
// protected:
//     //��ʼ������256key
//     void Init(bool bResetKey);
//     //�����Կ
//     void ClearKey();
// 
//     //����ģ��
// protected:
// 
//     //��������
// protected:
//     //��չ������Կ
//     bool ExpandEncKey(uint8_t *k, uint8_t *rc);
//     //����Կ�ӿ���
//     void AddRoundKey_cpy(uint8_t *buf, uint8_t *key, uint8_t *cpk);
// 
//     //�ֽڴ���
//     void SubBytes(uint8_t *buf);
//     //��������λ
//     void ShiftRows(uint8_t *buf);
//     //�����л���
//     void MixColumns(uint8_t *buf);
//     //�������
//     void AddRoundKey(uint8_t *buf, uint8_t *key);
// 
//     //��������λ
//     void ShiftRows_Inv(uint8_t *buf);
//     //�����ֽڴ���
//     void SubBytes_Inv(uint8_t *buf);
//     //������Կ��չ
//     void ExpandDecKey(uint8_t *k, uint8_t *rc);
//     //�����л���
//     void MixColumns_Inv(uint8_t *buf);
// 
// 
//     //٤�������ϵļ���
// protected:
// 
//     //s�м���
// #ifndef BACK_TO_TABLES
// 
//         //s����ȡ��Ӧ��ֵ
//     uint8_t RJ_SBox(uint8_t x);
//     //������٤�������ϵĳ���
//     uint8_t GF_MulInv(uint8_t x); // calculate multiplicative inverse
//     //��GF���ϼ������
//     uint8_t GF_Log(uint8_t x); // calculate logarithm gen 3
//     //��GF���ϼ�������ķ�����
//     uint8_t GF_ALog(uint8_t x); // calculate anti-logarithm gen 3
//     //s�е�����ȡ��Ӧ��ֵ
//     uint8_t RJ_SBox_Inv(uint8_t x);
// #endif
//     //��ָ��
//     uint8_t RJ_xTime(uint8_t x);
// 
// 
// 
// protected:
//     aes256_context m_stAesKey;			//����
//     aes256_context m_stBackAesKey;		//���ڼӽ���
// };

#endif
