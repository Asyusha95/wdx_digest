#define wdirtypemax 1024
#define longnameprefixmax 6

#ifndef countof
#define countof(str) (sizeof(str)/sizeof(str[0]))
#endif // countof

wchar_t* wcslcpy(wchar_t* str1, const wchar_t* str2, int maxSize);
wchar_t* wcslcat(wchar_t* str1, const wchar_t* str2, int maxSize);
char* walcopy(char* outname, const wchar_t* inname, int maxSize);
wchar_t* awlcopy(wchar_t* outname, const char* inname, int maxSize);

#define wafilenamecopy(outname, inname) walcopy(outname, inname, countof(outname))
#define awfilenamecopy(outname, inname) awlcopy(outname, inname, countof(outname))

BOOL MakeExtraLongNameW(wchar_t* outbuf, const wchar_t* inbuf, int maxSize);
