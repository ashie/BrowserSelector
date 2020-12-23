---
CJKmainfont: Noto Sans CJK JP
CJKoptions:
  - BoldFont=Noto Sans CJK JP Bold
title: "BrowserSelector利用ガイド"
author: "株式会社クリアコード"
date: "2020-12-01"
titlepage: true
logo: logo.png
logo-width: 250
toc-title: "目次"
toc-own-page: true
listings-disable-line-numbers: true
code-block-font-size: \footnotesize
footnotes-pretty: true
---

# BrowserSelectorとは

BrowserSelectorとは、Webページごとにブラウザを自動的に切り替えるツールです。
法人企業を対象ユーザーとして、複数ブラウザの利用を支援するために開発されました。

**機能のハイライト**

 - URLパターンで起動するブラウザを設定することができます。
 - IE・Edge・Chrome・Firefoxなどの主要なブラウザをサポートしています。
 - グループポリシーや共有フォルダによる設定の集中管理に対応しています。

## システム要件

BrowserSelectorはWindows上で動作するアプリケーションです（動作対象はWindows 7/8/8.1/10です）。
以下のブラウザをサポートしています。

------------------ -------------------
Internet Explorer  バージョン 9 以降
Mozilla Firefox    バージョン 60 以降
Google Chrome      バージョン 79 以降
Microsoft Edge     バージョン 79 以降
------------------ -------------------

## ソフトウェアの構成

BrowserSelectorは次のモジュールから構成されています。

ファイル                    説明
--------------------------- ----------------------------------------------
BrowserSelector.exe         BrowserSelectorの実行ファイル
BrowserSelectorBHO.dll      IE用のブラウザヘルパーオブジェクト
BrowserSelectorBHO64.dll    IE用のブラウザヘルパーオブジェクト （64ビット）
BrowserSelectorTalk.exe     Edge/Chrome/Firefox連携用のホストプログラム
BrowserSelectorFirefox.json Firefox向け定義ファイル
BrowserSelectorChrome.json  Chrome向け定義ファイル
BrowserSelectorEdge.json    Edge向け定義ファイル
--------------------------- ----------------------------------------------

# BrowserSelectorチュートリアル

この章では、BrowserSelectorをインストールして、ブラウザの切り替えをセットアップする手順を説明します。
この手順は、大きく4つのステップに分かれます。

 1. BrowseSelectorをインストールする。
 2. BrowseSelectorの設定ファイルを配置する。
 3. Edge/Chrome/Firefoxにアドオンをインストールする。
 4. ブラウザを起動し、動作を確認する。

以下では、Windowsに標準でインストールされているIEとEdgeを例に解説を行いますが、
導入の流れはFirefoxやChromeを利用する場合でも同じです
（具体的な手順は「その他のセットアップ」の章を参照ください）。

##  BrowseSelectorをインストールする

BrowseSelectorについてはMSI形式のインストーラが配布されています。

 1. 次のページから最新版のインストーラをダウンロードします。

    https://gitlab.com/clear-code/browserselector/-/releases

 2. MSIインストーラを実行して、ウィザードを進めます。

 3. 「BrowserSelectorは正常にインストールされました」というメッセージが表示されたら、
    「閉じる」をクリックして終了します。

## BrowseSelectorの設定ファイルを配置する

BrowseSelectorはINI形式の設定ファイルによって管理することができます。

 1. 次の内容を「BrowseSelector.ini」という名前で保存します。

    ```ini
    [Common]
    DefaultBrowser=ie
    CloseEmptyTab=1

    [URLPatterns]
    0001=http*://www.clear-code.com/*|edge
    ```

 2. 保存したファイルを、BrowseSelectorのインストールパスに配置します。

    ```powershell
    % move BrowserSelector.ini C:\Program Files (x86)\ClearCode\BrowserSelector
    ```

INIファイルに設定できる項目の詳細は「設定項目の一覧」の章を参照下さい。

## Edgeにアドオンをインストールする

