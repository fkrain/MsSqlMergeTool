////////////////////////////////////////////////////////////////////////////////////////////
//
// �ϲ��������ݿ�
// 1. ����һ��ȫ�¸ɾ������ݿ⣬�ݶ���Des
// 2. ��Ҫ�ϲ����������ݿ� ���������������ݿ��ݶ���ΪSrc1,Src2
// 3. ��һ��ִ�н�S1���ϲ���D�������������ID��
// 4. �ٴ�ִ�У��ڽ�S2���ϲ���D�������������ID
// 5. ���D���ݿ����S1��S2�ĺϲ����ݿ⣬����ID�������������еģ��ظ�����Ҳ�����滻
//
////////////////////////////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS
//#import "C:\\Program Files\\Common Files\\System\\ado\\msado15.dll" no_namespace rename("EOF","RSEOF")
#import "C:/Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF","RSEOF")

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <map>
//#include <set>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


// ���ݿ�����
struct SQL_SETUP {
	string Server;
	string Database;
	string Uid;
	string Pwd;
};


// ���ID����֯ID������ID
struct ID_SETUP {
	DWORD PlayerID;
	DWORD OrganizingID;
	DWORD LeavewordID;

	ID_SETUP() {
		PlayerID=0;
		OrganizingID=0;
		LeavewordID=0;
	}
};

