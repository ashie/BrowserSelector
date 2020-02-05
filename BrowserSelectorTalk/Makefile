CFLAGS=/DUNICODE /D_UNICODE
CC=cl

DESTDIR=%%ProgramFiles%%\ClearCode\BrowserSelector
REGKEY=HKLM\Software\Mozilla\NativeMessagingHosts\BrowserSelectorTalk

PROG=BrowserSelectorTalk.exe DummyTalker.exe

all: build

build: $(PROG)

install: build
	!if not exist "$(DESTDIR)" mkdir "$(DESTDIR)"
	COPY BrowserSelectorTalk.exe "$(DESTDIR)"
	COPY BrowserSelector.json "$(DESTDIR)"
	REG ADD $(REGKEY) /f /ve /t REG_SZ /d "$(DESTDIR)\BrowserSelector.json"

test: build
	.\DummyTalker.exe "Q firefox https://example.com" | .\BrowserSelectorTalk.exe

BrowserSelectorTalk.exe: BrowserSelectorTalk.cpp
	$(CC) $(CFLAGS) /Fe:BrowserSelectorTalk.exe BrowserSelectorTalk.cpp

DummyTalker.exe: DummyTalker.c
	$(CC) $(CFLAGS) /Fe:DummyTalker.exe DummyTalker.c

clean:
	DEL $(PROG)