BrowserSelectorはEdge/Chrome/Firefox向けに「IEView WE」という連携アドオンを提供しています。
このアドオンは次の手順でインストールできます。

 1. 次の内容を「edge.reg」という名前で保存します。

    ```ini
    Windows Registry Editor Version 5.00

    [HKEY_LOCAL_MACHINE\Software\Policies\Microsoft\Edge]
    "ExtensionSettings"="{\"ifacgepgdnnddnckleiinelkadppopgh\":{\"update_url\":\"https://edge.microsoft.com/extensionwebstorebase/v1/crx\",\"installation_mode\":\"force_installed\"}}"

    [HKEY_LOCAL_MACHINE\Software\Policies\Microsoft\Edge\3rdparty\extensions\ifacgepgdnnddnckleiinelkadppopgh\policy]
    "talkEnabled"=dword:00000001
    "talkServerName"="com.clear_code.browserselector_talk"
    "talkBrowserName"="edge"
    ```

 2. 管理者権限でレジストリ設定を読み込みます。

    ```powershell
    % REG IMPORT edge.reg
    ```

## ブラウザの動作を確認する

次の手順で、IEとEdgeを相互に切り替えられることを確認します。

 1. IEを立ち上げて https://www.clear-code.com にアクセスします。

    - Edgeが立ち上がることを確認します。

 2. Edgeを操作して www.clear-code.com 以外のサイトにアクセスします。

    - IEが立ち上がることを確認します。

動作確認後、導入したソフトウェアを削除したい場合は、次の手順に従います。

 1. Windowsのスタートメニューから「プログラムの追加と削除」を起動します。

 2. BrowserSelectorを選択し、アンインストールを実行します。

 3. レジストリに登録した設定を削除します。

    ```powershell
    % REG DELETE HKLM\Software\Policies\Microsoft\Edge /v ExtensionSettings
    % REG DELETE HKLM\Software\Policies\Microsoft\Edge\3rdparty\extensions\n
      ifnmbbelnhfhgeiampkfghakhmgggcf\policy
    ```

# 設定項目の一覧

この章では、BrowserSelectorの設定項目について解説します。

BrowserSelectorは、INI形式の設定ファイルで細かく挙動を管理することができます。
設定ファイルのアウトラインを示すと、以下のようになります。

```ini
[Common]
DefaultBrowser=edge
CloseEmptyTab=1

[URLPatterns]
0001=http*://www.clear-code.com/blog/*|chrome

[HostnamePatterns]
0001=www.clear-code.com|firefox

[ZonePatterns]
0001=intra|ie
```

## [Common] セクション

このセクションには、BrowserSelector全体に関わる基本的な設定を記述します。

| 項目                     |  設定内容                                               | 既定 |
| ------------------------ |  ------------------------------------------------------ | ---- |
| DefaultBrowser           | Webページを開くデフォルトのブラウザ (ie/edge/chrome/firefox) | ie   |
| SecondBrowser            | ブラウザの指定を省略した時に利用するブラウザ (ie/edge/chrome/firefox) | |
| FirefoxCommand           | 起動するFirefoxを指定する (Firefoxを複数インストールしている環境のみ設定が必要) | |
| CloseEmptyTab            | 他のブラウザでページを開いたときに元のタブを閉じる | 1 |
| OnlyOnAnchorClick        | リンククリック時のみリダイレクト判定を行う | 0 |
| UseRegex                 | ルールの記述に正規表現を利用する (0の場合は簡易パターンマッチが利用される) | 0 |
| Include                  | 追加で読み込む外部設定ファイルのパス | |
| EnableIncludeCache       | 外部の設定ファイルのキャッシュを生成する | 0 |

セクションの設定例を以下に示します。

```ini
[Common]
DefaultBrowser=ie
Include=C:\Users\Public\BrowserSelector.ini
EnableIncludeCache=1
```

## [URLPatterns] セクション

このセクションには、URLごとに起動するブラウザを定義します。

このURLの指定については、次のワイルドカードを利用することができます。

 - 「`*`」 (任意の文字列にマッチする)
 - 「`?`」 (任意の1文字にマッチする)

セクションの設定例を以下に示します。

```ini
[URLPatterns]
0001=http*://www.clear-code.com/*|firefox
0002=http*://www.example.com/*|chrome
```

## [HostnamePatterns] セクション

このセクションには、ホスト名ごとに起動するブラウザを定義します。

 - URLとホスト名の両方の定義がマッチする場合、URLPatternsが優先されます。
 - ホスト名の設定については、URLと同じワイルドカードが指定できます。

セクションの設定例を以下に示します。

```ini
[HostnamePatterns]
0001=www.example.com|ie
0002=*.clear-code.com|chrome
```

## [ZonePatterns] セクション

