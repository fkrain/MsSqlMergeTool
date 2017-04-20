#include "Merger2.h"


//////////////////////////////////////////////////////////////////////////////////////////
// ȫ�ֺ���
//////////////////////////////////////////////////////////////////////////////////////////

bool LoadConf(const char *filename);
void MakeGameConnStr();
void MakeLoginConnStr();

char *FixName(char *n_name, const char *o_name, bool existent = false);		//��1��ʹ��false
//char *GetGUIDString(char *s);
char *GetDateString(_variant_t v, char *s);
bool CopyChunk(_RecordsetPtr &des, const _RecordsetPtr &src, const char *fileds);
//ULONG ReassignGoodsID(ULONG o_id);
void Log(const char *str);
void LogChanged(FILE *fChanged, const char *str);

BYTE GetNum(char str[3]);

//////////////////////////////////////////////////////////////////////////////////////////
// ȫ�ֱ���
//////////////////////////////////////////////////////////////////////////////////////////
char sql[2048],msg[1024];
// ��¼�ļ�
FILE *file;

// �ʺż�¼ ��ʽ��ϵͳ��Ӫ�ṩ���ü�¼������Ҫ
FILE *fAC;		// Account changed
FILE *fPNC;		// Player name changed

string profix;
string spechar;

int nWorldID;

// ���ݿ�����
SQL_SETUP sqlDes; // Ŀ�����ݿ�����
SQL_SETUP sqlSrc; // Դ���ݿ�����

SQL_SETUP sqlLoginDes; // Ŀ�����ݿ�����
SQL_SETUP sqlLoginSrc; // Դ���ݿ�����


string cnstrDes;
string cnstrSrc;
string logincnstrDes;
string logincnstrSrc;

ID_SETUP idDes;	// Ŀ�����ݿ�ID����
//ID_SETUP idSrc; // Դ���ݿ�ID����

map<string,string> changed_player_account;
map<string,string> changed_player_name;
map<ULONG,ULONG> changed_player_id;
map<ULONG,ULONG> changed_faction_id;

clock_t begin_time,total_time;

BYTE num;
VARIANT_BOOL bEof;

HRESULT hr;
_ConnectionPtr cnDes;
_ConnectionPtr cnSrc;

_ConnectionPtr logincnDes;
_ConnectionPtr logincnSrc;


