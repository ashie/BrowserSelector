#include "../BrowserSelector/BrowserSelectorCommon.h"
#include <io.h>
#include <fcntl.h>

using namespace std;

/*
 * Create a new process for Internet Explorer.
 *
 * We need to use CREATE_BREAKAWAY_FROM_JOB in order to prevent the
 * newly-created process getting killed by Firefox.
 *
 * https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/Native_messaging#Closing_the_native_app
 */
static bool OpenIE(const wstring &url)
{
	int ret;
	wstring iepath;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	LoadBrowserPath(iepath, _T("ie"));

	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	if (url.find('"') !=  std::wstring::npos) {
		fprintf(stderr, "URL contains a double quote: '%ls'", url.c_str());
		return false;
	}

	wstring args(L" \"");
	args += url;
	args += L"\"";

	ret = CreateProcess(iepath.c_str(), /* lpApplicationName */
						const_cast<LPWSTR>(args.c_str()), /* lpCommandLine */
						NULL,           /* lpProcessAttributes  */
						NULL,           /* lpThreadAttributes */
						FALSE,          /* bInheritHandles */
						CREATE_BREAKAWAY_FROM_JOB,
						NULL,           /* lpEnvironment */
						NULL,           /* lpCurrentDirectory */
						&si,            /* lpStartupInfo */
						&pi);           /* lpProcessInformation */
	if (ret == 0) {
		fprintf(stderr, "failed to open '%ls' (%d)", url.c_str(), GetLastError());
		return false;
	}

	return true;
}

static bool BrowserOpen(const wstring browser, const wstring &url, const Config *config)
{
	if (browser != L"ie")
		return OpenByModernBrowser(browser, url, *config);

	if (!OpenByExistingIE(url))
		return OpenIE(url);

	return true;
}

/*
 * Implement "BroserSelector Talk" Protocol.
 */
static void TalkResponse(const char *msg, ...)
{
	va_list args;
	int len;

	/* Prevent Windows from translating CRLF */
	setmode(_fileno(stdout), O_BINARY);

	va_start(args, msg);

	// Print 4-byte header
	len = vsnprintf(NULL, 0, msg, args);
	fwrite(&len, sizeof(len), 1, stdout);

	// Push the remaining body to stdout
	vfprintf(stdout, msg, args);
	fflush(stdout);
	va_end(args);
}

static int HandleTalkQuery(wchar_t *wcmd, const Config *config)
{
	wchar_t *space;

	if (wcslen(wcmd) < 5) {
		fprintf(stderr, "request too short '%ls'", wcmd);
		return -1;
	}

	/*
	 *  Q firefox https://google.com
	 *    -------
	 */
	space = wcschr(wcmd + 2, L' ');
	if (!space) {
		fprintf(stderr, "invalid query request '%ls'", wcmd);
		return -1;
	}
	*space = L'\0';

	wstring origin(wcmd + 2);

	/*
	 *  Q firefox https://google.com
	 *            ------------------
	 */
	wstring url(space + 1);

	wstring browser = ::GetBrowserNameToOpenURL(url, *config);
	if (browser == origin) {
		TalkResponse("{\"status\":\"OK\",\"open\":0}");
	} else if (BrowserOpen(browser, url, config)) {
		TalkResponse("{\"status\":\"OK\",\"open\":1,\"close_tab\":%d}", config->m_closeEmptyTab);
	} else {
		fprintf(stderr, "cannot open '%ls' with '%ls'", url.c_str(), browser.c_str());
		return -1;
	}
	return 0;
}

static int HandleTalkConfig(wchar_t *wcmd, const Config *config)
{
	std::wstring buf(1024, '\0'); /* 1kb pre-allocation */
	config->dumpAsJson(buf);
	TalkResponse("{\"status\":\"OK\",\"config\":%ls}", buf.c_str());
	return 0;
}

static int HandleTalkProtocol(const Config *config)
{
	int len;
	int ret = -1;
	char *cmd = NULL;
	wchar_t *wcmd = NULL;

	/* Read Native Messaging string */
	if (fread(&len, sizeof(len), 1, stdin) < 1) {
		fprintf(stderr, "cannot read %i bytes", sizeof(len));
		return -1;
	}

	/* The shortest input is "C" (including quotes) */
	if (len < 3) {
		return -1;
	}

	cmd = (char *) calloc(1, len + 1);
	if (cmd == NULL) {
		perror("calloc");
		return -1;
	}

	if (fread(cmd, len, 1, stdin) < 1) {
		fprintf(stderr, "cannot read %i bytes", len);
		free(cmd);
		return -1;
	}

	/* Convert to wchar_t */
	wcmd = (wchar_t *) calloc(1, sizeof(wchar_t) * (len - 1));
	if (wcmd == NULL) {
		perror("calloc");
		free(cmd);
		return -1;
	}

	/* Here we trim double-quotes from '"Q firefox http://example.com"' */
	if (!MultiByteToWideChar(CP_UTF8, 0, cmd + 1, len - 2, wcmd, len - 1)) {
		fprintf(stderr, "cannot convert '%s' to wchar", cmd);
		free(cmd);
		free(wcmd);
		return -1;
	}

	switch (wcmd[0]) {
	case L'Q':
		ret = HandleTalkQuery(wcmd, config);
		break;
	case L'C':
		ret = HandleTalkConfig(wcmd, config);
		break;
	default:
		fprintf(stderr, "unknown command '%ls'", wcmd);
		break;
	}
	free(cmd);
	free(wcmd);
	return ret;
}

int main(int argc, char *argv[])
{
	Config config;
	config.LoadAll();
	return HandleTalkProtocol(&config);
}
