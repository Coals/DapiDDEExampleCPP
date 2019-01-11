//-----------------------------------------------------------------------------
// Name: dapidde.cpp
// Procedures: InitDDE				- initialize dde server
//			   KillDDE				- uninitialize dde server
//			   SCallBack			- dde callback function
//			   DDECommand			- make worldsaway dapi call via dde
//			   GetDapiError			- get error string from error code
//			   ParseReceiveData		- parse data received via send command
//			   GetArgBySep			- get argument from string by formfeed
//			   GetArgByLength		- get argument from string by length
//			   DapiRegister			- dapi register command
//			   DapiUnregister		- dapi unregister command
//			   DapiSend				- dapi send command
//			   DapiCommunicate		- dapi communicate command
//			   DapiGetAllText		- dapi get all text command
//			   IsWaThere			- check to see if wa is running
// Author: Pat Mui
// Create Date: 7/10/97
// Last Modified: 10/7/97
// Version: 1.9
// Notes: this file is designed to be independent of the application and
//		  can be drop loaded into any windows c++ program. the new application
//		  will just have to provide processing for the ack, receive and get
//		  all text calls. NEWAPP marks the location of the calls.
//
// Copyright 1997 Fujitsu Software Corporation and Fujitsu Systems Business
// of America, All Rights Reserved".
//-----------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <ddeml.h>
#include "dapidde.h"

// global variables
DWORD Id = 0;					// dde service id
char app[DDE_MAX_NAME_LEN];		// application name
HSZ server;						// DAPITEST service handle

// NEWAPP: define these in new app
extern void ProcessAckData(char *code);
extern void ProcessReceiveData(char *avatar, int dataLen, char *data);
extern void ProcessGetAllTextData(char *text);

// error strings
const int numErrors = 18;
static char *errorList[numErrors] =
	{
	"0 - command sent successfully",
	"1 - target avatar is not logged on",
	"2 - your avatar not allowed to send/receive API messages",
	"3 - target avatar is not allowed to send/receive API messages",
	"4 - you are in a region that does not allow API messages",
	"5 - target avatar is in a region that doesn't allow API messages",
	"6 - your avatar is currently busy",
	"7 - target avatar is currently busy",
	"8 - too many names already registered",
	"9 - app name already registered",
	"10 - app name was not registered",
	"11 - no text was specified",
	"12 - mode was esp but no avatar name was specified",
	"13 - mode was speak or think but your avatar is a ghost",
	"14 - one or more arguments are missing",
	"15 - data length <= 0",
	"16 - no data was specified",
	"17 - you are not logged in yet"
	};

//----------------------------------------------------------------------------
// Name: InitDDE
// Inputs: name - name of app
// Outputs: none
// Returns: 0 - initialize ok
//			-1 - could not initialize dde server
// Comments:
//----------------------------------------------------------------------------
int InitDDE(char *name)
	{
	UINT output;
	HSZ server;

	// initialize dde
	output = DdeInitialize(&Id,SCallBack,APPCMD_FILTERINITS,0L);
	if (output != DMLERR_NO_ERROR)
		{
		MessageBox(0,"DDE Initialization Failure","Init Failure",MB_OK);
		return -1;
		}

	// save application name
	strncpy(app,name,DDE_MAX_NAME_LEN-1);
	app[DDE_MAX_NAME_LEN-1] = 0;

	// create service handle and register app
	server = DdeCreateStringHandle(Id,name,CP_WINANSI);
	if (!server)
		{
		MessageBox(0,"DDE Server Creation Failure","Create Failure",MB_OK);
		return -1;
		}
	DdeNameService(Id,server,0,DNS_REGISTER);
	DdeFreeStringHandle(Id,server);
	return 0;
	}


//----------------------------------------------------------------------------
// Name: KillDDE
// Inputs:  none
// Outputs: none
// Returns: nothing
// Comments:
//----------------------------------------------------------------------------
void KillDDE()
	{
	HSZ server;

	// unregister application
	server = DdeCreateStringHandle(Id,app,CP_WINANSI);
	if (server)
		{
		DdeNameService(Id,server,0,DNS_UNREGISTER); 
		DdeFreeStringHandle(Id,server);
		}

	// uninitialize dde
	DdeUninitialize(Id);
	Id = 0;
	}

