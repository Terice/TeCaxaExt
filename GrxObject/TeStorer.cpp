#include "stdafx.h"
#include "CRxExtensionModule.h"
#include "dbents.h"
#include "dbapserv.h"
#include "dbpl.h"
#include "gearc2d.h"
#include "gept2dar.h"
#include "gept3dar.h"

#include"gemat3d.h"
#include "dbelipse.h"
#include "dbspline.h "
#include "dbmtext.h"
#include "crxChar.h "
#include "dbhatch.h"

#include "crxedcds.h"
#include "cdscodes.h"

#include "dbdim.h"
#include "geassign.h"
#include "geassign.h"

#include "crxutcds.h"

#include "crxed.h"
#include "dbmain.h"

#include "rxmfcapi.h"

#include "dbents.h"

#include <string.h>
#include <stdio.h>
#include <string>
#define szRDS _RXST("")



#include "TeFuncs.h"


#define AcDbIdPair CRxDbIdPair

#include <list>
static FILE* fp_g;



// 检查，并且加入模型块表
// 合并块表记录
bool MergeBlk(CRxDbDatabase *res, CRxDbDatabase *dst)
{

//	CDraft::ErrorStatus er1, er2, er3;	
	CDraft::ErrorStatus es;
	CRxDbBlockTable *pBlkTbl;


	es = res->getBlockTable(pBlkTbl, CRxDb::kForRead);
	if (Acad::eOk != es || NULL == pBlkTbl)
	{
		acutPrintf(_T("\n获取源数据库块表失败,错误码:%d"), (es));
		return false;
	}

	AcDbObjectIdArray objIdArray;
	AcDbObjectId objId;
	AcDbObjectId mSpaceId;

	if (true)
	{
		AcDbBlockTableIterator *itr_blktbl = NULL;

		CxCHAR *name;
		AcDbObjectId id;
		pBlkTbl->newIterator(itr_blktbl);
		for (itr_blktbl->start(); !itr_blktbl->done(); itr_blktbl->step())
		{
			AcDbBlockTableRecord *rec;
			itr_blktbl->getRecord(rec, AcDb::kForRead);
			id = rec->id();

			rec->getName(name);

			if (name[0] != L'*')
				objIdArray.append(objId);

			rec->close();

		}
		delete itr_blktbl;
	}
	else
	{
		AcDbBlockTableRecord *pMsBlkRcd = NULL;
		es = pBlkTbl->getAt(ACDB_MODEL_SPACE, pMsBlkRcd, AcDb::kForRead);
		pBlkTbl->close();
		if (Acad::eOk != es || NULL == pMsBlkRcd)
		{
			acutPrintf(_T("\n获取源数据库模型空间块表记录失败,错误码:%d"), (es));
			return false;
		}

		CRxDbBlockTableRecordIterator *pItr;
		pMsBlkRcd->newIterator(pItr);
		pMsBlkRcd->close();

		// 循环遍历存块表记录

		AcDbEntity *rec;
		//const CxCHAR *name;

		for (pItr->start(); !pItr->done(); pItr->step())
		{
			pItr->getEntity(rec, AcDb::kForRead);

			if (rec)
			{
				objId = rec->objectId();
				//name = res->isA()->name();


				// 只有不是图纸空间的块表才进行复制
				objIdArray.append(objId);

				rec->close();
			}
			else
				crxutPrintf(L"err");
		}
		delete pItr;

		//检查objectId
		if (objIdArray.isEmpty())
		{
			acutPrintf(_T("\n克隆失败，源数据库的模型空间没有实体!"));
			return false;
		}
	}


	AcDbIdMapping idMap;
	AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();
	//获取当前数据库的空间ID
	mSpaceId = dst->currentSpaceId();
	//写块克隆 忽略重定义模式
	es = res->wblockCloneObjects(objIdArray, mSpaceId, idMap, AcDb::kDrcIgnore);
	if (Acad::eOk != es)
	{
		acutPrintf(_T("\ndeepCloneObjects失败错误码:%d"), (es));
		return false;
	}
	return true;
}
// view every tablerecord
// 遍历块表记录中的组成，并且写入到目标块表记录中
bool DeepCopy(CRxDbBlockTableRecord *pres, CRxDbBlockTableRecord *pNew)
{
	CRxDbBlockTableRecordIterator *pItr;

	CDraft::ErrorStatus er = Acad::eOk;
	CDraft::ErrorStatus es = Acad::eOk;



	pres->newIterator(pItr);
	pres->close();

	CRxDbObjectIdArray objIdArray;
	CRxDbObjectId objId;
	for (pItr->start(); !pItr->done(); pItr->step())
	{
		pItr->getEntityId(objId);//(pEnt, CRxDb::kForRead);
		objIdArray.append(objId);
	}
	delete pItr;


	AcDbDatabase *pRes = pres->database();
	AcDbDatabase *pDb = pNew->database();
	AcDbIdMapping idMap;

	objId = pNew->objectId(); pDb->currentSpaceId();//

	idMap.setDestDb(pDb);

	//写块克隆 忽略重定义模式
	es = pRes->wblockCloneObjects(objIdArray, objId, idMap, AcDb::kDrcIgnore);
	if (Acad::eOk != es)
	{
		acutPrintf(_T("\ndeepCloneObjects失败错误码:%d"), es);
		return false;
	}
	// 读完遍历id
	// 数据库级别的深拷贝



	/*
	AcDbIdPair IdPair;
	AcDbObjectIdArray arrID2;
	//获取克隆后的和原id匹配的对象的objectid,克隆idmap是全部的id映射，包括一些附带的数据
	for (int i = 0; i< objIdArray.length(); i++)
	{
	IdPair.setKey(objIdArray[i]);
	if (idMap.compute(IdPair))
	{
	arrID2.append(IdPair.value());
	}
	}
	acutPrintf(_T("\n复制:%d个对象!"), arrID2.length());
	*/

	return true;
}

