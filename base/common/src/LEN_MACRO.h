#ifndef LEN_MACRO_H_
#define LEN_MACRO_H_

#pragma once

#define	LEN_AES_256_KEY			(256/8)

#define AES_PLAINTXT_PART_LEN_LIMIT		16			//���ķ��鳤��

//�궨��
#define		PLAIN_TEXT_MAX_LEN					64						//���ĳ������ƣ����÷�����ܣ�117��1024/8-11����rsa1024�����ĵ�����
#define		CIPHER_TEXT_OUT_ENCODE_LEN			256						//���ܲ����������ݳ��ȣ��������Ķ೤ �����������ĵ����ƣ�
#define		CIPHER_TEXT_OUT_DECODE_LEN			128						//Ϊ����ļ��ܺ�����ݳ���
#define		RSA_CONTAINER_NAME_LEN				128						//CSP������
#define		CSP_CONTAINER_NAME				"FengYanchenCsp"		//����CSP������
#define		PUBLIC_KEY_APPNAME				"PUBLIC_KEY"			//��Կ�ڵ���
#define		PRIVATE_KEY_APPNAME				"PRIVATE_KEY"			//˽Կ�ڵ���
#define		RSA_KEY_NAME					"key"					//����
#define		RSA_KEY_STR_LEN					8192						//��Կ����

#define		INI_FILE_NAME_LEN		512

#undef MD5_VERIFY_SIZE
#define MD5_VERIFY_SIZE	(MD5_DIGEST_LENGTH * 2 + 1)	//md5��У����

#endif
