#ifndef MY_AES_H_
#define MY_AES_H_
#include "BaseCode/MyDataType.h"
#include <BaseCode/myclassmacro.h>

/*
*   Byte-oriented AES-256 implementation.
*   All lookup tables replaced with 'on the fly' calculations.
*
*   Copyright (c) 2007-2009 Ilya O. Levin, http://www.literatecode.com
*   Other contributors: Hal Finney
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose with or without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies.
*
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
AES�Ǹ߼����ܱ�׼��
�߼����ܱ�׼��Ӣ�Advanced Encryption Standard����д��AES����������ѧ���ֳ�Rijndael���ܷ���
�����������������õ�һ��������ܱ�׼�������׼�������ԭ�ȵ�DES���Ѿ����෽�����ҹ�Ϊȫ������ʹ�á�
�����������ѡ���̣��߼����ܱ�׼���������ұ�׼�뼼���о�Ժ��NIST����2001��11��26�շ�����FIPS PUB 197��
����2002��5��26�ճ�Ϊ��Ч�ı�׼��2006�꣬�߼����ܱ�׼��Ȼ��Ϊ�Գ���Կ�����������е��㷨֮һ��

�����
���㷨Ϊ����ʱ����ѧ��Joan Daemen��Vincent Rijmen����ƣ������λ���ߵ����֣���Rijndael֮����֮��
Ͷ��߼����ܱ�׼����ѡ���̡�Rijdael�ķ������� "Rhinedoll����λ����ߵ����֣�
�ο�������ԭ��������Ϊ�Ȱ������ɡ���ɭ�ء����ɡ���Joan������ΪŮ�Ի������֡�����
���⣬��ŷ�������ܶ�������ͬ�����Ļ�ϣ����Դͷ�ģ�����������ǿ�����ͬ��

���ܱ�׼
�Գ��������Ƶķ�չ���ƽ��Է�������Ϊ�ص㡣���������㷨ͨ������Կ��չ�㷨�ͼ��ܣ����ܣ��㷨��������ɡ�
��Կ��չ�㷨��b�ֽ��û�����Կ��չ��r������Կ�������㷨��һ������ѧ�ϵ�������f��r������Կ����r����ɡ�
���Һ���Կ��ɢ�Ƿ��������㷨��ƵĻ���ԭ�򡣵�����֪���ĵĲ�ֺ����Թ������ɱ䳤��Կ�ͷ����Ǹ����Ƶ����Ҫ�㡣
AES���������ұ�׼�����о���NISTּ��ȡ��DES��21���͵ļ��ܱ�׼��
AES�Ļ���Ҫ���ǣ����öԳƷ����������ƣ���Կ���ȵ�����֧��Ϊ128��192��256�����鳤��128λ��
�㷨Ӧ���ڸ���Ӳ�������ʵ�֡�1998��NIST��ʼAES��һ�ַ��������Ժ���������������15����ѡ�㷨��

1999��3������˵ڶ���AES2�ķ��������ԡ�
2000��10��2������������ʽ����ѡ�б���ʱ����ѧ��Joan Daemen �� Vincent Rijmen �����һ�������㷨RIJNDAEL ��Ϊ AES.
��Ӧ�÷��棬����DES�ڰ�ȫ���Ǵ����ģ������ڿ���DESоƬ�Ĵ���������ʹ��DES������ʱ����ʹ�ã�
Ϊ��߰�ȫǿ�ȣ�ͨ��ʹ�ö�����Կ������DES������DES����Ҫ��AES���档���������ƽ�֮���������������ϳ����Ұ�ȫ��
��δ��������һ�����ܱ�׼��

AES�������ݿ���鳤�ȱ���Ϊ128���أ�
��Կ���ȿ�����128���ء�192���ء�256�����е�����һ����������ݿ鼰��Կ���Ȳ���ʱ���Ჹ�룩��
AES�����кܶ��ֵ��ظ��ͱ任�����²������£�1����Կ��չ��KeyExpansion����2����ʼ�֣�Initial Round����
3���ظ��֣�Rounds����ÿһ���ְ�����SubBytes��ShiftRows��MixColumns��AddRoundKey��4�������֣�Final Round����
������û��MixColumns��
*/

//#define BACK_TO_TABLES //ʹ������S�г�����

#include <BaseCode/LEN_MACRO.h>

