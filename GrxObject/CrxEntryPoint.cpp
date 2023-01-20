// GrxObjectEntryPoint.cpp : CGrxObjectEntryPoint 

#include "stdafx.h"
#include "resource.h"

//-----------------------------------------------------------------------------
#define szRDS _RXST("")

#include "TeFuncs.h"

//-----------------------------------------------------------------------------
//----- CGrxObjectEntryPoint
class CGrxObjectApp:public AcRxArxApp
{
public:
	CGrxObjectApp():AcRxArxApp()
	{
	}

	virtual AcRx::AppRetCode On_kInitAppMsg(void *pkt) 
	{
		// TODO: Load dependencies here

		// You *must* call On_kInitAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kInitAppMsg(pkt);

		crxedRegCmds->addCommand(_T("GrxApp"), _T("tel"), _T("tell"), ACRX_CMD_MODAL, &TeMainMenu);
		crxedRegCmds->addCommand(_T("GrxApp"), _T("tep"), _T("tepg"), ACRX_CMD_MODAL, &TeReadPage);
		crxedRegCmds->addCommand(_T("GrxApp"), _T("tew"), _T("tesw"), ACRX_CMD_MODAL, &TeReadSwePages);
		crxedRegCmds->addCommand(_T("GrxApp"), _T("teq"), _T("teqr"), ACRX_CMD_MODAL, &TeQuery);
		crxedRegCmds->addCommand(_T("GrxApp"), _T("tea"), _T("teat"), ACRX_CMD_MODAL, &TeAbout);

		// TODO: Add your initialization code here

		TeMainMenu();

		return (retCode) ;
	}

	virtual AcRx::AppRetCode On_kUnloadAppMsg(void *pkt)
	{
		// TODO: Add your code here

		// You *must* call On_kUnloadAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kUnloadAppMsg(pkt);		

		// TODO: Unload dependencies here
		return (retCode);
	}

	virtual void RegisterServerComponents()
	{

	}
};

IMPLEMENT_ARX_ENTRYPOINT(CGrxObjectApp)