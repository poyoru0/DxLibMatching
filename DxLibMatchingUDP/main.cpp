#include "DxLib.h"
#include "main.h"
#include "Matching.h"

//https://dxlib.xsrv.jp/dxfunc.html

int main()
{
	if (DxLib_Init() == -1)
		return -1;
	//‰Šú‰»
	ChangeWindowMode(true);
	MATCHING_C::instance().InitMatching();

	while (ProcessMessage() == 0)
	{
		//esc‚ÅƒƒCƒ“ƒ‹[ƒv‚ğ”²‚¯‚é
		if (CheckHitKey(KEY_INPUT_ESCAPE))break;

		ClearDrawScreen();
		clsDx();

		MATCHING_C::instance().Matching();

		IPDATA ip;
		int host = 0;
		if (MATCHING_C::instance().GetNetDATA(&ip, &host) != -1)
			printfDx("ip:%d.%d.%d.%d,host%d\n", ip.d1, ip.d2, ip.d3, ip.d4, host);

		ScreenFlip();
	}

	MATCHING_C::instance().EndMatching();
	DxLib_End();

	return 0;
}