// #ifdef __cplusplus
// extern "C" {
// #endif


	class  CAes
	{
		NO_COPY_NEED_RVALUE(CAes);
		HEAP_CREATE_NEED_CLEANS_DEFINE(CAes);
	public:
		typedef struct {
			uint8_t key[LEN_AES_256_KEY];
			uint8_t enckey[LEN_AES_256_KEY];
			uint8_t deckey[LEN_AES_256_KEY];
		} aes256_context;

	public:
		CAes(bool bGenerateKey);
		CAes(uint8_t *key);

	public:
		//��ȡkey
		CAes::aes256_context GetKey(){ return m_stAesKey; }
		//����key
		bool GenerateKey(uint8_t *szKey);

		//���ܽ���
	protected:
		//����ģ�� ֻ����һ��128bit 16�ֽ����� ��buf��С������128λ��������
		void Encrypt_ecb(uint8_t *buf);
		//����ģ�� ֻ����һ��128bit���� ��buf��С������128λ��������
		void Decrypt_ecb(uint8_t *buf);

		//��ʼ��
	protected:
		//��ʼ������256key
		void Init(bool bResetKey);
		//�����Կ
		void ClearKey();

		//����ģ��
	protected:

		//��������
	protected:
		//��չ������Կ
		bool ExpandEncKey(uint8_t *k, uint8_t *rc);
		//����Կ�ӿ���
		void AddRoundKey_cpy(uint8_t *buf, uint8_t *key, uint8_t *cpk);
		
		//�ֽڴ���
		void SubBytes(uint8_t *buf);
		//��������λ
		void ShiftRows(uint8_t *buf);
		//�����л���
		void MixColumns(uint8_t *buf);
		//�������
		void AddRoundKey(uint8_t *buf, uint8_t *key);

		//��������λ
		void ShiftRows_Inv(uint8_t *buf);
		//�����ֽڴ���
		void SubBytes_Inv(uint8_t *buf);
		//������Կ��չ
		void ExpandDecKey(uint8_t *k, uint8_t *rc);
		//�����л���
		void MixColumns_Inv(uint8_t *buf);


		//٤�������ϵļ���
	protected:

//s�м���
#ifndef BACK_TO_TABLES
		
		//s����ȡ��Ӧ��ֵ
		uint8_t RJ_SBox(uint8_t x);
		//������٤�������ϵĳ���
		uint8_t GF_MulInv(uint8_t x); // calculate multiplicative inverse
		//��GF���ϼ������
		uint8_t GF_Log(uint8_t x); // calculate logarithm gen 3
		//��GF���ϼ�������ķ�����
		uint8_t GF_ALog(uint8_t x); // calculate anti-logarithm gen 3
		//s�е�����ȡ��Ӧ��ֵ
		uint8_t RJ_SBox_Inv(uint8_t x);
#endif
		//��ָ��
		uint8_t RJ_xTime(uint8_t x);



	protected:
		aes256_context m_stAesKey;			//����
		aes256_context m_stBackAesKey;		//���ڼӽ���
	};


class CAesDataGroup :public CAes
{
	NO_COPY_NEED_RVALUE(CAesDataGroup);
	HEAP_CREATE_NEED_CLEANS_DEFINE(CAesDataGroup);
public:
	CAesDataGroup(bool bGenerateKey);
	CAesDataGroup(uint8_t *key);

	//���ܽ���
public:
	//����	��Ҫ��128bit 16�ֽڵı���
	bool Encrypt_Data(uint8_t *buf, unsigned long ulBufSize, uint8_t *BufOut, unsigned long ulBufOutSize);
	//����	��Ҫ��128bit 16�ֽڵı���
	bool Encrypt_Data(uint8_t *buf, unsigned long ulBufSize, uint8_t *BufOut, unsigned long ulBufOutSize, bool bHexEncode);
	//���� ��Ҫ�� 128bit 16�ֽڵı���
	bool Decrypt_Data(uint8_t *buf, unsigned long ulBufSize, uint8_t *BufOut, unsigned long ulBufOutSize);
	//���� ��Ҫ�� 128bit 16�ֽڵı���
	bool Decrypt_Data(uint8_t *buf, unsigned long ulBufSize, uint8_t *BufOut, unsigned long ulBufOutSize, bool bHexDecode);

	//16���Ʊ��뷴����
public:
	//���루16���ƣ�
	bool HexEncodeData(uint8_t *pucDataSrc, unsigned long ulDataSrcDataLen, uint8_t *pucDataOut, unsigned long *pulDataSizeInLenOut);
	//�����루16���ƣ�
	bool HexDecodeData(uint8_t *pucDataSrc, unsigned long ulDataSrcDataLen, uint8_t *pucDataOut, unsigned long *pulDataSizeInLenOut);
protected:
	//16����תʮ����
	bool HexToInt(uint8_t *HexData);


};

// #ifdef __cplusplus
// }
// #endif

#endif