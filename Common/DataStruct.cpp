#include "stdafx.h"
#include "DataStruct.h"

void Dbgprintf(char *pszFormat, ...)
{
#ifdef _DEBUG
    char szbufFormat[0x1000];
    char szbufFormat_Game[0x1008] = "Log:";
    va_list argList;
    va_start(argList, pszFormat);
    vsprintf_s(szbufFormat, pszFormat, argList);
    //printf(szbufFormat);
    strcat_s(szbufFormat_Game, szbufFormat);
    OutputDebugStringA(szbufFormat_Game);
    va_end(argList);
#endif
}


bool RecvData(SOCKET s, char* pBuf, int nSize)
{
    int nRecvedSize = 0;
    int nTotalSize = 0;

    while (nTotalSize < nSize)
    {
        nRecvedSize = recv(s, pBuf + nTotalSize, nSize - nTotalSize, 0);
        if (nRecvedSize == 0)
        {
            //表示优雅退出
            return false;
        }
        else if (nRecvedSize == SOCKET_ERROR)
        {
            return false;
        }
        nTotalSize += nRecvedSize;
    }
    return true;
}

bool SendData(SOCKET s, char* pBuf, int nSize)
{
    int nSendedSize = 0;
    int nTotalSize = 0;

    while (nTotalSize < nSize)
    {
        nSendedSize = send(s, pBuf + nTotalSize, nSize - nTotalSize, 0);
        if (nSendedSize == SOCKET_ERROR)
        {
            //表示优雅退出
            return false;
        }
        nTotalSize += nSendedSize;
    }
    return true;
}


bool MakeSureDirectoryPathExists(LPCTSTR pszDirPath)
{
    LPTSTR p, pszDirCopy;
    DWORD dwAttributes;

    // Make a copy of the string for editing.

    __try
    {
        pszDirCopy = (LPTSTR)malloc(sizeof(TCHAR) * (lstrlen(pszDirPath) + 1));

        if (pszDirCopy == NULL)
            return FALSE;

        lstrcpy(pszDirCopy, pszDirPath);

        p = pszDirCopy;

        //  If the second character in the path is "\", then this is a UNC
        //  path, and we should skip forward until we reach the 2nd \ in the path.

        if ((*p == TEXT('\\')) && (*(p + 1) == TEXT('\\')))
        {
            p++;            // Skip over the first \ in the name.
            p++;            // Skip over the second \ in the name.

            //  Skip until we hit the first "\" (\\Server\).

            while (*p && *p != TEXT('\\'))
            {
                p = CharNext(p);
            }

            // Advance over it.

            if (*p)
            {
                p++;
            }

            //  Skip until we hit the second "\" (\\Server\Share\).

            while (*p && *p != TEXT('\\'))
            {
                p = CharNext(p);
            }

            // Advance over it also.

            if (*p)
            {
                p++;
            }

        }
        else if (*(p + 1) == TEXT(':')) // Not a UNC.  See if it's <drive>:
        {
            p++;
            p++;

            // If it exists, skip over the root specifier

            if (*p && (*p == TEXT('\\')))
            {
                p++;
            }
        }

        while (*p)
        {
            if (*p == TEXT('\\'))
            {
                *p = TEXT('\0');
                dwAttributes = GetFileAttributes(pszDirCopy);

                // Nothing exists with this name.  Try to make the directory name and error if unable to.
                if (dwAttributes == 0xffffffff)
                {
                    if (!CreateDirectory(pszDirCopy, NULL))
                    {
                        if (GetLastError() != ERROR_ALREADY_EXISTS)
                        {
                            free(pszDirCopy);
                            return FALSE;
                        }
                    }
                }
                else
                {
                    if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // Something exists with this name, but it's not a directory... Error
                        free(pszDirCopy);
                        return FALSE;
                    }
                }

                *p = TEXT('\\');
            }

            p = CharNext(p);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // SetLastError(GetExceptionCode());
        free(pszDirCopy);
        return FALSE;
    }

    free(pszDirCopy);
    return TRUE;
}