
#include "stdafx.h"

#include "net_common.h"

#ifdef SNOW
#define _SNOW_ZERO 1
#endif

static const char *g_szABaseText[] = 
{
	"MessageBoxW",
	"SetWindowTextW",
	"\r\n",
	"\r\nSet-Cookie: ",
	"path=",
	"\r\nExpires: ",
	//"LOGIN_ERROR_USERNAME",
	//"LOGIN_ERROR_PASSWORD",
	"LOGIN_",
	/*"GET /login.php?u=%s&p=%s HTTP/1.1\r\n"
								"User-Agent: Snow/0.a7 (TengAttack App Engine/0.1)\r\n"
								"Accept: *[/要使用这段请修改"*\r\n"
								"Host: sigma.4bpa.org\r\n"
								"Accept-Encoding: gzip, deflate\r\n"
								"Connection: close\r\n\r\n",
	"GET /%s HTTP/1.1\r\n"
								"User-Agent: Snow/0.a7 (TengAttack App Engine/0.1)\r\n"
								"Accept: *[/要使用这段请修改"*\r\n"
								"Host: sigma.4bpa.org\r\n"
								"Accept-Encoding: gzip, deflate\r\n"
								"Connection: close\r\n"
								"Cookie: %s\r\n\r\n",
	"Allow\r\n",
	"Check Version\r\n",
	"Say: ",
	"Power: ",*/
	"sigma.4bpa.org",
	"tcp",
/*#ifdef _SNOW_ZERO
	"uplink.php?client=snowzero&action=init&version=0.a7",	//snowzero
#else
	"uplink.php?client=snow&action=init&version=0.a7",	//snow
#endif*/
	"[%04d] %04d/%02d/%02d %02d:%02d:%02d - %s\n",
	//Reg Dlg
	/*"[%s] 跳过。",
	"[%s] 注册成功！",
	"[%s] 注册失败！验证码错误！%s。",
	"跳过",
	"重试",
	"[%s] 注册失败！%s",
	"[%s] 注册失败！错误代码: %d",
	"[%s] 注册失败！未知程序错误！验证码是否输入了？",
	"[%s] 注册失败！获取信息失败！",
	"%s:%s\r\n",
	//ME REG
	"POST /register.gbza?&uid=bd_765432170432153%%7Cmo_2011-02-05-07-24-39-0688-0815&ua=bd_176_207_unknown%%7Ckjava_1-0-13-2_j1&from=757b HTTP/1.1\r\n"
								"Content-Type: application/octet-stream\r\n"
								"User-Agent: ja_176x207\r\n"
								"Connection: close\r\n"
								"Content-Length: %d\r\n"
								"Host: t.baidu.com\r\n\r\n",	//双%
	"\x04\x00\x00\x00\x8d\x00\x00\x00"
					"http://t.baidu.com/registerform.gbza?redirect=http%3A%2F%2Ft.baidu.com%2Ftieba%2Findex.gbza%3F\r\n"
					"http://t.baidu.com/register.gbza?\r\n"
					"utf-8,1,\r\n"
					"\x02\x00\x00\x00\x91\x00\x00\x00"
					"Referer:http://t.baidu.com/registerform.gbza?redirect=http%3A%2F%2Ft.baidu.com%2Ftieba%2Findex.gbza%3F\r\n"
					"Bd_Uid:00000000000000000000000000000000\r\n"
					"\x1E\x00\x00\x00\x00\x00\x00\x00",
	"username=%s\r\n"
						"password=%s\r\n"
						"sex=%d\r\n"
						"mobile=%s\r\n"
						"redirect=http://t.baidu.com/tieba/index.gbza?\r\n"
						"tpl=wc\r\n",
	"verifycode=%s\r\n"
						"username=%s\r\n"
						"password=%s\r\n"
						"sex=%d\r\n"
						"mobile=%s\r\n"
						"bdNeedVerify=1\r\n"
						"bdverify=%s\r\n"
						"bdstoken=%s\r\n"
						"bdtime=%s\r\n"
						"redirect=http://t.baidu.com/tieba/index.gbza?\r\n"
						"tpl=wc\r\n",
	"\xE9\xAA\x8C\xE8\xAF\x81\xE7\xA0\x81\xE9\x94\x99\xE8\xAF\xAF",*/
	"t.baidu.com",
	"您可能是第一次启动本程序，请做好相关设置。感谢您对本软件的支持。Copyright (C) 腾袭 TengAttack 2009 - 2013",
#ifdef _SNOW_ZERO
	"雪凛Zero v0.a7 - 腾袭 - 血色圣光技术联盟",
	"雪凛Zero",
#else
	"雪凛 v0.a7 - 腾袭 - 血色圣光技术联盟",
	"雪凛",
#endif
	"我在这里……",
	"TengAttack.Snow.1.0.0",
	"TengAttack - 雪凛",
	"bd_480_640_Snow:0996_1-0-10-17_w1",
	"http://sigma.4bpa.org",
	"PHPSESSID",
	"User-Agent: Snow/0.a7\r\n",
#ifdef _SNOW_OTHER_SERVER
	"http://mgsdyg.com/snow/",
	#else
	#ifdef _SNOW_ZERO
		"http://sigma.4bpa.org/?client=snowzero",	//snowzero
	#else
		"http://sigma.4bpa.org/?client=snow",	//snow
	#endif
#endif
	"tiebaclient!!!",
	"sign=",
	"sign_key=b6834583e7ca6f5959b29bb9d163c9cf",
	"sig=",
	"sigma->tool(%s){%s}",
	"sigma->sign(%s, %s){%s}",
	"tool->sign(%s, %s){%s}",
	"sign=",
#ifdef SNOW
	"snow",
	"1.2.1.6",
	"f7f54c158c76a9b1b7627e7e4dde13da464e3ae4"
#else
	"snowzero",
	"0.a7.1",
	"ebcac8212e2669d14019f3fe183d02e434d6d4e7"
#endif

};


const char* GetBaseTextA(uint32 uATextId)
{
	return g_szABaseText[uATextId];
}
