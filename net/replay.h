
#ifndef _SNOW_REPLAY_H_
#define _SNOW_REPLAY_H_ 1
#pragma once

#include <string>
#include <base/lock.h>

#ifdef SNOW
#include <../../Projects/tieba/NewSnow/Snow/interface/work/work_common.h>
namespace view {
	namespace frame {
		class CSnowMomo;
	}
}

#define ADD_COMMA			,
#define ADD_PARAM(...)		view::frame::CSnowMomo *momo ## __VA_ARGS__
#define ADD_CALLBACK		void *user, snow::work::SNOW_WORK_CALLBACK callback
#else
#define ADD_COMMA
#define ADD_PARAM(...)
#define ADD_CALLBACK
#endif

#define ADD_COMMA_PARAM(...) ADD_COMMA ADD_PARAM(## __VA_ARGS__)
#define ADD_COMMA_CALLBACK() ADD_COMMA ADD_CALLBACK

//-----断开连接-----
//BOOL RasClose(HRASCONN  hrasconn);

class CReplay {
public:
	CReplay();
	~CReplay();

	inline bool IsReplaying()
	{
		return m_replaying;
	}

	void Stop();

	int Replay(int try_again_times = 0, char** new_ip = NULL ADD_COMMA_PARAM(=NULL));
	int IPCompare(std::string& old_ip_address, char** new_ip = NULL);

	Lock m_lock;

protected:
	bool m_replaying;
	bool m_stopping;

	int ReplayInline(ADD_PARAM(=NULL));
};

namespace snow {
	extern CReplay m_replay;
};

int CloseRasConnecting(bool bGetLinkNumber = false ADD_COMMA_PARAM(=NULL));

int CreateADSLEntry(ADD_PARAM(=NULL));
void SnowADSLDial(ADD_PARAM(=NULL));

void SnowReplayADSL(ADD_PARAM(=NULL));
int SnowReplayLan(ADD_PARAM(=NULL));

void PrintCurIPAddress(std::string& ip_address);

int SnowReplay(int try_again_times = 3 ADD_COMMA_PARAM(=NULL));

bool SnowReplayAsyn(bool autoclean ADD_COMMA_CALLBACK());	//创建新线程重拨

#endif