#include "StdAfx.h"

#include "TeFuncs.h"
#include "CrxHeader.h"
#include <tchar.h>





// 
static FILE *fp_g;
static char buf[256];
static wchar_t wbuf[512];

// ����ļ��Ƿ����
bool Exist(const wchar_t *f)
{
	FILE *f_test = _wfopen(f, L"r");
	if (!f_test)
	{
		return false;
	}
	else
	{
		fclose(f_test);
		return true;
	}
	return false;
}

// ������һ��ͼֽ��λ��
CRxGePoint3d GenTrans(int& c, int& r, int w, int h, int widofall)
{
	CRxGePoint3d t;
	t.x = c*w;
	t.y = -r*h;
	t.z = 0;

	c++;
	if (c >= widofall)
	{
		c = 0;
		r++;
	}

	return t;
}

// ���õĺ���
bool Read1Page(CRxDbDatabase * &dpage, std::list<std::wstring> & flist, std::list<std::wstring>::iterator & fitor)
{
	if (fitor != flist.end())
	{
		const wchar_t * f = fitor->c_str();
		crxutPrintf(_T("\n�����ļ�: %s\n"), f);
		Acad::ErrorStatus es = dpage->readExbFile(_T("D:\\gmk\\test.exb"));
		if (Acad::eOk != es || NULL == dpage)
		{
			acutPrintf(_T("\n��dwg�ļ�ʧ��,������:%d"), (es));
			return false;
		}
		fitor++;
		return true;
	}
	else
		return false;
}



// view every tablerecord
// ��������¼�е����ݣ�����д�뵽�ļ���¼��
bool FindAtr(CRxDbBlockTableRecord *pBlkTbl, FILE* f)
{
	CRxDbBlockTableRecordIterator *pItr;

	CRxDbEntity * pEnt, *pEntNew;
	CDraft::ErrorStatus er;

	if (!(pBlkTbl && f))
	{
		acutPrintf(_T("\n��ѯ���/debug�ļ�ָ�����"));
		return false;
	}

	pBlkTbl->newIterator(pItr);
	fprintf(f, "\t@[%p]FindAtr : --D start\n", pBlkTbl);

	int i;
	for (i = 0; !pItr->done(); pItr->step())
	{
		pItr->getEntity(pEnt, CRxDb::kForRead);
		const char *name = (const char*)pEnt->isA()->name();
		pEnt->close();

		if (i < 20)
			fprintf(f, "\t@FindAtr : --D %s\n", name);
		else
		{
			fprintf(f, "\t@FindAtr : ...    \n");
			break;
		}
		i++;
	}
	fprintf(f, "\tʵ����, %5d\n", i);
	fprintf(f, "\t@[%p]FindAtr : --D end\n", pBlkTbl);
	fprintf(f, "\n");
	delete pItr;

	return true;
}
// �������ݿ��еĿ������д�뵽�ļ���
bool FindAtr(CRxDbDatabase *pDb, FILE* fp)
{
	CDraft::ErrorStatus es;
	CRxDbBlockTable *pBlkTbl;
	CRxDbBlockTableRecord *pBlkTblRcd;

	pDb->getBlockTable(pBlkTbl, CRxDb::kForRead);

	//CRxDbEntity *pEnt, *p; 
	int i = 0;

	if (false)
	{
		fprintf(fp, "\n");
		fprintf(fp, "@[%p] Db FindAtr : sta\n", pDb);
		es = pBlkTbl->getAt(ACDB_MODEL_SPACE, pBlkTblRcd, CRxDb::kForRead);
		CxCHAR *pName;
		if (es == Acad::eOk)
		{
			pBlkTblRcd->getName(pName);
			fprintf(fp, "%s , %ws\n", pBlkTblRcd->isA()->name(), pName);
			FindAtr(pBlkTblRcd, fp);
		}
		pBlkTblRcd->close();
		fprintf(fp, "@[%p] Db FindAtr : end\n", pDb);
		fprintf(fp, "\n");

		return true;
	}



	CRxDbBlockTableIterator *pItr;

	pBlkTbl->newIterator(pItr);
	pBlkTbl->close();

	fprintf(fp, "\n");
	fprintf(fp, "@[%p] Db FindAtr : sta\n", pDb);
	for (pItr->start(); !pItr->done(); pItr->step())
	{
		pItr->getRecord(pBlkTblRcd, CRxDb::kForRead);
		CxCHAR *pName;
		pBlkTblRcd->getName(pName);
		fprintf(fp, "%s , %ws\n", pBlkTblRcd->isA()->name(), pName);

		FindAtr(pBlkTblRcd, fp);

		pBlkTblRcd->close();
	}
	fprintf(fp, "@[%p] Db FindAtr : end\n", pDb);
	fprintf(fp, "\n");
	delete pItr;

	return true;
}