// 本文件复制块表记录
bool LocaCopy(CRxDbBlockTableRecord *pEve, CRxDbBlockTableRecord *pCur)
{
	CRxDbBlockTableRecordIterator *pItr;
	CRxDbEntity *pEnt;

	pEve->newIterator(pItr);
	for (pItr->start(); !pItr->done(); pItr->step())
	{
		pItr->getEntity(pEnt, CRxDb::kForRead);

		pCur->appendAcDbEntity(CRxDbEntity::cast(pEnt->clone()));

	}
	delete pItr;

	return true;
}
bool MergeMain(AcDbDatabase *pRes, AcDbDatabase *pCur)
{
	CDraft::ErrorStatus er = Acad::eOk;
	CDraft::ErrorStatus es = Acad::eOk;

	AcDbBlockTable *pBlkTbl = NULL;
	es = pRes->getBlockTable(pBlkTbl, AcDb::kForRead);

	AcDbBlockTableRecord *pMsBlkRcd = NULL;
	es = pBlkTbl->getAt(CRXDB_MODEL_SPACE, pMsBlkRcd, AcDb::kForRead);
	pBlkTbl->close();

	CRxDbBlockTableRecordIterator *pItr = NULL;
	pMsBlkRcd->newIterator(pItr);
	pMsBlkRcd->close();

	CRxDbObjectIdArray objIdArray;
	CRxDbObjectId objId;
	for (pItr->start(); !pItr->done(); pItr->step())
	{
		pItr->getEntityId(objId);//(pEnt, CRxDb::kForRead);
		objIdArray.append(objId);
	}
	delete pItr;

	if (objIdArray.isEmpty())
	{
		acutPrintf(_T("\n克隆失败，源数据库的模型空间没有实体!"));
		return false;
	}

	//获取当前数据库的空间ID
	es = pCur->getBlockTable(pBlkTbl, AcDb::kForRead);
	es = pBlkTbl->getAt(ACDB_MODEL_SPACE, pMsBlkRcd, AcDb::kForWrite);
	pBlkTbl->close();
	AcDbObjectId mSpaceId = pBlkTbl->objectId(); // = pCur->currentSpaceId();

	AcDbIdMapping idMap;
	es = pRes->wblockCloneObjects(objIdArray, objId, idMap, AcDb::kDrcIgnore);

	pMsBlkRcd->close();

	if (Acad::eOk != es)
	{
		acutPrintf(_T("\ndeepCloneObjects失败错误码:%d"), es);
		return false;
	}
	else
		return true;
}
bool Writ1Page(CRxDbDatabase *dpage, CRxDbDatabase *dall, CRxGeMatrix3d t)
{

	// 读取数据库 
	// 块表 记录 对象迭代器
	CRxDbBlockTable               *pBlkTbl;
	CRxDbBlockTableRecord         *pBlkTblRcd;
	CRxDbBlockTableRecordIterator *pBlkTblRcdItr;

	dpage->getBlockTable(pBlkTbl, CRxDb::kForRead);
	pBlkTbl->getAt(ACDB_MODEL_SPACE, pBlkTblRcd, CRxDb::kForRead);
	pBlkTblRcd->newIterator(pBlkTblRcdItr, true);


	// 转数据库文件到当前数据库
	// 块表 记录
	CRxDbBlockTable *p_cur_blkt;
	CRxDbBlockTableRecord *pBlockTableRecord_writein;

	//dall->getBlockTable(p_cur_blkt, CRxDb::kForRead);

	crxdbHostApplicationServices()->workingDatabase()->getBlockTable(p_cur_blkt, CRxDb::kForWrite);
	p_cur_blkt->getAt(ACDB_MODEL_SPACE, pBlockTableRecord_writein, CRxDb::kForWrite);

	CRxDbEntity *pEnt, *p;
	int i = 0;
	CDraft::ErrorStatus er1, er2;// 获取的错误状态 // 写入的错误状态

	CRxDbObjectId id;

	// 遍历模型空间，写入记录
	fprintf(fp_g, "--W wki[xxx]: write to model space start\n");
	for (; !pBlkTblRcdItr->done(); pBlkTblRcdItr->step())
	{


		er1 = pBlkTblRcdItr->getEntity(pEnt, CRxDb::kForRead);

		p = CRxDbEntity::cast(pEnt->clone());
		if (p)
		{
			//p->transformBy(t);// 完成变换
			//p->setVisibility(CRxDb::Visibility::kVisible);
			//er2 = pBlockTableRecord_writein->appendAcDbEntity(id, p);

			/*
			CRxGePoint3d ptStart(0, 0, 0);
			CRxGePoint3d ptEnd(100, 100, 0);
			CRxDbLine *pLine = new CRxDbLine(ptStart, ptEnd);

			er2 = pBlockTableRecord_writein->appendAcDbEntity(id, pLine);
			pLine->close();

			*/

			//p->close(); // 注意关闭实体

			er2 = Acad::eOk;
			fprintf(fp_g, "--V okk[%3d]: [to cur db] cast %s to %s \n", er2, (char*)(pEnt->isA()->name()), STR(CRxDbEntity));

		}
		else
		{
			i++;
			fprintf(fp_g, "--X err[%3d]: [to cur db] cast %s to %s \n", i, (char*)(pEnt->isA()->name()), STR(CRxDbEntity));
		}
		pEnt->close();

	};
	fprintf(fp_g, "--W wki[xxx]: write to model space end\n");
	delete pBlkTblRcdItr;

	pBlockTableRecord_writein->close();
	p_cur_blkt->close();


	pBlkTblRcd->close();
	pBlkTbl->close();

	return true;
}

