# DxLibMatching
DxLibで相手と自動接続をするためのリポジトリ

シングルトンパターンを採用しているので
すべてMatching_Cクラスからinstance()でインスタンス化してから使ってください

関数使い方
int Matching()
:接続が完了するまで呼び出し続けてください。
　接続が完了したら１を返します。

int GetNetDATA(IPDATA* partnerIP, int* host, int* handle)
:相手のipアドレス(partnerIP)、自分がホストがどうか(host)、接続時のハンドル(handle)を返します。
　接続していなかったら-1を返します。

int DisConnected(bool flag)
:これは接続が完了したら常に呼び出しつづけてください。
　フラグ(flag)はほかにUDPで送受信している箇所がなければtrueにしてください。
　相手が切断したら1を返す。