#ifndef RSA_H_
#define RSA_H_

#pragma once

/* RSA-DES ��Կ����ϵͳ����
*
* ʹ��rsa-des��ϼ�����Կ����ϵͳ��
*��ʹ��des �ж�������������Ŀռ䣺EncrypteMemoryNeeded ͬʱ�õ�des key
*�ٶ����Ľ��м��ܲ�����õ����ģ�Encrypte
*�ѵõ���des key ʹ�öԷ��ļ��ܹ�Կ rsa���� Encrypte �õ�des key ������
*���õ������� deskey �������������Ĵ���� �Է� 
*�Է�ͨ��rsa˽Կ����Decrypte �õ�des key ������
*��ͨ�����ܺ��des key ����Decrypte ����������������ݵ����� �õ�����
*rsa-des��ϼ�����Կ����ϵͳ�������
*end
*/

//RSAͷ�ļ�֧��
#include <Wincrypt.h>

//Կ�׽ṹ����Ϣ����
typedef struct stkey{
	unsigned char ucKey[RSA_KEY_STR_LEN];			//key
	unsigned long ulKeyLen;			//key����
}KEY_T;								//����Կ�׽ṹ����Ϣ						
#define S_KEY_T	sizeof(KEY_T)		//Կ�׽ṹ�峤��	

//��Կ����
static	const char const YCRsaPublicKey[] = "";
static const char const YCRsaPrivateKey[] = "";

#define MY_ENCODE_TYPE   X509_ASN_ENCODING | PKCS_7_ASN_ENCODING
#define CRYPT_STRING_NOCRLF                 0x40000000//2003�汾��������֧�ֲ���

/////////////////////////////////////////////////////////

NAMESPACE_BEGIN(gtool)

class CRsa
{
	NO_COPY(CRsa);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CRsa);

public:
	bool Encrypte();
	bool Decrypte();
	bool Init(const char *szKeyFile = NULL, const char *szMainKeyName = NULL);
	bool ImportKeyPairs();

	//��������
protected:
	//���ļ��ж�ȡpublic key ��private key
	bool ReadPublicPrivateKey(const char * szPublicKeyFile, const char * szPrivateKeyFile);
	//д�������ļ�
	bool WritePublicPrivateKey(const char * szPublicKeyFile, const char * szPrivateKeyFile);

protected:
	bool EncrypteOne();
	bool DecrypteOne();

	//������Կ��
	bool GenerateKeyPairs();

private:
	//���ݳ�Ա
	//CSP, PRIVATEKEY,PUBLICKEY,
	HCRYPTPROV		m_hCryptProv;		//CSP�������
	HCRYPTKEY		m_hPublicKey;		//��Կ���
	HCRYPTKEY		m_hPrivateKey;		//˽Կ���
	char			m_szCspName[RSA_CONTAINER_NAME_LEN];		//������
	KEY_T			m_stPublicKey;								//��Կ
	KEY_T			m_stPrivateKey;			//˽Կ

};

//ǩ����ǩ
class CRsaSignature
{
	NO_COPY(CRsaSignature);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CRsaSignature);

	//ǩ����ǩ
public:
	//ǩ��	20170323	shy	128�ֽ�
	std::string SignatureData(char * strData, long lDataSize);
	//��ǩ	20170323	shy
	bool VerifySignature(char * strUnSignData, long lUnSignDataSize, char * strSignData, long lSignDataSize);
	//����ǩ����Կ��	20170323	shy
	bool GenRsaSignatureKeyPairs(bool bGenerateKeyFile = false);	//���ù���
	
	//����ǩ����Կ	20170323	shy
	bool ImportRsaSignatureKey(HCRYPTPROV* hProv, HCRYPTKEY* hKey, const char* szRSAKey, bool isPrivKey);