void TeTestExb()
{
	AcDbDatabase *pNewDb = new AcDbDatabase(Adesk::kFalse);
	if (pNewDb == NULL)
	{
		return;
	}
	Acad::ErrorStatus es = pNewDb->readExbFile(_T("D:\\gmk\\test.exb"));
	if (Acad::eOk != es || NULL == pNewDb)
	{
		acutPrintf(_T("\n打开dwg文件失败,错误码:%d"), (es));
		return;
	}
	AcDbObjectIdArray objIdArray;
	AcDbBlockTable *pBlkTbl = NULL;
	es = pNewDb->getBlockTable(pBlkTbl, AcDb::kForRead);
	if (Acad::eOk != es || NULL == pBlkTbl)
	{
		acutPrintf(_T("\n获取源数据库块表失败,错误码:%d"), (es));
		delete pNewDb;
		pNewDb = NULL;
		return;
	}
	AcDbBlockTableRecord *pMsBlkRcd = NULL;
	es = pBlkTbl->getAt(ACDB_MODEL_SPACE, pMsBlkRcd, AcDb::kForRead);
	pBlkTbl->close();
	if (Acad::eOk != es || NULL == pMsBlkRcd)
	{
		acutPrintf(_T("\n获取源数据库模型空间块表记录失败,错误码:%d"), (es));
		delete pNewDb;
		pNewDb = NULL;
		return;
	}
	//遍历源数据库模型空间块记录中的实体
	CRxDbBlockTableRecordIterator *pIter = NULL;
	pMsBlkRcd->newIterator(pIter);
	pMsBlkRcd->close();
	AcDbObjectId objId;
	for (pIter->start(); !pIter->done(); pIter->step())
	{
		pIter->getEntityId(objId);
		objIdArray.append(objId);
	}
	delete pIter;

	//检查objectId
	if (objIdArray.isEmpty())
	{
		acutPrintf(_T("\n克隆失败，源数据库的模型空间没有实体!"));
		delete pNewDb;
		pNewDb = NULL;
		return;
	}
	AcDbIdMapping idMap;
	AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();
	//获取当前数据库的空间ID
	AcDbObjectId mSpaceId = pDb->currentSpaceId();
	//写块克隆 忽略重定义模式
	es = pNewDb->wblockCloneObjects(objIdArray, mSpaceId, idMap, AcDb::kDrcIgnore);
	if (Acad::eOk != es)
	{
		acutPrintf(_T("\ndeepCloneObjects失败错误码:%d"), (es));
		delete pNewDb;
		pNewDb = NULL;
		return;
	}
	//删除临时数据库
	delete pNewDb;
	pNewDb = NULL;
	
	/*
	AcDbIdPair IdPair;
	AcDbObjectIdArray arrID2;
	//获取克隆后的和原id匹配的对象的objectid,克隆idmap是全部的id映射，包括一些附带的数据
	for (int i = 0; i<objIdArray.length(); i++)
	{
		IdPair.setKey(objIdArray[i]);
		if (idMap.compute(IdPair))
		{
			arrID2.append(IdPair.value());
		}
	}
	if (!arrID2.isEmpty())
	{
		acutPrintf(_T("\n复制:%d个对象成功!"), arrID2.length());
	}
	*/
}


