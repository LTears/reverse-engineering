// hook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <shellapi.h>
#include <shlwapi.h>

#pragma comment(lib,"shlwapi.lib")

#define MYDLLEXPORT extern "C" __declspec(dllexport)

typedef int (WINAPI* SHFILEOPERATION)(LPSHFILEOPSTRUCTW lpFileOp);
Thunk GlobalThunk;


int WINAPI InjectCheck(LPSHFILEOPSTRUCTW lpFileOp)
{//SHFileOperation����Ǽ����������滻SHFileOperatio
	int errorcode=((SHFILEOPERATION)&GlobalThunk.codeseg.codedata)(lpFileOp);
	if(errorcode == ERROR_ACCESS_DENIED || errorcode == ERROR_SHARING_VIOLATION)
	{
		DWORD nRet=GetLastError();
		if(lpFileOp->wFunc ==  FO_MOVE || lpFileOp->wFunc == FO_DELETE || lpFileOp->wFunc == FO_RENAME)
		{
			if(nRet == ERROR_INVALID_HANDLE || nRet == ERROR_SUCCESS)//MSDN��˵��Ҫ���getlasterror������
			{
				SHELLEXECUTEINFOW ExecInfo;
				WCHAR szPath[1024],file[1024],fparam[1024];
				memset(&ExecInfo,0,sizeof(SHELLEXECUTEINFOW));
				ExecInfo.cbSize=sizeof(SHELLEXECUTEINFOW);
				ExecInfo.lpVerb=L"open";
				GetModuleFileNameW(GlobalThunk.hmod,szPath,1024);
				PathRemoveFileSpecW(szPath);
				wsprintfW(file,L"\"%s\\Unlocker.exe\"",szPath);
				ExecInfo.lpFile=file;
				wsprintfW(fparam,L"\"%s\"",lpFileOp->pFrom);
				ExecInfo.lpParameters=fparam;
				ExecInfo.nShow=SW_SHOWNORMAL;
				ShellExecuteExW(&ExecInfo);
			}
		}
	}
	return errorcode;
}

BOOL CALLBACK EnumFunc(HWND hwnd,LPARAM lParam)
{
#define UNKNOWNMSG		0x8005//�μ�unlockerassitant����
	TCHAR String[256];
	GetWindowText(hwnd,String,256);
	if(!lstrcmp(String,"UnlockerAssistant"))
		PostMessage(hwnd,UNKNOWNMSG,0,0);
	return TRUE;
}

LRESULT CALLBACK CBTProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	return CallNextHookEx(GlobalThunk.hhook,nCode,wParam,lParam);
}

MYDLLEXPORT void HookInstall()
{
	GlobalThunk.hhook=SetWindowsHookEx(WH_CBT,(HOOKPROC)CBTProc,GlobalThunk.hmod,0);
}

