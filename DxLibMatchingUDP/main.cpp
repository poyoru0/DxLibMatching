#include "DxLib.h"
#include "main.h"
#include "Matching.h"

//https://dxlib.xsrv.jp/dxfunc.html

int main()
{
	if (DxLib_Init() == -1)
		return -1;
	//初期化
	ChangeWindowMode(true);
	MATCHING_C::instance().InitMatching();

	while (ProcessMessage() == 0)
	{
		//escでメインループを抜ける
		if (CheckHitKey(KEY_INPUT_ESCAPE))
			break;

		//相手が切断されていれば終了
		if (MATCHING_C::instance().DisConnected(true))
			break;

		ClearDrawScreen();
		clsDx();

		MATCHING_C::instance().Matching();

		IPDATA ip;
		int host = 0;
		int handle;
		if (MATCHING_C::instance().GetNetDATA(&ip, &host, &handle) != -1)
			printfDx("ip:%d.%d.%d.%d,host%d\n", ip.d1, ip.d2, ip.d3, ip.d4, host);

		ScreenFlip();
	}

	MATCHING_C::instance().EndMatching();
	DxLib_End();

	return 0;
}