void TEReadEXB()
{
	int x, y; x = 0; y = 0;
	CRxGeMatrix3d t;

	FILE* fp = fopen("D:\\gmk\\test.txt", "w");
	fp_g = fp;

	CRxDbDatabase *pDbc = crxdbHostApplicationServices()->workingDatabase();

	std::list<wstring> flist;
	std::list<wstring>::iterator fitor;


	// 读入文件列表初始化
	fi_getlist(flist);
	fitor = flist.begin();

	int i = 0;
	while (i <= 2)
	{
		CRxDbDatabase *pDb = new CRxDbDatabase(Adesk::kFalse);
		if (!Read1Page(pDb, flist, fitor)) break; // 读入文件到pdb

												  // 合并没有的块表
		MergeBlk(pDb, pDbc);

		// 合并主模型空间的块表记录
		//MergeMain(pDb, pDbc);

		//t = GenTrans(x, y, 420, 297,5);           // 生成位置变换

		//Writ1Page(pDb, pDbc, t);                  // 写入文件档到当前数据库
		i++;

		delete pDb;
	};

	/*
	fprintf(fp, "以下为遍历读入的块表\n");
	FindAtr(pDb, fp);
	delete pDb;


	fprintf(fp, "以下为遍历工作区块表\n");
	FindAtr(pDbc, fp);

	*/
	fprintf(fp, "return 程序性返回\n");
	fclose(fp);




	// crxutPrintf(_T("\n写入文件数: %d\n"), i);


	// 写出到文件
	//std::wstring fpath = _T("F:\\gmk\\Test3.exb");
	//pDbc->saveAs(fpath.c_str());
}