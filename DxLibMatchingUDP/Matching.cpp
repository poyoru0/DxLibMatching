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
	p->host = false;

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
	//受信がなかったら
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
			printf("相手のIPアドレス:%d.%d.%d.%d\n\n", p->partnerIp.d1, p->partnerIp.d2, p->partnerIp.d3, p->partnerIp.d4);

			SetHost(ip);
		}
	}

	return 0;
}

/// <summary>
/// 相手と誰がホストになるか決める
/// </summary>
void MATCHING_C::SetHost(IPDATA ip)
{
	//変数宣言
	static bool setHost = false;
	char s[256];
	int test = true;
	int r = 0;
	//ホストが決まっていたら実行しない
	while (setHost == false)
	{
		//送られてきた物が数字だったら実行
		if (test == true)
		{
			//０か１どちらかを送る
			r = GetRand(100);
			std::snprintf(s, sizeof(s), "%d", r);
			NetWorkSendUDP(p->handle, ip, PORT, s, sizeof(s));
			test = false;
		}

		//受信するまで待つ
		while (CheckNetWorkRecvUDP(p->handle) == false);
		//相手の数値を取得
		NetWorkRecvUDP(p->handle, nullptr, nullptr, s, sizeof(s), false);
		//相手のランダム値を取得して自分と一緒ならやり直す
		int partnerR;
		int test = sscanf_s(s, "%d", &partnerR);
		//数字じゃなかったらやり直す
		if (partnerR < 0)
			continue;
		//自分の数字と相手の数字が違ったら実行
		if (r != partnerR)
		{
			setHost = true;
			//相手より大きければ自分がホストになる
			if (partnerR < r)
				p->host = true;
			else
				p->host = false;
			
			printf("SetHost:%d\n", p->host);
		}
	}
}

/// <summary>
/// 相手がreadyを送ってくるまで待つ
/// </summary>
void MATCHING_C::NetReady(IPDATA ip)
{
	char str[256] = "";
	NetWorkSendUDP(p->handle, ip, PORT, "ready", 6);
	while (1)
	{
		if (CheckNetWorkRecvUDP(p->handle) == true)
		{
			NetWorkRecvUDP(p->handle, nullptr, nullptr, str, sizeof(str), false);
			printf("readyTest:%s\n", str);
			if (strcmp(str, "ready") == 0)
				return;
		}
	}
}

/// <summary>
/// 相手の接続情報を取得
/// </summary>
/// <param name="partnerIP">接続相手のIPアドレス</param>
/// <returns>接続していなかったら-1を返す</returns>
int MATCHING_C::GetNetDATA(IPDATA* partnerIP, int* host)
{
	if (p->conected == false)
		return -1;

	*partnerIP = p->partnerIp;
	*host = p->host;
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