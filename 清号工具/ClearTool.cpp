#include "ClearTool.h"

//////////////////////////////////////////////////////////////////////////////////////////
// 全局函数
//////////////////////////////////////////////////////////////////////////////////////////

bool LoadConf(const char *filename);
void MakeGameConnStr();

bool CopyChunk(_RecordsetPtr &des, const _RecordsetPtr &src, const char *fileds);

void Log(const char *str);
void LogChanged(FILE *fChanged, const char *str);

BYTE GetNum(char str[3]);

//////////////////////////////////////////////////////////////////////////////////////////
// 全局变量
//////////////////////////////////////////////////////////////////////////////////////////
char sql[2048], msg[1024];
// 记录文件
FILE *file;

// 数据库配置
SQL_SETUP sqlDes; // 目的数据库配置
SQL_SETUP sqlLoginSrc; // 源数据库配置

string cnstrDes;
string loginStr;

ID_SETUP idDes;	// 目的数据库ID配置

clock_t begin_time, total_time;

map<long, bool> mDel_player_guid;
vector<long> vDel_player_guid;

BYTE num;
VARIANT_BOOL bEof;

HRESULT hr;
_ConnectionPtr cnDes;
_ConnectionPtr logincnstrSrc;
// 条件
long lDay = 60; // 多少日没有登陆过
long lLevel = 46; // 多少级以下账号
long isClearAccount = 1; // 清除账号

