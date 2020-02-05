BrowserSelectorTalk
===================

A native messaging host program for WebExtension addons.

Installation
------------

Launch developer command prompt (bundled with Microsoft Visual Studio)
and run the following commands.

    $ nmake
    $ nmake test

Then install the required resources:

    $ nmake install

Now you can use BrowserSelector with Firefox. Try install the latest
version of IEView-WE.

https://github.com/clear-code/ieview-we

BrowserSelector Talk Protocol
-----------------------------

This is a simple protocol for WebExtension addons to communicate with
BrowserSelector.

*SERVER/CLIENT* This protocol models BrowserSelector as "server" and
browser addons as "client".

*MESSAGE FORMAT* A client request is just a normal JSON string, while
server responses being JSON objects.

### Query command

Query BrowserSelector about the target URL.

REQUEST

    "Q chrome https://google.com"
     - ------ ------------------
       Origin    Target URL

RESPONSE

    {"status": "OK", "open": true, "close_tab": true}
    {"status": "OK", "open": false}

TODO
----

* Make the absolute path for IE (`IE_PATH`) configurable.
* Create a XPI package and get it registered on mozilla.org.
* Support Google Chrome
* Support Microsoft Edge
* Implement "Config" command to pass configurations to addons.
* Confirm that the call for CreateProcess() is not insecure.