// ����flist���б�������ͼֽ
void TeReadPagesByFlist(std::list<wstring> &flist)
{


	std::list<wstring>::iterator fitor;

	Acad::ErrorStatus es;
	AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbDatabase *pNewDb;

	// ��ʼһ��A3ͼֽ�����λ��Ϊ0��0
	int pos_r = 0, pos_c = 0;


	CRxDbBlockTable *blktbl;
	for (fitor = flist.begin(); fitor != flist.end(); fitor++)
	{
		es = pDb->getBlockTable(blktbl, AcDb::kForRead);
		if (es != Acad::eOk)
		{
			acutPrintf(_T("\n��ʼ��ȡ���ʧ��"));
			break;
		}

		// ȡ���ļ������ַ�ָ��
		const wchar_t * f = fitor->c_str();

		// ����ļ�����NULL
		// ��ôֱ������λ�ñ任
		if (!wcsncmp(f, L"NULL", 4))
		{
			GenTrans(pos_r, pos_c, 420, 297, 8);
			continue;
		}

		// �ж�����ļ�������Ӧ�Ŀ��ǲ����Ѿ�������
		// ��ֹ���޲�������
		CxCHAR *name = (CxCHAR*)f;
		if (blktbl->has(name))
		{
			acutPrintf(_T("\n���¼%s�Ѿ�����"), name);
			blktbl->close();
			//goto OpRcdFinish;
			continue;
		}



		// noDocument һ��ҪΪtrue�������ܲ���, 
		// defaultdrawingһ��ҪΪfalse���������ݿⲻΪ��
		pNewDb = new AcDbDatabase(Adesk::kFalse, true);
		// ������ʱ���ݿ���ڴ�
		if (pNewDb == NULL)
		{
			acutPrintf(_T("\n��dwg�ļ�ʧ��"));
			return;
		}

		// �ж�����ļ���exb����dwg�ļ�
		crxutPrintf(_T("\n�����ļ�: %s\n"), f);

		// ��ǰ����һ���ļ��Ƿ����
		// �������ִ򿪿�ָ������������
		FILE *f_test = _wfopen(f, L"r");
		if (!f_test)
		{
			crxutPrintf(_T("\n�ļ�%s������"), f);
			fclose(f_test);

			return;
		}
		else
		{
			fclose(f_test);
		}

		// ����ļ�����
		int ftype = 0;
		ftype = CheckFile(f);

		// �����ļ�����ʹ�ö�Ӧ���ļ����ͺ���
		// �����ļ���pNewDb������ݿ���
		Acad::ErrorStatus es;
		switch (ftype)
		{
		case FTYPE_DWG:
			es = pNewDb->readDwgFile(f, _SH_DENYRD); break;
		case FTYPE_EXB:
			es = pNewDb->readExbFile(f, _SH_DENYRD); break;
		default:
			crxutPrintf(_T("\n��֧�ֵ��ļ���ʽ: %s\n"), f);
			return;
		}


		if (Acad::eOk == es && NULL != pNewDb)
		{
			acutPrintf(_T("\n��cad�ļ��ɹ�"));

			AcDbObjectId id;
			//CxCHAR *name = (CxCHAR*)f;
			{
				es = pDb->insert(id, name, pNewDb);
				if (es != Acad::eOk)
					acutPrintf(_T("\n��������"));
				else
					acutPrintf(_T("\n�����%s�ɹ�"), name);
			}
			blktbl->getAt(name, id);

			// ����һ���µĿ�����ü�¼
			// ����ָ��ǰ������ļ���
			CRxGePoint3d ptInsert;
			ptInsert = GenTrans(pos_r, pos_c, 420, 297, 8);// ����A3��С��һ��8��ͼ����
			CRxDbBlockReference *pBlkRef = new CRxDbBlockReference();
			pBlkRef->setPosition(ptInsert);
			pBlkRef->setBlockTableRecord(id);

			// ���������ӵ�ģ�Ϳռ�
			CRxDbBlockTableRecord *pBlkTblRcd;
			blktbl->getAt(CRXDB_MODEL_SPACE, pBlkTblRcd, CRxDb::kForWrite);
			CRxDbObjectId entId;
			es = pBlkTblRcd->appendAcDbEntity(entId, pBlkRef);
			pBlkRef->close(); pBlkTblRcd->close();

			// 
			if (es != Acad::eOk)
			{
				acutPrintf(_T("�����ļ�%s����"), name);
			}



		OpRcdFinish:
			// һ�β�����ɣ��رտ��
			blktbl->close();

			/*
			AcDbBlockTable *pBlkTbl = NULL;
			if (pDba->getBlockTable(pBlkTbl, AcDb::kForRead) != Acad::eOk)
			{
			acutPrintf(_T("\n�����򿪿�����"));
			}
			if (!pBlkTbl->has(name))
			{
			acutPrintf(_T("\n����鲻����"));
			}
			else
			{
			acutPrintf(_T("\n�ҵ���¼%s"), name);
			}
			pBlkTbl->close();
			*/
		}
		else
		{
			acutPrintf(_T("\n��cad�ļ�ʧ��,������:%d"), (es));
		}
		delete pNewDb; pNewDb = NULL;
	}
}