int main(int argc, char *argv[]) {
	// 初始ADO
	::CoInitialize(NULL);
	srand(static_cast<unsigned int>(time(NULL)));

	// 读取配置文件
	if (argc >= 2) { // 从console读取
		sprintf(msg, argv[1]);
	}
	else { // 默认配置文件
		strcpy(msg, "ClearTool.ini");
	}

	if (!LoadConf(msg))
	{
		printf("Config file not found!\n");
		exit(1);
	}

	MakeGameConnStr();

	// connecte string initialized
	// 创建数据库连接对象
	hr = cnDes.CreateInstance(__uuidof(Connection));
	if (FAILED(hr)) exit(1);

	hr = logincnstrSrc.CreateInstance(__uuidof(Connection));
	if (FAILED(hr)) exit(1);
	// 设置查询超时
	cnDes->CommandTimeout = 1800;
	logincnstrSrc->CommandTimeout = 1800;

	time_t t = time(NULL);
	tm *n = localtime(&t);

	// main programer strating
	sprintf(msg, "Clear-%s-%02d%02d.log", sqlDes.Database.data(), n->tm_mday, n->tm_min);
	file = fopen(msg, "a+"); // 打开记录日志


	if (!file) printf("操作记录日志未启动!\r\n");

	Log("<Begin>\n");

	sprintf(msg, "<Info>\t操作数据库:%s %s\n", sqlDes.Server.data(), sqlDes.Database.data());
	Log(msg);

	_RecordsetPtr rs;

	try {
		// 打开数据库连接
		hr = cnDes->Open(cnstrDes.data(), "", "", adConnectUnspecified);
		if (FAILED(hr)) _com_issue_error(E_FAIL);

		hr = rs.CreateInstance(__uuidof(Recordset));
		if (FAILED(hr)) _com_issue_error(E_FAIL);

		total_time = begin_time = clock();
		////////////////////////////////////////////////////////////////////////////////////////
		//
		// 找出指定天数没有登录的账号
		//
		////////////////////////////////////////////////////////////////////////////////////////
		sprintf(msg, "<Info>\t找出%d天没有登录的账号_开始...\n", lDay);
		Log(msg);
		time_t curTime = time(NULL);
		time_t outTime = curTime - (lDay * 3600 * 24);

		tm *nOutTime = localtime(&outTime);
		char strOutTime[MAX_PATH] = { 0 };
		sprintf(strOutTime, "%d/%d/%d %d:%d:%d", nOutTime->tm_year + 1900, nOutTime->tm_mon, nOutTime->tm_mday, nOutTime->tm_hour, nOutTime->tm_min, nOutTime->tm_sec);

		sprintf(sql, "SELECT * FROM CSL_PLAYER_ABILITY WHERE SaveTime < '%s'", strOutTime);
		sprintf(msg, "%s\n", sql);
		Log(msg);
		hr = rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if (FAILED(hr)) _com_issue_error(E_FAIL);

		while (!rs->GetRSEOF()) {
			long lPlayerID = (long)rs->GetCollect("ID");
			mDel_player_guid[lPlayerID] = true;
			rs->MoveNext();
		} // end while
		rs->Close();
		sprintf(msg, "<Info>\t找出%d天没有登录的账号_完成！用时：%d\n", lDay, begin_time = clock() - begin_time);
		Log(msg);

		////////////////////////////////////////////////////////////////////////////////////////
		//
		// 找出小于指定等级的账号
		//
		////////////////////////////////////////////////////////////////////////////////////////
		sprintf(msg, "<Info>\t找出等级小于%d级的账号_开始...\n", lLevel);
		Log(msg);

		sprintf(sql, "SELECT * FROM CSL_PLAYER_BASE WHERE Levels < %d", lLevel);
		sprintf(msg, "%s\n", sql);
		Log(msg);
		hr = rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if (FAILED(hr)) _com_issue_error(E_FAIL);

		while (!rs->GetRSEOF()) {
			long lPlayerID = (long)rs->GetCollect("ID");
			mDel_player_guid[lPlayerID] = true;
			rs->MoveNext();
		} // end while
		rs->Close();
		sprintf(msg, "<Info>\t找出等级小于%d级的账号_完成！用时：%d\n", lLevel, begin_time = clock() - begin_time);
		Log(msg);

		////////////////////////////////////////////////////////////////////////////////////////
		//
		// 根据找到的结果进行删除
		//
		////////////////////////////////////////////////////////////////////////////////////////
		sprintf(msg, "<Info>\t即将清理账号：%d个\n", mDel_player_guid.size());
		Log(msg);

		map<long, bool>::iterator iter;
		for (iter = mDel_player_guid.begin(); iter != mDel_player_guid.end();)
		{
			//sprintf( msg, "%s\n", iter->first.c_str() );
			//Log(msg);
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_PLAYER_BASE 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			sprintf(sql, "DELETE FROM CSL_PLAYER_BASE WHERE ID = %d", iter->first);
			cnDes->Execute(sql, NULL, adCmdText);
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_PLAYER_ABILITY 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			sprintf(sql, "DELETE FROM CSL_PLAYER_ABILITY WHERE ID = %d", iter->first);
			cnDes->Execute(sql, NULL, adCmdText);
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_PLAYER_QUEST_EX 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			sprintf(sql, "DELETE FROM CSL_PLAYER_QUEST_EX WHERE PlayerID = %d", iter->first);
			cnDes->Execute(sql, NULL, adCmdText);
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 player_goods 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			// 获取GOODSID
			sprintf(sql, "SELECT * FROM player_goods WHERE playerID = %d", iter->first);
			hr = rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
			if (FAILED(hr)) _com_issue_error(E_FAIL);
			while (!rs->GetRSEOF()) {
				string sGoodsID = "";
				sGoodsID = (const char*)(_bstr_t)rs->GetCollect("id");
				// 删除 extend_properties
				try {
					sprintf(sql, "DELETE FROM extend_properties WHERE id = '%s'", sGoodsID.c_str());
					cnDes->Execute(sql, NULL, adCmdText);
				}
				catch (_com_error &e) {
					sprintf(msg, "<ADO ERR>\t数据库异常(extend_properties): %s\n无法删除：%s\n", static_cast<const char*>(e.Description()), sGoodsID.c_str());
					Log(msg);
				}
				rs->MoveNext();
			} // end while
			rs->Close();
			sprintf(sql, "DELETE FROM player_goods WHERE playerID = %d", iter->first);
			cnDes->Execute(sql, NULL, adCmdText);
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_FACTION_Apply 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			sprintf(sql, "DELETE FROM CSL_FACTION_Apply WHERE PlayerID = %d", iter->first);
			cnDes->Execute(sql, NULL, adCmdText);
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_FACTION_BaseProperty 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////

			// 玩家是否帮主
			long FactionID = 0;
			sprintf(sql, "SELECT * FROM CSL_FACTION_BaseProperty WHERE MasterID = %d", iter->first);
			hr = rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
			if (FAILED(hr)) _com_issue_error(E_FAIL);
			while (!rs->GetRSEOF()) {
				FactionID = (long)rs->GetCollect("ID");
				rs->MoveNext();
			} // end while
			rs->Close();

			if (FactionID != 0)
			{
				sprintf(sql, "DELETE FROM CSL_FACTION_BaseProperty WHERE MasterID = %d", iter->first);
				cnDes->Execute(sql, NULL, adCmdText);
			}
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_FACTION_LeaveWord 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			if (FactionID != 0)
			{
				sprintf(sql, "DELETE FROM CSL_FACTION_LeaveWord WHERE FactionID = %d", FactionID);
				cnDes->Execute(sql, NULL, adCmdText);
			}
			sprintf(sql, "DELETE FROM CSL_FACTION_LeaveWord WHERE LeaveWordPlayerID = %d", iter->first);
			cnDes->Execute(sql, NULL, adCmdText);
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_FACTION_Members 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			sprintf(sql, "DELETE FROM CSL_FACTION_Members WHERE PlayerID = %d", iter->first);
			cnDes->Execute(sql, NULL, adCmdText);
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_FACTION_Ability 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			if (FactionID != 0)
			{
				sprintf(sql, "DELETE FROM CSL_FACTION_Ability WHERE FactionID = %d", FactionID);
				cnDes->Execute(sql, NULL, adCmdText);
			}
			////////////////////////////////////////////////////////////////////////////////////////
			//
			// 删除 CSL_UNION_BaseProperty 中的数据
			//
			////////////////////////////////////////////////////////////////////////////////////////
			if (FactionID != 0)
			{
				try {
					sprintf(sql, "DELETE FROM CSL_UNION_BaseProperty WHERE MasterID = %d", FactionID);
					cnDes->Execute(sql, NULL, adCmdText);
				}
				catch (_com_error &e) {
					sprintf(msg, "<ADO ERR>\t数据库异常(CSL_UNION_BaseProperty): %s\n无法删除：%d\n", static_cast<const char*>(e.Description()), FactionID);
					Log(msg);
				}
				try {
					sprintf(sql, "DELETE FROM CSL_UNION_Members WHERE FactionID = %d", FactionID);
					cnDes->Execute(sql, NULL, adCmdText);
				}
				catch (_com_error &e) {
					sprintf(msg, "<ADO ERR>\t数据库异常(CSL_UNION_Members): %s\n无法删除：%d\n", static_cast<const char*>(e.Description()), FactionID);
					Log(msg);
				}
			}
			iter = mDel_player_guid.erase(iter++);
			sprintf(msg, "剩余：%d\n", mDel_player_guid.size());
			Log(msg);
		}


// 
// 		sprintf(msg, "<Info>\t完成！用时：%d\n", begin_time = clock() - begin_time);
// 		Log(msg);

	}
	catch (_com_error &e) {
		sprintf(msg, "<ADO ERR>\t数据库异常(GameDB): %s\n", static_cast<const char*>(e.Description()));
		Log(msg);
	}

	//  这里进行账号清除
	if (isClearAccount)
	{
		_RecordsetPtr rsLogin;
		try
		{
			if (cnDes->GetState() == adStateOpen) cnDes->Close();
			// 打开数据库连接
			hr = cnDes->Open(cnstrDes.data(), "", "", adConnectUnspecified);
			if (FAILED(hr)) _com_issue_error(E_FAIL);
			hr = rs.CreateInstance(__uuidof(Recordset));
			if (FAILED(hr)) _com_issue_error(E_FAIL);

			hr = logincnstrSrc->Open(loginStr.data(), "", "", adConnectUnspecified);
			if (FAILED(hr)) 
				_com_issue_error(E_FAIL);
			hr = rsLogin.CreateInstance(__uuidof(Recordset));
			if (FAILED(hr)) _com_issue_error(E_FAIL);
			hr = rsLogin->Open("SELECT * FROM TBL_Member_Data",
				logincnstrSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
			if (FAILED(hr)) _com_issue_error(E_FAIL);
			long size = 0;
		
			char account[30] = { 0 };
			while (!rsLogin->GetRSEOF())
			{
				// 获取帐号hh
				strcpy(account, (const char *)(_bstr_t)(rsLogin->GetCollect("AccountID")));
				sprintf(sql, "SELECT * FROM CSL_PLAYER_BASE WHERE Account = '%s'", account);
				hr = rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
				if (FAILED(hr)) _com_issue_error(E_FAIL);
				hr = rs->get_RecordCount(&size);
				if (FAILED(hr)) _com_issue_error(E_FAIL);

				if (rs->GetRSEOF())
				{
					// 删除账号
					sprintf(msg, "删除空账号 :%s \n", account);
					Log(msg);
					sprintf(sql, "DELETE FROM TBL_Member_Data WHERE AccountID='%s' ", account);
					logincnstrSrc->Execute(sql, NULL, adCmdText);
				}
				rs->Close();
				rsLogin->MoveNext();
			}
			rsLogin->Close();
		}
		catch (_com_error &e) {
			sprintf(msg, "<ADO ERR>\t数据库异常(GameDB): %s\n", static_cast<const char*>(e.Description()));
			Log(msg);
		}
	}

	sprintf(msg, "<Info>\t完成！用时：%d\n", begin_time = clock() - begin_time);
	Log(msg);

	map<long, bool>().swap(mDel_player_guid);

	if (file) fclose(file);

	if (cnDes->GetState() == adStateOpen) cnDes->Close();
	cnDes.Release();

	// 释放ADO
	::CoUninitialize();
	getchar();
	return 0;
}


// 设置IMAGE数据
bool CopyChunk(_RecordsetPtr &des, const _RecordsetPtr &src, const char *fileds) {
	_variant_t varBLOB;
	long size;

	try {
		// read
		size = src->GetFields()->GetItem(fileds)->GetActualSize();
		if (size) {
			varBLOB = src->GetFields()->GetItem(fileds)->GetChunk(size);

			/*varBLOB.vt != VT_NULL && varBLOB.vt != VT_EMPTY */
			des->GetFields()->GetItem(fileds)->AppendChunk(varBLOB);		//write
		}
		return true;
	}
	catch (_com_error &e) {
		printf((char*)e.Description());
		sprintf(msg, "<ADO ERR>\t数据库异常(Chunk): %s\n", static_cast<const char*>(e.Description()));
		fputs(msg, file);
		return false;
	}
}

// 记录日志
void Log(const char *str) {
	if (!str || !file) return;
	printf(str);
	fputs(str, file);
	fflush(file);
}
void LogChanged(FILE *fChanged, const char *str) {
	if (!str || !fChanged) return;
	fputs(str, fChanged);
	fflush(fChanged);
}

BYTE GetNum(char str[3]) {
	BYTE t;
	if (num >= 100) {
		num = 0;
	}

	t = num;

	sprintf(str, "%02d", t);
	str[2] = 0;

	num++;
	return t;
}



bool LoadConf(const char *filename)
{
	if (!filename) return false;

	ifstream stream;
	stream.open(filename);

	if (!stream.is_open())
	{
		printf("Configuration file: '%s' can not open!\r\n", msg);
		return false;
	}

	string strTemp;

	stream
		>> strTemp >> sqlDes.Server
		>> strTemp >> sqlDes.Database
		>> strTemp >> sqlDes.Uid
		>> strTemp >> sqlDes.Pwd
		>> strTemp >> lDay
		>> strTemp >> lLevel
		>> strTemp >> isClearAccount
		>> strTemp >> sqlLoginSrc.Server
		>> strTemp >> sqlLoginSrc.Database
		>> strTemp >> sqlLoginSrc.Uid
		>> strTemp >> sqlLoginSrc.Pwd

		; // end

	stream.close();
	return true;
}

void MakeGameConnStr()
{
	cnstrDes = "Provider='SQLOLEDB.1'; Data Source='" + sqlDes.Server +
		"'; Initial Catalog='" + sqlDes.Database.data() +
		"'; User ID='" + sqlDes.Uid.data() +
		"'; Password='" + sqlDes.Pwd.data() + "'";

	loginStr = "Provider='SQLOLEDB.1'; Data Source='" + sqlLoginSrc.Server +
		"'; Initial Catalog='" + sqlLoginSrc.Database.data() +
		"'; User ID='" + sqlLoginSrc.Uid.data() +
		"'; Password='" + sqlLoginSrc.Pwd.data() + "'";
}