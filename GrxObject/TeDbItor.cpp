#include "stdafx.h"
#include "TeDbItor.h"



TeTbItor::TeTbItor(CRxDbDatabase *d)
{

	database = d;
	database->getBlockTable(iblktbl, CRxDb::kForRead);
	
}