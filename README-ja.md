# BrowserSelector

## これは何？

[BrowserSelctor](https://gitlab.com/clear-code/browserselector)はMicrosoft
Windows上で動作するウェブブラウザラウンチャーです。開くURLに応じて適切なウェブ
ブラウザを選択します。イントラネット用ウェブアプリケーションでは今なおInternet
Explorer（以下IE）のみを対象としたものが数多く存在するため、企業ユーザーは状況
に応じてIEとモダンブラウザ（Mozilla FirefoxやGoogle Chrome）を切り替えて使用す
る必要性に迫られています。このアプリケーションは、URLをクリックしたり、ロケーシ
ョンエントリにURLを入力したりする際に自動的に適切なウェブブラウザを選択します。

このソフトウェアは以下の2つのモジュールから成ります。

  * BrowserSelector.exe:
    ウェブブラウザラウンチャーです。
    既定のウェブブラウザとして設定して使用します。
  * BrowserSelectorBHO.dll:
    IE用のBHO（ブラウザーヘルパーオブジェクト）です。
    URLを開く際に、自動的にMozilla FirefoxやGoogle Chromeに切り替えます。

このソフトウェアは主にIEから他のウェブブラウザへの切り替えの部分のみを担いま
す。他のウェブブラウザからIEへの切り替えは、他のソフトウェアと組み合わせて実
現することを想定しています。
セカンドブラウザとしてMozilla Firefoxを使用したい場合は、IE View WEを使用するこ
とをお勧めします。

  * IE View WE: https://addons.mozilla.org/en-US/firefox/addon/ie-view-we/

Google Chromeでの同等の拡張機能としてはIE Tabがあります。

  * IE Tab: https://www.ietab.net/

## ビルド方法

  * Microsoft Visual Studio 2010でBrowserSelector.slnを開きます
  * ツールバーの「ソリューション構成」で「リリース」を選択します
  * F7キーを押下してビルドします

## インストール方法

インストーラは BrowserSelectorSetup\Release\BrowserSelectorSetup.msi にビルドさ
れています。これを実行してインストールします。

## 既定のウェブブラウザの変更

あらゆるアプリケーションから使用するウェブブラウザを自動選択したい場合は、デス
クトップの既定のウェブブラウザをBrowserSelector.exeに変更する必要があります。
その方法は使用するOSや管理方法によって異なりますので、以下のページ等を参考にし
て下さい。

  * https://support.microsoft.com/en-us/help/4028606/windows-10-change-your-default-browser
  * https://docs.microsoft.com/en-us/internet-explorer/ie11-deploy-guide/set-the-default-browser-using-group-policy

BHOのみを使用してBrowserSelector.exeが必要無い場合、この手順は必要ありません。

## 設定方法

このソフトウェアにはユーザーインターフェースがありません。システム管理者がレジ
ストリ設定を直接管理し、ユーザーには設定を編集させないことを想定しています。

32ビット版のBrowserSelect（通常はこちらが既定のビルド）を使用している場合、レジ
ストリキーは以下になります。

  * `HKEY_LOCAL_MACHINE`（`HKLM`） あるいは `HKEY_CURRENT_USER`（`HKCU`）
    * 64ビットOSの場合: `SOFTWARE\WOW6432Node\ClearCode\BrowserSelector`
    * 32ビットOSの場合: `SOFTWARE\ClearCode\BrowserSelector`

HKLMとHKCUの両方に設定が存在する場合、それらの設定は統合されます（`HKCU`の設定
で上書きされます）。

設定例は以下のファイルを参照して下さい。

  * [64ビットOSの場合](sample/BrowserSelectorWOW64Example.reg)
  * [32ビットOSの場合](sample/BrowserSelectorExample.reg)

## 設定項目

### `HostNamePatterns` および `URLPatterns`

#### 単純なワイルドカードによるパターンマッチング

特定のURLを指定したウェブブラウザで開かせるために、ホスト名やURLのパターンを設
定する必要があります。これらの設定はレジストリキー`HostNamePatterns`あるいは
`URLPatterns`配下に文字列値として格納します。文字列値の名前には任意の文字列を
使用できますが、適用順序を明確化するために`0001`といった数字を使用することをお
勧めします。文字列値のデータにはホスト名やURLのパターンをセットします。また、
「|」を区切り文字として、末尾に使用するブラウザ名を付加することもできます。

例)

  * `HostNamePatterns`
    * `0001` = `*.example.org|firefox`
  * URLPatterns
    * `0001` = `http://*.example.com|firefox`

ホスト名やURL名のパターンを指定するために、以下のワイルドカードがサポートされて
います。

  * `*`: 任意の文字列にマッチします
  * `?`: 任意の一文字にマッチします

ブラウザ名としては、以下の値がサポートされています:

  * `ie`: Internet Explorer
  * `firefox`: Mozilla Firefox
  * `chrome`: Google Chrome

ブラウザ名を指定しない場合は後述の`DefaultBrowser`や`SecondBrowser`が使用されま
す。

#### 正規表現

ホスト名やURLのパターンを指定する方法として、単純なワイルドカードではなく、正規
表現を使用することもできます。正規表現を使用したい場合は、BrowserSelectorのレジ
ストリキー直下に以下のDWORD値をセットします:

  * `"UseRegex"` = `dword:00000001`

正規表現の具体的な文法については以下を参照して下さい:

  * https://ja.cppreference.com/w/cpp/regex/ecmascript

正規表現を使用する場合にも、使用するブラウザを個別に指定することができます。
以下のように、ホスト名やURL名の終端を表す`$`アサーションの直後にブラウザ名を
記述して下さい:

  * `0001` = `^http(s)?://(.+\\.)?example\\.(org|com)(/.*)?$firefox`

### `DefaultBrowser`

`BrowserSelector`キー直下に文字列値`DefaultBrowser`をセットすることで、パターン
に一致しなかった場合に使用するブラウザを指定することができます。

例)

  * `DefaultBrowser` = `firefox`

`DefaultBrowser`の既定の値は`ie`です。

### `SecondBrowser`

ホスト名やURLパターンでブラウザを指定しなかった場合、既定では`DefaultBrowser`で
指定したブラウザが使用されます。パターン毎に個別にブラウザを指定せずに、パター
ンにマッチしたURLに対して別のブラウザを使用したい場合は、`BrowserSelector`キー
直下に文字列値`SecondBrowser`をセットして下さい。

例)

  * `URLPagtterns`
    * `0001` = `http://*.example.com`
    * `0002` = `http://*.example.org`
    * ...
  * `SecondBrowser` = `chrome`

`SecondBrowser`の既定の値は空です（`DefaultBrowse`が使用されます）。
