# BrowserSelector

## What's this?

[BrowserSelctor](https://github.com/clear-code/BrowserSelector) is a
web-browser launcher for Microsoft Windows that aims to select an appropriate
browser for an URL. Since many tranditional intranet web applications still
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

If you use Mozilla Firefox as a second browser, we recommend to use them with
[IE View WE](https://addons.mozilla.org/en-US/firefox/addon/ie-view-we/) to
launch IE from Firefox to open intranet sites.
The equivalent for Google Chrome is [IE Tab](https://www.ietab.net/).

## How to Build

  * Open BrowserSelector.sln by Microsoft Visual Studio 2010 (or later)
  * Select "Release" (or "Debug") from the toolbar
  * Press F7 key

## How to Install

Run built BrowserSelectorSetup/Release/BrowserSelectorSetup.msi or setup.exe.

## Change the Default Browser

After the installation, you need to set BrowserSelector.exe as the default
browser on your desktop:

  * https://support.microsoft.com/en-us/help/4028606/windows-10-change-your-default-browser
  * https://docs.microsoft.com/en-us/internet-explorer/ie11-deploy-guide/set-the-default-browser-using-group-policy

If you use only the BHO and don't use BrowserSelector.exe, it's not needed.

## How to Configure

This software doesn't have any UI. System administrators have to edit its
registry entries directly and shouldn't allow users to edit them.

If you build 32bit binary of BrowserSelector (it's the default), the registry
key is:

* `HKEY_LOCAL_MACHINE` or `HKEY_CURRENT_USER`
  * for 64bit OS: `SOFTWARE\WOW6432Node\ClearCode\BrowserSelector`
  * for 32bit OS: `SOFTWARE\ClearCode\BrowserSelector`

If a same named value exists in both `HKLM` adn `HKCU`, they will be merged
(overriten by `HKCU`'s one).

Please see the following *.reg files by way of example:

  * [For 64bit OS](sample/BrowserSelectorWOW64Example.reg)
  * [For 32bit OS](sample/BrowserSelectorExample.reg)

### `HostNamePatterns` or `URLPatterns`

You need to set host name or URL patterns to open them by specific browsers.
They are stored under the registry key `HostNamePatterns` and `URLPatterns` as
string values. Although you can use any characters as the value names, we
recommend to use numbers such as `0001` to clarify the priority. The value is
URL pattern. Optionally you can add a browser name to open the URL pattern by
spliting the value by `|`.

e.g.)

  * `HostNamePatterns`
    * `0001` = `*.example.org|firefox`
  * URLPatterns
    * `0001` = `http://*.example.com|firefox`

The following wildcard characters are supported for specifying host name or URL:

  * `*`: matches any string
  * `?`: matches any single character

The following values are supported as browser names:

  * `ie`
  * `firefox`
  * `chrome`

### `DefaultBrowser`

You can change the browser for opening unmatched URLs by setting the registry
value "DefaultBrowser".

e.g)

  * `DefaultBrowser` = `firefox`

The default value is `ie`.

### `SecondBrowser`

If you leave the browser name empty for each URL patterns, the browser
specified by `DefaultBrowser` will be used for them. If you change the browser
for such URL patterns, you can do it by the registry value `SecondBrowser`.

e.g.)

  * `URLPagtterns`
    * `http://*.example.com` = ``
    * `http://*.example.org` = ``
    * ...
  * `SecondBrowser` = `chrome`

The default value is empty (use `DefaultBrowser`).