//----------------------------------------------------------------------------
// Name: SCallBack
// Inputs: as described in the Microsoft DdeCallback function description
// Outputs: none
// Returns: result code
// Comments: this routine processes the ack and receive messages that
//			 worldsaway sends
//----------------------------------------------------------------------------
HDDEDATA CALLBACK SCallBack(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1,
						HSZ hsz2, HDDEDATA hdata, DWORD dwData1, DWORD dwData2)
	{
	HDDEDATA result = 0;
	char *b,tmp[100];
	DWORD len;

	// process transactions
	switch (uType)
		{

		// DdeConnect
		case XTYP_CONNECT:

			// check to see if the connect is for this application
			DdeQueryString(Id,hsz2,tmp,100,CP_WINANSI);
			if (!strcmp(tmp,app))
				result = (HDDEDATA)TRUE;
			break;

		// DdeClientTransaction
		case XTYP_POKE:

			// get topic string
			DdeQueryString(Id,hsz1,tmp,100,CP_WINANSI);

			// check to see if this is an ack message
			if (!strcmp(tmp,WA_DDE_ACK))
				{

				// get data length
				len = DdeGetData(hdata,0,0,0);

				// allocate buffer and init it
				b = new char[len+1];
				memset(b,0,len+1);

				// get data
				len  = DdeGetData(hdata,(LPBYTE)b,len,0);

				// free data handle
				DdeFreeDataHandle(hdata);

				// NEWAPP: call external function to handle data
				ProcessAckData(b);

				// free buffer
				delete[] b;

				// set result code
				result = (HDDEDATA)DDE_FACK;
				break;
				}

			// check to see if this is a receive message
			else if (!strcmp(tmp,WA_DDE_RECEIVE))
				{

				// get data length
				len = DdeGetData(hdata,0,0,0);

				// allocate buffer and init it
				b = new char[len+1];
				memset(b,0,len+1);

				// get data
				len  = DdeGetData(hdata,(LPBYTE)b,len,0);

				// free data handle
				DdeFreeDataHandle(hdata);

				// parse arguments and call external function
				ParseReceiveData(len,b);

				// free buffer
				delete[] b;

				// set result code
				result = (HDDEDATA)DDE_FACK;
				break;
				}
			break;

		// DdeDisconnect
		case XTYP_DISCONNECT:

			// check to see if the disconnect is for this application
			DdeQueryString(Id,hsz2,tmp,100,CP_WINANSI);
			if (!strcmp(tmp,app))
				result = (HDDEDATA)TRUE;
			break;
		}
	return result;
	}

//----------------------------------------------------------------------------
// Name: DDECommand
// Inputs: type - XTYP_REQUEST or XTYP_POKE
//		   serviceStr - should be worldsaway server name - WorldsApi
//		   topicStr - command to send - WorldsApi_Register, etc.
//		   itemStr - item to send - WorldsApi_Register, etc.
//		   dataLen - length of data unless type is XTYP_REQUEST
//					 if type is XTYP_REQUEST, dataLen is not used
//		   data - data specific to topic string unless type is XTYP_REQUEST
//				  if type is XTYP_REQUEST, data is not used
// Outputs: none
// Returns: nothing
// Comments:
//----------------------------------------------------------------------------
void DDECommand(int type, char *serviceStr, char *topicStr, char *itemStr, int dataLen, char *data)
	{
	HSZ service,topic,item;
	HDDEDATA output;
	HCONV hconv;
	char *out;
	DWORD length;

	// create service string handle
	service = DdeCreateStringHandle(Id,serviceStr,CP_WINANSI);

	// create topic string handle
	topic = DdeCreateStringHandle(Id,topicStr,CP_WINANSI);

	// connect to worldsaway
	hconv = DdeConnect(Id,service,topic,0);
	if (!hconv)
		{

		// NEWAPP: call ack with -1
		ProcessAckData("-1");

		// free string handles
		if (service)
			DdeFreeStringHandle(Id,service);
		if (topic)
			DdeFreeStringHandle(Id,topic);
		return;
		}

	// create item string handle
	item = DdeCreateStringHandle(Id,itemStr,CP_WINANSI);

	// initiate command and process output if any
	if (type == XTYP_POKE)
		{

		// create data handle for data
		output = DdeCreateDataHandle(Id,(LPBYTE)data,(DWORD)dataLen,
													(DWORD)0,topic,CF_TEXT,0);

		// send data to server
		DdeClientTransaction((LPBYTE)output,(DWORD)-1,hconv,item,CF_TEXT,
												XTYP_POKE,DDE_DEF_TIMEOUT,0);
		}
	else
		{

		// get data from server
		output = DdeClientTransaction(0,0,hconv,item,CF_TEXT,
											XTYP_REQUEST,DDE_DEF_TIMEOUT,0);

		// if output not null, data was returned
		if (output)
			{

			// get data length
			length = DdeGetData(output,0,0,0);

			// allocate buffer
			out = new char[length+1];

			// get data
			length  = DdeGetData(output,(LPBYTE)out,length,0);

			// null terminate it for sure
			out[length] = 0;

			// free data handle
 			DdeFreeDataHandle(output);

			// NEWAPP: call external function to handle data
			ProcessGetAllTextData(out);

			// free data buffer
			delete[] out;
			}
		}

	// disconnect from worldsaway
	DdeDisconnect(hconv);

	// free string handles
	if (service)
		DdeFreeStringHandle(Id,service);
	if (topic)
		DdeFreeStringHandle(Id,topic);
	if (item)
		DdeFreeStringHandle(Id,item);
	}

