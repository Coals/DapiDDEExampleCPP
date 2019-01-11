//-----------------------------------------------------------------------------
// Name: dapitest.cpp
// Procedures: WinMain					- main windows function
//			   DapiTestProc				- dialog box routine
//			   ClearResults				- clear text results boxes
//			   ProcessAckData			- process WorldsApi_Ack message
//			   ProcessReceiveData		- process WorldsApi_Receive message
//			   ProcessGetAllTextData	- process WorldsApi_GetAllText message
// Author: Pat Mui
// Create Date: 7/10/97
// Last Modified: 10/7/97
// Version: 1.4
// Notes: this was written using only the Windows API so that it will work
//		  under Windows 3.x. it was developed using Microsoft Visual C++ 4.1.
//
// Copyright 1997 Fujitsu Software Corporation and Fujitsu Systems Business
// of America, All Rights Reserved".
//-----------------------------------------------------------------------------
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "dapitest.h"
#include "dapidde.h"

// macros
#define DAPITEST_DDE_SERVER			"DAPITEST" 		// dapitest server name
#define DAPITEST_MODE_SPEAK			0				// speak mode
#define DAPITEST_MODE_THINK			1				// think mode
#define DAPITEST_MODE_ESP			2				// esp mode

// function prototypes
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
											LPSTR lpCmdLine, int nCmdShow);
BOOL CALLBACK DapiTestProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ClearResults();
void ProcessAckData(int code);
void ProcessReceiveData(char *avatar, int dataLen, char *data);
void ProcessGetAllTextData(char *text);

// global variables
HWND dlgPtr;

//----------------------------------------------------------------------------
// Name: WinMain
// Inputs: as described in the Microsoft WinMain function description
// Outputs: none
// Returns: 0
// Comments:
//----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
	{

	// initialize DDE server
	InitDDE(DAPITEST_DDE_SERVER);

	// put up the dapitest dialog and exit when it returns
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_DAPITEST),0,(DLGPROC)DapiTestProc);

	// kill DDE server
	KillDDE();
	return 0;
	}

//----------------------------------------------------------------------------
// Name: DapiTestProc
// Inputs: as described in the Microsoft DLGPROC function description
// Outputs: none
// Returns: FALSE
// Comments:
//----------------------------------------------------------------------------
BOOL CALLBACK DapiTestProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	WORD wNotifyCode,wID;
	int mode,len;
	char avatar[DDE_MAX_MULTI_LEN];
	char data[DDE_MAX_DATA_LEN];

	// process messages
	switch (uMsg)
		{

		// initialization
		case WM_INITDIALOG:

			// set global dialog pointer
			dlgPtr = hwndDlg;

			// select speak button as default
			CheckDlgButton(hwndDlg,IDC_MODESPEAK,1);
			break;

		// process commands from buttons
		case WM_COMMAND:
			wNotifyCode = HIWORD(wParam);
			wID = LOWORD(wParam);
			switch (wID)
				{

				// register button
				case IDC_REGISTER:

					// clear results windows
					ClearResults();

					// send register command
					DapiRegister(DAPITEST_DDE_SERVER);
					break;

				// unregister button
				case IDC_UNREGISTER:

					// clear results windows
					ClearResults();

					// send unregister command
					DapiUnregister(DAPITEST_DDE_SERVER);
					break;

				// communicate button
				case IDC_COMMUNICATE:

					// clear results windows
					ClearResults();

					// get mode - speak, think or esp
					mode = DAPITEST_MODE_SPEAK;
					if (IsDlgButtonChecked(hwndDlg,IDC_MODETHINK))
						mode = DAPITEST_MODE_THINK;
					else if (IsDlgButtonChecked(hwndDlg,IDC_MODEESP))
						mode = DAPITEST_MODE_ESP;

					// if esp, get avatar name
					avatar[0] = 0;
					if (mode == DAPITEST_MODE_ESP)
						GetDlgItemText(hwndDlg,IDC_ESP,avatar,DDE_MAX_AVATAR_LEN);

					// get text to send
					data[0] = 0;
					GetDlgItemText(hwndDlg,IDC_TEXT,data,DDE_MAX_DATA_LEN);

					// send communicate command
					DapiCommunicate(DAPITEST_DDE_SERVER,mode,avatar,data);
					break;

				// send button
				case IDC_SEND:

					// clear results windows
					ClearResults();

					// get avatar name(s)
					avatar[0] = 0;
					GetDlgItemText(hwndDlg,IDC_AVATAR,avatar,DDE_MAX_MULTI_LEN);

					// get data
					data[0] = 0;
					GetDlgItemText(hwndDlg,IDC_DATA,data,DDE_MAX_DATA_LEN);

					// send send command
					DapiSend(DAPITEST_DDE_SERVER,avatar,strlen(data),data);
					break;

				// get all text button
				case IDC_GETALLTEXT:

					// clear results windows
					ClearResults();

					// send get all text command
					DapiGetAllText(DAPITEST_DDE_SERVER);
					break;

				// linefeed button
				case IDC_LF:

					// get name from edit box
					avatar[0] = 0;
					GetDlgItemText(hwndDlg,IDC_AVATAR,avatar,DDE_MAX_MULTI_LEN);

					// get length
					len = strlen(avatar);

					// append linefeed if not max length
					if (len < DDE_MAX_MULTI_LEN - 2)
						{
						avatar[len] = DDE_LINEFEED;
						avatar[len + 1] = 0;
						}

					// put name into edit box
					SetDlgItemText(hwndDlg,IDC_AVATAR,avatar);
					break;

				// clear button
				case IDC_CLEAR:
					ClearResults();
					break;

				// exit button
				case IDC_EXIT:
					PostMessage(hwndDlg,WM_CLOSE,0,0);
					break;
				}
			break;

		// close command
		case WM_CLOSE:
			EndDialog(hwndDlg,0);
			break;
		}
	return FALSE;
	}

