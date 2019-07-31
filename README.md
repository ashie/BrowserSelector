# BrowserSelector

## What's this?

This is a web-browser launcher application for Microsoft Windows that aims to
select an appropriate browser for an URL. Since many tranditional intranet web
applications still depend on Internet Explorer, such enterprise users need to
switch IE and modern web-browsers such as Mozilla Firefox or Google Chrome
according to the situation. This application will do it automatically on
clicking or entering an URL. Currently only Firefox is supported as the second
browser. In addition, we recommend to use it with the Firefox's add-on
[IE View WE](https://addons.mozilla.org/en-US/firefox/addon/ie-view-we/) to
switch from Firefox to IE.

## How to Build

  * Open BrowserSelector.sln by Microsoft Visual Studio 2010 (or later)
  * Select "Release" (or "Debug") from the toolbar
  * Press F7 key

## How to Install

Run built BrowserSelectorBHO/Release/BrowserSelectorSetup.msi or setup.exe.

## How to Configure

### FQDN or URL patterns

You need to set FQDN or URL patterns of you intranet to open them by IE.
The patterns are stored in the registry of Windows.

Please see the following examples for more detail:

  * [For 32bit OS](sample/BrowserSelectorExample.reg)
  * [For 64bit OS](sample/BrowserSelectorWOW64Example.reg)

The following wildcard characters are supported for specifying FQDN or URL:

  * `*`: matches any string
  * `?`: matches any single character

### Default Web Browser

You need to set BrowserSelector as your default browser. The way to do it is
depends on your version of Windows.
