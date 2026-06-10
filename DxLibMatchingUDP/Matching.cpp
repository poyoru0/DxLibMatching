#include "DxLib.h"
#include "Matching.h"
#include <cstdio>
#include <time.h>

/// <summary>
/// Matching初期化
/// </summary>
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
	p->disConnectLastTime = clock();

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
	constexpr int n = 0;
	char str[256] = "";
	//初回のみ実行
	if (init == false)
	{
		//送受信用ソケット作成
		p->handle = MakeUDPSocket(PORT);
		//自分のアドレスを取得
		GetMyIPAddress(p->myIp, 16, nullptr);
		//デバッグ
		printf("MyIP:%d.%d.%d.%d\n", p->myIp[n].d1, p->myIp[n].d2, p->myIp[n].d3, p->myIp[n].d4);//自身のアドレス表示
		printf("受信待ち\n");

		init = true;
	}
	//全アドレスに送信
	IPDATA ip;
	ip.d1 = 255; ip.d2 = 255; ip.d3 = 255; ip.d4 = 255;
	NetWorkSendUDP(p->handle, ip, PORT, "sendByPoyoru", 13);

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
			//デバッグ
			printf("受信済み\n");
			printf("%s\n", str);//受け取ったメッセージを表示
			printf("相手のIPアドレス:%d.%d.%d.%d\n\n", p->partnerIp.d1, p->partnerIp.d2, p->partnerIp.d3, p->partnerIp.d4);

			SetHost(ip);
			//接続完了フラグ
			p->conected = true;
			return 1;
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
	char str[256] = "";
	char myR[256];
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
			std::snprintf(myR, sizeof(myR), "%d", r);
			NetWorkSendUDP(p->handle, ip, PORT, myR, sizeof(myR));
			test = false;
		}

		//受信するまで待つ
		while (CheckNetWorkRecvUDP(p->handle) == false);
		//相手の数値を取得
		NetWorkRecvUDP(p->handle, nullptr, nullptr, str, sizeof(str), false);
		//相手のランダム値を取得して自分と一緒ならやり直す
		int partnerR;
		test = sscanf_s(str, "%d", &partnerR);
		//数字じゃなかったらやり直す
		if (partnerR < 0)
			continue;
		//自分の数字と相手の数字が違ったら実行
		if (r != partnerR)
		{
			//相手より大きければ自分がホストになる
			if (partnerR < r)
				p->host = true;
			//相手が準備できているかどうか確認
			NetWorkSendUDP(p->handle, ip, PORT, "ready", 6);
			float lastT = clock();
			while (1)
			{
				//受信したら実行
				if (CheckNetWorkRecvUDP(p->handle) == true)
				{
					NetWorkRecvUDP(p->handle, nullptr, nullptr, str, sizeof(str), false);
					//あっていたらループを抜ける
					if (strcmp(str, "ready") == 0)
						break;
				}
				else if (5.0f < (clock() - lastT) / 1000.0f)//5秒以上たったら自身実行
				{
					//自分の数字をもう一度送る
					NetWorkSendUDP(p->handle, ip, PORT, myR, sizeof(myR));
					//1秒待つ
					WaitTimer(1000);
					//準備完了を送る
					NetWorkSendUDP(p->handle, ip, PORT, "ready", 6);
				}
			}

			printf("SetHost:%d\n", p->host);
			setHost = true;
		}
	}
}

/// <summary>
/// 相手の接続情報を取得
/// </summary>
/// <param name="partnerIP">接続相手のIPアドレス</param>
/// <returns>接続していなかったら-1を返す</returns>
int MATCHING_C::GetNetDATA(IPDATA* partnerIP, int* host, int* handle)
{
	if (p->conected == false)
		return -1;

	*partnerIP = p->partnerIp;
	*host = p->host;
	*handle = p->handle;
	return 0;
}

/// <summary>
/// どちらかの接続が切れたら１を返す
/// </summary>
int MATCHING_C::DisConnected(bool flag)
{
	float nowT = clock();//現在時刻の取得
	//接続しているときのみ実行
	if (p->conected == true)
	{
		if (flag == true)//送り続ける
			NetWorkSendUDP(p->handle, p->partnerIp, PORT, "", 1);
		
		//受信があったら実行
		if (CheckNetWorkRecvUDP(p->handle) == true)
		{
			if (flag == true)//受信し続ける
			{
				char str[256];
				NetWorkRecvUDP(p->handle, NULL, NULL, str, sizeof(str), false);
			}
			p->disConnectLastTime = nowT;
		}
		//前回の受信から５秒以上たっていたら切断判定とする
		if (5.0f < (nowT - p->disConnectLastTime) / 1000.0f)
		{
			printf("相手との接続が切れました");
			return 1;
		}
	}
	else
		p->disConnectLastTime = nowT;

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