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



// ��飬���Ҽ���ģ�Ϳ��
// �ϲ�����¼
bool MergeBlk(CRxDbDatabase *res, CRxDbDatabase *dst)
{

//	CDraft::ErrorStatus er1, er2, er3;	
	CDraft::ErrorStatus es;
	CRxDbBlockTable *pBlkTbl;


	es = res->getBlockTable(pBlkTbl, CRxDb::kForRead);
	if (Acad::eOk != es || NULL == pBlkTbl)
	{
		acutPrintf(_T("\n��ȡԴ���ݿ���ʧ��,������:%d"), (es));
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
			acutPrintf(_T("\n��ȡԴ���ݿ�ģ�Ϳռ����¼ʧ��,������:%d"), (es));
			return false;
		}

		CRxDbBlockTableRecordIterator *pItr;
		pMsBlkRcd->newIterator(pItr);
		pMsBlkRcd->close();

		// ѭ�����������¼

		AcDbEntity *rec;
		//const CxCHAR *name;

		for (pItr->start(); !pItr->done(); pItr->step())
		{
			pItr->getEntity(rec, AcDb::kForRead);

			if (rec)
			{
				objId = rec->objectId();
				//name = res->isA()->name();


				// ֻ�в���ͼֽ�ռ�Ŀ��Ž��и���
				objIdArray.append(objId);

				rec->close();
			}
			else
				crxutPrintf(L"err");
		}
		delete pItr;

		//���objectId
		if (objIdArray.isEmpty())
		{
			acutPrintf(_T("\n��¡ʧ�ܣ�Դ���ݿ��ģ�Ϳռ�û��ʵ��!"));
			return false;
		}
	}


	AcDbIdMapping idMap;
	AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();
	//��ȡ��ǰ���ݿ�Ŀռ�ID
	mSpaceId = dst->currentSpaceId();
	//д���¡ �����ض���ģʽ
	es = res->wblockCloneObjects(objIdArray, mSpaceId, idMap, AcDb::kDrcIgnore);
	if (Acad::eOk != es)
	{
		acutPrintf(_T("\ndeepCloneObjectsʧ�ܴ�����:%d"), (es));
		return false;
	}
	return true;
}
// view every tablerecord
// ��������¼�е���ɣ�����д�뵽Ŀ�����¼��
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

	//д���¡ �����ض���ģʽ
	es = pRes->wblockCloneObjects(objIdArray, objId, idMap, AcDb::kDrcIgnore);
	if (Acad::eOk != es)
	{
		acutPrintf(_T("\ndeepCloneObjectsʧ�ܴ�����:%d"), es);
		return false;
	}
	// �������id
	// ���ݿ⼶������



	/*
	AcDbIdPair IdPair;
	AcDbObjectIdArray arrID2;
	//��ȡ��¡��ĺ�ԭidƥ��Ķ����objectid,��¡idmap��ȫ����idӳ�䣬����һЩ����������
	for (int i = 0; i< objIdArray.length(); i++)
	{
	IdPair.setKey(objIdArray[i]);
	if (idMap.compute(IdPair))
	{
	arrID2.append(IdPair.value());
	}
	}
	acutPrintf(_T("\n����:%d������!"), arrID2.length());
	*/

	return true;
}

