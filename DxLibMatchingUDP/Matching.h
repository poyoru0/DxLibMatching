#pragma once
#ifndef MATCHING_H
#define MATCHING_H

constexpr int PORT = 1324;

class MATCHING_C
{
private:
	struct MATCHING_P
	{
		IPDATA myIp[16];
		IPDATA partnerIp;
		bool conected;
		int handle;
		int host;
	}*p;

public:
	int InitMatching();
	void EndMatching();

	int Matching();
	void SetHost(IPDATA ip);
	void NetReady(IPDATA ip);
	int GetNetDATA(IPDATA* partnerIP, int* host);

	static MATCHING_C& instance();
};


#endif // !MATCHING_H