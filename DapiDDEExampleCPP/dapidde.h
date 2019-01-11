//-----------------------------------------------------------------------------

// Name: dapidde.h

// Purpose: include file for dapidde.cpp

// Author: Pat Mui

// Create Date: 7/10/97

// Last Modified: 12/4/97

// Version: 1.6

// Notes: 

//

// Copyright 1997 Fujitsu Software Corporation and Fujitsu Systems Business

// of America, All Rights Reserved".

//-----------------------------------------------------------------------------

#include "dapi.h"



#define DDE_DEF_TIMEOUT		2000			// default timeout value

#define DDE_MAX_NAME_LEN	32				// max length app name + null

#define DDE_MAX_AVATAR_LEN	(kDAPIMaxAvatarNameLength+1)// max length avatar name + null

#define DDE_MAX_MULTI_LEN	(kDAPIMaxAvatarListLength+1)// max length multi avatars + null

#define DDE_MAX_TEXT_LEN	(kDAPIMaxSpeechLength+1)// max length text + null

#define DDE_MAX_DATA_LEN	(kDAPIMaxDataLength+1)// max length data + null

#define DDE_LINEFEED		kDAPIAvatarListSeparator// linefeed character

#define DDE_FORMFEED		kDAPIArgumentListSeparator// formfeed character



#define WA_DDE_SERVER		"WorldsApi" 			// wa server name

#define WA_DDE_REGISTER		"WorldsApi_Register"	// register command topic

#define WA_DDE_UNREGISTER	"WorldsApi_Unregister"	// unregister command topic

#define WA_DDE_SEND			"WorldsApi_Send"		// send command topic

#define WA_DDE_COMMUNICATE	"WorldsApi_Communicate"	// communicate command topic

#define WA_DDE_GETALLTEXT	"WorldsApi_GetAllText"	// get all text command topic

#define WA_DDE_ACK			"WorldsApi_Ack"			// acknowledge message topic

#define WA_DDE_RECEIVE		"WorldsApi_Receive"		// receive message topic



int InitDDE(char *name);

void KillDDE();

HDDEDATA CALLBACK SCallBack(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1,

						HSZ hsz2, HDDEDATA hdata, DWORD dwData1, DWORD dwData2);

void DDECommand(int type, char *serviceStr, char *topicStr,

										char *itemStr, int dataLen, char *data);

char *GetDapiError(int errorno);

void ParseReceiveData(int len, char *str);

int GetArgBySep(const char *text, int len, int arg, char *buf);

int GetArgByLength(const char *text, int len, int arg, int argLen, char *buf);

void DapiRegister(char *appName);

void DapiUnregister(char *appName);

void DapiCommunicate(char *appName, int mode, char *avatar, char *text);

void DapiSend(char *appName, char *avatar, int dataLen, char *data);

void DapiGetAllText(char *appName);

int IsWaThere();

