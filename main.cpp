// Merge sections
//#pragma comment(linker, "/MERGE:.rdata=.data")
//#pragma comment(linker, "/MERGE:.text=.data")

// Favour small code
//#pragma optimize("gsy", on)


#include "stdafx.h"
#include "cunicode.h"
#include "contentplug.h"
#include "other.h"
#include <stdlib.h>
#include <list>
#include <string>

//typedef long time_t;
typedef std::list<std::pair<std::string, std::string>> SignList;
#define _detectstring ("[0]=\"M\" & [1]=\"Z\"")

#define CRC32 0
#define FIELDS 1

HINSTANCE hinst = nullptr;
SignList* signList = nullptr;

#define _DAY_SEC           (24 * 60 * 60)       /* secs in a day */
#define _YEAR_SEC          (365 * _DAY_SEC)     /* secs in a year */
#define _FOUR_YEAR_SEC     (1461 * _DAY_SEC)    /* secs in a 4 year interval */
#define _BASE_DOW          4                    /* 01-01-70 was a Thursday */

const char* ListeElement[] = {"CRC32"};

static char* strlcpy(char* str1, const char* str2, int maxSize)
{
	if ((int)strlen(str2) >= maxSize - 1)
	{
		strncpy(str1, str2, maxSize - 1);
		str1[maxSize - 1] = 0;
	}
	else
	{
		strcpy(str1, str2);
	}
	return str1;
}

static char* strlcat(char* str1, const char* str2, int maxSize)
{
	int l1 = (int)strlen(str1);
	if ((int)strlen(str2) >= maxSize - 1 - l1)
	{
		strncat(str1, str2, maxSize - 1 - l1);
		str1[maxSize - 1] = 0;
	}
	else
	{
		strcat(str1, str2);
	}
	return str1;
}

static __inline void Int2Hex(unsigned int Value, int Digits, char* Buf, int maxSize)
{
	if (Digits == 4)
		_snprintf(Buf, maxSize - 1, "0x%04X", Value);
	else if (Digits == 8)
		_snprintf(Buf, maxSize - 1, "0x%08X", Value);
}

static __inline void Int2Str(unsigned int Value, char* Buf, int maxSize)
{
	_snprintf(Buf, maxSize - 1, "%u", Value);
}

static __inline void MM2Str(unsigned short int Major, unsigned short int Minor, char* Buf, int maxSize)
{
	_snprintf(Buf, maxSize - 1, "%u.%02u", Major, Minor);
}

static __inline int pow2(const int ch)
{
	return 2 << ch;
}

static bool cmp(const char* str1, const char* str2)
{
	const char* s1 = str1;
	const char* s2 = str2;
	while (((*s1) == (*s2)) || ((*s1) == ':') || ((*s2) == ':')
		|| ((*s1) == ']') || ((*s2) == ']')
		|| ((*s1) == '\0') || ((*s2) == '\0'))
	{
		if (((*s1) == '\0') || ((*s2) == '\0'))
			break;
		s1++;
		s2++;
	}
	if (((*s1) != '\0') && ((*s2) != '\0'))
		return false;
	return true;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH :
		{
			hinst = (HINSTANCE) hModule;
			DisableThreadLibraryCalls(hinst);
			break;
		}

		case DLL_PROCESS_DETACH :
			break;
	}
	return TRUE;
}

int __stdcall ContentGetDetectString(char* DetectString, int maxLen)
{
	strlcpy(DetectString, _detectstring, maxLen + 1);
	return 0;
}

void __stdcall ContentSetDefaultParams(ContentDefaultParamStruct* dps)
{
//	MakeSignList();
}

void __stdcall ContentPluginUnloading(void)
{
	if (signList)
	{
		signList->clear();
		delete signList;
		signList = nullptr;
	}
}

int __stdcall ContentGetSupportedField(int FieldIndex, char* FieldName, char* Units, int maxSize)
{
	if ((FieldIndex < 0) || (FieldIndex >= FIELDS))
		return ft_nomorefields;
	strlcpy(FieldName, ListeElement[FieldIndex], maxSize);
	Units[0] = '\0';
	//switch (FieldIndex)
	//{
	//	case 1  : strlcpy(Units, DosHeader, maxSize); break;
	//	case 3  : strlcpy(Units, FileHeader, maxSize); break;
	//	case 4  : strlcpy(Units, OptionalHeader, maxSize); break;
	//	case 9  : strlcpy(Units, LEHeader, maxSize); break;
	//	default : Units[0] = '\0';
	//}
	return ft_string;
}

int __stdcall ContentGetValue(char* FileName, int FieldIndex, int UnitIndex, void* FieldValue, int maxSize, int flags)
{
	wchar_t FileNameW[wdirtypemax];
	int retVal = ContentGetValueW(awfilenamecopy(FileNameW, FileName), FieldIndex, UnitIndex, FieldValue, maxSize, flags);
	if (retVal == ft_numeric_floating)
	{
		wchar_t* p = _wcsdup((const wchar_t*)((char*)FieldValue + sizeof(double)));
		walcopy((char*)FieldValue + sizeof(double), p, maxSize - sizeof(double));
		free(p);
	}
	return retVal;
}



#include <crc32.h>

////////////////////////

#include <memory.h>
#include <malloc.h>
#include <wchar.h>
#include <io.h>

template <typename digest_t>
int fgetdg(const wchar_t* file, digest_t (__cdecl *getdg)(digest_t, ptr_t, size_t), digest_t* ptr_digest)
{
	int retval = 0;
	FILE* hfile = nullptr;
	ptr_t buffer = nullptr;
	intptr_t hfind = 0;
	_wfinddata_t data;
	try
	{
		if ((file == nullptr) || (getdg == nullptr) || (ptr_digest == nullptr)) throw (int)EINVAL;
		digest_t& digest = *ptr_digest;
		hfind = _wfindfirst(file, &data);
		if (hfind == 0)  throw (int)ENFILE;
		buffer = (ptr_t)malloc(BUFSIZ);
		if (buffer == nullptr)  throw (int)ENOMEM;
		hfile = _wfopen(file, L"rb");
		if (hfile == nullptr)  throw (int)EACCES;
		fseek(hfile, 0, SEEK_SET);
		do
		{
			size_t ready = fread((void*)buffer, sizeof(char), BUFSIZ, hfile);
			if (ferror(hfile)) throw (int)EIO;
			digest =
				getdg(digest, buffer, ready);
		} while (!feof(hfile));
		retval = 1;
	}
	catch (int err)
	{
		_set_errno(err);
	};
	if (hfile != nullptr) { fclose(hfile); hfile = nullptr; };
	if (hfind != 0) { _findclose(hfind); hfind = 0; };
	if (buffer != nullptr) { memset((void*)buffer, '\0', _msize((void*)buffer)); free((void*)buffer); buffer = nullptr; };
	return retval;
}

int __stdcall ContentGetValueW(wchar_t* FileName, int FieldIndex, int UnitIndex, void* FieldValue, int maxSize, int flags)
{
	if ((FieldIndex < 0) || (FieldIndex >= FIELDS))
		return ft_nomorefields;
	if (flags & CONTENT_DELAYIFSLOW)
		return ft_delayed;


	if (FieldIndex == CRC32)
	{

		crc_t digest = 0;
		if(fgetdg(FileName, &crc32, &digest))
		{
			_snprintf((char*)FieldValue, 8, "%08x", digest);
			return ft_string;
		}
		else ft_fileerror;
	}

	return ft_fileerror;
}