private:
	//���ݳ�Ա
	//CSP, PRIVATEKEY,PUBLICKEY,
	HCRYPTPROV		m_hCryptProv;		//CSP�������
	HCRYPTKEY		m_hPublicKey;		//��Կ���
	HCRYPTKEY		m_hPrivateKey;		//˽Կ���
	char			m_szCspName[RSA_CONTAINER_NAME_LEN];		//������
	KEY_T			m_stPublicKey;								//��Կ
	KEY_T			m_stPrivateKey;			//˽Կ
};

NAMESPACE_END(gtool)

//RSA����
class CRsaEngine
{
	NO_COPY(CRsaEngine);
	HEAP_CREATE_NEED_CLEANS_DEFINE(CRsaEngine);

	//�ӿں���
public:
	//���ܲ����� bFinal����ָ���ǲ������һ�����ݣ�ѭ�����ܵ�ʱ�����һ��Ҫ���ܵ�ΪTRUE������ı���ΪFALSE���������û����
	bool Encrypte(const unsigned char *plaintext, const unsigned long ulPlainTextLen, unsigned char *&pCipherTextOut, unsigned long& pCipherSizeInLenOut, bool bFinal = true);
	//���벢���� bFinal����ָ���ǲ������һ�����ܵ����ݣ�ѭ�����ܵ�ʱ�����һ��Ҫ���ܵ�ΪTRUE������ı���ΪFALSE���������û����
	bool Decrypte(unsigned char *ciphertext, unsigned long cipherlen, unsigned char **plaintext, unsigned long *pPlainSizeInLenOut, bool bFinal = TRUE);
	//Csp��ʼ������ʼ��������������Կ����Կ��˽Կ��������������Կ��
	bool CSPInit(bool bUseExistKeyPairs = false, const char * szPublicKeyFile = NULL, const char * szPrivateKeyFile = NULL);

	//���ù�Կ ���������
	bool SetPublicKey(unsigned char *ucKey);
	
	//����˽Կ ���������
	bool SetPrivateKey(unsigned char *ucKey);

	//ǩ����ǩ
public:
	//ǩ��	20170323	shy	128�ֽ�
	std::string SignatureData(char * strData, long lDataSize);
	//��ǩ	20170323	shy
	bool VerifySignature(char * strUnSignData, long lUnSignDataSize, char * strSignData, long lSignDataSize);
	//����ǩ����Կ��	20170323	shy
	bool GenRsaSignatureKeyPairs(bool bGenerateKeyFile = false);	//���ù���
	//����ǩ����Կ	20170323	shy
	bool ImportRsaSignatureKey(HCRYPTPROV* hProv, HCRYPTKEY* hKey, const char* szRSAKey, bool isPrivKey);


	//��Կ��ʼ��
private:
	//�����������ͷ�key
	bool Release();
	//������Կ��
	bool GenerateKeyPairs();
	//������Կ�ԣ����Ѿ�������Կ���������ٺ��룩
	bool ImportKeyPairs();

	//�ڲ����ܽ���
private:
	//�������� 
	bool EncrypteData(unsigned char *plaintext, unsigned long ulPlainTextLen, unsigned char **pCipherTextOut, unsigned long *pCipherSizeInLenOut, bool bFinal = TRUE);
	//�������� bFinal����ָ���ǲ������һ�����ܵ����ݣ�ѭ�����ܵ�ʱ�����һ��Ҫ���ܵ�ΪTRUE������ı���ΪFALSE���������û����
	bool DecrypteData(unsigned char *ciphertext, unsigned long cipherlen, unsigned char **plaintext, unsigned long *pPlainSizeInLenOut, bool bFinal = TRUE);


