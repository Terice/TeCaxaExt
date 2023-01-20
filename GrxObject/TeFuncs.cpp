#include "StdAfx.h"

#include "TeFuncs.h"
#include "CrxHeader.h"
#include <tchar.h>





// 
static FILE *fp_g;
static char buf[256];
static wchar_t wbuf[512];

// 检查文件是否存在
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

// 生成下一张图纸的位置
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

// 弃用的函数
bool Read1Page(CRxDbDatabase * &dpage, std::list<std::wstring> & flist, std::list<std::wstring>::iterator & fitor)
{
	if (fitor != flist.end())
	{
		const wchar_t * f = fitor->c_str();
		crxutPrintf(_T("\n读入文件: %s\n"), f);
		Acad::ErrorStatus es = dpage->readExbFile(_T("D:\\gmk\\test.exb"));
		if (Acad::eOk != es || NULL == dpage)
		{
			acutPrintf(_T("\n打开dwg文件失败,错误码:%d"), (es));
			return false;
		}
		fitor++;
		return true;
	}
	else
		return false;
}



// view every tablerecord
// 遍历块表记录中的内容，并且写入到文件记录中
bool FindAtr(CRxDbBlockTableRecord *pBlkTbl, FILE* f)
{
	CRxDbBlockTableRecordIterator *pItr;

	CRxDbEntity * pEnt, *pEntNew;
	CDraft::ErrorStatus er;

	if (!(pBlkTbl && f))
	{
		acutPrintf(_T("\n查询块表/debug文件指针错误"));
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
	fprintf(f, "\t实例数, %5d\n", i);
	fprintf(f, "\t@[%p]FindAtr : --D end\n", pBlkTbl);
	fprintf(f, "\n");
	delete pItr;

	return true;
}
// 遍历数据库中的块表，并且写入到文件中
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

// 按照flist的列表来插入图纸
void TeReadPagesByFlist(std::list<wstring> &flist)
{


	std::list<wstring>::iterator fitor;

	Acad::ErrorStatus es;
	AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbDatabase *pNewDb;

	// 初始一张A3图纸插入的位置为0，0
	int pos_r = 0, pos_c = 0;


	CRxDbBlockTable *blktbl;
	for (fitor = flist.begin(); fitor != flist.end(); fitor++)
	{
		es = pDb->getBlockTable(blktbl, AcDb::kForRead);
		if (es != Acad::eOk)
		{
			acutPrintf(_T("\n初始获取块表失败"));
			break;
		}

		// 取出文件名的字符指针
		const wchar_t * f = fitor->c_str();

		// 如果文件名是NULL
		// 那么直接生成位置变换
		if (!wcsncmp(f, L"NULL", 4))
		{
			GenTrans(pos_r, pos_c, 420, 297, 8);
			continue;
		}

		// 判断这个文件名所对应的块是不是已经存在了
		// 防止无限插入的情况
		CxCHAR *name = (CxCHAR*)f;
		if (blktbl->has(name))
		{
			acutPrintf(_T("\n块记录%s已经存在"), name);
			blktbl->close();
			//goto OpRcdFinish;
			continue;
		}



		// noDocument 一定要为true，否则不能插入, 
		// defaultdrawing一定要为false，否则数据库不为空
		pNewDb = new AcDbDatabase(Adesk::kFalse, true);
		// 分配临时数据库的内存
		if (pNewDb == NULL)
		{
			acutPrintf(_T("\n打开dwg文件失败"));
			return;
		}

		// 判断这个文件是exb还是dwg文件
		crxutPrintf(_T("\n读入文件: %s\n"), f);

		// 提前测试一下文件是否存在
		// 否则会出现打开空指针而崩溃的情况
		FILE *f_test = _wfopen(f, L"r");
		if (!f_test)
		{
			crxutPrintf(_T("\n文件%s不存在"), f);
			fclose(f_test);

			return;
		}
		else
		{
			fclose(f_test);
		}

		// 检查文件类型
		int ftype = 0;
		ftype = CheckFile(f);

		// 根据文件类型使用对应的文件类型函数
		// 读入文件到pNewDb这个数据库中
		Acad::ErrorStatus es;
		switch (ftype)
		{
		case FTYPE_DWG:
			es = pNewDb->readDwgFile(f, _SH_DENYRD); break;
		case FTYPE_EXB:
			es = pNewDb->readExbFile(f, _SH_DENYRD); break;
		default:
			crxutPrintf(_T("\n不支持的文件格式: %s\n"), f);
			return;
		}


		if (Acad::eOk == es && NULL != pNewDb)
		{
			acutPrintf(_T("\n打开cad文件成功"));

			AcDbObjectId id;
			//CxCHAR *name = (CxCHAR*)f;
			{
				es = pDb->insert(id, name, pNewDb);
				if (es != Acad::eOk)
					acutPrintf(_T("\n读入块错误"));
				else
					acutPrintf(_T("\n读入块%s成功"), name);
			}
			blktbl->getAt(name, id);

			// 创建一条新的块表引用记录
			// 引用指向当前读入的文件块
			CRxGePoint3d ptInsert;
			ptInsert = GenTrans(pos_r, pos_c, 420, 297, 8);// 按照A3大小，一行8张图计算
			CRxDbBlockReference *pBlkRef = new CRxDbBlockReference();
			pBlkRef->setPosition(ptInsert);
			pBlkRef->setBlockTableRecord(id);

			// 将块参照添加到模型空间
			CRxDbBlockTableRecord *pBlkTblRcd;
			blktbl->getAt(CRXDB_MODEL_SPACE, pBlkTblRcd, CRxDb::kForWrite);
			CRxDbObjectId entId;
			es = pBlkTblRcd->appendAcDbEntity(entId, pBlkRef);
			pBlkRef->close(); pBlkTblRcd->close();

			// 
			if (es != Acad::eOk)
			{
				acutPrintf(_T("插入文件%s错误"), name);
			}



		OpRcdFinish:
			// 一次操作完成，关闭块表
			blktbl->close();

			/*
			AcDbBlockTable *pBlkTbl = NULL;
			if (pDba->getBlockTable(pBlkTbl, AcDb::kForRead) != Acad::eOk)
			{
			acutPrintf(_T("\n后续打开块表错误"));
			}
			if (!pBlkTbl->has(name))
			{
			acutPrintf(_T("\n插入块不存在"));
			}
			else
			{
			acutPrintf(_T("\n找到记录%s"), name);
			}
			pBlkTbl->close();
			*/
		}
		else
		{
			acutPrintf(_T("\n打开cad文件失败,错误码:%d"), (es));
		}
		delete pNewDb; pNewDb = NULL;
	}
}

// 检查字符串对应的文件的类型
// 检查方法是判断末尾的三字符
int CheckFile(const wchar_t * w)
{
	// 字符串全部都是使用长字符传送的
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
// 按照tar文件的指示来合并文件
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
			// %hs用来把char字符串装进wchar字符串
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

	// swe 的导出的文件
	// 命名格式至少是这样
	// 1.dwg 
	// 1 - 1.dwg
	// 1 - 2.dwg
	// 2.dwg
	// 2 - 1.dwg
	// 3.dwg
	// 4.dwg
	// ……
	// 

	// 这个函数永远返回true
	// 需要根据文件列表的长度来判断到底有无文件
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
		// 后缀需要做8页对齐，
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
		// 所以读到不存在的文件后
		// 需要补到余数为7上，这之间的文件名全部为NULL
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

	// 读入文件列表初始化
	bool f_isready = GenFilesBySwe(flist);
	if (!f_isready)
	{
		acutPrintf(_T("\n目标文件列表初始化失败"));
		return;
	}

	TeReadPagesByFlist(flist);

	acutPrintf(_T("读入结束"));
	return;

}

void TeReadPage()
{
	std::list<wstring> flist;

	// 读入文件列表初始化
	bool f_isready = GenFilesByTar(flist);
	if (!f_isready)
	{
		acutPrintf(_T("\n目标文件列表初始化失败"));
		return;
	}

	TeReadPagesByFlist(flist);

	acutPrintf(_T("读入结束"));
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
		acutPrintf(_T("\n插件菜单已经存在"));
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