//----------------------------------------------------------------------------
// Name: GetDapiError
// Inputs: errorno - error number
// Outputs: none
// Returns: 0 if error number invalid
//			else pointer to error string
// Comments:
//----------------------------------------------------------------------------
char *GetDapiError(int errorno)
	{

	// return error string if found
	if (errorno < 0 || errorno > numErrors)
		return 0;
	return errorList[errorno];
	}


//----------------------------------------------------------------------------
// Name: ParseReceiveData
// Inputs: len - length of string
//		   str - pointer to data string
// Outputs: none
// Returns: nothing
// Comments:
//----------------------------------------------------------------------------
void ParseReceiveData(int len, char *str)
	{
	char *avatar,*data;
	int dataLen;

	// allocate and init buffers
	avatar = new char[len + 1];
	memset(avatar,0,len+1);
	data = new char[len + 1];
	memset(data,0,len+1);

	// get avatar name
	GetArgBySep(str,len,0,avatar);

	// get data length
	GetArgBySep(str,len,1,data);
	dataLen = atoi(data);

	// get data
	GetArgByLength(str,len,2,dataLen,data);

	// NEWAPP: call external function
	ProcessReceiveData(avatar,dataLen,data);

	// free buffers
	delete[] avatar;
	delete[] data;
	}

//----------------------------------------------------------------------------
// Name: GetArgBySep
// Inputs: text - pointer to source string
//		   len - length of text string
//		   arg - argument number to retrieve - first arg is 0
// Outputs: buf - argument if found
// Returns: 0 - argument retrieved succesfully
//			1 - invalid parameter
//			2 - argument not found
//			3 - argument was blank
// Comments: returns an argument separated by formfeed characters. if the
//			 first or last argument, second formfeed is not required
//----------------------------------------------------------------------------
int GetArgBySep(const char *text, int len, int arg, char *buf)
	{
	int i,numff,beg,end;

	// check for invalid parameter
	if (arg < 0 || !text || !buf || !len)
		return 1;

	// find begining index of argument
	i = numff = beg = 0;
	while (i < len && numff != arg)
		{
		if (text[i] == DDE_FORMFEED)
			{
			numff++;
			if (numff == arg)
				beg = i + 1;
			}
		i++;
		}

	// if number of formfeeds != arg return error
	if (numff != arg)
		return 2;

	// find ending index of argument
	i = numff = 0;
	end = len - 1;
	while (i < len && numff != arg + 1)
		{
		if (text[i] == DDE_FORMFEED)
			{
			numff++;
			if (numff == arg + 1)
				end = i - 1;
			}
		i++;
		}

	// check for blank argument
	if (beg > end)
		return 3;

	// copy argument into buf and null terminate it
	for(i=beg;i<=end;i++)
		buf[i-beg] = text[i];
	buf[end-beg+1] = 0;
	return 0;
	}

//----------------------------------------------------------------------------
// Name: GetArgByLength
// Inputs: text - pointer to source string
//		   len - length of text string
//		   arg - argument number to retrieve - first arg is 0
//		   argLen - length to retrieve
// Outputs: buf - argument if found
// Returns: 0 - argument retrieved succesfully
//			1 - invalid parameter
//			2 - argument not found
//			3 - argument runs past length of source string
// Comments:
//----------------------------------------------------------------------------
int GetArgByLength(const char *text, int len, int arg, int argLen, char *buf)
	{
	int i,numff,beg;

	// check for invalid parameter
	if (arg < 0 || !text || !buf || !len || !argLen || argLen > len)
		return 1;

	// find begining index of argument
	i = numff = beg = 0;
	while (i < len && numff != arg)
		{
		if (text[i] == DDE_FORMFEED)
			{
			numff++;
			if (numff == arg)
				beg = i + 1;
			}
		i++;
		}

	// if number of formfeeds != arg return error
	if (numff != arg)
		return 2;

	// check for overrun
	if (beg + argLen > len)
		return 3;

	// copy argument into buf and null terminate it
	for(i=beg;i<beg+argLen;i++)
		buf[i-beg] = text[i];
	buf[argLen] = 0;
	return 0;
	}

//----------------------------------------------------------------------------
// Name: DapiRegister
// Inputs: appName - name of app to register
// Outputs: none
// Returns: nothing
// Comments: return codes are sent via ProcessAckData function
//----------------------------------------------------------------------------
void DapiRegister(char *appName)
	{

	// check for null pointers
	if (!appName)
		return;

	// send register command to worldsaway
	DDECommand(XTYP_POKE,WA_DDE_SERVER,WA_DDE_REGISTER,WA_DDE_REGISTER,
												strlen(appName)+1,appName);
	}

