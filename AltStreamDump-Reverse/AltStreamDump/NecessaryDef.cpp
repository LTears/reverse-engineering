#include <stdio.h>
#include "NecessaryDef.h"

extern HANDLE hConsoleOutput;

MainClass::MainClass()
{
	FolderPath[0]=L'\0';
	SubfoldersDepth=0;
	NtQueryInfo=NULL;
	CurrentDirectory[0]=L'\0';
	GetCurrentDirectoryW(MAX_PATHLEN,CurrentDirectory);
	SearchSubFolders=FALSE;
	wcsncpy(FolderPath,CurrentDirectory,260);
}

void MainClass::SetParam(WCHAR* paramtype,WCHAR* paramdata)
{
	if(!wcscmp(paramtype,L"-d"))
	{
		SearchSubFolders=TRUE;
		SubfoldersDepth=_wtoi(paramdata);
	}
	if(!wcscmp(paramtype,L"-f"))
	{
		wcsncpy(FolderPath,paramdata,260);
	}
}

void MainClass::GoSearch()
{
	NotBeginSearch=FALSE;
	TotalStreamBytes=0;
	TotalStreamNum=0;
	mem9=0;
	mem10=0;
	RecurseFind(FolderPath,0);
	ShowStaticInfo();
}

NUMBERFMTW GlobalLocale;
BOOL HasGotLocale=FALSE;
WCHAR Data1[16],Data2[16],Data3[16];

void Int64ToString(LPWSTR lpNumberStr,LONGLONG num)
{//������ʾ
	WCHAR Value[256];
	memset(Value,0,sizeof(Value));
	_snwprintf(Value,255,L"%I64d",num);

	if(!HasGotLocale)
	{
		GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_ILZERO,Data1,16);
		GlobalLocale.LeadingZero=_wtoi(Data1);
		GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_SGROUPING,Data1,16);
		GlobalLocale.Grouping=_wtoi(Data1);
		GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,Data2,16);
		GlobalLocale.lpDecimalSep=Data2;
		GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_STHOUSAND,Data3,16);
		GlobalLocale.lpThousandSep=Data3;
		GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_INEGNUMBER,Data1,16);
		GlobalLocale.NegativeOrder=_wtoi(Data1);
		HasGotLocale=TRUE;
	}

	NUMBERFMTW fmt;
	memcpy(&fmt,&GlobalLocale,sizeof(NUMBERFMTW));
	fmt.NumDigits=0;
	GetNumberFormatW(LOCALE_USER_DEFAULT,0,Value,&fmt,lpNumberStr,32);
}

void MainClass::ShowStaticInfo()
{//��ʾ����ͳ����Ϣ
	WCHAR String1[256],String2[256],DestStr[1024];
	memset(DestStr,0,sizeof(DestStr));
	memset(String1,0,sizeof(String1));
	memset(String2,0,sizeof(String2));
	Int64ToString(String1,TotalStreamNum);
	Int64ToString(String2,TotalStreamBytes);
	_snwprintf(DestStr,1023,L"\r\n   %s streams, %s bytes",String1,String2);
	wprintf(DestStr);
}

void LinkStringToFullPath(LPWSTR dststr,LPCWSTR srcstr1,LPCWSTR srcstr2)
{//���ӳ��ļ�����·����srcstr1Ϊ·�� srcstr2Ϊ�ļ��� 
	wcscpy(dststr,srcstr1);
	int len=wcslen(dststr);
	if(len > 0 && dststr[len-1] != L'\\')
		wcscat(dststr,L"\\");//L'\\' ????
	wcscat(dststr,srcstr2);
}

