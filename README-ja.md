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

BrowserSelectorは、FirefoxおよびChrome用ブラウザエクステンションの
[IEView WE](https://github.com/clear-code/ieview-we)と統合されています。
アドオンと併用することで、IEとChrome/Firefoxを相互に行き来できます。

## ビルド方法

### 必要なもの

  * Microsoft Visual Studio 2019
    * v142ビルドツール用C++ MFC (x86およびx64)
  * Visual Studio拡張機能「Microsoft Visual Studio Installer Projects」
    * https://marketplace.visualstudio.com/items?itemName=VisualStudioClient.MicrosoftVisualStudio2017InstallerProjects

### ビルド手順

  * Microsoft Visual Studio 2010でBrowserSelector.slnを開きます
  * ツールバーの「ソリューション構成」で「リリース」を選択します
  * F7キーを押下してビルドします

あるいはコマンドラインで実行することもできます。

  * スタートメニューから「Developer Command Prompt for VS 2019」を開く
  * `cd \path\to\BrowserSelector`
  * `devenv.com BrowserSelector.sln /build Release`

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
ストリ設定あるいはINI形式の設定ファイルを直接管理し、ユーザーには設定を編集させ
ないことを想定しています。設定は以下の箇所から、以下の順番で読み込まれます。

  * `HKEY_LOCAL_MACHINE`（`HKLM`）レジストリ
    * HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\ClearCode\BrowserSelector
	* （32bit OSの場合: HKEY_LOCAL_MACHINE\SOFTWARE\ClearCode\BrowserSelector）
  * インストールフォルダのiniファイル
    * 例: C:\Program Files (x86)\ClearCode\BrowserSelector\BrowserSelector.ini
    * デフォルトでは上記ファイルは存在しません。
	  手動で設置する必要があります。
  * `HKEY_CURRENT_USER`（`HKCU`）レジストリ
    * HKEY_CURRENT_USER\SOFTWARE\WOW6432Node\ClearCode\BrowserSelector
    * （32bit OSの場合: HKEY_CURRENT_USER\SOFTWARE\ClearCode\BrowserSelector）
  * ユーザーフォルダのAppData内のiniファイル
    * 例: C:\Users\[UserName]\AppData\Roaming\ClearCode\BrowserSelector\BrowserSelectror.ini
    * デフォルトでは上記フォルダやファイルは存在しません。
	  手動で設置する必要があります。

上記複数の設定が存在する場合、それらの設定は統合されます（後に読み込まれる設定
で上書きされます）。

設定例は以下のファイルを参照して下さい。

  * [レジストリ: 64ビットOSの場合](sample/BrowserSelectorWOW64Example.reg)
  * [レジストリ: 32ビットOSの場合](sample/BrowserSelectorExample.reg)
  * [INIファイル](sample/BrowserSelector.ini)

設定できる項目は、一部を除いて共通です。
レジストリ設定においてレジストリキー直下に設定する項目は、INIファイルの場合には
`[Common]`セクションに記述します。

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

### `ZonePatterns`

Internet Explorerのセキュリティゾーンごとに起動するブラウザを設定します。
記法は`URLPatterns`および`HostNamePatterns`と同様です。

例)

  * `0001` = `intra|ie`
  * `0002` = `internet`

以下のセキュリティゾーン名がサポートされています。　

  * Type: String
    * `local`
    * `intra`
    * `trusted`
    * `internet`
    * `restricted`

`URLPatterns`と`HostNamePatterns`のルールは`ZonePatterns`に優先することに
注意してください。

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

  * 値の型: 文字列
    * `ie` （デフォルト）
    * `firefox`
    * `chrome`

例)

  * `DefaultBrowser` = `firefox`

### `SecondBrowser`

ホスト名やURLパターンでブラウザを指定しなかった場合、既定では`DefaultBrowser`で
指定したブラウザが使用されます。パターン毎に個別にブラウザを指定せずに、パター
ンにマッチしたURLに対して別のブラウザを使用したい場合は、`BrowserSelector`キー
直下に文字列値`SecondBrowser`をセットして下さい。

  * 値の型: 文字列
    * 空文字列 (デフォルト)
    * `ie`
    * `firefox`
    * `chrome`

例)

  * `SecondBrowser` = `chrome`
  * `URLPagtterns`
    * `0001` = `http://*.example.com`
    * `0002` = `http://*.example.org`
    * ...

### `FirefoxCommand`

システムに複数のバージョンのFirefoxをインストールしている場合に、どのFirefoxを
起動するかを指定できます。


 * 値の型: 文字列
   * 空文字列 (デフォルト)
   * 通常のWindowsパス形式: C:\path\to\firefox.exe

### `CloseEmptyTab`

外部ブラウザ起動後に残された空タブを閉じるか否かの設定です。

  * 値の型: DWORD
    * 1: 閉じる (デフォルト)
    * 0: 閉じない

### `OnlyOnAnchorClick`

リンクのクリックのみをブラウザ切り替えの対象とするか否かの設定です。

  * 値の型: DWORD
    * 0: すべてのページ遷移をブラウザ切り替えの対象とする（デフォルト）
    * 1: リンクのクリックのみをブラウザ切り替えの対象とする

### `Include`

追加で読み込む外部INIファイルのパスを指定します。読み込まれた外部INIファイルから
さらに外部INIファイルを読み込むことはできません。

  * 値の型: 文字列
    * 以下のいずれかの形式を使用可能
    * 通常のWindowsパス形式: C:\path\to\file.ini
    * UNC形式: \\HostName\ShareName\path\to\file.ini

### `EnableIncludeCache`

外部iniファイルにアクセスできない場合に、キャッシュを使用するかどうかを指定しま
す。

  * 値の型: DWORD
    * 0: 使用しない（デフォルト）
    * 1: 使用する

キャッシュはユーザーフォルダのLocalAppDataLow以下に保存されます。

  例) C:\Users\[UserName]\AppData\LocalLow\ClearCode\BrowserSelector\BrowserSelectror.ini