//----------------------------------------------------------------------------
// Name: ClearResults
// Inputs: none
// Outputs: none
// Returns: nothing
// Comments:
//----------------------------------------------------------------------------
void ClearResults()
	{

	// clear output text fields
	SetDlgItemText(dlgPtr,IDC_ACK,"");
	SetDlgItemText(dlgPtr,IDC_RECEIVE,"");
	SetDlgItemText(dlgPtr,IDC_RESULT,"");
	}

//----------------------------------------------------------------------------
// Name: ProcessAckData
// Inputs: code - ack code - -1 means could not connect to worldsaway
// Outputs: none
// Returns: nothing
// Comments:
//----------------------------------------------------------------------------
void ProcessAckData(char *code)
	{
	int i;
	char *err;

	// translate code string to a number
	i = atoi(code);

	// if code is -1, put message in all edit boxes
	if (i == -1)
		{
		SetDlgItemText(dlgPtr,IDC_ACK,"Could not connect to WorldsAway!");
		SetDlgItemText(dlgPtr,IDC_RECEIVE,"Could not connect to WorldsAway!");
		SetDlgItemText(dlgPtr,IDC_RESULT,"Could not connect to WorldsAway!");
		return;
		}

	// get error code string
	err = GetDapiError(i);

	// if found error string display
	if (err)
		SetDlgItemText(dlgPtr,IDC_ACK,err);

	// if unknown code, just display string
	else
		SetDlgItemText(dlgPtr,IDC_ACK,code);
	}

//----------------------------------------------------------------------------
// Name: ProcessReceiveData
// Inputs: avatar - avatar data sent from
//		   dataLen - length of data
//		   data - pointer to the data
// Outputs: none
// Returns: nothing
// Comments:
//----------------------------------------------------------------------------
void ProcessReceiveData(char *avatar, int dataLen, char *data)
	{
	char *str;

	// allocate room for display string
	str = new char[strlen(avatar) + dataLen + 50];

	// make display string - assumes text only data
	sprintf(str,"Data from: %s   Length: %d   Data: %s",avatar,dataLen,data);

	// put output in receive display box
	SetDlgItemText(dlgPtr,IDC_RECEIVE,str);

	// free string
	delete[] str;
	}

//----------------------------------------------------------------------------
// Name: ProcessGetAllTextData
// Inputs: text - pointer to the text
// Outputs: none
// Returns: nothing
// Comments:
//----------------------------------------------------------------------------
void ProcessGetAllTextData(char *text)
	{

	// put output in get all text display box
	SetDlgItemText(dlgPtr,IDC_RESULT,text);
	}