	//��������
private:
	//���루16���ƣ�
	bool HexEncodeData(unsigned char *pucDataSrc, unsigned long ulDataSrcDataLen, unsigned char *pucDataOut, unsigned long *pulDataSizeInLenOut);
	//�����루16���ƣ�
	bool HexDecodeData(unsigned char *pucDataSrc, unsigned long ulDataSrcDataLen, unsigned char *pucDataOut, unsigned long *pulDataSizeInLenOut);
	//16����תʮ����
	bool HexToInt(unsigned char *HexData);
	//UnicodeToAnsi
	bool UnicodeToAnsi(LPTSTR szUnicodeSrc, char *pAnsiStrOut, INT nAnsiStrSize, INT *pAnsiOutStrlenByte = NULL);
	//AnsiToUnicode
	bool AnsiToUnicode(char *pAnsiStrIn, LPTSTR szUnicodeStrOut, INT nUnicodeCacheSize, INT *nUnicodeStrLenOutCalculateZeroTail = NULL);
	//���ļ��ж�ȡpublic key ��private key
	bool ReadPublicPrivateKey(const char * szPublicKeyFile, const char * szPrivateKeyFile);
	//д�������ļ�
	bool WritePublicPrivateKey(const char * szPublicKeyFile, const char * szPrivateKeyFile);

private:
	//���ݳ�Ա
	//CSP, PRIVATEKEY,PUBLICKEY,
	HCRYPTPROV		m_hCryptProv;		//CSP�������
	HCRYPTKEY		m_hPublicKey;		//��Կ���
	HCRYPTKEY		m_hPrivateKey;		//˽Կ���
	char			m_szCspName[RSA_CONTAINER_NAME_LEN];		//������
	KEY_T			m_stPublicKey;								//��Կ
	KEY_T			m_stPrivateKey;			//˽Կ
};

//�������
class CBlockRsa : public CRsaEngine
{
public:
	CBlockRsa(){}
	virtual ~CBlockRsa(){}

public:
	//����ѭ���������� ÿ�����ݹ̶�64�ֽ�����
	bool DataBlockCircleEncrypte(unsigned char *plaintext, unsigned long ulPlainTextLen, unsigned char **pCipherTextOut, unsigned long *pCipherSizeInLenOut);
	//�����������
	bool DataBlockCircleDecrypte(unsigned char *ciphertext, unsigned long cipherlen, unsigned char **plaintext, unsigned long *pPlainSizeInLenOut);
};



////////////////////////////////////////////////////////////////////////////////////

#define DES_KEY_BIT			8				//key��λ��

#define		CSP_DES_NAME					TEXT("FengDesCsp")			//des������

//����ⶨ��
#define DES_NUM_TYPE			0			//����
#define DES_CHAR_TYPE			1			//Ӣ��
#define DES_SYMBOL_TYPE			2			//�ַ�
#define DES_KEY_TYPE_NUM		3			//��������
#define DES_NUM_LEN				10
#define DES_CHAR_LEN			26
#define DES_SYMBOL_LEN			14

//#ifdef _UNICODE
typedef struct stKeyBlob
{
	BLOBHEADER hdr;
	DWORD cbKeySize;			//ֻ֧��8λ����
	BYTE rgbKeyData[DES_KEY_BIT];		//ֻ֧��8λ����
} KEY_BLOB_T;    //�ṹ�ο�MSDN - CryptImportKey
#define S_KEY_BLOB_T sizeof(KEY_BLOB_T)
//#endif


//����
static const char DesKeyNumLib[DES_NUM_LEN] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
//��ĸ
static const char DesKeyCharLib[DES_CHAR_LEN] = { 'a', 'b', 'c', 'd', 'e'
, 'f', 'g', 'h', 'i', 'j'
, 'k', 'l', 'm', 'n', 'o'
, 'p', 'q', 'r', 's', 't'
, 'u', 'v', 'w', 'x', 'y'
, 'z' };

//����
static const char DesKeySymbolLib[DES_SYMBOL_LEN] = { '~', '!', '@', '#'
, '$', '%', '^'
, '&', '*', '('
, ')', '_', '+'
, '`' };