// ����ַ�����Ӧ���ļ�������
// ��鷽�����ж�ĩβ�����ַ�
int CheckFile(const wchar_t * w)
{
	// �ַ���ȫ������ʹ�ó��ַ����͵�
	int r = 0;
	int l = int(wcslen(w));

	int t1 = l - 3, t2 = l - 2, t3 = l - 1;

	if (t3 < 0 || t2 < 0 || t1 < 0) return 0;
	else
	{
		bool exb = true, dwg = true;
		exb = exb && (w[t1] == L'e' || w[t1] == L'E');
		exb = exb && (w[t2] == L'x' || w[t1] == L'X');
		exb = exb && (w[t3] == L'b' || w[t1] == L'B');
		if (exb) return  FTYPE_EXB;
		dwg = dwg && (w[t1] == L'd' || w[t1] == L'D');
		dwg = dwg && (w[t2] == L'w' || w[t1] == L'W');
		dwg = dwg && (w[t3] == L'g' || w[t1] == L'G');
		if (dwg) return FTYPE_DWG;

		return FTYPE_NON;

	}
	return r;
}

void TeQuery()
{
	FILE* fp = fopen("D:\\gmk\\quer.txt", "w");

	FindAtr(crxdbHostApplicationServices()->workingDatabase(), fp);
	fclose(fp);
}
// ����tar�ļ���ָʾ���ϲ��ļ�
bool GenFilesByTar(std::list<wstring>& flist)
{
	FILE *fp = fopen("D:\\gmk\\tar.txt", "r");
	if (fp)
	{
		while (fgets(buf, 256, fp))
		{
			int l = strlen(buf) - 1;
			bool a = (buf[l] == '\n');
			buf[l] = buf[l] == '\n' ? 0 : buf[l];
			// %hs������char�ַ���װ��wchar�ַ���
			swprintf(wbuf, 256, L"D:\\gmk\\tar\\%hs", buf);
			std::wstring s(wbuf);
			if (s.length() > 4)
				flist.push_back(s);
		}
		fclose(fp);

		return true;
	}
	else
		return false;
}
bool GenFilesBySwe(std::list<wstring>& flist)
{

	// swe �ĵ������ļ�
	// ������ʽ����������
	// 1.dwg 
	// 1 - 1.dwg
	// 1 - 2.dwg
	// 2.dwg
	// 2 - 1.dwg
	// 3.dwg
	// 4.dwg
	// ����
	// 

	// ���������Զ����true
	// ��Ҫ�����ļ��б�ĳ������жϵ��������ļ�
	int pre, suf;
	for (pre = 1; ; pre++)
	{
		;
		swprintf(wbuf, L"D:\\gmk\\tar\\%d.dwg", pre);
		std::wstring s(wbuf);
		if (Exist(s.c_str()))
			flist.push_back(s);
		else
			break;
		// ��׺��Ҫ��8ҳ���룬
		for (suf = 1; ; suf++) 
		{
			swprintf(wbuf, L"D:\\gmk\\tar\\%d - %d.dwg", pre, suf);
			std::wstring s2(wbuf);
			if (Exist(s2.c_str()))
				flist.push_back(s2);
			else
				break;
		}
		suf--;
		// ���Զ��������ڵ��ļ���
		// ��Ҫ��������Ϊ7�ϣ���֮����ļ���ȫ��ΪNULL
		while ((suf + 1) % 8)
		{
			flist.push_back(L"NULL");
			suf++;
		}
	}
	return true;
}

