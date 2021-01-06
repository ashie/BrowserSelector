# BrowserSelector

## What's this?

[BrowserSelctor](https://gitlab.com/clear-code/browserselector) is a
web-browser launcher for Microsoft Windows that aims to select an appropriate
browser for an URL. Since many traditional intranet web applications still
depend on Internet Explorer, so such enterprise users need to switch IE and
modern web-browsers (such as Mozilla Firefox or Google Chrome) according to
the situation. This application will do it automatically on clicking or
entering an URL.

This software consists of two modules:

  * BrowserSelector.exe:
    The web-browser launcher. Should be set as the default web-browser on
    your desktop.
  * BrowserSelectorBHO.dll:
    A BHO (Browser Helper Object) for IE. It will launch Mozilla Firefox
    or Google Chrome to open internet sites.

BrowserSelector is fully integrated with the Firefox/Chrome extension
[IEView WE](https://github.com/clear-code/ieview-we). With this addon
combined, you can smoothly switch between IE and Chrome/Firefox.

## How to Build

### Prerequisites

  * Microsoft Visual Studio 2019
    * C++ MFC for v142 build tools (x86 & x64)
  * Visual Studio extension "Microsoft Visual Studio Installer Projects"
    * https://marketplace.visualstudio.com/items?itemName=VisualStudioClient.MicrosoftVisualStudio2017InstallerProjects

### Build Steps

  * Open BrowserSelector.sln by Microsoft Visual Studio 2019
  * Select "Release" (or "Debug") from the toolbar
  * Press F7 key

or you can do it in command line:

  * Open "Developer Command Prompt for VS 2019" from the start menu
  * `cd \path\to\BrowserSelector`
  * `devenv.com BrowserSelector.sln /build Release`

## How to Install

Run built BrowserSelectorSetup\Release\BrowserSelectorSetup.msi.

## Change the Default Browser

If you want to switch web browsers automatically from all applications, you
need to set BrowserSelector.exe as the default browser on your desktop.
How to do it depends on your OS and situation. For example:

  * https://support.microsoft.com/en-us/help/4028606/windows-10-change-your-default-browser
  * https://docs.microsoft.com/en-us/internet-explorer/ie11-deploy-guide/set-the-default-browser-using-group-policy

If you use only the BHO and don't use BrowserSelector.exe, it's not needed.

## How to Configure

This software doesn't have any UI. System administrators have to edit its
registry entries or INI files directly and shouldn't allow users to edit them.
Configurations are loaded from following locations by this order:

  * Registry: `HKEY_LOCAL_MACHINE` (`HKLM`)
    * HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\ClearCode\BrowserSelector
	* (For 32bit OS: HKEY_LOCAL_MACHINE\SOFTWARE\ClearCode\BrowserSelector)
  * INI file: BrowserSelector.ini under the application folder
    * e.g.) C:\Program Files (x86)\ClearCode\BrowserSelector\BrowserSelector.ini
	* It doesn't exist by default. Please create it manually.
  * Registry: `HKEY_CURRENT_USER` (`HKCU`)
    * HKEY_CURRENT_USER\SOFTWARE\WOW6432Node\ClearCode\BrowserSelector
    * (For 32bit OS: HKEY_CURRENT_USER\SOFTWARE\ClearCode\BrowserSelector)
  * INI file: BrowserSelector.ini under a user's AppData folder
    * e.g.) C:\Users\[UserName]\AppData\Roaming\ClearCode\BrowserSelector\BrowserSelectror.ini
	* These folder or file don't exist by default. Please create them manually.

If two or more configurations exist, they are merged (overridden by later one).

Please see the following files by way of example:

  * [For 64bit OS](sample/BrowserSelectorWOW64Example.reg)
  * [For 32bit OS](sample/BrowserSelectorExample.reg)
  * [INI file](sample/BrowserSelector.ini)
  
Available config items are almost same.
One of difference between them is that the items which are placed at the top of
the registry key, are placed at `[Common]` section in INI file.

## Config items

### `HostNamePatterns` or `URLPatterns`

