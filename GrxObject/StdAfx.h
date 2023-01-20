//-----------------------------------------------------------------------------
//- StdAfx.h : include file for standard system include files,
//-      or project specific include files that are used frequently,
//-      but are changed infrequently
//-----------------------------------------------------------------------------
#pragma once

#pragma pack (push, 8)
#pragma warning(disable: 4786 4996 4251)
//#pragma warning(disable: 4098)

//-----------------------------------------------------------------------------
#include <windows.h>

//- ObjectARX and OMF headers needs this
#include <map>



//CAXA Include
//#include "crxHeaders.h"

#pragma comment (lib ,"Crx.lib")
#pragma comment (lib ,"CrxDb.lib")
#pragma comment (lib ,"CrxEdApi.lib")
#pragma comment (lib ,"CrxGe.lib")
#pragma comment (lib ,"CrxGi.lib")
#include "CRxDefine.h"

//----- CAXA editor API
#include "rxdict.h"
#include "crxcmd.h"
#include "crxEntryPoint.h"

#include <tchar.h>
#pragma pack (pop)