void MainClass::RecurseFind(LPWSTR path,int CurrentDepth)
{//���ݷ���������Ŀ¼����Ŀ¼
	if(NotBeginSearch) return;
	ShowCurrentFileStream(path);//��ʾ�����������ļ���
	GetStream(path);//��ȡ��ǰ�ļ����ļ���
	FindFileClass findfile;
	findfile.GenSearchString(path);//�����ڵ�ǰĿ¼���ҵ��ַ��������濪ʼѰ���ļ�
	while(findfile.EnumNextFile() && !NotBeginSearch)
	{
		if(!findfile.IsDir())
		{
			LPCWSTR cFileName,CAlterName;
			WCHAR str[261];
			if(!wcscmp(findfile.FindData.cFileName,L"..") && findfile.FindData.cAlternateFileName[0])
			{//�����8.3�ɸ�ʽ�ļ���
				memset(str,0,sizeof(str));
				if(wcslen(path)+wcslen(findfile.FindData.cAlternateFileName)+1 >= 260)
					str[0]=L'\0';
				else
					LinkStringToFullPath(str,path,findfile.FindData.cAlternateFileName);//���ӳ�����·��8.3�ļ���
				cFileName=findfile.CurrentFileName;//����·����ͨ�ļ���
				CAlterName=str;
			}
			else
			{//��������ͨ�ļ���
				CAlterName=findfile.CurrentFileName;
				cFileName=findfile.CurrentFileName;
			}
			GetStream(CAlterName,cFileName);
		}
	}

	//��������findfile�Ѿ��ı䣬������Ҫ���������ڵ�ǰĿ¼���ҵ��ַ��������濪ʼѰ���ļ��У������е㱿����^_^
	findfile.GenSearchString(path);
	while(findfile.EnumNextFile() && !NotBeginSearch)
	{	
		if(findfile.IsDir() && SearchSubFolders && (SubfoldersDepth == 0 || SubfoldersDepth > CurrentDepth))
			RecurseFind(findfile.CurrentFileName,CurrentDepth+1);//���ݲ����ļ���
	}
	findfile.ReleaseFile();//�ͷŲ����ļ����
}

void MainClass::AdaptStatic(StreamStruct* mystream)
{//��ͳ�����ֽ��и��£������ļ������������ļ����ֽڴ�С��ͬʱ��ʾ��ǰ������
	WCHAR String1[1024],String2[1024],NumberStr[100];
	memset(String1,0,sizeof(String1));
	memset(String2,0,sizeof(String2));
	memset(NumberStr,0,sizeof(NumberStr));
	Int64ToString(NumberStr,mystream->StreamSize);
	LPWSTR ptr=String1;
	if(!mystream->IsDirectory)
	{
		LPWSTR tmp=mystream->FileName;
		LPWSTR pos=mystream->FileName;
		while(*tmp)
		{
			if(*tmp == L'\\' || *tmp == L'/')
				pos=tmp+1;
			tmp++;
		}
		wcscpy(String1,pos);
		ptr=String1+wcslen(String1);
	}
	wcsncpy(ptr,mystream->StreamName,260);
	int len=wcslen(ptr);
	if(len > 6 && ptr[len-6] == ':' && ptr[len-5] == '$')
	{
		ptr[len-6]=L'\0';
	}
	_snwprintf(String2,1023,L"%8s %s\r\n",NumberStr,String1);
	wprintf(String2);
	TotalStreamBytes += mystream->StreamSize;
	TotalStreamNum++;
}

