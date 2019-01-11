// 97/07/25 @(#)renocom/src/common/include/dapi.h	1.4

//*********************************************

// dapi.h

//

// Common equates for Developer API

// (Exportable to external developers?)

//*********************************************

// Author: Rick Wong

// Company Confidential

// Copyright (c)1997 Fujitsu Software Corporation

//



#ifndef _DAPI_H

#define _DAPI_H



enum

{	kDAPIMaxSimultaneousApplications	=	25,

	kDAPIMaxAppNameLength				=	50,

	kDAPIMaxAvatarListLength			=	255,

	kDAPIMaxSpeechLength				=	255,

	kDAPIMaxAvatarNameLength			=	31,

	kDAPIMaxDataLength					=	4000

};



typedef enum

{	DAPI_success				= 0,

	DAPI_targetNotLoggedOn		= 1,	// Send

	DAPI_avatarBlocked			= 2,

	DAPI_targetBlocked			= 3,

	DAPI_avatarInBlockedRegion	= 4,

	DAPI_targetInBlockedRegion	= 5,

	DAPI_avatarBusy				= 6,

	DAPI_targetBusy				= 7,

	DAPI_tooManyApplications	= 8,	// Register

	DAPI_nameAlreadyRegistered	= 9,

	DAPI_notRegistered			= 10,	// Unregister

	DAPI_noTextSpecified		= 11,

	DAPI_missingTargetAvatar	= 12,

	DAPI_speechNotPermitted		= 13,

	DAPI_missingArguments		= 14,	// Windows only

	DAPI_invalidDataLength		= 15,	// Windows only

	DAPI_noDataSpecified		= 16,	// Windows only

	DAPI_notYetLoggedIn			= 17,

	DAPI_outOfMemory			= 18,	// Mac only

	DAPI_worldsAwayUnavailable	= 19,	// Mac only

	DAPI_resultTimedOut			= 20,	// Mac only; reply timed out without result status

	DAPI_unknownErr				= 21

} DAPI_result;



typedef	char *	DAPI_Name_t;

typedef	char *	DAPI_AvatarList_t;



enum

{	kDAPIAvatarListSeparator	= 0x0A,

	kDAPIArgumentListSeparator	= 0x0C

};



#endif /* _DAPI_H */

