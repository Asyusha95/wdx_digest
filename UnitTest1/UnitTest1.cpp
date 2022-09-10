
#include "pch.h"

#define _CRT_SECURE_NO_WARNINGS
#include "CppUnitTest.h"

#include <crc32.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(UnitMethod1)
		{
			const char* buf =  "123456789";
			int len = 9;

			crc_t Check = 0xCBF43926UL;
			crc_t didgest = 0;
			didgest = crc32(
				didgest,
				(const unsigned char*)buf,
				(size_t)len);
			Assert().IsTrue(didgest == Check);
		}

		TEST_METHOD(UnitMethod2)
		{
			const char* buf = "123456788";
			int len = 9;

			crc_t Check = 0xCBF43926UL;
			crc_t didgest = 0;
			didgest = crc32(
				didgest,
				(const unsigned char*)buf,
				(size_t)len);
			Assert().IsTrue(didgest == Check);
		}
	};
}