//des���ܽ���
/*
DES���ܡ����ܺ�����ʹ��PKCS 5 padding��CBCģʽ

������
pbKey       DES��Կ
cbKeyLen    pbKey�ֽڳ���
pbData      Ҫ���ܡ����ܵ�����
cbDataLen   pbData�ֽڳ���
pbBuf       �����������������ܺ󡢽��ܺ�����ݣ�����ΪNULL
pcbBufLen   pbBuf�ֽڳ���
��pbBuf��ΪNULLʱ������ʵ�ʸ��Ƶ�pbBuf���ֽڳ���
��pbBufΪNULLʱ��������Ҫ��pbBuf�ֽڳ���
bIsDecrypt  ΪTRUEʱִ�н��ܲ���������ִ�м��ܲ���

���أ�
�ɹ�����TRUE�����򷵻�FALSE
*/

#define __in	/*������in*/
#define __out	/*�����out*/
#define __inout	/*�������inout*/

//1.�Ȳ����������ռ��С��new�����Ŀռ� EncrypteMemoryNeeded
//2.���ܲ����� Encrypte
//3.���벢���� Decrypte
class CDESEngine
{
public:
	CDESEngine();
	virtual ~CDESEngine();
	//�ͷŶ���
	virtual void FreeObj(){ try{ delete this; } catch (...){} }

	//�ӿں���
public:
	//���ܲ�����
	bool Encrypte(__inout const BYTE* pbKey,
		__inout DWORD *pcbKeyLen,
		__in const BYTE* pbPlainText,
		__in DWORD cbPlainDataLen,
		__out BYTE* pbCipherBuf,
		__inout DWORD* pcbCipherBufLen);

	//���벢����
	bool Decrypte(__in const BYTE* pbKey,
		__in DWORD cbKeyLen,
		__in const BYTE* pbCipherData,
		__in DWORD cbCipherDataLen,
		__out BYTE* pbPlainTextBuf,
		__inout DWORD* pcbPlainTextBufLen);

	//��������Ҫ�Ŀռ�
	int    EncrypteMemoryNeeded(__inout const BYTE* pbKey,
		__inout DWORD *pcbSizeInKeyLenOut,
		__in const BYTE* pbPlainText,
		__in DWORD cbPlainDataLen);

	//��������Ҫ�Ŀռ�
	int		DecrypteMemoryNeeded(__in const BYTE* pbKey,
		__in DWORD cbKeyLen,
		__in const BYTE* pbCipherData,
		__in DWORD cbCipherDataLen);

	//CSP �ڲ�����
private:
	//������ʼ��
	bool	CspInit();

	//�������8λ��Կ
	bool	GenerateKey(BYTE * pbKey = NULL, DWORD *pnKeyLen = NULL);

	//������Կ
	bool	ImportKey();

	//����
	bool	EncrypteData(__inout const BYTE* pbKey,
		__inout DWORD *pcbKeyLen,
		__in const BYTE* pbPlainText,
		__in DWORD cbPlainDataLen,
		__out BYTE* pbCipherBuf,
		__inout DWORD* pcbCipherBufLen);

	//����
	bool	DecrypteData(__in const BYTE* pbKey,
		__in DWORD cbKeyLen,
		__in const BYTE* pbCipherData,
		__in DWORD cbCipherDataLen,
		__out BYTE* pbPlainTextBuf,
		__inout DWORD* pcbPlainTextBufLen);

	//�ͷ�����
	bool	Release();

	//��������
private:
	//���루16���ƣ�
	bool HexEncodeData(unsigned char *pucDataSrc, unsigned long ulDataSrcDataLen, unsigned char *pucDataOut, unsigned long *pulDataSizeInLenOut);
	//�����루16���ƣ�
	bool HexDecodeData(unsigned char *pucDataSrc, unsigned long ulDataSrcDataLen, unsigned char *pucDataOut, unsigned long *pulDataSizeInLenOut);
	//16����תʮ����
	bool HexToInt(unsigned char *HexData);


private:
	KEY_BLOB_T		m_stKey;
	HCRYPTKEY		m_hKey;
	HCRYPTPROV		m_hCryptProv;		//CSP�������
	TCHAR			m_szCspName[RSA_CONTAINER_NAME_LEN];		//������

};

#endif