MYDLLEXPORT void HookUninstall()
{
	UnhookWindowsHookEx(GlobalThunk.hhook);
}

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				WCHAR String[1024];
				HMODULE hmod;
				GlobalThunk.hmod=(HMODULE)hModule;
				DisableThreadLibraryCalls((HMODULE)hModule);
				lstrcpyW(String,L"");
				GetModuleFileNameW(NULL,String,1024);
				if(lstrlenW(String))
				{
					PathStripPathW(String);
					if(!lstrcmpiW(String,L"explorer.exe") && (hmod=GetModuleHandle("Shell32.dll")))
					{
						DWORD flOldProtect;
						GlobalThunk.FuncBaseAddress=GetProcAddress(hmod,"SHFileOperationW");
						if(!GlobalThunk.FuncBaseAddress || !VirtualProtect(&GlobalThunk,11,PAGE_EXECUTE_READWRITE,&flOldProtect))
						{//�޸����ݶ�Ϊ��������пɶ�дȨ��
							CloseHandle(hmod);
							return TRUE;
						}
						HANDLE hProcess=GetCurrentProcess();
						BYTE Buffer[5];//�޸�SHFileOperation��ǰ���ֽڣ�ʹ������ת��InjectCheck
						ReadProcessMemory(hProcess,(LPVOID)GlobalThunk.FuncBaseAddress,&GlobalThunk,6,NULL);
						if(GlobalThunk.codeseg.codedata[3] == 0x83 && GlobalThunk.codeseg.codedata[4] == 0xEC)
						{
/*
��֪��32λxp��SHFileOperationW�����Ļ�����ǰ10�ֽ�:558BEC83EC2C5356578B7D0833C0 ��Ӧ�����룺
	seg000:00000000                 push    ebp
	seg000:00000001                 mov     ebp, esp
	seg000:00000003                 sub     esp, 2Ch
	seg000:00000006                 push    ebx		
	seg000:00000007                 push    esi
	seg000:00000008                 push    edi
	seg000:00000009                 mov     edi, [ebp+8]
	seg000:0000000C                 xor     eax, eax
	............................................
*/
							int offset=((int)GlobalThunk.FuncBaseAddress+6)-int(GlobalThunk.codeseg.codedata+11);
/*GlobalThunk.codeseg.codedata+11������ִ��GlobalThunk����λ�ã��൱���Ѿ�ִ����SHFileOperation��һ����
jmpָ��ռ5�ֽڣ���GlobalThunk���ص�SHFileOperation����ִ������Ĳ��֣���jmp��ƫ�ƴ�jmp�¾俪ʼ�㣬
�������codedata+0000000B����SHFileOperationҪִ�е�λ����FuncBaseAddress+00000006��push    ebx
thunk�е�ָ�
	seg000:00000000                 push    ebp
	seg000:00000001                 mov     ebp, esp
	seg000:00000003                 sub     esp, 2Ch
	seg000:00000006					jmp		offset
	seg000:0000000B					00000000000
	............................................
*/
//���²�����Ŀ����Ҫ�ǽ�SHFileOperation�𿪣���ÿ�γ������SHFileOperation�Ϳ���ִ��InjectCheckһ��
							GlobalThunk.codeseg.xp.jmp=0xE9;//д��jmpָ�������
							memcpy(&GlobalThunk.codeseg.xp.offset,&offset,4);//д��jmp��offset
							FlushInstructionCache(hProcess,&GlobalThunk,11);
							offset=(int)InjectCheck-((int)GlobalThunk.FuncBaseAddress+5);//��GlobalThunk�Ĵ�������InjectCheck����
							Buffer[0]=0xE9;//д����תָ��
							memcpy(Buffer+1,&offset,4);
							WriteProcessMemory(hProcess,GlobalThunk.FuncBaseAddress,Buffer,5,NULL);
							FlushInstructionCache(hProcess,GlobalThunk.FuncBaseAddress,5);
						}
						else if((GlobalThunk.codeseg.codedata[3] == 0x8B && GlobalThunk.codeseg.codedata[4] == 0xEC) || 
									GlobalThunk.codeseg.codedata[3] == 0x53 || GlobalThunk.codeseg.codedata[4] == 0x55)
						{//32λwin7 ������ԭ�����ƣ�����������
							int offset=((int)GlobalThunk.FuncBaseAddress+5)-int(GlobalThunk.codeseg.codedata+10);
							GlobalThunk.codeseg.win7.jmp=0xE9;
							memcpy(&GlobalThunk.codeseg.win7.offset,&offset,4);
							FlushInstructionCache(hProcess,&GlobalThunk,11);
							offset=(int)InjectCheck-((int)GlobalThunk.FuncBaseAddress+5);//��GlobalThunk�Ĵ�������InjectCheck����
							Buffer[0]=0xE9;//д����תָ��
							memcpy(Buffer+1,&offset,4);
							WriteProcessMemory(hProcess,GlobalThunk.FuncBaseAddress,Buffer,5,NULL);
							FlushInstructionCache(hProcess,GlobalThunk.FuncBaseAddress,5);
						}
						else
						{
							EnumWindows((WNDENUMPROC)EnumFunc,0);
						}
						CloseHandle(hmod);
					}
				}
/*
ִ��˳�򣺢٢ڢۢܢ�   ���������൱���滻SHFileOperationΪInjectCheck
��������ĺ���Ӧ�����������£�
GlobalThunk.CodeSeg����:
push	ebp.//����������
mov		ebp,esp
sub		esp,2Ch
jmp		SHFileOperation//�ܼ���ִ��ʣ�µ�SHFileOperation
..........�����޴���

SHFileOperation����:
jmp		InjectCheck//��һ���г�����øú�������ֱ����
push	ebx//�������������ִ��
push	esi
...ʡ��

InjectCheck����:
GlobalThunk.CodeSeg(LPSHFILEOPSTRUCTW lpFileOp);//����������ִ��GlobalThunk.CodeSeg��ʵҲ����ִ��SHFileOperation��ǰ����
...��鷵��ֵ//�޷���InjectCheck
...
...ʡ��

*/
			}
			break;

		case DLL_PROCESS_DETACH:
			if(GlobalThunk.FuncBaseAddress)
			{
				HANDLE hproc=GetCurrentProcess();
				WriteProcessMemory(hproc,(LPVOID)GlobalThunk.FuncBaseAddress,&GlobalThunk,5,NULL);//����ʱ�ָ�SHFileOperationԭʼ������
				FlushInstructionCache(hproc,(LPVOID)GlobalThunk.FuncBaseAddress,5);
			}
			break;

		default:
			break;
	}

    return TRUE;
}