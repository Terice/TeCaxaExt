// (C) Copyright 2009 by CAXA, Inc. 
//


//-----------------------------------------------------------------------------
//- GrxObject.cpp : Initialization functions
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "resource.h"


//-----------------------------------------------------------------------------
//- DLL Entry Point
extern "C"
BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
	//- Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved) ;

	if ( dwReason == DLL_PROCESS_ATTACH ) {
        _hdllInstance =hInstance ;
	} else if ( dwReason == DLL_PROCESS_DETACH ) {
	}
	return (TRUE) ;
}
