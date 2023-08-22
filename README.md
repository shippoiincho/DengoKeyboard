# DengoKeyboard

![converter outer](dengo1.png)
![converter inner](dengo2.png)

PlayStation 用、初代電車でGo! コントローラ(ツーハンドル)で JR 東日本トレインシミュレーターをプレイするための変換機です。
Arduino Microのような ATmega32U4 を使ったArduino (+互換機)で動作します。<br>

PS コントローラ制御ライブラリとして、"PsxNewLib" を使っています。
ライブラリマネージャからインストールしてください。<br>

コードは PsxNewLib のサンプル DenGo を改変しています。
電車でGo! コントローラの動きをキーボードの入力として出力していますので、パソコンからは USB キーボードとして見えます。<br>

ニュートラル、または非常ブレーキによって、パソコンとコントローラのノッチの位置が合うようになっています。
ゲームの開始状況とマスコンの位置によっては、ノッチの位置がずれることがあるかもしれません。<br>

手元にツーハンドルコントローラしかないので、ツーハンドルコントローラしか対応していませんが、
ワンハンドル車でもそのまま運転できます。(元々トレインシミュレーターの仕様)<br>

TM1651 ライブラリは、いわゆる"Gotek LED" でマスコンの位置を表示するために使っていますので、
不要であればコメントアウトしてください。<br>