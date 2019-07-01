#ifndef __Test_TestXor_H__
#define __Test_TestXor_H__

#pragma once

#include "stdafx.h"

class TestXor
{
public:
    static void Run()
    {
//         fs::FS_Int64Random random(0, 127);
//         char pwd[MAX_PWD_LEN / 8] = {0};
//         for(Int32 i = 0; i < sizeof(pwd); ++i)
//         {
//             pwd[i] = static_cast<char>(random(g_RandomSeed));
//             printf("%02x", pwd[i]);
//         }
// 
//         char plaintext[MAX_PWD_LEN / 8] = "wo ai ni";
//         for(Int32 i = 0; i < sizeof(plaintext); ++i)
//         {
//             printf("%d", plaintext[i]);
//         }
// 
//         std::cout << std::endl;
// 
//         // ����
//         for(Int32 i = 0; i < sizeof(plaintext); ++i)
//         {
//             plaintext[i] = plaintext[i] ^ pwd[i];
//             printf("%d", plaintext[i]);
//         }
// 
//         std::cout << std::endl;
// 
//         // ��ԭ
//         for(Int32 i = 0; i < sizeof(plaintext); ++i)
//             plaintext[i] = plaintext[i] ^ pwd[i];
// 
//         std::cout << plaintext << std::endl;

        // ����key
        char key[KEY_128BIT] = {0};
        fs::KeyGeneratorUtil::GetKey_128Bit(key);

        // ����
        char plainText[1024] = "�� �����ļ����˿��Ǵ�� ���ϵĿ���������jdsalkdjkf***@@dalk545";
        char cypherText[1024];

        // ����
        fs::XorEncrypt::Encrypt(plainText, 1024, cypherText, 1024, key, KEY_128BIT);

        // ����
        memset(plainText, 0, sizeof(plainText));
        fs::XorEncrypt::Decrypt(cypherText, 1024, plainText, 1024, key, KEY_128BIT);

        // ��ӡ
        std::cout << plainText << std::endl;
        
    }
};

#endif
