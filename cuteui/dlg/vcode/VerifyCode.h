
#ifndef _TA_VERIFY_CODE_H_
#define _TA_VERIFY_CODE_H_ 1

#include <windows.h>
#include <OCIdl.h>
#include <vector>
#include <base/lock.h>
#include <common/buffer.h>
#include <ocr/ocr_base.h>

#if !defined(_SNOW_ZERO)
#define _TA_VCODE_USE_SKIA 1
#endif

#ifdef _TA_VCODE_USE_SKIA
class SkBitmap;
class SkCanvas;
class SkGIFMovie;
#else
namespace Gdiplus {
	class Image;
	class Graphics;
};
#endif

class CVerifyCode
{
public:
	friend class CVerifyCodeQueue;

  explicit CVerifyCode(BYTE* picdata, int len, int type, int kind, OCRBase *ocr, bool unknowlen = false);
	~CVerifyCode();

	int m_width, m_height;

	inline bool Succeed()
	{
		return m_succeed;
	}
	inline int GetType()
	{
		return m_type;
	}

	inline int GetKind()	//种类，神兽或一般
	{
		return m_kind;
	}

	inline bool IsUnknowLen()
	{
		return m_unknowlen;
	}

	inline bool animate()
	{
#ifdef _TA_VCODE_USE_SKIA
		return (m_vcode_gif != NULL);
#else
		return false;
#endif
	}

	bool animateShift();

	inline std::string& GetVcodeText()
	{
		return m_vcodetext;
	}

	inline HANDLE GetFinishEvent()
	{
		return m_finish_event;
	}

#ifdef _TA_VCODE_USE_SKIA
	inline bool isSkBitmap() {
		return m_vcode_bitmap != NULL;
	}

	inline SkBitmap* getSkBitmap() {
		return m_vcode_bitmap;
	}
#else
	inline Gdiplus::Image* GetGPImage()
	{
		return m_gpimage;
	}
#endif

	virtual void Draw(HDC hdc, int x, int y, int width, int height);
	virtual void Draw(
#ifdef _TA_VCODE_USE_SKIA
		SkCanvas* canvas,
#else
		Gdiplus::Graphics* canvas,
#endif
		int x, int y, int width, int height, int srcwdith = 0, int srcheight = 0);

	virtual HANDLE Finish(LPCSTR vcodetext, bool destoryevent = false);
	virtual void Wait();

  inline OCRBase* EnableOCR() {
    m_ocr = m_ocr_wait;
    if (m_ocr) {
      m_ocr->setVerifyCode(this);
    }
    return m_ocr;
  }
  inline bool HasOCR() {
    return m_ocr != NULL;
  }
  inline void StopOCR() {
    if (m_ocr) {
      m_ocr->stop();
    }
  }

  void CopyImageBuffer(uint8* image_data, uint32 image_length);
  inline CBuffer* GetImageBuffer() {
    return &m_image_buffer;
  }


protected:
	int m_type;
	int m_kind;
	bool m_unknowlen;
	bool m_succeed;
	bool m_finish;
	HANDLE m_finish_event;
	HANDLE m_destory_event;

  bool m_has_ocr;
  OCRBase *m_ocr, *m_ocr_wait;
  CBuffer m_image_buffer;

#ifdef _TA_VCODE_USE_SKIA
	SkBitmap* m_vcode_bitmap;
	SkGIFMovie* m_vcode_gif;
#else
	IStream *m_pStm;
	Gdiplus::Image *m_gpimage;

	HGLOBAL m_hImageMem;
#endif
	std::string m_vcodetext;

	//static ::Lock m_draw_lock;
};

class CVerifyCodeQueue
{
public:

	struct AccumulateVerifyCode {
		CVerifyCode* vcode;
		std::string vcodetext;
	};

	friend class AutoLock;
	friend class CVerifyCode;

	explicit CVerifyCodeQueue(int count);
	~CVerifyCodeQueue();

	/*void LockDraw()
	{
		CVerifyCode::m_draw_lock.Acquire();
	}

	void UnlockDraw()
	{
		CVerifyCode::m_draw_lock.Release();
	}*/

	void Callback(BOOL bSetInfo = FALSE);
	int SkipAll();

	int GetCount()
	{
		return m_verifycode.size();
	}

	void Draw(int i, HDC hdc, int x, int y, int width, int height);
	void Draw(int i, 
#ifdef _TA_VCODE_USE_SKIA
		SkCanvas* canvas,
#else
		Gdiplus::Graphics* canvas,
#endif
		 int x, int y, int width, int height, int srcwdith = 0, int srcheight = 0);

  void JumpHead();
	void Finish(int i, LPCSTR vcodetext);
  void Finish(CVerifyCode* p, LPCSTR vcodetext);

  int Add(BYTE* picdata, int len, int type, int kind, OCRBase *ocr, bool unknowlen = false);

	void Delete(CVerifyCode* p, bool destory = true, bool checkaccumulate = true);
	void Delete(int i, bool destory = true, bool checkaccumulate = true);

	void FinishType(int type);
	bool GetVerifyCodeSize(int i, HDC hdc, SIZE& size);

	LPCSTR GetVcodeText(int i);

	CVerifyCode* GetVerifyCode(int i = 0);
	inline int GetVerifyCodeType(int i) {
		CVerifyCode* v = GetVerifyCode(i);
		if (v) {
			return v->GetType();
		}
		return 0;
	}
	inline int GetVerifyCodeKind(int i) {
		CVerifyCode* v = GetVerifyCode(i);
		if (v) {
			return v->GetKind();
		}
		return 0;
	}

	//跳过验证码
	bool IsSkipType(int type);
	void AddSkipType(int type);
	void DeleteSkipType(int type);

	//积攒验证码
	void Accumulate(CVerifyCode* p, LPCSTR vcodetext);
	void ReleaseAccumulate();
	int GetAccumulateCount();

	void SetAutoReleaseTime(int time);	//-1或0就是关闭
	void SetAutoReleaseCount(int count);	//-1或0就是关闭
	void SetAutoReplay(int times, bool autoclean = false);	//-1或0就是关闭

	static DWORD WINAPI AutoReleaseProc(LPVOID lParam);
	
	inline void SetAccumulate(bool accumulate)
	{
		m_accumulate = accumulate;
	}

	int m_autorelease_time;
	int m_autoreplay_times;
	bool m_autoreplay_autoclean;
	HANDLE m_autorelease_event;
	//HANDLE m_ar_thread;

  void OCRDetect();
  static DWORD WINAPI OCRDetectProc(LPVOID lParam);

	void Lock();
	void UnLock();

protected:
	
	//HANDLE m_wait_event;

	int m_count;
	int m_release_accu_count;
	int m_release_times;

	bool m_closed;
	bool m_accumulate;

	std::vector<CVerifyCode *> m_verifycode;
	std::vector<AccumulateVerifyCode> m_accu_vcode;
	std::vector<int> m_skip_type;
	
	//::Lock m_callback_lock;
	::Lock m_add_lock;
	::Lock m_delete_lock;

	//static bool m_callbacking;
};

#endif