このセクションには、IEのセキュリティゾーンに対応するブラウザを設定します。

利用可能なセキュリティゾーン名は以下の通りです。

| 項目       |  説明                                          |
| ---------- | ---------------------------------------------- |
| local      | コンピューターのローカルに存在するページ       |
| intra      | イントラネット上のサイト                       |
| trusted    | 「信頼済みサイト」として登録されたサイト       |
| internet   | 他のいずれのゾーン定義にも該当しないサイト     |
| restricted | 「制限付きサイト」として登録されたサイト       |

セクションの設定例を以下に示します。

```ini
[ZonePatterns]
0001=intra|ie
0002=internet|firefox
```

# その他のセットアップ

## BrowserSelectorを既定のブラウザとして登録する

例えば、OutlookやThunderbirdなどのメールアプリでリンクをクリックした際に、
自動的に対応するブラウザを開きたい場合は、次のように設定します。

 1. Windowsのスタートメニューを開きます、

 2. 「既定のアプリ」を検索し、設定ダイアログを起動します。

 3. 「Webブラウザ」の項目を開き、リストからBrowserSelectorを選択します。

## 共有フォルダで設定を集中管理する

BrowserSelectorの設定を次の手順で共有フォルダに配置することで、
複数の端末の設定を集中管理することができます。

 1. 各端末のBrowserSelector.iniに以下の内容を記述します。

    ```ini
    [Common]
    Include=\\shared\BrowserSelectorRemote.ini
    EnableIncludeCache=1
    ```

 2. 共有フォルダの指定位置に設定ファイルを配置します。

    ```powershell
    % cp BrowserSelectorRemote.ini \\shared\
    ```

## グループポリシーで設定を集中管理する

BrowserSelectorは管理者向けにADMX形式のテンプレートを提供しています。

このテンプレートを利用することで、INI形式の設定ファイルに代えて
Windowsのグループポリシーで設定を集中管理することができます。

 1. BrowserSelectorのインストールディレクトリに移動します。

 2. 「policy」配下にあるテンプレートを、システムの所定位置に配置します。

    ```powershell
    % cp policy\*.admx C:\Windows\PolicyDefinitions
    % cp policy\ja-JP\*.adml C:\Windows\PolicyDefinitions\ja-JP
    ```

 3. 「gpedit.msi」を起動して設定を編集します。

    - 管理用テンプレート > Clear Code > BrowserSelector を確認ください。

## Firefoxに連携アドオンを導入する

Firefox向けの連携アドオンを導入する場合は、次の手順でインストールします。

 1. Firefoxのインストールパスに「distribution」というフォルダを作成します。

    (例: `C:\Program Files\Mozilla Firefox\distribution`)

 2. 作成したフォルダにファイル「policies.json」を作成し、次の内容を保存します。

    ```json
    {
      "policies": {
        "Extensions": {
          "Install": ["https://addons.mozilla.org/firefox/downloads/latest/ie-view-we/latest.xpi"],
          "Locked": ["ieview-we@clear-code.com"]
        },
        "3rdparty": {
          "Extensions": {
            "ieview-we@clear-code.com": {
              "talkEnabled": true,
              "talkServerName": "com.clear_code.browserselector_talk"
            }
          }
        }
      }
    }
    ```

 3. Firefoxを再起動し、IEView WEが導入されていることを確認します。

## Chromeに連携アドオンを導入する

Chrome向けの連携アドオンを導入する場合は、次の手順でインストールします。

 1. 次の内容を「chrome.reg」というファイル名で保存します。

    ```ini
    Windows Registry Editor Version 5.00

    [HKEY_LOCAL_MACHINE\Software\Policies\Google\Chrome]
    "ExtensionSettings"="{\"nifnmbbelnhfhgeiampkfghakhmgggcf\":{\"update_url\":\"https://clients2.google.com/service/update2/crx\",\"installation_mode\":\"force_installed\"}}"

    [HKEY_LOCAL_MACHINE\Software\Policies\Google\Chrome\3rdparty\extensions\nifnmbbelnhfhgeiampkfghakhmgggcf\policy]
    "talkEnabled"=dword:00000001
    "talkServerName"="com.clear_code.browserselector_talk"
    "talkBrowserName"="chrome"
    ```

 2. 管理者権限で「`REG IMPORT chrome.reg`」を実行します。

 3. Chromeを起動し、IEView WEが導入されていることを確認します。

