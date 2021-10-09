//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Win.ScktComp.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdIPWatch.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.jpeg.hpp>
//---------------------------------------------------------------------------
typedef struct _PACKET
{
	String ID;   //4byte
	String Host;        //unfixed length
	String separation;  //1 byte
	String MainFunc;   //1 byte
	String SubFunc;    //1 byte
	int MsgLength;  //3 byte
	String Msg;       //unfixed length
	String EndSymbol;  ////1 byte,@

}PACKET,*pPACKET;

typedef struct _CLIENT_SOCKET
{
	bool active;
	TCustomWinSocket *pSocket;
	String ClientID;
}CLIENTSOCKET,*pCLIENTSOCKET;
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TClientSocket *ClientSocket1;
	TMainMenu *MainMenu1;
	TMenuItem *File1;
	TMenuItem *Listen1;
	TMenuItem *Connect1;
	TMenuItem *Disconnect1;
	TMenuItem *N1;
	TMenuItem *Edit1;
	TMenuItem *Clear1;
	TMenuItem *ReciveLog1;
	TMenuItem *Sendlog1;
	TServerSocket *ServerSocket1;
	TMenuItem *Debug1;
	TMenuItem *Functions1;
	TMenuItem *Opendebugwindow1;
	TTimer *Timer1;
	TTimer *Timer2;
	TIdIPWatch *IdIPWatch1;
	TComboBox *ComboBox1;
	TMemo *Memo1;
	TMemo *Memo2;
	TStatusBar *StatusBar1;
	TLabel *Label1;
	TLabel *Label2;
	TButton *Button1;
	TTimer *Timer3;
	TLabel *Label8;
	TLabel *Label3;
	TLabel *Label9;
	TTimer *Timer4;
	TButton *Button3;
	TButton *Button5;
	TTimer *Timer5;
	TGroupBox *GroupBox2;
	TLabel *Label14;
	TLabel *Label15;
	TButton *Button6;
	TTimer *Timer6;
	TImage *Image1;
	TGroupBox *GroupBox1;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *Label10;
	TLabel *Label11;
	TLabel *Label12;
	TLabel *Label13;
	TButton *Button2;
	TButton *Button4;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall Listen1Click(TObject *Sender);
	void __fastcall Opendebugwindow1Click(TObject *Sender);
	void __fastcall Connect1Click(TObject *Sender);
	void __fastcall Disconnect1Click(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall ClientSocket1Connect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocket1Disconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocket1Read(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall Edit1Click(TObject *Sender);
	void __fastcall ClientSocket1Error(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
	void __fastcall ServerSocket1Accept(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall Memo2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall Timer2Timer(TObject *Sender);
	void __fastcall ServerSocket1ClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall Memo1Change(TObject *Sender);
	void __fastcall ServerSocket1ClientError(TObject *Sender, TCustomWinSocket *Socket,
          TErrorEvent ErrorEvent, int &ErrorCode);
	void __fastcall ServerSocket1ClientRead(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ServerSocket1ClientConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Timer3Timer(TObject *Sender);
	void __fastcall Timer4Timer(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall Button4Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);
	void __fastcall Timer5Timer(TObject *Sender);
	void __fastcall Button6Click(TObject *Sender);
	void __fastcall Timer6Timer(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);







private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
	String AssemblePacket(PACKET pkg);
	pPACKET DisassemblePacket(String);
	void ShowPacket(PACKET);
	void ShowPacket(pPACKET);
	void ParsePacket(pPACKET);
	void ReflashClientList();
	bool CheckClients(TCustomWinSocket *);
	void send_playerNum();
	void hand_shake();
	void discnt();
	void askCard();
	void send_card();
	void start_game();
	void prnt_card();
	void dont_tcard();
	void Game_Over();
	void initiative_dnct();
    void prnt_img();//顯示圖片
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif

