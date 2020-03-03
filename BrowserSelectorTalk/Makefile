CFLAGS=/DUNICODE /D_UNICODE
CC=cl

DESTDIR=%%ProgramFiles%%\ClearCode\BrowserSelector

PROG=BrowserSelectorTalk.exe DummyTalker.exe

all: build

build: $(PROG)

install: build registry
	!if not exist "$(DESTDIR)" mkdir "$(DESTDIR)"
	COPY BrowserSelectorTalk.exe "$(DESTDIR)"
	COPY manifest/BrowserSelectorTalkChrome.json "$(DESTDIR)"
	COPY manifest/BrowserSelectorTalkFirefox.json "$(DESTDIR)"

# Both Firefox and Chrome query 32bit view first, and then 64bit view.
# So we can just use WOW3264Node here (even on 64bit app).
registry:
	REG ADD "HKLM\Software\Mozilla\NativeMessagingHosts\com.clear_code.browserselector_talk" /f /ve /t REG_SZ /d "$(DESTDIR)\BrowserSelectorTalkFirefox.json"
	REG ADD "HKLM\Software\Google\Chrome\NativeMessagingHosts\com.clear_code.browserselector_talk" /f /ve /t REG_SZ /d "$(DESTDIR)\BrowserSelectorTalkChrome.json"

test: build
	.\DummyTalker.exe "Q firefox https://example.com" | .\BrowserSelectorTalk.exe

BrowserSelectorTalk.exe: BrowserSelectorTalk.cpp ../BrowserSelector/BrowserSelectorCommon.h
	$(CC) $(CFLAGS) /Fe:BrowserSelectorTalk.exe BrowserSelectorTalk.cpp

DummyTalker.exe: DummyTalker.c
	$(CC) $(CFLAGS) /Fe:DummyTalker.exe DummyTalker.c

clean:
	DEL $(PROG)