void TeReadSwePages()
{

	std::list<wstring> flist;

	// �����ļ��б��ʼ��
	bool f_isready = GenFilesBySwe(flist);
	if (!f_isready)
	{
		acutPrintf(_T("\nĿ���ļ��б��ʼ��ʧ��"));
		return;
	}

	TeReadPagesByFlist(flist);

	acutPrintf(_T("�������"));
	return;

}

void TeReadPage()
{
	std::list<wstring> flist;

	// �����ļ��б��ʼ��
	bool f_isready = GenFilesByTar(flist);
	if (!f_isready)
	{
		acutPrintf(_T("\nĿ���ļ��б��ʼ��ʧ��"));
		return;
	}

	TeReadPagesByFlist(flist);

	acutPrintf(_T("�������"));
	return;
}

void TeAbout()
{

	acutPrintf(_T("\nAuthor: terice  @ 2022,GMK"));
}

void TeMainMenu()
{
	CRxUIManager* dasd = crxUIManager;
	CRxMenuBar* pMenuBar = dasd->getCRxMenuBar();
	unsigned int unCount = pMenuBar->getCount();

	CRxPopupMenu* pPopupMenu = pMenuBar->getPopupMenu(_T("GmkExt"));
	if (pPopupMenu)
	{
		acutPrintf(_T("\n����˵��Ѿ�����"));
	}
	else
	{
		CRxPopupMenu* pSubPopupMenu;
		unsigned int index = 0;
		pPopupMenu = pMenuBar->addPopupMenu(unCount, _T("GmkExt"));
		pPopupMenu->addMenuItem(index++, _T("TarMerge"), _T("tep"));
		pPopupMenu->addMenuItem(index++, _T("SweMerge"), _T("tew"));
		pPopupMenu->addMenuItem(index++, _T("Query"), _T("teq"));

		/*
		pSubPopupMenu = pPopupMenu->addSubMenu(0, _T("Main"));
		{
		pSubPopupMenu->addMenuItem(0, _T("Merge"), _T("tel"));
		pSubPopupMenu->addMenuItem(1, _T("Query"), _T("teq"));
		}
		*/
		pSubPopupMenu = pPopupMenu->addSubMenu(index++, _T("Othe"));
		{
			pSubPopupMenu->addMenuItem(0, _T("Udf"), _T("teq"));
		}
		pPopupMenu->addSeparator(index++);
		pPopupMenu->addMenuItem(index++, _T("About"), _T("tea"));

	}



	return;

}