#include "stdafx.h"
#include "common.h"

wstring AsciiToUnicode(_In_ CONST string& pszText)
{
    wstring str;
    int cchWideChar = MultiByteToWideChar(CP_ACP, 0, pszText.c_str(), -1, NULL, 0) + 1;

    WCHAR* ptszText = new wchar_t[cchWideChar];
    ZeroMemory(ptszText, cchWideChar * sizeof(WCHAR));

    MultiByteToWideChar(CP_ACP, 0, pszText.c_str(), -1, ptszText, cchWideChar);

    ptszText[cchWideChar - 1] = '\0';
    str = ptszText;
    delete[] ptszText;

    return str;
}

string UnicodeToAscii(_In_ CONST wstring& pszText)
{
    string str;
    int iTextLen;
    iTextLen = WideCharToMultiByte(CP_ACP, 0, pszText.c_str(), -1, NULL, 0, NULL, 0);
    char* lText = (char*)calloc(iTextLen, sizeof(char));
    memset(lText, 0, iTextLen*sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, pszText.c_str(), -1, lText, iTextLen, NULL, 0);
    str = lText;
    free(lText);
    return str;
}

wchar_t* AnsiToUnicode(char *str)
{
    DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    wchar_t *pwText;
    pwText = new wchar_t[dwNum];
    if (!pwText)
    {
        delete[]pwText;
    }
    MultiByteToWideChar(CP_ACP, 0, str, -1, pwText, dwNum);
    return pwText;
}
//wchar_t *strUnicode = AnsiToUnicode(str);
/*
//Unicode×ªansi
wchar_t wText[20] = { L"¿í×Ö·û×ª»»ÊµÀý!" };
DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
char *psText;
psText = new char[dwNum];
if (!psText)
{
    delete[]psText;
}
WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
delete[]psText;

*/