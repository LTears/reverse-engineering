// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2ADEAEDE_D507_4201_8E51_D93D95FD91A9__INCLUDED_)
#define AFX_STDAFX_H__2ADEAEDE_D507_4201_8E51_D93D95FD91A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

struct Thunk
{
	union//Ҫ��ִ̬�еĴ���
	{//�˴���xp��win7����ΪSHFileOperationW����ʵ������
		struct
		{//32λxp
			BYTE	origincode[6];//xp��SHFileOperationW����ǰ6�ֽڻ�����
			BYTE	jmp;//��תָ��E9
			int		offset;//Ϊjmpָ�����ƫ��
		}xp;
		
		struct
		{//32λwin7
			BYTE	origincode[5];//win7��SHFileOperationW����ǰ5�ֽڻ�����
			BYTE	jmp;//��תָ��E9
			int		offset;//Ϊjmpָ�����ƫ��
		}win7;
		
		BYTE codedata[12];
	}codeseg;
	
	DWORD	initfuncnum;//�˴�����
	HMODULE hmod;
	HHOOK	hhook;
	FARPROC	FuncBaseAddress;
};

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2ADEAEDE_D507_4201_8E51_D93D95FD91A9__INCLUDED_)
