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
			BrowserSelector app;
			wstring url(L"https://www.example.com");
			wstring pattern(L"https://*.example.com");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(UnmatchedHost)
		{
			BrowserSelector app;
			wstring url(L"https://www2.example.com");
			wstring pattern(L"https://www.example.com");
			Assert::IsFalse(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveScheme)
		{
			BrowserSelector app;
			wstring url(L"HTTPS://www.example.com");
			wstring pattern(L"https://*.example.com");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveHostName)
		{
			BrowserSelector app;
			wstring url(L"https://WWW.example.com");
			wstring pattern(L"https://www.example.com");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveDomain)
		{
			BrowserSelector app;
			wstring url(L"https://www.EXAMPLE.COM");
			wstring pattern(L"https://www.example.com");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitivePath)
		{
			BrowserSelector app;
			wstring url(L"https://www.example.com/Path/To/RESOURCE");
			wstring pattern(L"https://www.example.com/path/to/resource");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}
		TEST_METHOD(UNCPath)
		{
			BrowserSelector app;
			wstring url(L"\\\\shared\\folder");
			wstring pattern(L"\\\\\\\\shared\\\\folder");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}
	};
}