void MainClass::ShowCurrentFileStream(LPWSTR FolderPath)
{//��ʾ��ǰ�����������ļ���
	SetConsoleTextAttribute(hConsoleOutput,FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
	wprintf(L"\r\nScanning Folder: ");
	SetConsoleTextAttribute(hConsoleOutput,FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	wprintf(FolderPath);
	SetConsoleTextAttribute(hConsoleOutput,FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	wprintf(L"\r\n");
}

void WINAPI MainClass::GetStream(LPCWSTR FileName)
{//���ļ�
	HANDLE hFile=CreateFileW(FileName,0,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		GetStreamData(hFile,FileName,TRUE);
		CloseHandle(hFile);
	}
}

void WINAPI MainClass::GetStream(LPCWSTR cAlterName,LPCWSTR cFileName)
{//���ļ���
	HANDLE hFile=CreateFileW(cAlterName,0,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		GetStreamData(hFile,cFileName,FALSE);
		CloseHandle(hFile);
	}
}

void WINAPI MainClass::GetStreamData(HANDLE hFileHandle,LPCWSTR FileName,BOOL IsDir)
{//��������������ȡ���ļ���������Ϣ
	BYTE data[32*512];//32����
	memset(data,0,sizeof(data));
	FILE_STREAM_INFORMATION* itor=(FILE_STREAM_INFORMATION*)data;//����
	if(!NtQueryInfo)
	{
		HMODULE hmod=GetModuleHandleW(L"ntdll.dll");
		NtQueryInfo=(NTQUERYINFORMATIONFILE)GetProcAddress(hmod,"NtQueryInformationFile");
	}
	if(!NtQueryInfo)
		return;

	IO_STATUS_BLOCK IoStatusBlock;
	NtQueryInfo(hFileHandle,&IoStatusBlock,data,sizeof(data),FileStreamInformation);
	while(itor->StreamNameLength)
	{
		WCHAR str[261];
		memset(str,0,sizeof(str));
		memcpy(str,itor->StreamName,itor->StreamNameLength);
		str[itor->StreamNameLength/2]=L'\0';
		StreamStruct mystream;
		memset(&mystream,0,sizeof(StreamStruct));
		wcsncpy(mystream.StreamName,str,260);
		mystream.StreamAllocationSize=itor->StreamAllocationSize.QuadPart;
		mystream.StreamSize=itor->StreamSize.QuadPart;
		if(wcscmp(str,L"::$DATA"))
		{
			wcsncpy(mystream.FileName,FileName,260);
			mystream.IsDirectory=IsDir;
			AdaptStatic(&mystream);
		}
		if(itor->NextEntryOffset == 0)
			break;
		itor=(FILE_STREAM_INFORMATION*)((BYTE*)itor+itor->NextEntryOffset);
	}
}

FindFileClass::FindFileClass()
{
	hFindFile=INVALID_HANDLE_VALUE;
	LastFileName[0]=L'\0';
	CurrentFolderPath[0]=L'\0';
	CurrentFileName[0]=L'\0';
}

void FindFileClass::ReleaseFile()
{
	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFindFile);
		hFindFile=INVALID_HANDLE_VALUE;
	}
}

bool FindFileClass::IsDir()
{
	DWORD flag=FindData.dwFileAttributes;
	return (flag&FILE_ATTRIBUTE_DIRECTORY) && !(flag&FILE_ATTRIBUTE_REPARSE_POINT) &&
		wcscmp(FindData.cFileName,L"..") && wcscmp(FindData.cFileName,L".");
}

BOOL FindFileClass::EnumNextFile()
{//ö���¸��ļ����ļ���
	if(hFindFile == INVALID_HANDLE_VALUE)
	{
		if((hFindFile=FindFirstFileW(LastFileName,&FindData)) == INVALID_HANDLE_VALUE)
			return FALSE;
	}
	else if(!FindNextFileW(hFindFile,&FindData))
	{
		ReleaseFile();
		return FALSE;
	}
	if(wcslen(FindData.cFileName)+wcslen(CurrentFolderPath)+1 > 324)
		CurrentFileName[0]=L'\0';
	else
		LinkStringToFullPath(CurrentFileName,CurrentFolderPath,FindData.cFileName);
	return TRUE;
}

void FindFileClass::GenSearchString(LPCWSTR Path)
{//���ɲ��ҵ�ǰ�ļ����������ļ����ļ��еĲ����ַ���
	ReleaseFile();
	wcsncpy(CurrentFolderPath,Path,323);
	if(wcslen(CurrentFolderPath)+wcslen(L"*.*") > 324)
		LastFileName[0]=L'\0';
	else
		LinkStringToFullPath(LastFileName,CurrentFolderPath,L"*.*");
}
