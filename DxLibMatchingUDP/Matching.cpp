#include "DxLib.h"
#include "Matching.h"
#include <cstdio>

/// <summary>
/// Matching初期化
/// </summary>
/// <returns></returns>
int MATCHING_C::InitMatching()
{
	if (!p)
	{
		p = (struct MATCHING_P*)malloc(sizeof(struct MATCHING_P));
	}
	if (!p)
		return -1;

	//初期化
	p->conected = false;

	return 0;
}

/// <summary>
/// Matching終了処理
/// </summary>
void MATCHING_C::EndMatching()
{
	DeleteUDPSocket(p->handle);
	free(p);
	p = nullptr;
}

/// <summary>
/// ほかのマシンを自動で検索して接続する
/// </summary>
int MATCHING_C::Matching()
{
	//接続完了したら実行しないようにする
	if (p->conected == true)
		return 1;

	//宣言
	static bool init = false;
	int n = 0;
	char str[256] = "";
	//初回のみ実行
	if (init == false)
	{
		//送受信用ソケット作成
		p->handle = MakeUDPSocket(PORT);
		//自分のアドレスを取得
		GetMyIPAddress(p->myIp, 16, nullptr);
		printf("MyIP:%d.%d.%d.%d\n", p->myIp[n].d1, p->myIp[n].d2, p->myIp[n].d3, p->myIp[n].d4);//自身のアドレス表示

		printf("受信待ち\n");
		init = true;
	}
	//全アドレスに送信
	IPDATA ip;
	ip.d1 = 255;
	ip.d2 = 255;
	ip.d3 = 255;
	ip.d4 = 255;
	NetWorkSendUDP(p->handle, ip, PORT, "sendByPoyoru", 13);

	printfDx("受信待ち\n");

	//受信があったら実行
	if (CheckNetWorkRecvUDP(p->handle) == true)
	{
		//相手のマシンアドレス取得用
		IPDATA ip;
		//メッセージを受信
		NetWorkRecvUDP(p->handle, &ip, nullptr, str, 256, false);
		//受け取ったメッセージが特定のものかつ自分以外からの受信だったら実行
		if (strcmp(str, "sendByPoyoru") == 0 &&//特定のメッセージ
			(ip.d1 != p->myIp[n].d1 ||
			ip.d2 != p->myIp[n].d2 ||
			ip.d3 != p->myIp[n].d3 ||
			ip.d4 != p->myIp[n].d4))//アドレス
		{
			//相手のIPアドレスを取得
			p->partnerIp = ip;
			//接続完了フラグ
			p->conected = true;
			printf("受信済み\n");
			printf("%s\n", str);//受け取ったメッセージを表示
			printf("相手のIPアドレス:%d.%d.%d.%d\n", p->partnerIp.d1, p->partnerIp.d2, p->partnerIp.d3, p->partnerIp.d4);
		}
	}

	return 0;
}

/// <summary>
/// 相手の接続情報を取得
/// </summary>
/// <param name="partnerIP">接続相手のIPアドレス</param>
/// <returns>接続していなかったら-1を返す</returns>
int MATCHING_C::GetNetDATA(IPDATA* partnerIP)
{
	if (p->conected == false)
		return -1;

	*partnerIP = p->partnerIp;
	return 0;
}

/// <summary>
/// インスタンス化
/// </summary>
MATCHING_C& MATCHING_C::instance()
{
	static MATCHING_C instance;
	return instance;
}