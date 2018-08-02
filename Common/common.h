
#include <Windows.h>
#include <string>

using namespace std;

wstring AsciiToUnicode(_In_ CONST string& pszText);
string UnicodeToAscii(_In_ CONST wstring& pszText);
//ANSIתunicode  
wchar_t* AnsiToUnicode(char *str);