// ���ļ����ƿ���¼
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
		acutPrintf(_T("\n��¡ʧ�ܣ�Դ���ݿ��ģ�Ϳռ�û��ʵ��!"));
		return false;
	}

	//��ȡ��ǰ���ݿ�Ŀռ�ID
	es = pCur->getBlockTable(pBlkTbl, AcDb::kForRead);
	es = pBlkTbl->getAt(ACDB_MODEL_SPACE, pMsBlkRcd, AcDb::kForWrite);
	pBlkTbl->close();
	AcDbObjectId mSpaceId = pBlkTbl->objectId(); // = pCur->currentSpaceId();

	AcDbIdMapping idMap;
	es = pRes->wblockCloneObjects(objIdArray, objId, idMap, AcDb::kDrcIgnore);

	pMsBlkRcd->close();

	if (Acad::eOk != es)
	{
		acutPrintf(_T("\ndeepCloneObjectsʧ�ܴ�����:%d"), es);
		return false;
	}
	else
		return true;
}
bool Writ1Page(CRxDbDatabase *dpage, CRxDbDatabase *dall, CRxGeMatrix3d t)
{

	// ��ȡ���ݿ� 
	// ��� ��¼ ���������
	CRxDbBlockTable               *pBlkTbl;
	CRxDbBlockTableRecord         *pBlkTblRcd;
	CRxDbBlockTableRecordIterator *pBlkTblRcdItr;

	dpage->getBlockTable(pBlkTbl, CRxDb::kForRead);
	pBlkTbl->getAt(ACDB_MODEL_SPACE, pBlkTblRcd, CRxDb::kForRead);
	pBlkTblRcd->newIterator(pBlkTblRcdItr, true);


	// ת���ݿ��ļ�����ǰ���ݿ�
	// ��� ��¼
	CRxDbBlockTable *p_cur_blkt;
	CRxDbBlockTableRecord *pBlockTableRecord_writein;

	//dall->getBlockTable(p_cur_blkt, CRxDb::kForRead);

	crxdbHostApplicationServices()->workingDatabase()->getBlockTable(p_cur_blkt, CRxDb::kForWrite);
	p_cur_blkt->getAt(ACDB_MODEL_SPACE, pBlockTableRecord_writein, CRxDb::kForWrite);

	CRxDbEntity *pEnt, *p;
	int i = 0;
	CDraft::ErrorStatus er1, er2;// ��ȡ�Ĵ���״̬ // д��Ĵ���״̬

	CRxDbObjectId id;

	// ����ģ�Ϳռ䣬д���¼
	fprintf(fp_g, "--W wki[xxx]: write to model space start\n");
	for (; !pBlkTblRcdItr->done(); pBlkTblRcdItr->step())
	{


		er1 = pBlkTblRcdItr->getEntity(pEnt, CRxDb::kForRead);

		p = CRxDbEntity::cast(pEnt->clone());
		if (p)
		{
			//p->transformBy(t);// ��ɱ任
			//p->setVisibility(CRxDb::Visibility::kVisible);
			//er2 = pBlockTableRecord_writein->appendAcDbEntity(id, p);

			/*
			CRxGePoint3d ptStart(0, 0, 0);
			CRxGePoint3d ptEnd(100, 100, 0);
			CRxDbLine *pLine = new CRxDbLine(ptStart, ptEnd);

			er2 = pBlockTableRecord_writein->appendAcDbEntity(id, pLine);
			pLine->close();

			*/

			//p->close(); // ע��ر�ʵ��

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
		acutPrintf(_T("\n��dwg�ļ�ʧ��,������:%d"), (es));
		return;
	}
	AcDbObjectIdArray objIdArray;
	AcDbBlockTable *pBlkTbl = NULL;
	es = pNewDb->getBlockTable(pBlkTbl, AcDb::kForRead);
	if (Acad::eOk != es || NULL == pBlkTbl)
	{
		acutPrintf(_T("\n��ȡԴ���ݿ���ʧ��,������:%d"), (es));
		delete pNewDb;
		pNewDb = NULL;
		return;
	}
	AcDbBlockTableRecord *pMsBlkRcd = NULL;
	es = pBlkTbl->getAt(ACDB_MODEL_SPACE, pMsBlkRcd, AcDb::kForRead);
	pBlkTbl->close();
	if (Acad::eOk != es || NULL == pMsBlkRcd)
	{
		acutPrintf(_T("\n��ȡԴ���ݿ�ģ�Ϳռ����¼ʧ��,������:%d"), (es));
		delete pNewDb;
		pNewDb = NULL;
		return;
	}
	//����Դ���ݿ�ģ�Ϳռ���¼�е�ʵ��
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

	//���objectId
	if (objIdArray.isEmpty())
	{
		acutPrintf(_T("\n��¡ʧ�ܣ�Դ���ݿ��ģ�Ϳռ�û��ʵ��!"));
		delete pNewDb;
		pNewDb = NULL;
		return;
	}
	AcDbIdMapping idMap;
	AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();
	//��ȡ��ǰ���ݿ�Ŀռ�ID
	AcDbObjectId mSpaceId = pDb->currentSpaceId();
	//д���¡ �����ض���ģʽ
	es = pNewDb->wblockCloneObjects(objIdArray, mSpaceId, idMap, AcDb::kDrcIgnore);
	if (Acad::eOk != es)
	{
		acutPrintf(_T("\ndeepCloneObjectsʧ�ܴ�����:%d"), (es));
		delete pNewDb;
		pNewDb = NULL;
		return;
	}
	//ɾ����ʱ���ݿ�
	delete pNewDb;
	pNewDb = NULL;
	
	/*
	AcDbIdPair IdPair;
	AcDbObjectIdArray arrID2;
	//��ȡ��¡��ĺ�ԭidƥ��Ķ����objectid,��¡idmap��ȫ����idӳ�䣬����һЩ����������
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
		acutPrintf(_T("\n����:%d������ɹ�!"), arrID2.length());
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


	// �����ļ��б��ʼ��
	fi_getlist(flist);
	fitor = flist.begin();

	int i = 0;
	while (i <= 2)
	{
		CRxDbDatabase *pDb = new CRxDbDatabase(Adesk::kFalse);
		if (!Read1Page(pDb, flist, fitor)) break; // �����ļ���pdb

												  // �ϲ�û�еĿ��
		MergeBlk(pDb, pDbc);

		// �ϲ���ģ�Ϳռ�Ŀ���¼
		//MergeMain(pDb, pDbc);

		//t = GenTrans(x, y, 420, 297,5);           // ����λ�ñ任

		//Writ1Page(pDb, pDbc, t);                  // д���ļ�������ǰ���ݿ�
		i++;

		delete pDb;
	};

	/*
	fprintf(fp, "����Ϊ��������Ŀ��\n");
	FindAtr(pDb, fp);
	delete pDb;


	fprintf(fp, "����Ϊ�������������\n");
	FindAtr(pDbc, fp);

	*/
	fprintf(fp, "return �����Է���\n");
	fclose(fp);




	// crxutPrintf(_T("\nд���ļ���: %d\n"), i);


	// д�����ļ�
	//std::wstring fpath = _T("F:\\gmk\\Test3.exb");
	//pDbc->saveAs(fpath.c_str());
}