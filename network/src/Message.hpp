#ifndef _Message_hpp_
#define _Message_hpp_
//�������籨��

enum MessageTypes
{
	T_Login=0,
	T_Login_Result,
	T_Logout,
	T_Logout_Result,
	T_ERROR
};

struct MesageHeader
{
	short lenth;
	short type;
};

struct Login :public MesageHeader
{
	Login()
	{
		lenth = sizeof(Login);
		type = T_Login;
	}
	char name[32];
	char password[32];
};
struct Login_Result :public MesageHeader
{
	Login_Result()
	{
		lenth = sizeof(Login_Result);
		type = T_Login_Result;
		result = 0;				//0��ʾ�ɹ�
	}
	char name[32] = { 0 };
	int result;
};

struct Logout :public MesageHeader
{
	Logout()
	{
		lenth = sizeof(Logout);
		type = T_Logout;
	}
	char name[32];
	char password[32];	//Ҫ��֤�˳���Ϣ�ĺϷ��ԣ��������������������֤
};


struct Logout_Result :public MesageHeader
{
	Logout_Result()
	{
		lenth = sizeof(Logout);
		type = T_Logout_Result;
		result = 0;		//0��ʾ�ɹ�
	}
	int result;
};

#endif