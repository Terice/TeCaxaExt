#pragma once



#include "dbmain.h"
#include "dbsymtb.h"

class TeTbItor
{
private:
	CRxDbDatabase *database;
	CRxDbBlockTable *iblktbl;


	CRxDbBlockTableIterator *pItr;


public:
	void que();


	TeTbItor(CRxDbDatabase *d);
	~TeTbItor();
};