//----------------------------------------------------------------------------
// Name: DapiUnregister
// Inputs: appName - name of app to unregister
// Outputs: none
// Returns: nothing
// Comments: return codes are sent via ProcessAckData function
//----------------------------------------------------------------------------
void DapiUnregister(char *appName)
	{

	// check for null pointers
	if (!appName)
		return;

	// send unregister command to worldsaway
	DDECommand(XTYP_POKE,WA_DDE_SERVER,WA_DDE_UNREGISTER,WA_DDE_UNREGISTER,
												strlen(appName)+1,appName);
	}

//----------------------------------------------------------------------------
// Name: DapiCommunicate
// Inputs: appName - app name
//		   mode - 0 for speak, 1 for think, 2 for esp
//		   avatar - avatar name if mode is esp
//		   text - text to send
// Outputs: none
// Returns: nothing
// Comments: return codes are sent via ProcessAckData function
//----------------------------------------------------------------------------
void DapiCommunicate(char *appName, int mode, char *avatar, char *text)
	{
	int len;
	char *buf;

	// check for null pointers
	if (!appName || (!avatar && mode == 2) || !text)
		return;

	// calculate length of send buffer string
	len = strlen(appName) + strlen(text) + 10;
	if (mode == 2)
		len += strlen(avatar);

	// allocate buffer
	buf = new char[len];

	// build data string
	if (mode == 2)
		sprintf(buf,"%s%c%d%c%s%c%s",appName,DDE_FORMFEED,mode,
							DDE_FORMFEED,avatar,DDE_FORMFEED,text);
	else
		sprintf(buf,"%s%c%d%c%c%s",appName,DDE_FORMFEED,mode,
							DDE_FORMFEED,DDE_FORMFEED,text);

	// send communicate command to worldsaway
	DDECommand(XTYP_POKE,WA_DDE_SERVER,WA_DDE_COMMUNICATE,
											WA_DDE_COMMUNICATE,len,buf);

	// free memory
	delete[] buf;
	}

//----------------------------------------------------------------------------
// Name: DapiSend
// Inputs: appName - app name
//		   avatar - avatar name(s)
//		   dataLen - length of data
//		   data - data to send
// Outputs: none
// Returns: nothing
// Comments: return codes are sent via ProcessAckData function
//----------------------------------------------------------------------------
void DapiSend(char *appName, char *avatar, int dataLen, char *data)
	{
	int i,j,len;
	char *buf;

	// check for null pointers
	if (!appName || !avatar || !data)
		return;

	// calculate length of send buffer string
	len = strlen(appName) + strlen(avatar) + dataLen + 10;

	// allocate buffer
	buf = new char[len];

	// build data string
	sprintf(buf,"%s%c%s%c%d%c",appName,DDE_FORMFEED,avatar,
									DDE_FORMFEED,dataLen,DDE_FORMFEED,data);

	// append data to end of data string
	j = strlen(buf);
	for(i=0;i<dataLen;i++)
		buf[j+i] = data[i];
	buf[j+dataLen] = 0;

	// send send command to worldsaway
	DDECommand(XTYP_POKE,WA_DDE_SERVER,WA_DDE_SEND,WA_DDE_SEND,len,buf);

	// free memory
	delete[] buf;
	}

//----------------------------------------------------------------------------
// Name: DapiGetAllText
// Inputs: appName - app name
// Outputs: none
// Returns: nothing
// Comments: return data is send via ProcessGetAllTextData function
//----------------------------------------------------------------------------
void DapiGetAllText(char *appName)
	{

	// check for null pointers
	if (!appName)
		return;

	// send get all text command
	DDECommand(XTYP_REQUEST,WA_DDE_SERVER,WA_DDE_GETALLTEXT,appName,0,0);
	}

//----------------------------------------------------------------------------
// Name: IsWaThere
// Inputs: none
// Outputs: none
// Returns: 0 if wa is not running or not accepting dde commands
//			1 if wa is running and accepting dde commands
// Comments:
//----------------------------------------------------------------------------
int IsWaThere()
	{
	int ret = 0;
	HSZ service,topic;
	HCONV hconv;

	// create service string handle
	service = DdeCreateStringHandle(Id,WA_DDE_SERVER,CP_WINANSI);

	// create topic string handle
	topic = DdeCreateStringHandle(Id,WA_DDE_REGISTER,CP_WINANSI);

	// connect to worldsaway
	hconv = DdeConnect(Id,service,topic,0);
	if (hconv)
		{
		DdeDisconnect(hconv);
		ret = 1;
		}

	// free string handles
	if (service)
		DdeFreeStringHandle(Id,service);
	if (topic)
		DdeFreeStringHandle(Id,topic);
	return ret;
	}
