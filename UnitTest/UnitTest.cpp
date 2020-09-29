#include "pch.h"
#include "CppUnitTest.h"
#include "../BrowserSelector/BrowserSelectorCommon.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(MatchSimpleWildCard)
	{
	public:

		TEST_METHOD(WildCard)
		{
			wstring url(L"https://www.example.com");
			wstring pattern(L"https://*.example.com");
			Assert::IsTrue(matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(UnmatchedHost)
		{
			wstring url(L"https://www2.example.com");
			wstring pattern(L"https://www.example.com");
			Assert::IsFalse(matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveScheme)
		{
			wstring url(L"HTTPS://www.example.com");
			wstring pattern(L"https://*.example.com");
			Assert::IsTrue(matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveHostName)
		{
			wstring url(L"https://WWW.example.com");
			wstring pattern(L"https://www.example.com");
			Assert::IsTrue(matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveDomain)
		{
			wstring url(L"https://www.EXAMPLE.COM");
			wstring pattern(L"https://www.example.com");
			Assert::IsTrue(matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitivePath)
		{
			wstring url(L"https://www.example.com/Path/To/RESOURCE");
			wstring pattern(L"https://www.example.com/path/to/resource");
			Assert::IsTrue(matchSimpleWildCard(url, pattern));
		}
		TEST_METHOD(UNCPath)
		{
			wstring url(L"\\\\shared\\folder");
			wstring pattern(L"\\\\\\\\shared\\\\folder");
			Assert::IsTrue(matchSimpleWildCard(url, pattern));
		}
	};
}