int main(int argc, char *argv[]) {
	// ��ʼADO
	::CoInitialize(NULL);
	srand(static_cast<unsigned int>(time(NULL)));

	// ��ȡ�����ļ�
	if(argc >= 2) { // ��console��ȡ
		sprintf(msg, argv[1]);
	}
	else { // Ĭ�������ļ�
		strcpy(msg,"merger2.ini");
	}

	if( !LoadConf(msg) )
	{
		printf("Config file not found!\n");
		exit(1);
	}


	MakeGameConnStr();
	MakeLoginConnStr();

	// connecte string initialized
	// �������ݿ����Ӷ���
	hr = cnDes.CreateInstance(__uuidof(Connection));
	if(FAILED(hr)) exit(1);
	hr = cnSrc.CreateInstance(__uuidof(Connection));
	if(FAILED(hr)) exit(1);
	// �������ݿ����Ӷ���
	hr = logincnDes.CreateInstance(__uuidof(Connection));
	if(FAILED(hr)) exit(1);
	hr = logincnSrc.CreateInstance(__uuidof(Connection));
	if(FAILED(hr)) exit(1);

	// ���ò�ѯ��ʱ
	cnDes->CommandTimeout = 1800;
	cnSrc->CommandTimeout = 1800;
	logincnDes->CommandTimeout = 1800;
	logincnSrc->CommandTimeout = 1800;


	time_t t = time(NULL);
	tm *n = localtime(&t);

	// main programer strating
	sprintf(msg, "mearge-%s-%s-%02d%02d.log", sqlDes.Database.data(), sqlSrc.Database.data(), n->tm_mday, n->tm_min);
	file = fopen(msg, "a+"); // �򿪼�¼��־
	
	sprintf(msg, "Account-%s-%s-%02d%02d.log", sqlDes.Database.data(), sqlSrc.Database.data(), n->tm_mday, n->tm_min);
	fAC = fopen(msg, "a+"); 

	sprintf(msg, "PlayerName-%s-%s-%02d%02d.log", sqlDes.Database.data(), sqlSrc.Database.data(), n->tm_mday, n->tm_min);
	fPNC = fopen(msg, "a+"); 


	if(!file) printf("������¼��־δ����!\r\n");
	if(!fAC) printf("������¼��־δ����!\r\n");
	if(!fPNC) printf("������¼��־δ����!\r\n");

	LogChanged(fAC, "<Begin>\n");
	LogChanged(fPNC, "<Begin>\n");

	Log("<Begin>\n");

	sprintf(msg, "<info>\tĿ�����ݿ�:%s %s\n", sqlDes.Server.data(), sqlDes.Database.data());
	Log(msg);
	sprintf(msg, "<info>\tԴ���ݿ�:%s %s\n", sqlSrc.Server.data(), sqlSrc.Database.data());
	Log(msg);


	//profix
	sprintf(msg, "<info>\t��������ǰ׺�������ַ�:%s%s\tWORLDID%d\n",profix.data(),spechar.data(),nWorldID);
	Log(msg);


	_RecordsetPtr rs;
	
	//try
	//{

	//	//////////////////////////////////////////////////////////////////////////////////////////
	//	//// 
	//	//// ���� ���Ͳ���
	//	//// 
	//	//////////////////////////////////////////////////////////////////////////////////////////
	//	//Log("<info>\t�������Ͳ������ݿ�ʼ...\n");

	//	//// ������λ���͵���Ʒ
	//	//hr = rs->Open( "SELECT * FROM Largess WHERE SendNum > ObtainedNum", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
	//	//if(FAILED(hr)) _com_issue_error(E_FAIL);
	//	//while(!rs->GetRSEOF()) {
	//	//	try {
	//	//		_variant_t var = rs->GetCollect("SendTime");
	//	//		char sendtime[20] = "";
	//	//		if( var.vt != VT_NULL ) {
	//	//			GetDateString(var, sendtime);
	//	//		}

	//	//		sprintf(sql, "INSERT INTO Largess(cdkey,GoodsIndex,GoodsName,GoodsLevel,SendNum,WorldID,SendTime,IsProcessed) \
	//	//					 VALUES('%s',%d,'%s',%d,%d,%d,'%s',0)", (const char*)(_bstr_t)rs->GetCollect("cdkey"),
	//	//					 (long)rs->GetCollect("GoodsIndex"),(const char*)(_bstr_t)rs->GetCollect("GoodsName"),(long)rs->GetCollect("GoodsLevel"),
	//	//					 (long)rs->GetCollect("SendNum")-(long)rs->GetCollect("ObtainedNum"),nWorldID,sendtime );

	//	//		logincnDes->Execute(sql, NULL, adCmdText);

	//	//	} // end try
	//	//	catch(_com_error &e) {
	//	//		sprintf( msg,"<ado err>\t���ݿ��쳣(for player Largess): %s", static_cast<const char*>(e.Description()));
	//	//		Log(msg);
	//	//	}
	//	//	rs->MoveNext();
	//	//}

	//	//rs->Close();
	//	//sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
	//	//Log(msg);
	//}
	//catch(_com_error &e) {
	//	sprintf( msg,"<ado err>\t���ݿ��쳣(GameDBLoginDB): %s\n", static_cast<const char*>(e.Description()) );
	//	Log(msg);
	//}


	////////////////////////////////////////////////////////////////////////////////////////
	// 
	// ���� loginDB
	// 
	////////////////////////////////////////////////////////////////////////////////////////
	
	//sprintf(cnstrDes,"Provider='sqloledb'; Data Source='%s'; Initial Catalog='%s'; User ID='%s'; Password='%s'",
	//	sqlLoginDes.Server, sqlLoginDes.Database, sqlLoginDes.Uid, sqlLoginDes.Pwd);
	//sprintf(cnstrSrc,"Provider='sqloledb'; Data Source='%s'; Initial Catalog='%s'; User ID='%s'; Password='%s'",
	//	sqlLoginSrc.Server, sqlLoginSrc.Database, sqlLoginSrc.Uid, sqlLoginSrc.Pwd);
	
#if 1  // �ѳɹ����ִ���
	sprintf(msg, "<info>\tLoginDBĿ�����ݿ�:%s %s\n", sqlLoginDes.Server.data(), sqlLoginDes.Database.data());
	Log(msg);
	sprintf(msg, "<info>\tLoginDBԴ���ݿ�:%s %s\n", sqlLoginSrc.Server.data(), sqlLoginSrc.Database.data());
	Log(msg);

	try {

		LogChanged(fAC, "<info>\t����LoginDB�������ʺſ�ʼ...\n");
		// ��ǰ��loginDB���ݿ⣬��ֻ�õ�����des����
		// �����ݿ�����
		hr = logincnDes->Open(logincnstrDes.data(), "", "", adConnectUnspecified);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		hr = logincnSrc->Open(logincnstrSrc.data(), "", "", adConnectUnspecified);
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		hr = rs.CreateInstance(__uuidof(Recordset));
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		hr = rs->Open("SELECT * FROM TBL_Member_Data ORDER BY serial_number",
			logincnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		

		char date[20] = "";
		_RecordsetPtr rs11;
		hr = rs11.CreateInstance(__uuidof(Recordset));
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		while( !rs->GetRSEOF() ) {
			
			char old_playeraccount[32]="";
			char new_playeraccount[32]="";

			sprintf(sql, "SELECT TOP 1 * FROM TBL_Member_Data WHERE AccountID='%s' ORDER BY serial_number", (const char *)(_bstr_t)(rs->GetCollect("AccountID")));
			hr = rs11->Open(sql, logincnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
			if(FAILED(hr)) _com_issue_error(E_FAIL);

			bEof = rs11->GetRSEOF();
			rs11->Close();
			
			// ��ȡ�ʺ�
			strcpy(old_playeraccount, (const char *)(_bstr_t)(rs->GetCollect("AccountID")) );

			if( !bEof ) { // Ŀ�����ݿ���ڸ��ʺ��ˡ������˺Ÿ���
				FixName(new_playeraccount, old_playeraccount);
				do {
					sprintf(sql, "SELECT TOP 1 * FROM TBL_Member_Data WHERE AccountID='%s' ORDER BY serial_number", new_playeraccount);
					hr = rs11->Open(sql, logincnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
					if(FAILED(hr)) _com_issue_error(E_FAIL);

					bEof = rs11->GetRSEOF();
					rs11->Close();
						
					if(!bEof) {
						new_playeraccount[0] = 0;
						FixName(new_playeraccount, old_playeraccount, true);
					}
				}
				while(!bEof);

				sprintf(msg,"<record>\t ԭʼ����˺�:[%s] ��������˺�:[%s]\n", old_playeraccount, new_playeraccount);
				Log(msg);
				sprintf(msg,"%s,%s\n", old_playeraccount, new_playeraccount);
				Log(msg);
			}
			else { // û���ظ����˺�
				strcpy(new_playeraccount, old_playeraccount);
			}
			changed_player_account[old_playeraccount] = new_playeraccount;
			
			// ���Ƹ��ʺ�
			_variant_t var = rs->GetCollect("iptTime");
			if( var.vt != VT_NULL ) {
				GetDateString(var, date);
			}
			auto datas  = (const char *)(_bstr_t)(rs->GetCollect("passwd"));
			_variant_t var1 = rs->GetCollect("cs_email");
			BSTR  data = var1.bstrVal;
			//auto data1 = wstring(data).;
			sprintf( sql, "INSERT INTO TBL_Member_Data(AccountID,passwd,iptTime,last_point, cs_email,cs_number) VALUES('%s','%s','%s','%d','%s','%s')",
				new_playeraccount,
				(const char *)(_bstr_t)(rs->GetCollect("passwd")),
				date,
				(long)(rs->GetCollect("last_point").intVal),
				(const char *)(_bstr_t)(rs->GetCollect("cs_email")),
				(const char *)(_bstr_t)(rs->GetCollect("cs_number")));
			logincnDes->Execute(sql, NULL, -1);

			rs->MoveNext();
		}
		rs->Close();

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);

	}
	catch(_com_error &e) {
		sprintf( msg,"<ado err>\t���ݿ��쳣(LoginDB): %s\n", static_cast<const char*>(e.Description()) );
		Log(msg);
	}
#endif // 0

	try {
		// �����ݿ�����
		hr = cnDes->Open(cnstrDes.data(), "", "", adConnectUnspecified);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		hr = cnSrc->Open(cnstrSrc.data(), "", "", adConnectUnspecified);
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		hr = rs.CreateInstance(__uuidof(Recordset));
		if(FAILED(hr)) _com_issue_error(E_FAIL);


		total_time = begin_time = clock();
		
		LogChanged(fPNC, "<info>\t����Ŀ�����ݿ�CSL_SETUP��ֵ\n");
		// ��ʼDes��ID_SETUP
		hr = rs->Open("SELECT TOP 1 * FROM CSL_SETUP", cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		if(rs->GetRSEOF()) {
			printf("fetal:[ Des ] no record in setup table\r\n");
			Log("<fetal>\t����Ŀ�����ݿ�SETUP���Ƿ񱻳�ʼ����\n");
			_com_issue_error(E_FAIL);
		}

		idDes.PlayerID = (long)rs->GetCollect("PlayerID");
		idDes.OrganizingID = (long)rs->GetCollect("OrganizingID");
		idDes.LeavewordID =  (long)rs->GetCollect("LeaveWordID");
		rs->Close();


		sprintf(msg, "<setup>\tPlayerID\t= %d\n", idDes.PlayerID);
		Log(msg);
		sprintf(msg, "<setup>\tOrganizingID\t= %d\n",idDes.OrganizingID);
		Log(msg);
		sprintf(msg, "<setup>\tLeavewordID\t= %d\n",idDes.LeavewordID);
		Log(msg);


		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);


		////////////////////////////////////////////////////////////////////////////////////////
		//
		// ����ѽ�ɫ��Src��Des  CSL_Countrys
		//
		////////////////////////////////////////////////////////////////////////////////////////
#if 1
		Log("<info>\t����������ݿ�ʼ...\n");

		hr = rs->Open("SELECT * FROM CSL_Countrys" ,
			cnSrc.GetInterfacePtr(),  adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		while( !rs->GetRSEOF() ) {
			_RecordsetPtr rs11;
			try {
				hr = rs11.CreateInstance(__uuidof(Recordset));
				if(FAILED(hr)) _com_issue_error(E_FAIL);

				sprintf(sql, "SELECT TOP 1 * FROM CSL_Countrys WHERE id=%d ORDER BY id",(long)rs->GetCollect("id"));
				hr = rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
				if(FAILED(hr)) _com_issue_error(E_FAIL);
				
				if( !rs11->GetRSEOF() )
				{
					sprintf(sql, "UPDATE CSL_Countrys SET power=%d, tech_lel=%d WHERE id=%d",
						(long)rs->GetCollect("power") + (long) rs11->GetCollect("power") ,
						(long)rs->GetCollect("tech_lel") + (long) rs11->GetCollect("tech_lel"),
						(long)rs11->GetCollect("id"));
				}
				else
				{
					sprintf(sql, "INSERT INTO CSL_Countrys(id, power, tech_lel) VALUES(%d,%d,%d)",
						(long)rs->GetCollect("id") ,
						(long)rs->GetCollect("power"),
						(long)rs->GetCollect("tech_lel"));
				}
				cnDes->Execute(sql, NULL, adCmdText);

				rs11->Close();
			}
			catch(_com_error &e) {
				sprintf( msg,"<ado err>\t���ݿ��쳣(while cournty): %s\n", static_cast<const char*>(e.Description()) );
				Log(msg);
			}
			rs11.Release();
			rs->MoveNext();
		}
		rs->Close();

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);
#endif // 0
		////////////////////////////////////////////////////////////////////////////////////////
		//
		// ����ѽ�ɫ��Src��Des  CSL_PALYER_BASE
		//
		////////////////////////////////////////////////////////////////////////////////////////

		Log("<info>\t������һ������ݿ�ʼ...\n");
		_RecordsetPtr player_rs;
		hr = player_rs.CreateInstance(__uuidof(Recordset));
		if (FAILED(hr)) _com_issue_error(E_FAIL);
		sprintf(sql, "SELECT max(ID) as id from CSL_PLAYER_BASE");
		player_rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		player_rs->GetRSEOF();
		idDes.PlayerID = player_rs->GetCollect("id");
		player_rs->Close();
				
		hr = rs->Open("SELECT * FROM CSL_PLAYER_BASE ORDER BY ID", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		while( !rs->GetRSEOF() ) {
			_RecordsetPtr rs11;

			ULONG old_playerid = 0;
			ULONG new_playerid = 0;
			char old_playeraccount[32]="";
			char new_playeraccount[32]="";
			char old_playername[32]="";
			char new_playername[32]="";

			try {

				hr = rs11.CreateInstance(__uuidof(Recordset));
				if(FAILED(hr)) _com_issue_error(E_FAIL);

				// ���������������ID
				old_playerid = (long)rs->GetCollect("ID");
				new_playerid = ++idDes.PlayerID;
				changed_player_id[old_playerid] = new_playerid;

				// ��ȡName
				strcpy(old_playername, (const char*)(_bstr_t)(rs->GetCollect("Name")) );

				sprintf(sql, "SELECT TOP 1 Name FROM CSL_PLAYER_BASE WHERE Name='%s' ORDER BY name", old_playername);
				rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
				bEof = rs11->GetRSEOF();
				rs11->Close();

				if(!bEof) { //���ظ���NAME
					FixName(new_playername, old_playername);
					do {
						sprintf(sql, "SELECT TOP 1 Name FROM CSL_PLAYER_BASE WHERE Name='%s' ORDER BY id", new_playername);
						hr = rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
						if(FAILED(hr)) _com_issue_error(E_FAIL);

						bEof = rs11->GetRSEOF();
						rs11->Close();
						
						if(!bEof) {
							new_playername[0] = 0;
							FixName(new_playername, old_playername, true);
						}
					}
					while(!bEof);
					sprintf(msg,"<record>\t ԭʼ�����:[%s] ���������:[%s]\n", old_playername, new_playername);
					Log(msg);
					LogChanged(fPNC,msg);
				}
				else { // û���ظ���NAME
					strcpy(new_playername, old_playername);
				}
				
				changed_player_name[old_playername] = new_playername;
				
				string temp;
				// ��ȡ�˺�
				strcpy( old_playeraccount, (const char *)(_bstr_t)rs->GetCollect("Account") );
				temp = changed_player_account[old_playeraccount];
				strcpy( new_playeraccount, temp.data() );

				// ɾ��ʱ��
				_variant_t var = rs->GetCollect("DelDate");
				char delDate[20] = "";
				if( var.vt != VT_NULL ) {
					GetDateString(var, delDate);
				}

				if( strcmp(delDate,"1900-1-1") ) {
				sprintf( sql,"INSERT INTO CSL_PLAYER_BASE \
							VALUES(%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s')",
							new_playerid,
							new_playername,
							new_playeraccount,
							(long)rs->GetCollect("Levels"),
							(long)rs->GetCollect("Occupation"),
							(long)rs->GetCollect("Sex"),
							(long)rs->GetCollect("Country"),

							(long)rs->GetCollect("HEAD"),

							(long)rs->GetCollect("HELM"),
							(long)rs->GetCollect("BODY"),
							(long)rs->GetCollect("GLOV"),
							(long)rs->GetCollect("BOOT"),
							(long)rs->GetCollect("WEAPON"),
							(long)rs->GetCollect("BACK"),

							(long)rs->GetCollect("Headgear"),
							(long)rs->GetCollect("Frock"),
							(long)rs->GetCollect("Wing"),
							(long)rs->GetCollect("Manteau"),
							(long)rs->GetCollect("Fairy"),

							(long)rs->GetCollect("HelmLevel"),
							(long)rs->GetCollect("BodyLevel"),
							(long)rs->GetCollect("GlovLevel"),
							(long)rs->GetCollect("BootLevel"),
							(long)rs->GetCollect("WeaponLevel"),
							(long)rs->GetCollect("BackLevel"),
							
							(long)rs->GetCollect("HeadgearLevel"),
							(long)rs->GetCollect("FrockLevel"),
							(long)rs->GetCollect("WingLevel"),
							(long)rs->GetCollect("ManteauLevel"),
							(long)rs->GetCollect("FairyLevel"),

							(long)rs->GetCollect("Region"),
							delDate );
				}
				else {
				sprintf( sql,"INSERT INTO CSL_PLAYER_BASE \
							VALUES(%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
							new_playerid,
							new_playername,
							new_playeraccount,
							(long)rs->GetCollect("Levels"),
							(long)rs->GetCollect("Occupation"),
							(long)rs->GetCollect("Sex"),
							(long)rs->GetCollect("Country"),
							(long)rs->GetCollect("HEAD"),
							(long)rs->GetCollect("HELM"),
							(long)rs->GetCollect("BODY"),
							(long)rs->GetCollect("GLOV"),
							(long)rs->GetCollect("BOOT"),
							(long)rs->GetCollect("WEAPON"),
							(long)rs->GetCollect("BACK"),

							(long)rs->GetCollect("Headgear"),
							(long)rs->GetCollect("Frock"),
							(long)rs->GetCollect("Wing"),
							(long)rs->GetCollect("Manteau"),
							(long)rs->GetCollect("Fairy"),

							(long)rs->GetCollect("HelmLevel"),
							(long)rs->GetCollect("BodyLevel"),
							(long)rs->GetCollect("GlovLevel"),
							(long)rs->GetCollect("BootLevel"),
							(long)rs->GetCollect("WeaponLevel"),
							(long)rs->GetCollect("BackLevel"),
							
							(long)rs->GetCollect("HeadgearLevel"),
							(long)rs->GetCollect("FrockLevel"),
							(long)rs->GetCollect("WingLevel"),
							(long)rs->GetCollect("ManteauLevel"),
							(long)rs->GetCollect("FairyLevel"),

							(long)rs->GetCollect("Region") );
				}


				cnDes->Execute(sql, NULL, adCmdText);

			} // end try
			catch(_com_error &e) {
				sprintf( msg,"<ado err>\t���ݿ��쳣(for player base): %s - ���:��[%d] ��[%d] ��[%s] ��[%s]\n", 
							 static_cast<const char*>(e.Description()), 
							 old_playerid, new_playerid,old_playername, new_playername);
				Log(msg);
			}

			rs11.Release();
			rs->MoveNext();
		} // end while
		rs->Close();

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);



		////////////////////////////////////////////////////////////////////////////////////////
		//
		// ����ѽ�ɫ��Src��Des  CSL_PALYER_ABILITY
		//
		////////////////////////////////////////////////////////////////////////////////////////
		Log("<info>\t���������ϸ���ݿ�ʼ...\n");
		hr = rs->Open( "SELECT * FROM CSL_PLAYER_ABILITY ORDER BY ID", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		while(!rs->GetRSEOF()) {

			_RecordsetPtr rs11;

			ULONG old_playerid = 0;
			ULONG new_playerid = 0;
			char old_playeraccount[32]="";
			char new_playeraccount[32]="";
			char old_playername[32]="";
			char new_playername[32]="";
			try {

				hr = rs11.CreateInstance(__uuidof(Recordset));
				if(FAILED(hr)) _com_issue_error(E_FAIL);

				// ��ȡID
				old_playerid = (long)rs->GetCollect("ID");
				new_playerid = changed_player_id[old_playerid];

				// ������ability�
				if(!new_playerid) {
					sprintf(msg,"<warrning>\t��ɫID[%d]������Player_ability��\n",old_playerid);
					Log(msg);
					rs->MoveNext();
					continue;
				}
				
				string temp;
				// ��ȡ�˺�
				strcpy( old_playeraccount, (const char*)(bstr_t)rs->GetCollect("Account") );
				temp = changed_player_account[old_playeraccount];
				strcpy( new_playeraccount, temp.data() );
				// ��ȡNAME
				strcpy( old_playername, (const char *)(_bstr_t)rs->GetCollect("Name") );
				temp = changed_player_name[old_playername];
				strcpy( new_playername, temp.data() );

				_variant_t var = rs->GetCollect("SaveTime");
				char saveDate[20] = "";
				if( var.vt != VT_NULL ) {
					GetDateString(var, saveDate);
				}

				var = rs->GetCollect("depotpassword");
				char depostpassword[20] = "";
				if( var.vt != VT_NULL ) {
					strcpy(depostpassword,(const char*)(_bstr_t)rs->GetCollect("depotpassword"));
				}
				//[UnionID] [HotHit] [RemainPoint] [HotKey] [Pk_Country] [ListGoods][ListSkill][ListState][ListFriendName]
				//[VariableList] [Tribe][Exploit][Kudos][FairyEnabled][FosterNum][HatcherNum][Mode][FreezeProcessed][FetchPower][MaxFetchPower]
				//[DaysHonorElimilateNum] [WeeksHonorElimilateNum][MonthsHonorElimilateNum][TotalHonorElimilateNum] [RankOfNobilityID]
				sprintf(sql, "INSERT INTO CSL_PLAYER_ABILITY(ID,Name,SaveTime,RegionID,PosX,PosY,Dir,Account,Title,Levels,Exp,\
							 HeadPic,FacePic,Occupation,Sex,SpouseID,MurdererTime,PkCount,KillCount,HitTopLog,\
							 LoanMax,Loan,LoanTime,Pk_Normal,Pk_Team,Pk_Union,Pk_Badman,\
							 Yp,Hp,Mp,Rp,BaseMaxHp,BaseMaxMp,BaseMaxYp,BaseMaxRp,BaseStr,BaseDex,\
							 BaseCon,BaseInt,BaseMinAtk,BaseMaxAtk,BaseHit,BaseBurden,BaseCCH,BaseDef,BaseDodge,\
							 BaseAtcSpeed,BaseElementResistant,BaseHpRecoverSpeed,BaseMpRecoverSpeed,silence, \
							 UnionID,RemainPoint,Tribe,BaseVigour,BaseMaxVigour,BaseCredit,DisplayHeadPiece, \
							 country, contribute, IsCharged, QuestTimeBegin, QuestTimeLimit, Quest, BaseEnergy, BaseMaxEnergy, \
                             Exploit, Kudos, FairyEnabled, FosterNum, HatcherNum, Mode, FreezeProcessed, FetchPower, \
                             MaxFetchPower, DaysHonorElimilateNum, WeeksHonorElimilateNum, MonthsHonorElimilateNum, TotalHonorElimilateNum, RankOfNobilityID, AppellationID, LastExitFactionTime, \
							 BattleFairyEnabled, dwAuctionSpace, GodsBattleFaction, SZL, dwExalt, dwNimbus,\
					         HotHit, Pk_Country, depotpassword) \
							 VALUES(%d,'%s','%s',%d,%f,%f,%d,'%s','temptitle',%d,%u, \
							 %d,%d,%d,%d,%d,%d,%d,%d,%d, \
							 %d,%d,%d,%d,%d,%d,%d,\
							 %d,%d,%d,%d,%d,%d,%d,%d,%d,%d, \
							 %d,%d,%d,%d,%d,%d,%d,%d,%d, \
							 %d,%d,%d,%d,%d, \
							 0,%d,0,%d,%d,%d,%d, \
							 %d,%d,%d,%d,%d,%d,%d,%d, \
                             %d,%d,%d,%d,%d,%d,%d,%d, \
                             %d,%d,%d,%d,%d,%d,%d,%d, \
                             %d,%d,%d,%d,%d,%d, \
							 %d,%d,'%s')",
							 new_playerid,new_playername, saveDate, (long)rs->GetCollect("RegionID"),(float)rs->GetCollect("PosX"),(float)rs->GetCollect("PosY"),(long)rs->GetCollect("Dir"), new_playeraccount,(long)rs->GetCollect("Levels"),(unsigned long)rs->GetCollect("Exp"),
							 (long)rs->GetCollect("HeadPic"),(long)rs->GetCollect("FacePic"),(long)rs->GetCollect("Occupation"),(long)rs->GetCollect("Sex"),(long)rs->GetCollect("SpouseID"),(long)rs->GetCollect("MurdererTime"),(long)rs->GetCollect("PkCount"),(long)rs->GetCollect("KillCount"),(long)rs->GetCollect("HitTopLog"),
							 (long)rs->GetCollect("LoanMax"),(long)rs->GetCollect("Loan"),(unsigned long)rs->GetCollect("LoanTime"),(long)rs->GetCollect("Pk_Normal"),(long)rs->GetCollect("Pk_Team"),(long)rs->GetCollect("Pk_Union"),(long)rs->GetCollect("Pk_Badman"),
							 (long)rs->GetCollect("Yp"),(long)rs->GetCollect("Hp"),(long)rs->GetCollect("Mp"),(long)rs->GetCollect("Rp"),(long)rs->GetCollect("BaseMaxHp"),(long)rs->GetCollect("BaseMaxMp"),(long)rs->GetCollect("BaseMaxYp"),(long)rs->GetCollect("BaseMaxRp"),(long)rs->GetCollect("BaseStr"),(long)rs->GetCollect("BaseDex"),
							 (long)rs->GetCollect("BaseCon"),(long)rs->GetCollect("BaseInt"),(long)rs->GetCollect("BaseMinAtk"),(long)rs->GetCollect("BaseMaxAtk"),(long)rs->GetCollect("BaseHit"),(long)rs->GetCollect("BaseBurden"),(long)rs->GetCollect("BaseCCH"),(long)rs->GetCollect("BaseDef"),(long)rs->GetCollect("BaseDodge"),
							 (long)rs->GetCollect("BaseAtcSpeed"),(long)rs->GetCollect("BaseElementResistant"),
							 (long)rs->GetCollect("BaseHpRecoverSpeed"),(long)rs->GetCollect("BaseMpRecoverSpeed"),(long)rs->GetCollect("silence"),
							 (long)rs->GetCollect("RemainPoint"),(long)rs->GetCollect("BaseVigour"),(long)rs->GetCollect("BaseMaxVigour"),(long)rs->GetCollect("BaseCredit"),(long)rs->GetCollect("DisplayHeadPiece"),
							 (long)rs->GetCollect("country"),(long)rs->GetCollect("contribute"),(long)rs->GetCollect("IsCharged"), (unsigned long)rs->GetCollect("QuestTimeBegin"),(long)rs->GetCollect("QuestTimeLimit"),(long)rs->GetCollect("Quest"),(long)rs->GetCollect("BaseEnergy"),(long)rs->GetCollect("BaseMaxEnergy"),
			                 (long)rs->GetCollect("Exploit"), (long)rs->GetCollect("Kudos"), (long)rs->GetCollect("FairyEnabled"), (unsigned long)rs->GetCollect("FosterNum"), (long)rs->GetCollect("HatcherNum"), (long)rs->GetCollect("Mode"), (long)rs->GetCollect("FreezeProcessed"), (long)rs->GetCollect("FetchPower"),
					         (long)rs->GetCollect("MaxFetchPower"), (long)rs->GetCollect("DaysHonorElimilateNum"), (long)rs->GetCollect("WeeksHonorElimilateNum"), (unsigned long)rs->GetCollect("MonthsHonorElimilateNum"), (long)rs->GetCollect("TotalHonorElimilateNum"), (long)rs->GetCollect("RankOfNobilityID"), (long)rs->GetCollect("AppellationID"), (long)rs->GetCollect("LastExitFactionTime"),
					         (long)rs->GetCollect("BattleFairyEnabled"), (long)rs->GetCollect("dwAuctionSpace"), (long)rs->GetCollect("GodsBattleFaction"), (long)rs->GetCollect("SZL"), (long)rs->GetCollect("dwExalt"), (long)rs->GetCollect("dwNimbus"),
					         (long)rs->GetCollect("HotHit"),(long)rs->GetCollect("Pk_Country"), depostpassword);


				cnDes->Execute(sql, NULL, adCmdText);

				sprintf(sql, "SELECT ciqing,medon,HotKey,ListGoods,ListSkill,ListState,ListFriendName,VariableList FROM CSL_PLAYER_ABILITY WHERE ID=%d ORDER BY ID", new_playerid);
				rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);
				
				if (!CopyChunk(rs11, rs, "ciqing")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}

				if (!CopyChunk(rs11, rs, "medon")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}

				if(!CopyChunk(rs11, rs, "HotKey")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}
				if(!CopyChunk(rs11, rs, "ListGoods")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}
				if(!CopyChunk(rs11, rs, "ListSkill")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}
				if(!CopyChunk(rs11, rs, "ListState")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}
				if(!CopyChunk(rs11, rs, "ListFriendName")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}

				if(!CopyChunk(rs11, rs, "VariableList")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}
			
				rs11->Update();
				rs11->Close();
			
			} // end try
			catch(_com_error &e) {
				sprintf( msg,"<ado err>\t���ݿ��쳣(for player ability): %s - ���:��[%d] ��[%d] ��[%s] ��[%s]\n", 
							 static_cast<const char*>(e.Description()), 
							 old_playerid, new_playerid,old_playername, new_playername);
				Log(msg);
			}
			rs11.Release();
			rs->MoveNext();
		} // end while
		rs->Close();

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);


		
		////////////////////////////////////////////////////////////////////////////////////////
		//
		// �������ҵ����� csl_player_quest_ex
		//
		////////////////////////////////////////////////////////////////////////////////////////
		Log("<info>\t�����������ʼ...\n");
		hr = rs->Open("SELECT * FROM CSL_PLAYER_QUEST_EX ORDER BY PlayerID", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		while(!rs->GetRSEOF()) {
			
			_RecordsetPtr rs11;

			ULONG old_playerid = 0;
			ULONG new_playerid = 0;
			try {

				hr = rs11.CreateInstance(__uuidof(Recordset));
				if(FAILED(hr)) _com_issue_error(E_FAIL);

				// ��ȡID
				old_playerid = (long)rs->GetCollect("PlayerID");
				new_playerid = changed_player_id[old_playerid];

				// ������Player_quest_ex�
				if(!new_playerid) {
					sprintf(msg,"<warrning>\t��ɫID[%d]������CSL_PLAYER_QUEST_EX��\n",old_playerid);
					Log(msg);
					rs->MoveNext();
					continue;
				}

				sprintf(sql, "INSERT INTO CSL_PLAYER_QUEST_EX(PlayerID) VALUES(%d)", new_playerid);
				cnDes->Execute(sql, NULL, adCmdText);

				sprintf(sql, "SELECT QuestData FROM CSL_PLAYER_QUEST_EX WHERE PlayerID=%d ORDER BY PlayerID", new_playerid);
				rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);

				if(!CopyChunk(rs11, rs, "QuestData")) {
					rs11->Update();
					rs11->Close();
					_com_issue_error(E_FAIL);
				}

				rs11->Update();
				rs11->Close();
			
			} // end try
			catch(_com_error &e) {
				sprintf( msg,"<ado err>\t���ݿ��쳣(for player quest_ex): %s - ���:��[%d] ��[%d]\n", 
							 static_cast<const char*>(e.Description()), old_playerid, new_playerid);
				Log(msg);
			}
			rs11.Release();
			rs->MoveNext();
		} // end while
		rs->Close();

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);

		////////////////////////////////////////////////////////////////////////////////////////
		//
		// �������ҵ���Ʒ player_goods
		//
		////////////////////////////////////////////////////////////////////////////////////////
		Log("<info>\t���������Ʒ���ݿ�ʼ...\n");
		hr = rs->Open("SELECT * FROM player_goods ORDER BY playerID", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		while(!rs->GetRSEOF()) { //�޼�¼����û��Ʒ
	
			ULONG old_playerid=0, new_playerid=0;

			// ������Ʒ Index
			long lGoodsIndex = (long)rs->GetCollect("goodsIndex");
			if(lGoodsIndex >= 403029110 && lGoodsIndex <= 403029118)
			{
				lGoodsIndex -= 40;
			}
			
			try {
				// ��ȡID
				old_playerid = (long)rs->GetCollect("playerID");
				new_playerid = changed_player_id[old_playerid];

				if(old_playerid != 0 && new_playerid == 0) { // ��Ϊ0����������ϵ���Ʒ����Ҫ�����Ƿ���Ұ��
					sprintf(msg,"<warrning>\t��ɫID[%d]���ݲ���(playerID)���޷�����.��player_goods��\n",old_playerid);
					Log(msg);
					rs->MoveNext();
					continue;
				}

				// 
				_RecordsetPtr goods_rs;
				hr = goods_rs.CreateInstance(__uuidof(Recordset));
				if (FAILED(hr)) _com_issue_error(E_FAIL);
				sprintf(sql, "SELECT TOP 1 1 from player_goods where id = '%s'", (const char*)(_bstr_t)rs->GetCollect("ID"));
				goods_rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
				if(!goods_rs->GetRSEOF())
				{
					goods_rs->Close();
					rs->MoveNext();
					continue;
				}
				goods_rs->Close();

				sprintf(sql, "INSERT INTO player_goods \
							 VALUES('%s','%s',%d,%d,'%s',%d,%d,%d,%d)",
							 (const char*)(_bstr_t)rs->GetCollect("ID"),
							 (const char*)(_bstr_t)rs->GetCollect("goodsID"), lGoodsIndex,
							 new_playerid,(const char*)(_bstr_t)rs->GetCollect("name"),
							 (long)rs->GetCollect("price"),(long)rs->GetCollect("amount"),
							 (long)rs->GetCollect("place"),(long)rs->GetCollect("position"));

				cnDes->Execute(sql, NULL, adCmdText);

			} // end try
			catch(_com_error &e) {
				sprintf( msg,"<ado err>\t���ݿ��쳣(for player goods): %s\n", static_cast<const char*>(e.Description()) );
				Log(msg);
			}

			rs->MoveNext();
		} // end whileѭ����Ʒ
		rs->Close();
		// �ý�ɫ�µ���Ʒ�ϲ���

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);

		////////////////////////////////////////////////////////////////////////////////////////
		//
		// ��Ʒ����
		//
		////////////////////////////////////////////////////////////////////////////////////////
		Log("<info>\t���������Ʒ�������ݿ�ʼ...\n");
		hr = rs->Open("SELECT * FROM extend_properties ORDER BY id", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		while(!rs->GetRSEOF()) { //�޼�¼����û����
			try
			{
				// ��������Ʒ ���ָ������� ����
				long lType = (long)rs->GetCollect("type");
				long lmodifierValue1 = (long)rs->GetCollect("modifierValue1");
				long lmodifierValue2 = (long)rs->GetCollect("modifierValue2");
#if 0

				switch(lType)
				{
				case 0x0E:
				case 0x0F:
				case 0x10:
				case 0x11:
				case 0x13:
				case 0x14:
				case 0x17:
				case 0x1B:
				case 0x1C:
				case 0x1D:
				case 0x1E:
				case 0x1F:
				case 0x20:
				case 0x42:
				case 0x5D:
				case 0x5E:
				case 0x5F:
				case 0x60:
					lmodifierValue1 = 0;
					lmodifierValue2 = 0;
					break;
				default:
					break;
				}
#endif

				_RecordsetPtr goods_rs;
				hr = goods_rs.CreateInstance(__uuidof(Recordset));
				if (FAILED(hr)) _com_issue_error(E_FAIL);
				sprintf(sql, "SELECT TOP 1 * from extend_properties where type = %d and id = '%s'", (long)rs->GetCollect("TYPE"),(const char*)(_bstr_t)rs->GetCollect("ID"));
				goods_rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
				if (!goods_rs->GetRSEOF())
				{
					goods_rs->Close();
					rs->MoveNext();
					continue;
				}
				goods_rs->Close();

				sprintf(sql, "INSERT INTO extend_properties(type,modifierValue1,modifierValue2,ID) \
							 VALUES(%d,%d,%d,'%s')",
							 lType,
							 lmodifierValue1,
							 lmodifierValue2,
							 (const char*)(_bstr_t)rs->GetCollect("ID") );

				cnDes->Execute(sql, NULL, adCmdText);
				
			} // end try
			catch(_com_error &e) {
				sprintf( msg,"<ado err>\t���ݿ��쳣(for player goods propertis):%s\n", static_cast<const char*>(e.Description()) );
				Log(msg);
			}
			rs->MoveNext();
		}
		rs->Close();


		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);


		////////////////////////////////////////////////////////////////////////////////////////
		//
		// ������
		// 
		////////////////////////////////////////////////////////////////////////////////////////
		Log("<info>\t���������ݿ�ʼ...\n");
		_RecordsetPtr faction_rs;
		hr = faction_rs.CreateInstance(__uuidof(Recordset));
		if (FAILED(hr)) _com_issue_error(E_FAIL);
		sprintf(sql, "SELECT max(ID) as id from CSL_FACTION_BaseProperty");
		faction_rs->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		faction_rs->GetRSEOF();
		idDes.OrganizingID = faction_rs->GetCollect("id");
		faction_rs->Close();

		// ���ȴ��� CSL_FACTION_BaseProperty
		hr = rs->Open("SELECT * FROM CSL_FACTION_BaseProperty", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		Log("<info>\tfaction_baseproperty��ʼ...\n");
		while( !rs->GetRSEOF() ) {
			_RecordsetPtr rs12;
			ULONG old_factionid = 0;
	
			_RecordsetPtr rs11;
			ULONG new_factionid = 0;
			char old_factionname[32] = "";
			char new_factionname[32] = "";
			ULONG old_playerid = 0;
			ULONG new_playerid = 0;

			try {
				hr = rs11.CreateInstance(__uuidof(Recordset));
				if(FAILED(hr)) _com_issue_error(E_FAIL);

				hr = rs12.CreateInstance(__uuidof(Recordset));
				if(FAILED(hr)) _com_issue_error(E_FAIL);
				// ��ȡ�ղŸ��º�����ID
				old_playerid = (long)rs->GetCollect("MasterID");
				new_playerid = changed_player_id[old_playerid];

				if(!new_playerid) {
					sprintf(msg,"<warrning>\t��ɫID[%d]���ݲ��㣬�޷�����.��CSL_FACTION_BaseProperty��\n",old_playerid);
					Log(msg);
					rs->MoveNext();
					continue;
				}


				// ��ȡID��NMAE
				old_factionid = (long)rs->GetCollect("ID");
				strcpy( old_factionname, (const char *)(_bstr_t)rs->GetCollect("Name") );
				// ��ȡ�µ�ID��NAME
				new_factionid = ++idDes.OrganizingID;

				sprintf(sql, "SELECT TOP 1 Name FROM CSL_FACTION_BaseProperty WHERE Name='%s'",old_factionname);	
				hr = rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
				if(FAILED(hr)) _com_issue_error(E_FAIL);

				bEof = rs11->GetRSEOF();
				rs11->Close();

				if( !bEof ) { //���ִ���
					FixName(new_factionname, old_factionname);

					do {
						sprintf(sql, "SELECT TOP 1 Name FROM CSL_FACTION_BaseProperty WHERE Name='%s'", new_factionname);
						hr = rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
						if(FAILED(hr)) _com_issue_error(E_FAIL);

						bEof = rs11->GetRSEOF();
						rs11->Close();

						if(!bEof) {
							new_factionname[0] = 0;
							FixName(new_factionname, old_factionname, true);
						}
					}
					while(!bEof);

					sprintf(msg, "<record>\tԭʼ�����:[%s] ���������:[%s]\n", old_factionname, new_factionname);
					Log(msg);
				}
				else { // ���ظ�����ֱ���� 
					strcpy(new_factionname, old_factionname);
				}
				

				char date[20] = "";
				_variant_t var = rs->GetCollect("EstablishedTime");
				if( var.vt != VT_NULL ) {
					GetDateString(var, date);
				}


				sprintf(sql, "INSERT INTO \
							 CSL_FACTION_BaseProperty(ID,Name,MasterID,Levels,Experience,MemberNums,\
							 VillageWarVictorCounts,OffenseVictorCounts,DefenceVictorCounts,bPermit,\
							 lPro1,lPro2,DelRemainTime,EstablishedTime,country, UnionID, GoodsWarCount, GoodsWarLastTime, Force, LastestGoodsWarWiner, LastExitUnionTime) \
							 VALUES(%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s',%d, %d, %d, '%s', %d, %d, '%s')",
							 new_factionid,new_factionname,new_playerid,(long)rs->GetCollect("Levels"),
							 (long)rs->GetCollect("Experience"),							 
							 (long)rs->GetCollect("MemberNums"),(long)rs->GetCollect("VillageWarVictorCounts"),
							 (long)rs->GetCollect("OffenseVictorCounts"),(long)rs->GetCollect("DefenceVictorCounts"),
							 (long)rs->GetCollect("bPermit"),(long)rs->GetCollect("lPro1"),
							 (long)rs->GetCollect("lPro2"),(long)rs->GetCollect("DelRemainTime"), date ,
							 (long)rs->GetCollect("country"), (long)rs->GetCollect("UnionID"),
							 (long)rs->GetCollect("GoodsWarCount"), (const char*)(_bstr_t)rs->GetCollect("GoodsWarLastTime"),
							 (long)rs->GetCollect("Force"), (unsigned long)rs->GetCollect("LastestGoodsWarWiner"), 
							 (const char*)(_bstr_t)rs->GetCollect("LastExitUnionTime")
					);

				Log(sql);
				cnDes->Execute(sql, NULL, -1);


				////////////////////////////////////////////////////////////////////////////////////////
				//
				// ����ý�ɫ�İ�� CSL_FACTION_Ability
				//
				////////////////////////////////////////////////////////////////////////////////////////

				sprintf(sql, "SELECT * FROM CSL_FACTION_Ability WHERE FactionID=%d", old_factionid);
				hr = rs11->Open(sql, cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
				if(FAILED(hr)) _com_issue_error(E_FAIL);

				if( !rs11->GetRSEOF() ) { // �������ĳ��İ������ϲ����İ������

					char date[20] = "";
					_variant_t var = rs11->GetCollect("LastUploadIconDataTime");
					if( var.vt != VT_NULL ) {
						GetDateString(var, date);
					}

					sprintf(sql, "INSERT INTO \
								 CSL_FACTION_Ability(FactionID,LastUploadIconDataTime,EnemyOrganizingID) \
								 VALUES(%d,'%s',%d)",
								 new_factionid,date,0 );

					cnDes->Execute(sql, NULL, -1);


					sprintf(sql, "SELECT * FROM CSL_FACTION_Ability WHERE FactionID=%d", new_factionid);
					rs12->Open(sql, cnDes.GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);

					if(!CopyChunk(rs12, rs11, "Pronounce")) {
						rs11->Close();
						rs12->Update();
						rs12->Close();
						_com_issue_error(E_FAIL);
					}

					//if(!CopyChunk(rs12, rs11, "OwnedCities")) {
					//	rs11->Close();
					//	rs12->Update();
					//	rs12->Close();
					//	_com_issue_error(E_FAIL);
					//}

					if(!CopyChunk(rs12, rs11, "IconData")) {
						rs11->Close();
						rs12->Update();
						rs12->Close();
						_com_issue_error(E_FAIL);
					}

					rs11->Close();
					rs12->Update();
					rs12->Close();
				}
				else {
					sprintf(msg, "<error>\t ���[%d][%s]��ability�����޶�Ӧ��¼\n", old_factionid, old_factionname);
					Log(msg);
				}


				// ��ӵ����ı�
				changed_faction_id[old_factionid] = new_factionid;
			}
			catch(_com_error &e) {

				sprintf( msg,"<ado err>\t���ݿ��쳣(while faction): %s\n", static_cast<const char*>(e.Description()) );
				Log(msg);
			}
			rs->MoveNext();
			rs11.Release();
			rs12.Release();

		} // end while faction
		rs->Close();

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);


		////////////////////////////////////////////////////////////////////////////////////////
		// ����changed_player_id �� changed_faction_id
		// �ϲ� CSL_FACTION_Members
		//
		////////////////////////////////////////////////////////////////////////////////////////

		Log("<info>\t�������Ա��ʼ...\n");
		hr = rs->Open("SELECT * FROM CSL_FACTION_Members ORDER BY FactionID", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		
		Log("<info>\tfaction_member��ʼ...\n");

		while( !rs->GetRSEOF() ) { // �������ĳ��ĳ�Ա���ϲ����İﵽDES
			ULONG old_factionid = 0;
			ULONG new_factionid = 0;
			ULONG old_playerid = 0;
			ULONG new_playerid = 0;
			char date[20] = "";
			try {
				
				//

				old_factionid = (long)rs->GetCollect("FactionID");
				old_playerid = (long)rs->GetCollect("PlayerID");

				
				new_factionid = changed_faction_id[old_factionid];
				new_playerid = changed_player_id[old_playerid];


				
				if(!new_playerid) {
					sprintf(msg,"<warrning>\t��ɫID[%d]���ݲ��㣬�޷�����.��CSL_FACTION_Members��\n",old_playerid);
					Log(msg);
					rs->MoveNext();
					continue;
				}


				_variant_t var = rs->GetCollect("LastOnlineTime");
				if( var.vt != VT_NULL ) {
					GetDateString(var, date);
				}

				sprintf(sql, 
					"INSERT INTO CSL_FACTION_Members \
					VALUES(%d,%d,%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s')",
					new_factionid,new_playerid,(long)rs->GetCollect("MemberLvl"),(const char*)(_bstr_t)rs->GetCollect("Title"),
					(long)rs->GetCollect("bControbute"),(long)rs->GetCollect("PV_Disband"),(long)rs->GetCollect("PV_Exit"),
					(long)rs->GetCollect("PV_DubJobLvl"),(long)rs->GetCollect("PV_ConMem"),(long)rs->GetCollect("PV_FireOut"),
					(long)rs->GetCollect("PV_Pronounce"),(long)rs->GetCollect("PV_LeaveWord"),(long)rs->GetCollect("PV_EditLeaveWord"),
					(long)rs->GetCollect("PV_ObtainTax"),(long)rs->GetCollect("PV_OperCityGate"),(long)rs->GetCollect("PV_EndueROR"),
					date);

				cnDes->Execute(sql, NULL, -1);

			}
			catch(_com_error &e) {

				sprintf( msg,"<ado err>\t���ݿ��쳣(while membsers faction): %s\n", static_cast<const char*>(e.Description()) );
				Log(msg);
			}
			rs->MoveNext();
		} // end while CSL_FACTION_Members
		rs->Close();
		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);


		////////////////////////////////////////////////////////////////////////////////////////
		// 
		// �������� CSL_UNION_BaseProperty
		// 
		////////////////////////////////////////////////////////////////////////////////////////

		Log("<info>\t�����������ݿ�ʼ...\n");
		hr = rs->Open("SELECT * FROM CSL_UNION_BaseProperty ORDER BY ID", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);
		while( !rs->GetRSEOF() ) {
		
			_RecordsetPtr rs11;

			ULONG old_factionid = 0;
			ULONG new_factionid = 0;
			ULONG old_unionid = 0;
			ULONG new_unionid = 0;
			char old_unionname[32] = "";
			char new_unionname[32] = "";

			try {
				hr = rs11.CreateInstance(__uuidof(Recordset));
				if(FAILED(hr)) _com_issue_error(E_FAIL);


				// ��ȡID��NMAE
				old_unionid = (long)rs->GetCollect("ID");
				strcpy( old_unionname, (const char *)(_bstr_t)rs->GetCollect("Name") );
				// ��ȡ�µ�ID��NAME
				new_unionid = ++idDes.OrganizingID;

				sprintf(sql, "SELECT TOP 1 Name FROM CSL_UNION_BaseProperty WHERE Name='%s'",old_unionname);	
				hr = rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
				if(FAILED(hr)) _com_issue_error(E_FAIL);
				bEof = rs11->GetRSEOF();
				rs11->Close();

				if( !bEof ) { //���ִ���
					FixName(new_unionname, old_unionname);

					do {
						sprintf(sql, "SELECT TOP 1 Name FROM CSL_UNION_BaseProperty WHERE Name='%s'", new_unionname);
						hr = rs11->Open(sql, cnDes.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
						if(FAILED(hr)) _com_issue_error(E_FAIL);

						bEof = rs11->GetRSEOF();
						rs11->Close();

						if(!bEof) {
							new_unionname[0] = 0;
							FixName(new_unionname, old_unionname ,true);
						}
					}
					while(!bEof);

					sprintf(msg, "<record>\tԭʼ������:[%s] ����������:[%s]\n", old_unionname, new_unionname);
					Log(msg);
				}
				else { // ���ظ�����ֱ���� 
					strcpy(new_unionname, old_unionname);
				}
				

				// ��ȡ�ղŸ��º�����ID
				old_factionid = (long)rs->GetCollect("MasterID");
				new_factionid = changed_faction_id[old_factionid];


				sprintf(sql, "INSERT INTO \
							 CSL_UNION_BaseProperty(ID,Name,MasterID) \
							 VALUES(%d,'%s',%d)",
							 new_unionid,new_unionname,new_factionid);
				cnDes->Execute(sql, NULL, -1);

				// ����������

				// ��ӵ����ı�
				changed_faction_id[old_unionid] = new_unionid; // ��������ID������ͬ�����Ǵ�orgID�����ɵ�
			}
			catch(_com_error &e) {
				sprintf( msg,"<ado err>\t���ݿ��쳣(while union): %s\n", static_cast<const char*>(e.Description()) );
				Log(msg);
			}
			rs->MoveNext();
			rs11.Release();

		} // end while union
		rs->Close();

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);


		////////////////////////////////////////////////////////////////////////////////////////
		// 
		// �������� CSL_UNION_Members
		// 
		////////////////////////////////////////////////////////////////////////////////////////
		Log("<info>\t�������˳�Ա��ʼ...\n");
		hr = rs->Open("SELECT * FROM CSL_UNION_Members ORDER BY UnionID", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		if(FAILED(hr)) _com_issue_error(E_FAIL);

		while( !rs->GetRSEOF() ) { // �������ĳ��ĳ�Ա���ϲ����İﵽDES
			ULONG old_unionid = 0;
			ULONG new_unionid = 0;
			ULONG old_factionid = 0;
			ULONG new_factionid = 0;

			char date[20] = "";
			try {
				old_unionid = (long)rs->GetCollect("UnionID");
				old_factionid = (long)rs->GetCollect("FactionID");
				
				_variant_t var = rs->GetCollect("LastOnlineTime");
				if( var.vt != VT_NULL ) {
					GetDateString(var, date);
				}

				new_unionid = changed_faction_id[old_unionid];
				new_factionid = changed_faction_id[old_factionid];

				sprintf(sql, 
					"INSERT INTO CSL_UNION_Members \
					VALUES(%d,%d,%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s')",
					new_unionid,new_factionid,(long)rs->GetCollect("MemberLvl"),(const char*)(_bstr_t)rs->GetCollect("Title"),
					(long)rs->GetCollect("bControbute"),(long)rs->GetCollect("PV_Disband"),(long)rs->GetCollect("PV_Exit"),
					(long)rs->GetCollect("PV_DubJobLvl"),(long)rs->GetCollect("PV_ConMem"),(long)rs->GetCollect("PV_FireOut"),
					(long)rs->GetCollect("PV_Pronounce"),(long)rs->GetCollect("PV_LeaveWord"),(long)rs->GetCollect("PV_EditLeaveWord"),
					(long)rs->GetCollect("PV_ObtainTax"),(long)rs->GetCollect("PV_OperCityGate"),(long)rs->GetCollect("PV_EndueROR"),
					date);

				cnDes->Execute(sql, NULL, -1);
				// ��������
			}
			catch(_com_error &e) {
				sprintf( msg,"<ado err>\t���ݿ��쳣(while members union): %s\n", static_cast<const char*>(e.Description()) );
				Log(msg);
			}
			rs->MoveNext();
		} // end while CSL_FACTION_Members
		rs->Close();
		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		Log(msg);


		////////////////////////////////////////////////////////////////////////////////////////
		// 
		// ���� CSL_setup
		// 
		////////////////////////////////////////////////////////////////////////////////////////
		Log("<info>\t����CSL_SETUP\n");
		// ���д��SETUP��
		sprintf(sql,
			"UPDATE CSL_SETUP SET PlayerID=%d,OrganizingID=%d,LeaveWordID=%d",
			idDes.PlayerID,
			idDes.OrganizingID,
			idDes.LeavewordID);
		cnDes->Execute(sql, NULL, -1);

		sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );

		Log(msg);

		sprintf(msg, "<setup>\tPlayerID\t= %d\n", idDes.PlayerID);
		Log(msg);
		sprintf(msg, "<setup>\tOrganizingID\t= %d\n",idDes.OrganizingID);
		Log(msg);
		sprintf(msg, "<setup>\tLeaveWordID\t= %d\n",idDes.LeavewordID);
		Log(msg);

		////////////////////////////////////////////////////////////////////////////////////////
		// 
		// ���� �㿨�𲿷�
		// 
		////////////////////////////////////////////////////////////////////////////////////////
		//Log("<info>\t����㿨�𲿷־ݿ�ʼ...\n");
		//// ������λ���͵���Ʒ
		//hr = rs->Open( "SELECT * FROM CSL_Card_Largess WHERE SendNum > ObtainedNum", cnSrc.GetInterfacePtr(), adOpenForwardOnly, adLockReadOnly, adCmdText);
		//if(FAILED(hr)) _com_issue_error(E_FAIL);
		//while(!rs->GetRSEOF()) {
		//	char old_cdkey[32]="";
		//	char new_cdkey[32]="";
		//	
		//	strcpy( old_cdkey, (const char *)(_bstr_t)rs->GetCollect("Cdkey") );

		//	_strlwr(old_cdkey);

		//	string temp;
		//	// �Ƿ������ʺ�
		//	temp = changed_cdkey[old_cdkey];
		//	strcpy( new_cdkey, temp.data() );

		//	if( !temp.length() ) { // ���û�������ʺţ����þ��ʺ�
		//		strcpy( new_cdkey, old_cdkey );
		//	}
		//	
		//	try {
		//		sprintf(sql, "INSERT INTO CSL_Card_Largess(cdkey,sendnum,ObtainedNum) VALUES('%s',%d,%d)",
		//			new_cdkey, (long)rs->GetCollect("sendnum"), (long)rs->GetCollect("ObtainedNum") );
		//		cnDes->Execute(sql, NULL, adCmdText);
		//	} // end try
		//	catch(_com_error &e) {
		//		sprintf( msg,"<ado err>\t���ݿ��쳣(for CSL_Card_Largess): %s\n", static_cast<const char*>(e.Description()) );
		//		Log(msg);
		//	}
		//	rs->MoveNext();
		//} // end while
		//rs->Close();

		//sprintf( msg, "<info>\t��ɣ���ʱ��%d\n", begin_time = clock()-begin_time );
		//Log(msg);
	}
	catch(_com_error &e) {
		sprintf( msg,"<ado err>\t���ݿ��쳣(GameDB): %s\n", static_cast<const char*>(e.Description()) );
		Log(msg);
	}


	sprintf( msg, "<info>\t������ɣ�����ʱ��%d\n", clock()-total_time );
	Log(msg);

	LogChanged(fAC, "<End>\n");
	LogChanged(fPNC, "<End>\n");

	Log("<End>\n");


	changed_player_id.clear();
	changed_faction_id.clear();
	changed_player_account.clear();
	changed_player_name.clear();

	if(file) fclose(file);
	if(fAC) fclose(fAC);
	if(fPNC) fclose(fPNC);

	if(cnDes->GetState() == adStateOpen) cnDes->Close();
	if(cnSrc->GetState() == adStateOpen) cnSrc->Close();
	if(logincnDes->GetState() == adStateOpen) logincnDes->Close();
	if(logincnSrc->GetState() == adStateOpen) logincnSrc->Close();

	cnDes.Release();
	cnSrc.Release();
	logincnDes.Release();
	logincnSrc.Release();

	// �ͷ�ADO
	::CoUninitialize();
	return 0;

}


// ����IMAGE����
bool CopyChunk(_RecordsetPtr &des, const _RecordsetPtr &src, const char *fileds ) {
	_variant_t varBLOB;
	long size;

	try {
		// read
		size = src->GetFields()->GetItem(fileds)->GetActualSize();
		if(size) {
			varBLOB = src->GetFields()->GetItem(fileds)->GetChunk(size);

			/*varBLOB.vt != VT_NULL && varBLOB.vt != VT_EMPTY */
			des->GetFields()->GetItem(fileds)->AppendChunk(varBLOB);		//write
		}
		return true;
	}
	catch(_com_error &e) {
		printf((char*)e.Description());
		sprintf(msg,"<ado err>\t���ݿ��쳣(Chunk): %s\n", static_cast<const char*>(e.Description()));
		fputs(msg, file);
		return false;
	}
}

// ��¼��־
void Log(const char *str) {
	if(!str || !file) return;
	printf(str);
	fputs(str,file);
	fflush(file);
}
void LogChanged(FILE *fChanged, const char *str) {
	if(!str || !fChanged) return;
	fputs(str,fChanged);
	fflush(fChanged);
}

char *FixName(char *n_name, const char *o_name, bool existent) {

	if( !n_name || !o_name ) return NULL;

	char temp[32];
	char buf[8];
	if( existent ) {
		// �����˵ģ���Ҫ��������
		sprintf(temp,"%s%02d%s%s", profix.data() , GetNum(buf), spechar.data(), o_name);
	}
	else {
		// ��1�Σ��϶���֪���Ƿ���ڣ�ֱ������
		sprintf(temp,"%s%s%s", profix.data() , spechar.data(), o_name);
	}

	//�޸�˫�ֽ��ַ� ����ȡ��16�ֽ���
	unsigned char c;
	while( strlen(temp) > 16 ) {
		c = temp[strlen(temp)-1];
		int s = static_cast<int>(strlen(temp));
		if( c <= 128 ) {
			// ASCII�ַ�
			temp[s-1] = 0;
		}
		else {
			// ˫�ֽ��ַ�
			temp[s-1] = 0;
			temp[s-2] = 0;
		}
	}

	strcpy(n_name, temp);

	return n_name;
}
//char *GetGUIDString(char *s) {
//	if(!s) return NULL;
//	GUID guid;
//	CoCreateGuid(&guid);
//	sprintf(s, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
//		guid.Data1, guid.Data2, guid.Data3, guid.Data4[0],guid.Data4[1],
//		guid.Data4[2],guid.Data4[3],guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]);
//	return s;
//}
char *GetDateString(_variant_t v, char *s) {
	if(!s) return NULL;
	if( v.vt != VT_NULL )
	{
		SYSTEMTIME st;
		VariantTimeToSystemTime(v.date, &st);
		sprintf(s,"%d-%d-%d %d:%d:%d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	}
	return s;
}

BYTE GetNum(char str[3]) {
	BYTE t;
	if( num>=100 ) {
		num=0;
	}

	t = num;

	sprintf(str, "%02d", t);
	str[2] = 0;

	num++;
	return t;
}



bool LoadConf(const char *filename)
{
	if( !filename ) return false;

	ifstream stream;
	stream.open(filename);

	if( !stream.is_open() )
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

		>> strTemp >> sqlSrc.Server
		>> strTemp >> sqlSrc.Database
		>> strTemp >> sqlSrc.Uid
		>> strTemp >> sqlSrc.Pwd

		>> strTemp >> profix
		>> strTemp >> spechar
		>> strTemp >> nWorldID

		>> strTemp >> sqlLoginDes.Server
		>> strTemp >> sqlLoginDes.Database
		>> strTemp >> sqlLoginDes.Uid
		>> strTemp >> sqlLoginDes.Pwd

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

	cnstrSrc = "Provider='SQLOLEDB.1'; Data Source='" + sqlSrc.Server +
		"'; Initial Catalog='" + sqlSrc.Database.data() +
		"'; User ID='" + sqlSrc.Uid.data() +
		"'; Password='" + sqlSrc.Pwd.data() + "'";
}

void MakeLoginConnStr()
{
	logincnstrDes = "Provider='SQLOLEDB.1'; Data Source='" + sqlLoginDes.Server +
		"'; Initial Catalog='" + sqlLoginDes.Database.data() +
		"'; User ID='" + sqlLoginDes.Uid.data() +
		"'; Password='" + sqlLoginDes.Pwd.data() + "'";

	logincnstrSrc = "Provider='SQLOLEDB.1'; Data Source='" + sqlLoginSrc.Server +
		"'; Initial Catalog='" + sqlLoginSrc.Database.data() +
		"'; User ID='" + sqlLoginSrc.Uid.data() +
		"'; Password='" + sqlLoginSrc.Pwd.data() + "'";
}