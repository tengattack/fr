
#ifndef _SNOW_REPLAY_LOAD_SETTINGS_H_
#define _SNOW_REPLAY_LOAD_SETTINGS_H_ 1
#pragma once

#include <string>
#include <base/windows_version.h>

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

enum SnowNetState {
	kSNSLan = 0,
	kSNSModem,
	kSNSProxy,
	kSNSNotConnected,
	kSNSCount
};

typedef struct _TA_SNOW_LAN_REPLAY_SETTINGS {
	char *szUser;
	char *szPassword;
	char *szUrl;
	DWORD dwFlags;
	char *szUrlDisconnect;
	char *szUrlConnect;

	char *szDataDisconnect;
	char *szDataConnect;
} TA_SNOW_LAN_REPLAY_SETTINGS;

typedef struct _TA_SNOW_ADSL_REPLAY_SETTINGSA {
	char *szUser;
	char *szPassword;
} TA_SNOW_ADSL_REPLAY_SETTINGSA;

typedef struct _TA_SNOW_ADSL_REPLAY_SETTINGSW {
	wchar_t *szUser;
	wchar_t *szPassword;
} TA_SNOW_ADSL_REPLAY_SETTINGSW;

#ifdef _UNICODE
#define TA_SNOW_ADSL_REPLAY_SETTINGS _TA_SNOW_ADSL_REPLAY_SETTINGSW
#define Tstring	std::wstring
#else
#define TA_SNOW_ADSL_REPLAY_SETTINGS _TA_SNOW_ADSL_REPLAY_SETTINGSA
#define Tstring std::string
#endif

#define SNOW_REPLAY_UNKNOW		0
#define SNOW_REPLAY_LAN			1
#define SNOW_REPLAY_ADSL		2

#define SNOW_REPLAY_LAN_USELINK				1
#define SNOW_REPLAY_LAN_DISCONNECT_GET		0	//2
#define SNOW_REPLAY_LAN_DISCONNECT_POST		4
#define SNOW_REPLAY_LAN_CONNECT_GET			0	//8
#define SNOW_REPLAY_LAN_CONNECT_POST		16

namespace ReplaySettings {
	extern BYTE m_bReplayType;
	extern TA_SNOW_LAN_REPLAY_SETTINGS m_setLan;
	extern TA_SNOW_ADSL_REPLAY_SETTINGS m_setADSL;
};

extern base::win::Version m_win_version;

SnowNetState SnowGetNetState();

void InitReplaySettings(LPCTSTR datapath);
void FreeReplaySettings();

void SnowLanSettings(BOOL bLoad);
void SnowADSLSettings(BOOL bLoad);

void LoadReplaySettings();
void SaveReplaySettings();

void SetLanSettingsChanged(bool changed = true);
void SetADSLSettingsChanged(bool changed = true);

#endif