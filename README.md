# px4chset

BS放送を録画したTSファイルに含まれるNIT情報から録画プログラムのBS放送用チャンネル定義ファイルを作成するプログラムです。以下の形式に対応しています。

* json ([chsetconv][link_chsetconv])
* dvbv5_channels_isdbs.conf ([dvbv5-zap][link_dvbv5])
* channels.yml ([mirakurun][link_mirakurun])
* BonDriver_DVB.conf ([BonDriverProxy_Linux][link_bdpl])
* BonDriver_LinuxPT.conf ([BonDriverProxy_Linux][link_bdpl])
* BonDriver_LinuxPTX.ini ([BonDriver_LinuxPTX][link_bonptx])
* BonDriver_PX4-S.ChSet.txt ([px4_drv][link_px4_drv],BonDriver_PX4)
* BonDriver_PX_W3U3.ini (BonDriver_PX_W3U3)
* BonDriver_BDA.ini ([BonDriver_BDA][link_bonbda])
* BonDriver_PlexPX_S.ini ([BDASpecial_PlexPX][link_bonplexpx])

## インストール

動作確認環境は以下のとおりです。

* Ubuntu 24.04
* Windows 11

### Linux

```console
git clone --recursive https://github.com/hendecarows/px4chset.git
cd px4chset
mkdir build
cd build
cmake ..
make -j
```

### Windows

```console
git clone --recursive https://github.com/hendecarows/px4chset.git
```

ソリューション(.sln)ファイルをVisual Studio 2022以降で開いてビルドします。
`vcpkg integrate install`を実行し、vcpkgとVisual Studioを統合しておく必要があります。

## 使用方法

### Linux

recpt1等でBS放送を30秒程度録画したファイルを用意します。
どのチャンネルでも良いですが正常なNITが含まれている必要があります。
[tsselect][link_tsselect]や[tspacketchk][link_tspacketchk]などでチェックし、エラーやドロップのないファイルを用意して下さい。

```console
recpt1 --device /dev/isdb2056video0 BS01_0 30 bs.ts
```

出力したTSファイルを使用してチャンネル定義ファイルを出力します。

```console
px4chset --format=str input [optput]
```

`str`には以下の形式を指定します。また、`input`は録画したTSファイル名を`output`には出力するチャンネル定義ファイルのファイル名を指定します。
`output`を省略すると下表ファイル名でカレントディレクトリに出力します。mirakurun, BonDriver_DVB, BonDriver_LinuxPT, BonDriver_LinuxPTXは、ISDB-S部分のみを出力します。

| 形式         | ファイル名                         | 出力形式             |
| ----------- | ------------------------------- | ------------------- |
| `json`      | `tsidsyymmdd.json`              | chsetconv json      |
| `dvbv5`     | `dvbv5_channels_isdbs.conf`     | dvbv5-zap           |
| `dvbv5lnb`  | `dvbv5_channels_isdbs_lnb.conf` | dvbv5-zap LNB有効    |
| `mirakurun` | `channelsyymmdd.yml`            | mirakurun           |
| `bondvb`    | `BonDriver_DVB-S.conf`          | BonDriver_DVB       |
| `bonpt`     | `BonDriver_LinuxPT-S.conf`      | BonDriver_LinuxPT   |
| `bonptx`    | `BonDriver_LinuxPTX-S.ini`      | BonDriver_LinuxPTX  |
| `bonpx4`    | `BonDriver_PX4-S.ChSet.txt`     | BonDriver_PX4       |
| `bonpx3`    | `BonDriver_PX3_S.ini`           | BonDriver_PX_W3U3   |
| `bonbda`    | `BonDriver_BDA_S.ini`           | BonDriver_BDA       |
| `bonplexpx` | `BonDriver_PlexPX_S.ini`        | BonDriver_PlexPX    |

`BonDriver_PX4-S.ChSet.txt`を出力するには以下を実行します。

```console
px4chset --format=bonpx4 bs.ts
```

### Windows

Linuxと同様のコンソールアプリです。Terminal等から実行して下さい。

[link_chsetconv]: https://github.com/hendecarows/chsetconv
[link_bdpl]: https://github.com/hendecarows/BonDriverProxy_Linux
[link_mirakurun]: https://github.com/Chinachu/Mirakurun
[link_dvbv5]: https://github.com/hendecarows/dvbconf-for-isdb/tree/master/conf
[link_bonptx]: https://github.com/hendecarows/BonDriver_LinuxPTX
[link_px4_drv]: https://github.com/tsukumijima/px4_drv
[link_bonbda]: https://github.com/radi-sh/BonDriver_BDA
[link_bonplexpx]: https://github.com/radi-sh/BDASpecial-PlexPX
[link_tsselect]: https://github.com/xtne6f/tsselect_gcc
[link_tspacketchk]: https://github.com/kaikoma-soft/tspacketchk
