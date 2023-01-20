#pragma once




#define  STR(name) #name
#define AcDbIdPair CRxDbIdPair
#define pDba acdbHostApplicationServices()->workingDatabase()


#include <list>
#include <xstring>
#include "dbmain.h"

bool fi_getlist(std::list<std::wstring> & flist);
bool Read1Page(CRxDbDatabase * &dpage, std::list<std::wstring> & flist, std::list<std::wstring>::iterator & fitor);




void TeQuery();
void TeAbout();
void TeReadPage(); 
void TeReadSwePages();
void TeMainMenu();


#define FTYPE_NON 0
#define FTYPE_DWG 1
#define FTYPE_EXB 2


#define FPATH_TAR "D:\\gmk\\tar\\"