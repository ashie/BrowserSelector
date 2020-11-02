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
			DefaultConfig config;
			BrowserSelector app(config);
			wstring url(L"https://www.example.com");
			wstring pattern(L"https://*.example.com");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(UnmatchedHost)
		{
			DefaultConfig config;
			BrowserSelector app(config);
			wstring url(L"https://www2.example.com");
			wstring pattern(L"https://www.example.com");
			Assert::IsFalse(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveScheme)
		{
			DefaultConfig config;
			BrowserSelector app(config);
			wstring url(L"HTTPS://www.example.com");
			wstring pattern(L"https://*.example.com");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveHostName)
		{
			DefaultConfig config;
			BrowserSelector app(config);
			wstring url(L"https://WWW.example.com");
			wstring pattern(L"https://www.example.com");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitiveDomain)
		{
			DefaultConfig config;
			BrowserSelector app(config);
			wstring url(L"https://www.EXAMPLE.COM");
			wstring pattern(L"https://www.example.com");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}

		TEST_METHOD(CaseInsensitivePath)
		{
			DefaultConfig config;
			BrowserSelector app(config);
			wstring url(L"https://www.example.com/Path/To/RESOURCE");
			wstring pattern(L"https://www.example.com/path/to/resource");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}
		TEST_METHOD(UNCPath)
		{
			DefaultConfig config;
			BrowserSelector app(config);
			wstring url(L"\\\\shared\\folder");
			wstring pattern(L"\\\\\\\\shared\\\\folder");
			Assert::IsTrue(app.matchSimpleWildCard(url, pattern));
		}
	};

	TEST_CLASS(Config)
	{
	public:
		TEST_METHOD(DumpAsJson)
		{
			DefaultConfig config;
			std::wstring buf;
			config.dumpAsJson(buf);
			Assert::AreEqual(
				L"{\"DefaultBrowser\":\"ie\",\"SecondBrowser\":\"\",\"FirefoxCommand\":\"\",\"CloseEmptyTab\":1,\"OnlyOnAnchorClick\":0,\"UseRegex\":0,\"URLPatterns\":[],\"HostNamePatterns\":[],\"ZonePatterns\":[]}",
				buf.c_str());
		}
	};
}
