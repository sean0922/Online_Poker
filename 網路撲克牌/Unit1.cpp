//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "Unit2.h"

#define CLIENT_MAX 3//the maxmum of client
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
TStringList *sList = new TStringList();
int All_playersc[4];//所有玩家的分數
int askplayer=0; //server問目前的人拿不拿牌
int cont=0;//確認是否找到斷線的那個人
int time_count=4;//玩家選擇牌倒數的時間
int time_count2=6;
int player_salive[3]={0,1,2};
//bool player_present[3]={true,true,true};//確認玩家是否還在線上
int a=0;//分割字串時暫時用的
int disId=0;//斷線時先從0開始傳
bool Game_start=false;//開始遊戲的話就會變TRUE
int poker[52];//確認52張牌剩餘多少0=沒了1=1張
int pokerValue[52];//每張牌代表的分數
int player_card[4][52];//四位玩家的牌
int playerScore=0;
int playerNum;//紀錄Client端玩家編號
String strHost;
bool blnServer;
String newClient="";
String ClientHostName="";
String ServerName="";
bool fgConnectState;
int clientNum;
int dis_num=0;//目前執行握手的代號
int dis_clinum[3]={0,0,0};//目前需要執行握手的client
bool kick_out=false;//當遊戲開始後有人又加進來要踢掉
TImage *img[4][10];//圖片顯示
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
	randomize();
	fgConnectState = false;
	Disconnect1->Enabled=false;
	Form1->Caption="Local host"+IdIPWatch1->LocalIP();
	for(int i=0;i<10;i++)
	{
		pokerValue[i]=i+1;
		pokerValue[i+13]=i+1;
		pokerValue[i+26]=i+1;
		pokerValue[i+39]=i+1;
	}
	for(int i=0;i<3;i++)
	{
		pokerValue[i+10]=10;
		pokerValue[i+23]=10;
		pokerValue[i+36]=10;
		pokerValue[i+49]=10;
	}

	for(int i=0;i<4;i++)//一開始圖片都是空的
	{
		for(int j=0;j<10;j++)
		{
			img[i][j]=NULL;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Listen1Click(TObject *Sender)
{
	PACKET pkg;
	String strPkg;

	Form1->Caption="I am Server"+IdIPWatch1->LocalIP();
	Listen1->Checked = !Listen1->Checked;
	ComboBox1->Clear();

	if(Listen1->Checked)
	{
		ClientSocket1->Active = false;
		try
		{
			ServerSocket1->Active = true;
			Label2->Caption="1人";
            Timer3->Enabled=true;
		}
		catch(...)
		{
			Listen1->Checked=false;
			ShowMessage("Be a Server failuer");

			return;
        }
		ServerName="Server-"+IdIPWatch1->LocalIP();
		StatusBar1->SimpleText="Status: Chat Server Listening....";
		Connect1->Enabled=false;
		Form2->Memo1->Lines->Add("Server: "+ServerName+", socket:"+
		IntToStr(ServerSocket1->Socket->SocketHandle));
        ComboBox1->Enabled=true;
	}
	else
	{
		if(ServerSocket1->Active)
		{
			//build packet
			pkg.ID="CHAT";
			pkg.MainFunc=1;
			pkg.SubFunc=2;
			pkg.MsgLength=0;
			pkg.Msg="";
			pkg.separation="#";
			pkg.EndSymbol="@";
			pkg.Host=ServerName;

			strPkg=AssemblePacket(pkg);
			for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
			{
				ServerSocket1->Socket->Connections[i]->SendText(strPkg);
				ServerSocket1->Active = true;
            }
		}
		Connect1->Enabled=true;
        StatusBar1->SimpleText="Status: Chat Server Close";
	}
	blnServer=true;
    Form1->Tag=0;
}
//---------------------------------------------------------------------------

String TForm1:: AssemblePacket(PACKET pkg)
{
	String str="",tmp;

	if(pkg.ID.Length()!=3)
	{
		ShowMessage("檔頭錯誤");
		return NULL;
	}
	else
	{
		str+=pkg.ID;
	}
	//-----------------------host----------------------
	if(pkg.Host.Length()==0)
	{

		return NULL;
	}
	else
	{
		str+=pkg.Host;
		str+=pkg.separation;
	}
	//----------------------------main-------------------------------
	str+=pkg.MainFunc;
	//--------------------------------------sub function---------------
	str+=pkg.SubFunc;
	//-------------msg length-----------------------------------------
	tmp =IntToStr(pkg.MsgLength);
	if(tmp.Length()>3)
	{
		ShowMessage("msg長度紀錄error");
		return NULL;
	}
	else
	{
		for(int i=0;i<3-tmp.Length();i++)
		{
			str+="0";
		}
        str+=tmp;
    }
	//---------------------------message------------------------------------------
	if(pkg.MsgLength != pkg.Msg.Length())
	{
		ShowMessage("msg length error");
		return NULL;
	}
	else
	{
		str+=pkg.Msg;
		str+=pkg.EndSymbol;
	}

    return str;
}

void __fastcall TForm1::Opendebugwindow1Click(TObject *Sender)
{
	Form2->Top=Form1->Top;
	Form2->Left=Left+Form1->Width+10;
	Form2->Show();
}
//---------------------------------------------------------------------------



void __fastcall TForm1::Connect1Click(TObject *Sender)
{
	Form1->Caption="I am Client";

	if(ClientSocket1->Active)
		ClientSocket1->Active=false;
	strHost="127.0.0.1";

	if(InputQuery("Chat connection","Chat Server IP:",strHost))
	{
		if(strHost.Length()>0)
		{
			ClientSocket1->Host=strHost;

			ClientSocket1->Active=true;
			Timer1->Enabled=true;

			Listen1->Checked=false;
			Disconnect1->Enabled=false;
			blnServer=false;

			ComboBox1->Enabled=false;
        }
    }

}
//---------------------------------------------------------------------------




void __fastcall TForm1::Disconnect1Click(TObject *Sender)
{
	initiative_dnct();

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender)//處理持續握手的
{
	Timer1->Enabled=false;
	ShowMessage("Time out");
	ClientSocket1->Active=false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ClientSocket1Connect(TObject *Sender, TCustomWinSocket *Socket)

{
	Timer1->Enabled=false;

	StatusBar1->SimpleText="Status: Connect to"+Socket->RemoteHost;
	fgConnectState=true;
	Memo2->Clear();
	Memo2->Visible=true;
	Listen1->Enabled=false;
	Connect1->Enabled=false;
	Disconnect1->Enabled=true;

    ClientSocket1->ClientType=ctNonBlocking;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ClientSocket1Disconnect(TObject *Sender, TCustomWinSocket *Socket)

{
	Timer1->Enabled=false;
	StatusBar1->SimpleText="server disconnect";
	fgConnectState=false;
	Memo2->Visible=false;
	Listen1->Enabled=true;
	Disconnect1->Enabled=false;
	Connect1->Enabled=true;
	ShowMessage("已離開房間");


}
//---------------------------------------------------------------------------

void __fastcall TForm1::ClientSocket1Read(TObject *Sender, TCustomWinSocket *Socket)

{
	pPACKET pPkg;
	String strPacket;

	if(!fgConnectState)
	{
		return;
	}

	strPacket=Socket->ReceiveText();
	Form2->Memo1->Lines->Add("["+Time()+"]"+strPacket);

	pPkg=DisassemblePacket(strPacket);

	if(pPkg==NULL)
	{
		Memo1->Lines->Add("Packet error");
		return;
	}
	ParsePacket(pPkg);
}
//---------------------------------------------------------------------------
pPACKET TForm1::DisassemblePacket(String strPkg)
{
   PACKET tmpPkg;
   pPACKET pPkg;
   int separator;
   String strTmp;

   Form2->Memo1->Lines->Add("===== Diassemble packet ====");

   if(strPkg[strPkg.Length()]!='#')
   {
	  Form2->Memo1->Lines->Add("Packet error 1");
	  return NULL;
   }

   if(strPkg.Pos("@")<=0)
   {
	  Form2->Memo1->Lines->Add("Packet error 2");
	  return NULL;
   }

   if(strPkg.SubString(1,3)!="PON")
   {
	  Form2->Memo1->Lines->Add("Packet error 3");
	  return NULL;
   }

   //---------------------------------------
   tmpPkg.ID=strPkg.SubString(1,3);

   separator= strPkg.Pos("@");
   tmpPkg.Host=strPkg.SubString(4,separator-4);
   tmpPkg.separation=strPkg.SubString(separator,1);

   tmpPkg.MainFunc=strPkg.SubString(separator+1,1);
   tmpPkg.SubFunc=strPkg.SubString(separator+2,1);

   //-----------------------------------------
   strTmp=strPkg.SubString(separator+3,3);
   try{
	   tmpPkg.MsgLength=StrToInt(strTmp);
   }
   catch(...)
   {
	  Form2->Memo1->Lines->Add("Packet error 4");
	  return NULL;
   }

   if(tmpPkg.MsgLength<0)
   {
	  Form2->Memo1->Lines->Add("Packet error 5");
	  return NULL;
   }

   strTmp=strPkg.SubString(separator+6,strPkg.Length()-separator-6);
   if(strTmp.Length()!=tmpPkg.MsgLength)
   {
	  Form2->Memo1->Lines->Add("Packet error 6");
	  return NULL;
   }
   else
	 tmpPkg.Msg=strTmp;

   pPkg=new PACKET;
   pPkg->ID=tmpPkg.ID;
   pPkg->Host=tmpPkg.Host;
   pPkg->separation=tmpPkg.separation;
   pPkg->MainFunc=tmpPkg.MainFunc;
   pPkg->SubFunc=tmpPkg.SubFunc;
   pPkg->MsgLength=tmpPkg.MsgLength;
   pPkg->Msg=tmpPkg.Msg;
   pPkg->EndSymbol=tmpPkg.EndSymbol;

   return pPkg;

}

void __fastcall TForm1::Edit1Click(TObject *Sender)
{
	ServerSocket1->Close();
	ClientSocket1->Close();

	Close();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ClientSocket1Error(TObject *Sender, TCustomWinSocket *Socket,
		  TErrorEvent ErrorEvent, int &ErrorCode)
{
	Memo2->Lines->Add("Error:"+strHost);
    ErrorCode = 0;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ServerSocket1Accept(TObject *Sender, TCustomWinSocket *Socket)

{
	fgConnectState=true;
	Memo2->Visible=true;

    ReflashClientList();
}
//---------------------------------------------------------------------------
void TForm1::ParsePacket(pPACKET pPkg)
{
	int mFunc,sFunc;
	clientNum = ServerSocket1->Socket->ActiveConnections;
	mFunc=StrToInt(pPkg->MainFunc);
	sFunc=StrToInt(pPkg->SubFunc);

	String msg;
	PACKET pkg;
	String strPkg;
	String srtClient;
	switch(mFunc)
	{
		case 1://get a new name
			switch(sFunc)
			{
				case 1:
					ClientHostName=pPkg->Msg;
					Form2->Memo1->Lines->Add("New name:"+ClientHostName);
					Form1->Caption=ClientHostName;
					break;
				case 2:
					if(blnServer)
						Form2->Memo1->Lines->Add("["+Time()+"]Client'"+pPkg->Host+"'requests to disconnect.");
					else
						Form2->Memo1->Lines->Add("["+Time()+"]Server'"+pPkg->Host+"'disconnected.");
					break;
				case 3: //接收大家的排面
					sList->Delimiter = ',';
					sList->DelimitedText=(pPkg->Msg);

					for(int i=0;i<4;i++)
					{
						player_card[i][0]=StrToInt(sList->Strings[i]);
					}
                    	GroupBox1->Visible=true;
						Label3->Visible=true;
						Label9->Visible=true;
					playerScore=pokerValue[StrToInt(sList->Strings[playerNum-1])-1];
					Label9->Caption=playerScore;
					a=0;

					hand_shake();
                    prnt_card();
					ShowMessage("遊戲開始");
					break;

				case 4:
						playerNum=StrToInt(pPkg->Msg);
						Label8->Caption="玩家("+IntToStr(playerNum)+")";
					break;

			}
        break;
		case 2:
			switch(sFunc)
			{
				case 0:
					Memo1->Lines->Add("["+pPkg->Host+"]:"+pPkg->Msg);
					Button2->Visible=true;
					Button4->Visible=true;
					Timer5->Enabled=true;//限時5秒內選擇
					break;
				case 1://client要牌
					Timer6->Enabled=false;
					time_count2=6;
					dis_num=0;   //已收到握手停止倒數

					Button3->Visible=true;
					send_card();
					Sleep(500);
					if(All_playersc[player_salive[askplayer-1]+1]>=21&&player_salive[askplayer-1]==player_salive[clientNum-1])
					{
						Button3->Visible=false;
						//ShowMessage("結算成績 玩家一:"+IntToStr(All_playersc[0])+",玩家二:"+IntToStr(All_playersc[1])
						//+",玩家三:"+IntToStr(All_playersc[2])+",玩家四:"+IntToStr(All_playersc[3]));
						String msg=""; //傳給client 讓他們知道遊戲已經結束了
						pkg.ID="PON";
						pkg.Host=IdIPWatch1->LocalIP();
						pkg.separation="@";
						pkg.MainFunc=2; //傳送資料
						pkg.SubFunc=5;
						pkg.EndSymbol="#";
						pkg.MsgLength=msg.Length();
						pkg.Msg=msg;
						strPkg=AssemblePacket(pkg);
						for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
						{
							ServerSocket1->Socket->Connections[i]->SendText(strPkg);
						}
						Game_Over();
                        dis_num=4;
						Timer6->Enabled=true;
                    }
					break;
				case 2:
					Timer6->Enabled=false;
					time_count2=6;
					dis_num=0;    //已收到握手停止倒數

					Button3->Visible=true;
					if(askplayer+1<=clientNum)
					{
					   askplayer+=1;
						//askCard();
					}
					else
					{
						//askplayer=0;
						Button3->Visible=false;
						String msg=""; //傳給client 讓他們知道遊戲已經結束了
						pkg.ID="PON";
						pkg.Host=IdIPWatch1->LocalIP();
						pkg.separation="@";
						pkg.MainFunc=2; //傳送資料
						pkg.SubFunc=5;
						pkg.EndSymbol="#";
						pkg.MsgLength=msg.Length();
						pkg.Msg=msg;
						strPkg=AssemblePacket(pkg);
						for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
						{
							ServerSocket1->Socket->Connections[i]->SendText(strPkg);
						}
						Sleep(500);
						dis_num=4;
						Timer6->Enabled=true;

						//Game_Over();
						//ShowMessage("結算成績 玩家一:"+IntToStr(All_playersc[0])+",玩家二:"+IntToStr(All_playersc[1])
						//+",玩家三:"+IntToStr(All_playersc[2])+",玩家四:"+IntToStr(All_playersc[3]));
					}
					break;
				case 3://傳給Server目前的編號
				pkg.ID="PON";
				pkg.Host=ClientHostName;
				pkg.separation="@";
				pkg.MainFunc=3; //傳送資料
				pkg.SubFunc=3;
				pkg.EndSymbol="#";
				pkg.MsgLength=001;
				pkg.Msg=IntToStr(playerNum-2);
				strPkg=AssemblePacket(pkg);
				ClientSocket1->Socket->SendText(strPkg);
					break;
				case 4: //client接收誰收到甚麼排
					//ShowMessage(pPkg->Msg);
					hand_shake();//回傳握手

                    sList->Delimiter = ',';
					sList->DelimitedText=(pPkg->Msg);
					if(StrToInt(sList->Strings[0])==0)
					{
						a=0;
						while(player_card[StrToInt(sList->Strings[0])][a]!=0)
						{
							a+=1;
						}
						player_card[0][a]=StrToInt(sList->Strings[1]);
					}
					else
					{
						a=0;
						while(player_card[player_salive[StrToInt(sList->Strings[0])-1]+1][a]!=0)
						{
							a+=1;
						}
						player_card[player_salive[StrToInt(sList->Strings[0])-1]+1][a]=StrToInt(sList->Strings[1]);
					}
                    prnt_card();
					break;

					case 5://遊戲結束時
						hand_shake();
                        Sleep(500);
						Game_Over();
						//ShowMessage("結算成績 玩家一:"+IntToStr(All_playersc[0])+",玩家二:"+IntToStr(All_playersc[1])
						//+",玩家三:"+IntToStr(All_playersc[2])+",玩家四:"+IntToStr(All_playersc[3]));
					break;
			}
			break;
		case 3:
			switch(sFunc)
			{
				case 0:

					break;

				case 1://client回答握手
					//dis_clinum[3]={0,0,0};
					if(dis_num==1||dis_num==3||dis_num==4)//
					{
						dis_clinum[StrToInt(pPkg->Msg)-2]=1;
						Form2->Memo1->Lines->Add(pPkg->Msg+"=1");
						if(dis_clinum[0]+dis_clinum[1]+dis_clinum[2]==clientNum)//1代表有傳回來
						{
							if(dis_num==4)
							{
								Game_Over();
							}
							Form2->Memo1->Lines->Add("Timer已經關閉");
							Timer6->Enabled=false;
							time_count2=6;
							dis_num=0;
							dis_clinum[0]=0;
							dis_clinum[1]=0;
							dis_clinum[2]=0;
						}
					}

					break;

				case 2: //client接收誰斷線
					for(int i=StrToInt(pPkg->Msg);i<2;i++)
						{
							player_salive[i]=player_salive[i+1];
						}

					break;
				case 3://server接收client的編號查詢誰斷線
				int clientNum;
				pkg.ID="PON";
				pkg.Host=IdIPWatch1->LocalIP();
				pkg.separation="@";
				pkg.MainFunc=3; //傳送資料
				pkg.EndSymbol="#";
				pkg.SubFunc=2;

				clientNum = ServerSocket1->Socket->ActiveConnections;

					if(player_salive[disId]!=(StrToInt(pPkg->Msg)))
					{
						ShowMessage("玩家"+IntToStr(player_salive[disId]+2)+"以離線");

						msg =IntToStr(disId);
                        pkg.MsgLength=msg.Length();
						pkg.Msg=msg;
						strPkg=AssemblePacket(pkg);
						for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
						{
							ServerSocket1->Socket->Connections[i]->SendText(strPkg);//傳誰斷線
						}

						if((disId+1)<askplayer)
						{
							askplayer-=1;
						}
						cont=1;
						for(int i=disId;i<clientNum;i++)
						{
							player_salive[i]=player_salive[i+1];
						}
						cont=1;
						Button3->Visible=true;
						/*if(disId==clientNum+1)
						{
						Form2->Memo1->Lines->Add("最後玩家離線了");
						}
						Form2->Memo1->Lines->Add("disId="+IntToStr(disId)+"clientNum+1="+IntToStr(clientNum+1));
                        */
					}
					if(cont==0&&(disId+1)==clientNum)
					{
						if(clientNum+1==askplayer)
						{
                             askplayer=0;
						}
                        Button3->Visible=true;
						ShowMessage("玩家"+IntToStr(player_salive[disId+1]+2)+"以離線");
						/*if(disId==clientNum+1)
						{
						Form2->Memo1->Lines->Add("最後玩家離線了");
						}
						Form2->Memo1->Lines->Add("disId="+IntToStr(disId)+"clientNum+1="+IntToStr(clientNum+1));
						*/
					}
					if(cont==0&&(disId+1)<clientNum)
					{
						disId+=1;
						discnt();
					}

					break;
            }
			break;
	   default:
				break;
	}
}
//-----------------------------------------------------------------------
void TForm1:: hand_shake()//握手回傳我有收到
{
	String msg;
	PACKET pkg;
	String strPkg;
	String srtClient;
	String tmmp=IntToStr(playerNum);//client自己的編號
	   pkg.ID="PON";
	   pkg.Host=ClientHostName;

	   pkg.separation="@";
	   pkg.MainFunc=3; //傳送資料
	   pkg.SubFunc=1;
	   pkg.MsgLength=tmmp.Length();
	   pkg.Msg=tmmp;
	   pkg.EndSymbol="#";
	   strPkg=AssemblePacket(pkg);

	   ClientSocket1->Socket->SendText(strPkg);
	   Memo2->Clear();
}
void __fastcall TForm1::Memo2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	String msg;
	PACKET pkg;
	String strPkg;
  	String srtClient;

  if (Key == VK_RETURN)
	{
	   pkg.ID="PON";

	   if(blnServer)
		 pkg.Host=IdIPWatch1->LocalIP();
	   else
		 pkg.Host=ClientHostName;

	   pkg.separation="@";
	   pkg.MainFunc=2; //傳送資料
	   pkg.SubFunc=0;
	   pkg.MsgLength=Memo2->Lines->Strings[Memo2->Lines->Count-1].Length();
	   pkg.Msg=Memo2->Lines->Strings[Memo2->Lines->Count-1];
	   pkg.EndSymbol="#";
	   strPkg=AssemblePacket(pkg);

		if (blnServer)
		  {
			int index;

			if(ComboBox1->Text=="All") //brocaste
			  {
				for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
				  ServerSocket1->Socket->Connections[i]->SendText(strPkg);
			  }
			else // specific client
			  {
				for(index=0;index<ServerSocket1->Socket->ActiveConnections;index++)
				  {
					srtClient=ServerSocket1->Socket->Connections[index]->RemoteAddress +
							 "-"+IntToStr(ServerSocket1->Socket->Connections[index]->SocketHandle);
					if(ComboBox1->Text==srtClient)
					  break;
				  }

				ServerSocket1->Socket->Connections[index]->SendText(strPkg);
			  }
          }
		else // Client
			ClientSocket1->Socket->SendText(strPkg) ;

        Memo2->Clear();

	}
}
//---------------------------------------------------------------------------

void TForm1::ReflashClientList()
{

	ComboBox1->Clear();

	if(ServerSocket1->Socket->ActiveConnections>1)
		ComboBox1->Items->Add("All");

	for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
	{
		ComboBox1->Items->Add(ServerSocket1->Socket->Connections[i]->RemoteAddress+"-"+
					IntToStr(ServerSocket1->Socket->Connections[i]->SocketHandle));
	}
		ComboBox1->ItemIndex=0;
		ComboBox1->Enabled=true;
}


void __fastcall TForm1::Timer2Timer(TObject *Sender)//刷新combobox的host
{
	Timer2->Enabled=false;
	ReflashClientList();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ServerSocket1ClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)

{
	int clientNum;
	clientNum = ServerSocket1->Socket->ActiveConnections;
	Form2->Memo1->Lines->Add("'"+Socket->RemoteAddress+"-"+IntToStr(Socket->SocketHandle)+"'disconnection comfirmer") ;
	StatusBar1->SimpleText = "Status: Listening...";
	if(ServerSocket1->Socket->ActiveConnections==1)
	{
		fgConnectState=false;
		Memo2->Visible=false;
	}

	ComboBox1->Enabled=false;
	Timer2->Enabled=true;
	Button1->Visible=false;
	Label2->Caption=IntToStr(clientNum);
	if(Game_start==true&&kick_out==false)
	{
		cont=0;//確認是否找到斷線的那個人
        disId=0;
		Timer4->Enabled=true;
	}
	kick_out=false;
}
//---------------------------------------------------------------------------
void TForm1:: discnt()//遊戲中有人斷線要重新確認
{
	Timer4->Enabled=false;
	clientNum = ServerSocket1->Socket->ActiveConnections;
	String msg;
	PACKET pkg;
	String strPkg;
	String srtClient;

	pkg.ID="PON";
	pkg.Host=IdIPWatch1->LocalIP();
	pkg.separation="@";
	pkg.MainFunc=2; //傳送資料
	pkg.SubFunc=3;
	pkg.EndSymbol="#";
	pkg.MsgLength=001;
	pkg.Msg=IntToStr(player_salive[disId]);
	strPkg=AssemblePacket(pkg);
	if(clientNum>=1)
		ServerSocket1->Socket->Connections[disId]->SendText(strPkg);
	else
		ShowMessage("其他玩家皆離線");

}

void __fastcall TForm1::Memo1Change(TObject *Sender)
{
	//client斷線後
	Timer2->Enabled=false;
	ReflashClientList();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::ServerSocket1ClientError(TObject *Sender, TCustomWinSocket *Socket,
          TErrorEvent ErrorEvent, int &ErrorCode)
{
	StatusBar1->SimpleText="Error from Client Socket.";
    ReflashClientList();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ServerSocket1ClientRead(TObject *Sender, TCustomWinSocket *Socket)

{
	pPACKET pPkg;
	String strPacket;

	strPacket=Socket->ReceiveText();
	Form2->Memo1->Lines->Add("["+Time()+"]"+strPacket);

	pPkg=DisassemblePacket(strPacket);
	if(pPkg==NULL)
	{
		Memo1->Lines->Add("Packet  error");
		return;
	}

    ParsePacket(pPkg);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ServerSocket1ClientConnect(TObject *Sender, TCustomWinSocket *Socket)

{
	PACKET pkg;
	String strPkg;
	int clientNum;
	clientNum = ServerSocket1->Socket->ActiveConnections;

	if(clientNum<=3&&Game_start==false)
	{
		Label2->Caption=(IntToStr(clientNum+1)+"人");//顯示目前房間人數
		Timer3->Enabled=true;
	}
	if(clientNum>CLIENT_MAX||Game_start==true)//當遊戲開始或滿人後不能再加進來
	{                       \
		kick_out=true;
		Form2->Memo1->Lines->Add("已達連線最大數量");
		Socket->Close();
		return;
	}

	StatusBar1->SimpleText="Status:  Connect from "+Socket->RemoteHost+
				"socket ID: "+IntToStr(Socket->SocketHandle);

	if(!CheckClients(Socket))//new connection
	{
		pkg.ID="PON";
		pkg.Host=ServerName;
		pkg.separation="@";
		pkg.MainFunc=1; //新連線
		pkg.SubFunc=1;  //設定client的名稱
		pkg.MsgLength=newClient.Length();
		pkg.Msg=newClient;
		pkg.EndSymbol="#";
		strPkg=AssemblePacket(pkg);

		ServerSocket1->Socket->Connections[clientNum-1]->SendText(strPkg);
	}


}
//---------------------------------------------------------------------------
bool TForm1::CheckClients(TCustomWinSocket *Socket)
{
	bool fg;
	int sHnd;
	String hostIP;

	hostIP=Socket->RemoteAddress;
	sHnd=Socket->SocketHandle;

	newClient=hostIP+"-"+IntToStr(sHnd);
	Form2->Memo1->Lines->Add("New Connection: "+newClient);
	Form2->Memo1->Lines->Add("Connection  number: "+IntToStr(ServerSocket1->Socket->ActiveConnections));

	fg=false;
	return fg;
}


void __fastcall TForm1::Button1Click(TObject *Sender)
{
	start_game();
}
//---------------------------------------------------------------------------
void TForm1:: start_game()//開始遊戲


{
	String tmpMsg="";//組合Msg
	//Button2->Visible=true;
	Button3->Visible=true;
	//Timer5->Enabled=true;

	for(int i=0;i<52;i++)//讓紀錄直都各有一張
	{
		poker[i]=1;
		player_card[0][i]=0;//玩家的排數
		player_card[1][i]=0;
		player_card[2][i]=0;
		player_card[3][i]=0;
	}
	int tmpNum[4];//開局各發四張

	for(int i=0;i<4;i++)//不重複的數字
	{
		tmpNum[i]=(rand()%(52-1+1))+1;

		while(poker[(tmpNum[i]-1)]==0)
		{
			tmpNum[i]=(rand()%(52-1+1))+1;
		}

		poker[tmpNum[i]-1]=0;
		player_card[i][0]=tmpNum[i];//把牌數加進玩家紀錄
		tmpMsg+=IntToStr(tmpNum[i]);
		if(i<3)
		{
			tmpMsg+=",";
		}
	}
	//ShowMessage(tmpMsg);
	for(int i=0;i<4;i++)//將poker紀錄在Debug視窗
	{
		String tmppk="";
		for(int j=0;j<13;j++)
		{
			tmppk+=IntToStr(poker[(i*13)+j])+",";
		}
		Form2->Memo1->Lines->Add("玩家"+IntToStr(i+1)+"手牌有");
		Form2->Memo1->Lines->Add(IntToStr(tmpNum[i]));
		Form2->Memo1->Lines->Add("第"+IntToStr(i+1)+"層");
		Form2->Memo1->Lines->Add(tmppk);
	}
	playerScore+=pokerValue[tmpNum[0]-1];//把Server的分數加進去
	Label9->Caption=IntToStr(playerScore);
	if(blnServer)
	{
		send_playerNum();
	}

	Button1->Visible=false;
	GroupBox1->Visible=true;
	Label3->Visible=true;
	Label9->Visible=true;
}
void  TForm1::send_playerNum()//開始遊戲時傳送玩家編號
{

	String msg;
	PACKET pkg;
	String strPkg;
	String srtClient;
	Game_start=true;
	   pkg.ID="PON";

	   if(blnServer)
		 pkg.Host=IdIPWatch1->LocalIP();
	   else
		 pkg.Host=ClientHostName;

	   pkg.separation="@";
	   pkg.MainFunc=1; //傳送資料
	   pkg.EndSymbol="#";

			for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
			{
				String tmp="";
				for(int z=0;z<4;z++)
				{
					tmp+=(player_card[z][0]);
					if(z<3)
						tmp+=",";
				}
				pkg.SubFunc=3;
				pkg.MsgLength=tmp.Length();
				pkg.Msg=tmp;
				strPkg=AssemblePacket(pkg);

				ServerSocket1->Socket->Connections[i]->SendText(strPkg);//傳牌面

			}

	   Memo2->Clear();
	   prnt_card();
	   dis_num=1;
       Timer6->Enabled=true;
}

void __fastcall TForm1::Timer3Timer(TObject *Sender)//遊戲還沒開始時需要確定房間目前人數為多少
{
	int clientNum;
	clientNum = ServerSocket1->Socket->ActiveConnections;
	if(clientNum<=3)
		Label2->Caption=(IntToStr(clientNum+1)+"人");//顯示目前房間人數
	Label1->Visible=true;
	Label2->Visible=true;

	String msg;
	PACKET pkg;
	String strPkg;
	String srtClient;

	   pkg.ID="PON";

	   if(blnServer)
		 pkg.Host=IdIPWatch1->LocalIP();
	   else
		 pkg.Host=ClientHostName;

	   pkg.separation="@";
	   pkg.MainFunc=1; //傳送資料
	   pkg.SubFunc=4;
	   pkg.EndSymbol="#";
			for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
			{
				pkg.MsgLength=001;
				pkg.Msg=(IntToStr(i+2));//第一位client玩家編號為2
				strPkg=AssemblePacket(pkg);

				ServerSocket1->Socket->Connections[i]->SendText(strPkg);//傳玩家編號
			}
	   Label8->Caption=("玩家(1)");
	if(clientNum==3)//人數滿3人後就可以開始遊戲
	{
        Timer3->Enabled=false;
		Button1->Visible=true;
		Game_start=true;
		//ShowMessage("請按下開始遊戲");
		start_game();
		ShowMessage("遊戲開始");
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer4Timer(TObject *Sender)//處理遊戲有人中途離線時
{
    Sleep(500);
	discnt();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
	clientNum = ServerSocket1->Socket->ActiveConnections;

	while(All_playersc[askplayer]>21)//如果這位玩家爆了就換下一位
	{
		askplayer+=1;
		if(askplayer>3)
		{
        	askplayer=0;
		}
	}

	if(askplayer!=0)//問client
	{
		askCard();

	}
	else//問server
	{
		Button2->Visible=true;
		Button4->Visible=true;
        Timer5->Enabled=true;
	}
    Button3->Visible=false;
}
//---------------------------------------------------------------------------
void TForm1:: askCard()//server問client要不要牌

{
	dis_num=2;
	Timer6->Enabled=true;//開始握手計時

	int clientNum;
	clientNum = ServerSocket1->Socket->ActiveConnections;

	String msg="";

	PACKET pkg;
	String strPkg;
	String srtClient;
	pkg.ID="PON";
	pkg.Host=IdIPWatch1->LocalIP();
	pkg.separation="@";
	pkg.MainFunc=2; //傳送資料
	pkg.SubFunc=0;
	pkg.EndSymbol="#";

	pkg.MsgLength=msg.Length();
	pkg.Msg=msg;
	strPkg=AssemblePacket(pkg);

	ServerSocket1->Socket->Connections[askplayer-1]->SendText(strPkg);

}

void __fastcall TForm1::Button4Click(TObject *Sender) //不要拿牌的時候
{
 dont_tcard();
}
//---------------------------------------------------------------------------
void  TForm1::dont_tcard()//不要拿牌的時候
{
	clientNum = ServerSocket1->Socket->ActiveConnections;
	if(blnServer)//換下一位 傳給下一位
	{
		askplayer+=1;
		//askCard();
		Button3->Visible=true;
	}
	else
	{

	String msg="";
	PACKET pkg;
	String strPkg;
	String srtClient;
	pkg.ID="PON";
	pkg.Host=IdIPWatch1->LocalIP();
	pkg.separation="@";
	pkg.MainFunc=2; //傳送資料
	pkg.SubFunc=2;
	pkg.EndSymbol="#";

	pkg.MsgLength=msg.Length();
	pkg.Msg=msg;
	strPkg=AssemblePacket(pkg);
	ClientSocket1->Socket->SendText(strPkg) ;
	}
	Button2->Visible=false;
	Button4->Visible=false;
	Timer5->Enabled=false;
	GroupBox2->Visible=false;
	time_count=4;
}

void __fastcall TForm1::Button2Click(TObject *Sender)//要拿牌的時候
{
	if(blnServer)
	{
		send_card();//直接傳給其他人誰拿了什麼牌
		//askplayer+=1;
        Button3->Visible=true;
	}
	else
	{
	String msg="";
	PACKET pkg;
	String strPkg;
	String srtClient;
	pkg.ID="PON";
	pkg.Host=IdIPWatch1->LocalIP();
	pkg.separation="@";
	pkg.MainFunc=2; //傳送資料
	pkg.SubFunc=1;
	pkg.EndSymbol="#";
	pkg.MsgLength=msg.Length();
	pkg.Msg=msg;
	strPkg=AssemblePacket(pkg);
	ClientSocket1->Socket->SendText(strPkg) ;
	}
	Button2->Visible=false;
	Button4->Visible=false;


	Timer5->Enabled=false;
	GroupBox2->Visible=false;
    time_count=4;
}
//---------------------------------------------------------------------------
void TForm1:: send_card()//傳送他們抽到的排  直接傳給其他人誰拿了什麼牌
{
	dis_num=3;
	Timer6->Enabled=true;//開始握手計時

	String tmpMsg="";//組合Msg

	int tmpNum;

	tmpNum=(rand()%(52-1+1))+1;

	while(poker[(tmpNum-1)]==0)
	{
		tmpNum=(rand()%(52-1+1))+1;
	}

	poker[tmpNum-1]=0;
	int tt=0;
	if(askplayer==0)
	{
		while(player_card[0][tt]!=0)
		{
			tt+=1;
		}
		player_card[0][tt]=tmpNum;//把牌數加進玩家紀錄

	}
	else
	{
		while(player_card[player_salive[askplayer-1]+1][tt]!=0)
		{
			tt+=1;
		}
		player_card[player_salive[askplayer-1]+1][tt]=tmpNum;//把牌數加進玩家紀錄
	}
	
	//tmpMsg+=IntToStr(tmpNum[i]);
	String msg;
	msg=IntToStr(askplayer)+","+IntToStr(tmpNum);

	PACKET pkg;
	String strPkg;
	String srtClient;
	Game_start=true;
	pkg.ID="PON";
	pkg.Host=IdIPWatch1->LocalIP();

	pkg.separation="@";
	pkg.MainFunc=2; //傳送資料
	pkg.EndSymbol="#";
	pkg.SubFunc=4;
	pkg.MsgLength=msg.Length();
	pkg.Msg=msg;
	strPkg=AssemblePacket(pkg);

    for(int i=0;i<ServerSocket1->Socket->ActiveConnections;i++)
	{
		ServerSocket1->Socket->Connections[i]->SendText(strPkg);//傳牌面
	}
    prnt_card();
}
void __fastcall TForm1::Button5Click(TObject *Sender)
{
	String aa="";
	clientNum = ServerSocket1->Socket->ActiveConnections;
		int a=0;
		for(int i=0;i<4;i++)
		{
		Form2->Memo1->Lines->Add("玩家"+IntToStr(i+1)+"牌有");
		a=0;
		aa="";
			while(player_card[i][a]!=0)
			{
				aa+=IntToStr(player_card[i][a]);
				aa+=",";
                a+=1;
			}
		Form2->Memo1->Lines->Add(aa);
		}
		Form2->Memo1->Lines->Add(IntToStr(askplayer));
		Form2->Memo1->Lines->Add("玩家2="+IntToStr(player_salive[0]));
		Form2->Memo1->Lines->Add("玩家3="+IntToStr(player_salive[1]));
		Form2->Memo1->Lines->Add("玩家4="+IntToStr(player_salive[2]));
		//prnt_card();
		Form2->Memo1->Lines->Add("playerslalive="+IntToStr(player_salive[0])+IntToStr(player_salive[1])+IntToStr(player_salive[2]));
		Form2->Memo1->Lines->Add("client="+IntToStr(clientNum));
}
//---------------------------------------------------------------------------
void TForm1:: prnt_card()//顯示牌號
{
	String tmmp="";
	a=0;
	playerScore=0;
	while(player_card[0][a]!=0)
	{
		tmmp+=IntToStr(player_card[0][a]);
		tmmp+=",";
		a+=1;
	}
	Label10->Caption=tmmp;

	tmmp="";
	a=0;
	while(player_card[1][a]!=0)
	{
		tmmp+=IntToStr(player_card[1][a]);
		tmmp+=",";
		a+=1;
	}
	Label11->Caption=tmmp;

	tmmp="";
	a=0;
	while(player_card[2][a]!=0)
	{
		tmmp+=IntToStr(player_card[2][a]);
		tmmp+=",";
		a+=1;
	}
	Label12->Caption=tmmp;

	tmmp="";
	a=0;
	while(player_card[3][a]!=0)
	{
		tmmp+=IntToStr(player_card[3][a]);
		tmmp+=",";
		a+=1;
	}
	Label13->Caption=tmmp;
    playerScore=0;
	if(!blnServer)
	{
			a=0;
		while(player_card[playerNum-1][a]!=0)
		{
			playerScore+=pokerValue[(player_card[playerNum-1][a])-1];
			a+=1;
		}

	}
	if(blnServer)
	{
        a=0;
		while(player_card[0][a]!=0)
		{
			playerScore+=pokerValue[(player_card[0][a])-1];
			a+=1;
		}

	}
	Label9->Caption=IntToStr(playerScore);

	for(int i=0;i<4;i++)//紀錄所有人的分數
	{
		a=0;
        int tmpsc=0;//暫時算分
		while(player_card[i][a]!=0)
		{
			tmpsc+=pokerValue[(player_card[i][a])-1];
			a+=1;
		}
		All_playersc[i]=tmpsc;
		if(tmpsc>21)
		{
			switch(i)
			{
				case 0:
					Label10->Visible=true;
					Label10->Caption="爆了";
				break;
				case 1:
					Label11->Visible=true;
					Label11->Caption="爆了";
				break;
				case 2:
					Label12->Visible=true;
					Label12->Caption="爆了";
				break;
				case 3:
                	Label13->Visible=true;
                	Label13->Caption="爆了";
				break;
			}
		}
    }
	prnt_img();//顯示圖片
}

void  TForm1::prnt_img()//顯示圖片
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<10;j++)
		{
			if(img[i][j]!=NULL)
			{
			delete img[i][j];
			img[i][j]=NULL;
			}
		}
	}
	Sleep(100);
	for(int i=0;i<4;i++)
	{
		a=0;
		while(player_card[i][a]!=0)
		{
			img[i][a]=new TImage(this);
			img[i][a]->Top=200-(a*20);
			img[i][a]->Left=17+(i*80);
			img[i][a]->Height=50;
			img[i][a]->Width=50;
			img[i][a]->Stretch=true;
			img[i][a]->Parent=this;
			img[i][a]->Picture->LoadFromFile(IntToStr(player_card[i][a])+".jpg");
			a=a+1;
		}
	}
	for(int i=0;i<4;i++)
	{
		if(blnServer&&i!=0)
		{
			img[i][0]->Picture->LoadFromFile("back.jpg");
		}
		if(!blnServer&&i!=playerNum-1)
		{
			img[i][0]->Picture->LoadFromFile("back.jpg");
		}
	}

}
//--------------------------------------------------------------------------------
void __fastcall TForm1::Timer5Timer(TObject *Sender)//選擇牌面倒數
{
	time_count-=1;
	GroupBox2->Visible=true;
	Label15->Caption=IntToStr(time_count)+"秒";
	if(time_count==0)//client選擇時
	{
		 Timer5->Enabled=false;
		 time_count=4;
		 dont_tcard();
		 GroupBox2->Visible=false;
	}
	/*if(time_count==0&&askplayer==0)//server選擇時
	{
		 Timer5->Enabled=false;
		 time_count=5;
		 GroupBox2->Visible=false;
		 //dont_tcard();
	}*/

}
//---------------------------------------------------------------------------
void TForm1:: Game_Over()//遊戲結束時
{
	String str="";//顯示遊戲結果的字串
	int tmpa=1;//排名暫時用的
	GroupBox1->Visible=false;
	Button1->Visible=false;
	Button3->Visible=false;
	Label1->Visible=false;
	Label2->Visible=false;
	Label3->Visible=false;
	Label8->Caption="";
	Label9->Visible=false;
	askplayer=0; //server問目前的人拿不拿牌
	cont=0;//確認是否找到斷線的那個人
	player_salive[0]=0;
	player_salive[1]=1;
    player_salive[2]=2;
	a=0;//分割字串時暫時用的
	disId=0;//斷線時先從0開始傳
	Game_start=false;//開始遊戲的話就會變TRUE
	playerScore=0;
	for(int i=0;i<52;i++)
	{
		player_card[0][i]=0;//玩家的排數
		player_card[1][i]=0;
		player_card[2][i]=0;
		player_card[3][i]=0;
	}
	if(blnServer)
	{
		Button6->Visible=true;
	}

	for(int i=0;i<4;i++) //顯示排名
	{
		tmpa=1;
		for(int j=0;j<4;j++)
		{
			if(All_playersc[i]<All_playersc[j]&&All_playersc[i]<=21&&All_playersc[j]<=21)
			{
				tmpa+=1;
			}
		}
        if(All_playersc[i]>21)
		{
			str+="玩家"+IntToStr(i+1)+":爆了, ";
		}
		else
		{
			str+="玩家"+IntToStr(i+1)+":第"+IntToStr(tmpa)+"名, ";
        }
	}
    for(int i=0;i<4;i++)
	{
		for(int j=0;j<10;j++)
		{
			if(img[i][j]!=NULL)
			{
			delete img[i][j];
			img[i][j]=NULL;
			}
		}
	}
    ShowMessage(str);

}
//---------------------------------------------------------------------------
void TForm1:: initiative_dnct()//主動給予斷線
{
PACKET pkg;
	String strPkg;

	if(blnServer)
	{
		return;
	}

	pkg.ID="PON";
	pkg.MainFunc=1;
	pkg.SubFunc=2;
	pkg.MsgLength=0;
	pkg.Msg="";
	pkg.separation="@";
	pkg.EndSymbol="#";
	pkg.Host=ClientHostName;


	strPkg=AssemblePacket(pkg);

	ClientSocket1->Socket->SendText(strPkg);
	ClientSocket1->Active=false;

	Memo2->Lines->Add("["+Time()+"] Dissconnect.");
	Listen1->Checked = false;
	Connect1->Checked=false;
	Disconnect1->Checked=false;

	fgConnectState=false;
	Memo2->Visible=false;
}


void __fastcall TForm1::Button6Click(TObject *Sender)
{
	Timer3->Enabled=true;
    Button6->Visible=false;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Timer6Timer(TObject *Sender)//處理握手
{
	int tt=0;
	int tmpNum=0;
	PACKET pkg;
	String strPkg;
	String srtClient;
	String msg;
	time_count2=time_count2-1;
	if(time_count2==0)
	{
	switch(dis_num)
		{
			case 0:
				return;
				break;
			case 1: //重新傳送一開始的牌面給client //ok了
				send_playerNum();
                time_count2=6;
				break;
			case 2: //問client要不要牌//ok了
				Timer6->Enabled=false;
				time_count2=6;
				Button3->Visible=true;
				ShowMessage("再傳一次");
				time_count2=6;
				break;
			case 3://server傳抽到的牌給client
                if(askplayer==0)
				{
					while(player_card[0][tt]!=0)
					{
						tt+=1;
					}
					tmpNum=player_card[0][tt-1];

				}
				else
				{
					while(player_card[player_salive[askplayer-1]+1][tt]!=0)
					{
						tt+=1;
					}
					tmpNum=player_card[player_salive[askplayer-1]+1][tt-1 ];//把牌數加進玩家紀錄
				}
				msg=IntToStr(askplayer)+","+IntToStr(tmpNum);
				pkg.ID="PON";
				pkg.Host=IdIPWatch1->LocalIP();
				pkg.separation="@";
				pkg.MainFunc=2; //傳送資料
				pkg.EndSymbol="#";
				pkg.SubFunc=4;
				pkg.MsgLength=msg.Length();
				pkg.Msg=msg;
				strPkg=AssemblePacket(pkg);

				for(int i=0;i<clientNum;i++)
				{
					if(dis_clinum[i]==0)
						ServerSocket1->Socket->Connections[i]->SendText(strPkg);//傳牌面
				}
				time_count2=6;
				break;
			case 4://傳遊戲結束給全client
					pkg.ID="PON";
					pkg.Host=IdIPWatch1->LocalIP();
					pkg.separation="@";
					pkg.MainFunc=2; //傳送資料
					pkg.SubFunc=5;
					pkg.EndSymbol="#";
					pkg.MsgLength=msg.Length();
					pkg.Msg="";
					strPkg=AssemblePacket(pkg);
					for(int i=0;i<clientNum;i++)
					{
						if(dis_clinum[i]==0)
							ServerSocket1->Socket->Connections[i]->SendText(strPkg);
					}
					time_count2=6;
				break;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
for(int i=0;i<4;i++)
	{
		for(int j=0;j<10;j++)
		{
			if(img[i][j]!=NULL)
			{
			delete img[i][j];
			img[i][j]=NULL;
			}
		}
	}
}
//---------------------------------------------------------------------------



