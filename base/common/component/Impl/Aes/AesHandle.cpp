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
 * @file  : AesHandle.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/asyn/Lock/Lock.h"
#include "base/common/component/Impl/FS_String.h"
#include "base/common/component/Impl/Aes/AesHandle.h"
#include "base/common/component/Impl/FS_Random.h"

#define F(x)   (((x)<<1) ^ ((((x)>>7) & 1) * 0x1b))
#define FD(x)  (((x) >> 1) ^ (((x) & 1) ? 0x8d : 0))
#define COMBINE_BYTE(High_Bit, Low_Bit)     ( (( (High_Bit)&0x0F)<<4)|( (Low_Bit)&0x0F) )   // ����λ����λ�ϲ�һ���ֽ�,0x0F��ֻ֤��4λ����

FS_NAMESPACE_BEGIN

// #define BACK_TO_TABLES
#ifdef SBOX_TO_TABLES

const U8 AES_SBox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};
const U8 AES_SBoxInv[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,
    0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,
    0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,
    0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,
    0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,
    0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,
    0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,
    0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,
    0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,
    0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,
    0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,
    0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,
    0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,
    0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,
    0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,
    0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

#define _RJ_SBox(x)     AES_SBox[(x)]
#define _RJ_SBox_Inv(x) AES_SBoxInv[(x)]
#else /* tableless subroutines */
// s����ȡ��Ӧ��ֵ
U8 AesHandle::_RJ_SBox(U8 x)
{
    U8 y = 0, sb = 0;
    sb = y = _GF_MulInv(x);
    y = (y << 1) | (y >> 7); sb ^= y;  y = (y << 1) | (y >> 7); sb ^= y;
    y = (y << 1) | (y >> 7); sb ^= y;  y = (y << 1) | (y >> 7); sb ^= y;
    return (sb ^ 0x63);
}

// ������٤�������ϵĳ���
U8 AesHandle::_GF_MulInv(U8 x) // calculate multiplicative inverse
{
    return (x) ? _GF_ALog(255 - _GF_Log(x)) : 0;
    //return (x) ? gf_alog(AES_ALGORITHM_256_BIT-1 - gf_log(x)) : 0;
}

// ��GF���ϼ������
U8 AesHandle::_GF_Log(U8 x) // calculate logarithm gen 3
{
    U8 atb = 1, i = 0, z = 0;

    do {
        if(atb == x) break;
        z = atb; 
        atb <<= 1; 
        if(z & 0x80) 
            atb ^= 0x1b; 
        atb ^= z; //��ֹ�������
    } while(++i > 0);

    return i;
}
/*
AES�㷨�ǻ���GF(2^8)�����,���е�0x02�����������ʱ,������0x80����,����Ҫ��0x1b���,0x1b������
����,���0x1b��ʲô��������ô?0x80������ʲô��Ϊ��׼����?
�㿴�ĺ�ϸ��,��ʵ���������ô��ϸ,�Ҹ���˵һ��
���ڲ�����0x80��ʵָ�����λ�ǲ���1,��ΪС��1x80����ֻҪ���ƣ���2���Ϳ�����,���漰����λ,�����ڻ����0x80����,���λ��1,���ƾͻ������,�����������Ͳ�������������,Ϊ��ʵ�ֽ�λ�������Ȼͣ�������������ì�ܾ�Ҫ�Ҹ�����
������������ʱ����0x1b���,0x1b��ô������,��϶�֪��GF��28���Ĳ���Լ����ʽ��,m��x��=x8+x4+x3+x+1,���Ķ����ƾ���0x1b
*/

// ��GF���ϼ�������ķ�����
U8 AesHandle::_GF_ALog(U8 x) // calculate anti-logarithm gen 3
{
    U8 atb = 1, z = 0;
    while(x--) 
    { 
        z = atb; 
        atb <<= 1; 
        if(z & 0x80) 
            atb ^= 0x1b; 
        atb ^= z; 
    }

    return atb;
}


// s�е�����ȡ��Ӧ��ֵ
U8 AesHandle::_RJ_SBox_Inv(U8 x)
{
    U8 y = 0, sb = 0;
    y = x ^ 0x63;
    sb = y = (y << 1) | (y >> 7);
    y = (y << 2) | (y >> 6); 
    sb ^= y; 
    y = (y << 3) | (y >> 5); 
    sb ^= y;
    return _GF_MulInv(sb);
}
#endif

U8 AesHandle::_RJ_xTime(U8 x)
{
    return (x & 0x80) ? ((x << 1) ^ 0x1b) : (x << 1);
}

bool AesHandle::Encrypt(FS_AesDefs::Aes256Context key, const FS_String &plaintext, FS_String &cyphertext)
{
    if(plaintext.empty())
        return false;
    
    // ����
    cyphertext = plaintext;

    // 1.��Կ��չ
    {
        U8 rcon = 1;
        register U8 i = 8;
        for(; --i;)
            _ExpandEncKey(key.deckey, &rcon);
    }

    // 2.����Կ����
    U8 i = 0, rcon = 0;
    _AddRoundKey_cpy(cyphertext, key.enckey, key.key);
    for(i = 1, rcon = 1; i < 14; ++i)
    {
        // �ֽڴ���
        _SubBytes(cyphertext);

        // ��������λ
        _ShiftRows(cyphertext);

        // �����л���
        _MixColumns(cyphertext);
        
        if(i & 1) 
            _AddRoundKey(cyphertext, &key.key[16]);
        else
        {
            _ExpandEncKey(key.key, &rcon);
            _AddRoundKey(cyphertext, key.key);
        }
    }

    // 3.������
    _SubBytes(cyphertext);
    _ShiftRows(cyphertext);
    _ExpandEncKey(key.key, &rcon);
    _AddRoundKey(cyphertext, key.key);
    return true;
}

bool AesHandle::Decrypt(FS_AesDefs::Aes256Context key, const FS_String &cyphertext, FS_String &plaintext)
{
    if(cyphertext.empty())
        return false;

    plaintext = cyphertext;
    // ��ʼ��
    {
        // ��Կ��չ
        U8 rcon = 1;
        register U8 i = 8;
        for(;--i;)
            _ExpandEncKey(key.deckey, &rcon);
    }

    // ����Կ����
    U8 i, rcon;
    _AddRoundKey_cpy(plaintext, key.deckey, key.key);
    // ��������λ
    _ShiftRows_Inv(plaintext);
    // �����ֽڴ���
    _SubBytes_Inv(plaintext);

    for(i = 14, rcon = 0x80; --i;)
    {
        if((i & 1))
        {
            // ������Կ��չ
            _ExpandDecKey(key.key, &rcon);
            // ����Կ��
            _AddRoundKey(plaintext, &key.key[16]);
        }
        else _AddRoundKey(plaintext, key.key);

        // �����л���
        _MixColumns_Inv(plaintext);

        // ��������λ
        _ShiftRows_Inv(plaintext);
        // �����ֽڴ���
        _SubBytes_Inv(plaintext);
    }
    _AddRoundKey(plaintext, key.key);

    return true;
}

bool AesHandle::_ExpandEncKey(U8 *k, U8 *rc)
{
    register U8 i = 0;
    k[0] ^= _RJ_SBox(k[29]) ^ (*rc);
    k[1] ^= _RJ_SBox(k[30]);
    k[2] ^= _RJ_SBox(k[31]);
    k[3] ^= _RJ_SBox(k[28]);
    *rc = F(*rc);

    for(i = 4; i < 16; i += 4)
    {
        k[i] ^= k[i - 4];
        k[i + 1] ^= k[i - 3];
        k[i + 2] ^= k[i - 2];
        k[i + 3] ^= k[i - 1];
    }

    k[16] ^= _RJ_SBox(k[12]);
    k[17] ^= _RJ_SBox(k[13]);
    k[18] ^= _RJ_SBox(k[14]);
    k[19] ^= _RJ_SBox(k[15]);

    for(i = 20; i < 32; i += 4)
    {
        k[i] ^= k[i - 4];
        k[i + 1] ^= k[i - 3];
        k[i + 2] ^= k[i - 2];
        k[i + 3] ^= k[i - 1];
    }

    return true;
}

void AesHandle::_AddRoundKey_cpy(FS_String &context, U8 *key, U8 *cpk)
{
    register U8 i = 16;
    while(i--)
    {
        context[i] ^= (cpk[i] = key[i]);
        cpk[16 + i] = key[16 + i];
    }
}

void AesHandle::_SubBytes(FS_String &context)
{
    register U8 i = 16;
    while(i--) 
        context[i] = _RJ_SBox(context[i]);
}

void AesHandle::_ShiftRows(FS_String &context)
{
    /* to make it potentially parallelable) */
    register U8 i = 0, j = 0;

    i = context[1]; 
    context[1] = context[5]; 
    context[5] = context[9]; 
    context[9] = context[13]; 
    context[13] = i;
    i = context[10]; 
    context[10] = context[2]; 
    context[2] = i;

    j = context[3]; 
    context[3] = context[15];
    context[15] = context[11];
    context[11] = context[7]; 
    context[7] = j;
    j = context[14]; 
    context[14] = context[6];
    context[6] = j;
}

void AesHandle::_MixColumns(FS_String &context)
{
    register U8 i = 0, a = 0, b = 0, c = 0, d = 0, e = 0;
    for(i = 0; i < 16; i += 4)
    {
        a = context[i];
        b = context[i + 1]; 
        c = context[i + 2]; 
        d = context[i + 3];

        e = a ^ b ^ c ^ d;
        context[i] ^= e ^ _RJ_xTime(a^b);
        context[i + 1] ^= e ^ _RJ_xTime(b^c);
        context[i + 2] ^= e ^ _RJ_xTime(c^d);
        context[i + 3] ^= e ^ _RJ_xTime(d^a);
    }
}

void AesHandle::_AddRoundKey(FS_String &context, U8 *key)
{
    register U8 i = 16;
    while(i--) 
        context[i] ^= key[i];
}

void AesHandle::_ShiftRows_Inv(FS_String &context)
{
    /* same as above :) */
    register U8 i = 0, j = 0;
    i = context[1]; 
    context[1] = context[13]; 
    context[13] = context[9]; 
    context[9] = context[5]; 
    context[5] = i;
    i = context[2]; 
    context[2] = context[10]; 
    context[10] = i;

    j = context[3]; 
    context[3] = context[7]; 
    context[7] = context[11]; 
    context[11] = context[15];
    context[15] = j;
    j = context[6]; 
    context[6] = context[14]; 
    context[14] = j;
}

void AesHandle::_SubBytes_Inv(FS_String &context)
{
    register U8 i = 16;
    while(i--)
        context[i] = _RJ_SBox_Inv(context[i]);
}

void AesHandle::_ExpandDecKey(U8 *k, U8 *rc)
{
    U8 i;
    for(i = 28; i > 16; i -= 4)
    {
        k[i + 0] ^= k[i - 4];
        k[i + 1] ^= k[i - 3];
        k[i + 2] ^= k[i - 2];
        k[i + 3] ^= k[i - 1];
    }

    k[16] ^= _RJ_SBox(k[12]);
    k[17] ^= _RJ_SBox(k[13]);
    k[18] ^= _RJ_SBox(k[14]);
    k[19] ^= _RJ_SBox(k[15]);

    for(i = 12; i > 0; i -= 4)
    {
        k[i + 0] ^= k[i - 4];
        k[i + 1] ^= k[i - 3];
        k[i + 2] ^= k[i - 2];
        k[i + 3] ^= k[i - 1];
    }

    *rc = FD(*rc);
    k[0] ^= _RJ_SBox(k[29]) ^ (*rc);
    k[1] ^= _RJ_SBox(k[30]);
    k[2] ^= _RJ_SBox(k[31]);
    k[3] ^= _RJ_SBox(k[28]);
}

void AesHandle::_MixColumns_Inv(FS_String &context)
{
    register U8 i = 0, a = 0, b = 0, c = 0, d = 0, e = 0, x = 0, y = 0, z = 0;
    for(i = 0; i < 16; i += 4)
    {
        a = context[i]; 
        b = context[i + 1]; 
        c = context[i + 2]; 
        d = context[i + 3];
        e = a ^ b ^ c ^ d;
        z = _RJ_xTime(e);
        x = e ^ _RJ_xTime(_RJ_xTime(z^a^c));
        y = e ^ _RJ_xTime(_RJ_xTime(z^b^d));
        context[i] ^= x ^ _RJ_xTime(a^b);
        context[i + 1] ^= y ^ _RJ_xTime(b^c);
        context[i + 2] ^= x ^ _RJ_xTime(c^d);
        context[i + 3] ^= y ^ _RJ_xTime(d^a);
    }
}

FS_NAMESPACE_END