#### Simple Pattern Matching

You need to set hostname or URL patterns to open them by specific browsers.
They are stored under the registry key `HostNamePatterns` and `URLPatterns` as
string values. Although you can use any characters as the value names, we
recommend to use numbers such as `0001` to clarify the priority. The value is
hostname or URL pattern. Optionally you can add a browser name to open the URL
pattern by splitting the value by `|`.

e.g.)

  * `HostNamePatterns`
    * `0001` = `*.example.org|firefox`
  * URLPatterns
    * `0001` = `http://*.example.com|firefox`

The following wildcard characters are supported for specifying host names or
URLs:

  * `*`: matches any string
  * `?`: matches any single character

For browser names, following values are supported:

  * `ie`
  * `firefox`
  * `chrome`

When the browser name is unspecified, `DefaultBrowser` or `SecondBrowser` will
be used as described later.

#### Regular Expression

You can use also regular expression for hostname or URL patterns when you set
the following registry value under the top of `BrowserSelector` key:

  * `"UseRegex"` = `dword:00000001`

Please see the following page for the grammar of the regular expression:

  https://en.cppreference.com/w/cpp/regex/ecmascript

A browser names can be added in this case too. Please add a browser name after
`$` assertion like this:

  * `0001` = `^http(s)?://(.+\\.)?example\\.(org|com)(/.*)?$firefox`

### `ZonePatterns`

Use this option to map browsers to security zones in Internet Explorer.
The configuration syntax is the same as URLPatterns and HostNamePatterns.

e.g.)

  * `0001` = `intra|ie`
  * `0002` = `internet`

The following security zone names are supported.

  * Type: String
    * `local`
    * `intra`
    * `trusted`
    * `internet`
    * `restricted`

Note that `URLPatterns` and `HostNamePatterns` take precedence over
`ZonePatterns`.

### `DefaultBrowser`

You can change the browser for opening unmatched URLs by setting the registry
value `DefaultBrowser` under the top of `BrowserSelector` key.

  * Type: String
    * `ie`
    * `firefox`
    * `chrome`

e.g)

The default value of `DefaultBrowser` is `ie`.

### `SecondBrowser`

If you leave the browser name empty for each URL patterns, the browser
specified by `DefaultBrowser` will be used for them. If you want to change the
browser for such URL patterns, you can do it by the registry value
`SecondBrowser` under the top of `BrowserSelector` key.

  * Type: String
    * Empty (Default)
    * `ie`
    * `firefox`
    * `chrome`

e.g.)

  * `SecondBrowser` = `chrome`
  * `URLPagtterns`
    * `0001` = `http://*.example.com`
    * `0002` = `http://*.example.org`
    * ...

### `FirefoxCommand`

If you have a multiple versions of Firefox installed on your system, you can
use this option to specify which Firefox you want to launch.

 * Type: String
   * Empty (Default)
   * Local Windows Path Fromat: C:\path\to\firefox.exe

### `CloseEmptyTab`

Whether or not to close the remaining empty tab after an alternate browser is
launched.

  * Type: DWORD
    * 1: Close (Default)
    * 0: Don't close

### `OnlyOnAnchorClick`

Whether or not to launch an alternate browser only on clicking an link.

  * Type: DWORD
    * 0: Launch an alternate browser on all navigations
    * 1: Launch an alternate browser only on clicking an link

### `Include`

The path of external INI file to load additionally. Note that it does not
support recursive loading (`Include` in the child config is ignored).

  * Type: String
    * Both following formats are available.
    * Local Windows Path Fromat: C:\path\to\file.ini
    * UNC Format: \\HostName\ShareName\path\to\file.ini

### `EnableIncludeCache`

Whether or not to use a cache file when a specified external INI file isn't
available.

  * Type: DWORD
    * 0: Don't Use (Default)
    * 1: Use

The cache files are stored under a user's LocalAppDataLow folder.

  e.g.) C:\Users\[UserName]\AppData\LocalLow\ClearCode\BrowserSelector\BrowserSelectror.ini