## Edgeに連携アドオンを導入する

Edge向けの連携アドオンを導入する場合は、次の手順でインストールします。

 1. 次の内容を「edge.reg」という名前で保存します。

    ```ini
    Windows Registry Editor Version 5.00

    [HKEY_LOCAL_MACHINE\Software\Policies\Microsoft\Edge]
    "ExtensionSettings"="{\"ifacgepgdnnddnckleiinelkadppopgh\":{\"update_url\":\"https://edge.microsoft.com/extensionwebstorebase/v1/crx\",\"installation_mode\":\"force_installed\"}}"

    [HKEY_LOCAL_MACHINE\Software\Policies\Microsoft\Edge\3rdparty\extensions\ifacgepgdnnddnckleiinelkadppopgh\policy]
    "talkEnabled"=dword:00000001
    "talkServerName"="com.clear_code.browserselector_talk"
    "talkBrowserName"="edge"
    ```

 2. 管理者権限でレジストリ設定を読み込みます。

    ```powershell
    % REG IMPORT edge.reg
    ```

 3. Edgeを起動し、IEView WEが導入されていることを確認します。

## BrowserSelectorをアンインストールする

BrowserSelectorをシステムから削除したい場合は次の手順に従います。

 1. Windowsのスタートメニューから「プログラムの追加と削除」を起動します。

 2. BrowserSelectorを選択し、アンインストールを実行します。

 3. 連携アドオン「IEView WE」を導入した場合は、次の手順で削除します。


Chromeのアドオンを削除する

```powershell
% REG DELETE HKLM\Software\Policies\Google\Chrome /v ExtensionSettings
% REG DELETE HKLM\Software\Policies\Google\Chrome\3rdparty\extensions\n
  ifnmbbelnhfhgeiampkfghakhmgggcf\policy
```

Edgeのアドオンを削除する

```powershell
% REG DELETE HKLM\Software\Policies\Microsoft\Edge /v ExtensionSettings
% REG DELETE HKLM\Software\Policies\Microsoft\Edge\3rdparty\extensions\n
  ifnmbbelnhfhgeiampkfghakhmgggcf\policy
```

Firefoxのアドオンを削除する

```powershell
% DEL C:\path\to\distribution\policies.json
```

# よくある質問

## IEの「アドオンの有効化」ポップアップを抑止したい

IEの初回起動時に表示される「BrowserSelectorを有効化しますか？」というポップアップは、
グループポリシーで抑止することができます。

詳細は、下記の記事を参照ください。

https://docs.microsoft.com/en-us/archive/blogs/jpieblog/internet-explorer-11-212-2

## ピン留めサイトから起動した場合にリダイレクトが機能しない

ピン留めサイト (.websiteショートカット) から起動した場合、
IEのセキュリティの仕組みにより、アドオンが一時的にすべて無効化されます。

https://docs.microsoft.com/en-us/archive/blogs/ie/internet-explorer-9-security-part-3-browse-more-securely-with-pinned-sites

この制約を回避するためには、ピン留めサイトのショートカットを
通常のインターネットショートカット（.url）に差し替える必要があります。

## IEのBHOが有効化されない

IEの「ブラウザ拡張禁止」の設定が適用されている可能性があります。
次の二つの設定を確認して、アドオンが禁止されていないか確認ください。

 1. 「Enhanced Security Configuration」が有効化されている場合

    インターネットオプション > 詳細設定 > 「サードパーティ製のブラウザ拡張を有効にする」を確認します。

 2. IEのアドオンが一律無効化されている場合

    グループポリシーの「アドオンの一覧で許可されたものを除き、アドオンを一律に無効化する」を確認します。

## BrowserSelectorをサイレントインストールしたい

BrowserSelectorをサイレントインストールで導入したい場合は、
`msiexec`コマンドを次のように実行してください。

```powershell
% msiexec BrowserSelectorSetup.msi /quiet
```

## 環境に複数のFirefoxがインストールされている

一つの端末に複数のFirefoxがインストールされている場合、
BrowserSelectorがどのFirefoxを起動するかが問題となります。

この場合は、BrowserSelectorの`FirefoxCommand`の設定を利用して、
起動するFirefoxの実行ファイルを絶対パスで指定ください。

```ini
[Common]
...
FirefoxCommand=%ProgramFiles%\Mozilla Firefox\firefox.exe
...
```
