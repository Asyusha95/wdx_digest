#include "StdAfx.h"
#include "cunicode.h"

char* walcopy(char* outname, const wchar_t* inname, int maxSize)
{
	if (inname) {
		WideCharToMultiByte(CP_ACP, 0, inname, -1, outname, maxSize, NULL, NULL);
		outname[maxSize - 1] = 0;
		return outname;
	} else
		return nullptr;
}

wchar_t* awlcopy(wchar_t* outname, const char* inname, int maxSize)
{
	if (inname) {
		MultiByteToWideChar(CP_ACP, 0, inname, -1, outname, maxSize);
		outname[maxSize - 1] = 0;
		return outname;
	} else
		return nullptr;
}

wchar_t* wcslcpy(wchar_t* str1, const wchar_t* str2, int maxSize)
{
	if ((int)wcslen(str2) >= maxSize - 1) {
		wcsncpy(str1, str2, maxSize - 1);
		str1[maxSize - 1] = 0;
	} else
		wcscpy(str1, str2);
	return str1;
}

wchar_t* wcslcat(wchar_t* str1, const wchar_t* str2, int maxSize)
{
	int l1 = (int)wcslen(str1);
	if ((int)wcslen(str2) >= maxSize - 1 - l1) {
		wcsncat(str1, str2, maxSize - 1 - l1);
		str1[maxSize - 1] = 0;
	} else
		wcscat(str1, str2);
	return str1;
}

// return true if name wasn't cut
BOOL MakeExtraLongNameW(wchar_t* outbuf, const wchar_t* inbuf, int maxSize)
{
	if (wcslen(inbuf) > 259) {
		if (inbuf[0] == '\\' && inbuf[1] == '\\') {   // UNC-Path! Use \\?\UNC\server\share\subdir\name.ext
			wcslcpy(outbuf, L"\\\\?\\UNC", maxSize);
			wcslcat(outbuf, inbuf + 1, maxSize);
		} else {
			wcslcpy(outbuf, L"\\\\?\\", maxSize);
			wcslcat(outbuf, inbuf, maxSize);
		}
	} else
		wcslcpy(outbuf, inbuf, maxSize);
	return (int)wcslen(inbuf) + 3 <= maxSize;
}
