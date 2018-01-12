
#include "stdafx.h"
#include "VerifyCode.h"

#ifdef _TA_VCODE_USE_SKIA
#include <skiahelper.h>
#include <core/SkMovie_gif.h>
#include <core/SkTime.h>
#include <images/SkImageEncoder.h>
#include <base/frame/frame_skia.h>
#else
#include <OleCtl.h>
#endif

#ifdef _SNOW_ZERO
#include <Settings/Replay.h>
#else
#include <net/replay.h>
#endif

#include <common/strconv.h>
#include <net/baidu/BaiduBase.h>


extern int WINAPI VcodeChangeCallback(BOOL bSetInfo = FALSE);

//::Lock CVerifyCode::m_draw_lock;

#ifndef _TA_VCODE_USE_SKIA
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  UINT  num = 0;          // number of image encoders  
  UINT  size = 0;         // size of the image encoder array in bytes  

  ImageCodecInfo* pImageCodecInfo = NULL;  

  //2.获取GDI+支持的图像格式编码器种类数以及ImageCodecInfo数组的存放大小  
  GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;  // Failure  

  //3.为ImageCodecInfo数组分配足额空间  
  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
  if (pImageCodecInfo == NULL)
    return -1;  // Failure  

  //4.获取所有的图像编码器信息  
  GetImageEncoders(num, size, pImageCodecInfo);

  //5.查找符合的图像编码器的Clsid  
  for (UINT j = 0; j < num; ++j)
  {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success  
    }
  }

  //6.释放步骤3分配的内存  
  free(pImageCodecInfo);
  return -1;  // Failure  
}
#endif

CVerifyCode::CVerifyCode(BYTE* picdata, int len, int type, int kind, OCRBase *ocr, bool unknowlen)
	: m_succeed(false)
	, m_finish(false)
#ifdef _TA_VCODE_USE_SKIA
	, m_vcode_bitmap(NULL)
	, m_vcode_gif(NULL)
#else
	, m_pStm(NULL)
	, m_gpimage(NULL)
	, m_hImageMem(NULL)
#endif
  , m_ocr(NULL)
  , m_ocr_wait(ocr)
  , m_has_ocr(false)
	, m_width(0)
	, m_height(0)
	, m_type(type)
	, m_kind(kind)
	, m_unknowlen(unknowlen)
	, m_finish_event(NULL)
	, m_destory_event(NULL)
{
#ifdef _TA_VCODE_USE_SKIA
	SkMemoryStream stream(picdata, len);

	SkGIFMovie *vcode_gif = new SkGIFMovie(&stream);
	if (vcode_gif->GetGif()) {
		if (vcode_gif->duration() > 0) {
			m_vcode_gif = vcode_gif;
		} else {
			m_vcode_bitmap = new SkBitmap;

			int width = vcode_gif->width();
			int height = vcode_gif->height();

			SkRect srcrect = {0, 0, SkIntToScalar(width), SkIntToScalar(height)};
			SkRect dstrect = {0, 0, SkIntToScalar(width), SkIntToScalar(height)};

			m_vcode_bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);
			m_vcode_bitmap->allocPixels();
			m_vcode_bitmap->setIsOpaque(false);

			SkCanvas canvas(*m_vcode_bitmap);

			canvas.drawBitmapRectToRect(vcode_gif->bitmap(), &srcrect, dstrect, NULL);

			SkDELETE(vcode_gif);
		}
		m_succeed = true;
	} else {

		SkDELETE(vcode_gif);

		SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
		if (codec) {
			stream.rewind();
			codec->setDitherImage(false);
		
			SkBitmap *bitmap = new SkBitmap;
			bitmap->setIsOpaque(false);
			if (codec->decode(&stream, bitmap, SkBitmap::kARGB_8888_Config, //kIndex8_Config
				SkImageDecoder::kDecodePixels_Mode)) {
				m_vcode_bitmap = bitmap;
				m_succeed = true;
			} else {
				SkDELETE(bitmap);
			}
			SkDELETE(codec);
		}
	}
	if (m_vcode_gif) {
		m_width = m_vcode_gif->bitmap().width();
		m_height = m_vcode_gif->bitmap().height();
	} else if (m_vcode_bitmap) {
		m_width = m_vcode_bitmap->width();
		m_height = m_vcode_bitmap->height();
	}
#else
	::CoInitialize(NULL);

	m_hImageMem = GlobalAlloc(GMEM_MOVEABLE, len);
	if (m_hImageMem != NULL)
	{
		LPVOID pvData = NULL;
		if ((pvData = GlobalLock(m_hImageMem)) == NULL) //锁定分配内存块
		{
			GlobalFree(m_hImageMem);
			m_hImageMem = NULL;
			m_succeed = false;
		} else {
			memcpy(pvData, picdata, len);
			GlobalUnlock(m_hImageMem);
			CreateStreamOnHGlobal(m_hImageMem, TRUE, &m_pStm);

			m_succeed = false;
			m_gpimage = Gdiplus::Image::FromStream(m_pStm);
			if (m_gpimage) {
				if (m_gpimage->GetWidth() > 0 && m_gpimage->GetHeight() > 0) {
					m_succeed = true;
				} else {
					delete m_gpimage;
					m_gpimage = NULL;
				}
			}
		}
	}
#endif
	if (m_succeed)
	{
    CopyImageBuffer(picdata, len);
		m_finish_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}
}

CVerifyCode::~CVerifyCode()
{
	if (!m_finish)
	{
		m_finish = true;
		Finish(NULL);
	}
	if (m_finish_event)
	{
		CloseHandle(m_finish_event);
		m_finish_event = NULL;
	}
#ifdef _TA_VCODE_USE_SKIA
	SkDELETE(m_vcode_bitmap);
	SkDELETE(m_vcode_gif);
#else
	if (m_gpimage)
	{
		delete m_gpimage;
		m_gpimage = NULL;
	}
	if (m_pStm)
	{
		m_pStm->Release();
		m_pStm = NULL;
	}
	if (m_hImageMem)
	{
		GlobalFree(m_hImageMem);
		m_hImageMem = NULL;
	}
#endif
	m_succeed = false;

	if (m_destory_event)
	{
		SetEvent(m_destory_event);
		//CloseHandle(m_destory_event);
	}
}

void CVerifyCode::CopyImageBuffer(uint8* image_data, uint32 image_length)
{
#define CALC_POSITION_OF_SMALL_IMAGE(src_width, src_height) \
  int x = 0, y = 0, width = 0, height = 0; \
  int o_w = (src_width); \
  int o_h = (src_height); \
  if (o_h > 180) { \
    width = 180 * o_w / o_h; \
    if (width > 145) { \
      width = 145; \
      height = 145 * o_h / o_w; \
    } else { \
      height = 180; \
    } \
  } \
  x = (145 - width) / 2; \
  y = (180 - height) / 2; \
  /* end of CALC_POSITION_OF_SMALL_IMAGE */

  //convert to gif
#ifdef _TA_VCODE_USE_SKIA
  const SkBitmap *bitmap;
  if (m_vcode_gif) {
    bitmap = &m_vcode_gif->bitmap();
  } else {
    bitmap = m_vcode_bitmap;
  }
  if (bitmap) {
    bool need_del = false;
    if (m_kind == _VCODE_FRIENDLY && bitmap->width() > 145) {
      SkBitmap *bm = new SkBitmap();

      CALC_POSITION_OF_SMALL_IMAGE(bitmap->width(), bitmap->height());

      SkRect srcrect = { 0, 0, SkIntToScalar(o_w), SkIntToScalar(o_h) };
      SkRect dstrect = { x, y, SkIntToScalar(width), SkIntToScalar(height) };

      bm->setConfig(SkBitmap::kARGB_8888_Config, width, height);
      bm->allocPixels();
      bm->setIsOpaque(true);  //FOR JPEG CONVERTION

      SkCanvas canvas(*bm);

      canvas.clear(SK_ColorWHITE);
      canvas.drawBitmapRectToRect(*bitmap, &srcrect, dstrect, NULL);

      bitmap = bm;
      need_del = true;
    }

    /*struct jpeg_compress_struct toWriteCinfo;  //定义jpeg对象
    struct jpeg_error_mgr jerr; //定义错误处理对象
    uint8 *jpeg_buf = NULL;
    size_t jpeg_len = 0;

    toWriteCinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&toWriteCinfo);

    open_memstream
    jpeg_mem_dest(&toWriteCinfo, jpeg_buf, jpeg_len);

    toWriteCinfo.image_width = bitmap->width();   //jpeg图像宽
    toWriteCinfo.image_height = bitmap->height();  //jpeg图像高
    toWriteCinfo.input_components = 4; //jpeg图像色彩通道数 
    toWriteCinfo.in_color_space = JCS_RGB; //jpeg图像色彩空间

    jpeg_set_defaults(&toWriteCinfo);
    jpeg_set_quality(&toWriteCinfo, 80, TRUE);

    jpeg_start_compress(&toWriteCinfo, TRUE);

    JSAMPROW row_pointer[1];     //JSAMPROW一般是unsigned char *类型
    //row_stride = toWriteCinfo.image_width * 3;
    //一行像素点的字节数，如果是索引图,就不需要乘以3了
    while (toWriteCinfo.next_scanline < toWriteCinfo.image_height)
    {
      //row_pointer[0] = &bmp_data[toWriteCinfo.next_scanline * row_stride];  //bmp_data是我malloc的一段内存用于存储bmp数据，下面再具体描述
      (void)jpeg_write_scanlines(&toWriteCinfo, row_pointer, 1);
      //将一行数据写到jpeg_buf里
    }

    jpeg_finish_compress(&toWriteCinfo);

    jpeg_destroy_compress(&toWriteCinfo);
    
    if (jerr.msg_code) {*/
    SkDynamicMemoryWStream stream;
    // !this failed to encode? (win)
    if (SkImageEncoder::EncodeStream(&stream, *bitmap, SkImageEncoder::kJPEG_Type, 80)) {
      size_t jpeg_len = stream.bytesWritten();
      uint8 *jpeg_buf = (uint8 *)malloc(jpeg_len);
      stream.copyTo(jpeg_buf);
    
      //set to our buffer finally
      m_image_buffer.SetBufferLink(jpeg_buf, jpeg_len, jpeg_len);
    } else {
      m_image_buffer.Write(image_data, image_length);
    }

    if (need_del) {
      SkDELETE(bitmap);
    }
  } else {
    m_image_buffer.Write(image_data, image_length);
  }
#else
  Gdiplus::Image *gdiimg = NULL;
  if (m_kind == _VCODE_FRIENDLY && m_gpimage->GetWidth() > 145) {
    CALC_POSITION_OF_SMALL_IMAGE(m_gpimage->GetWidth(), m_gpimage->GetHeight());

    Gdiplus::Bitmap *pTemp = new Gdiplus::Bitmap(145, 180);
    Gdiplus::Graphics *g = Gdiplus::Graphics::FromImage(pTemp);
    g->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g->Clear(Gdiplus::Color::White);

    Gdiplus::Rect dstrect(x, y, width, height);
    g->DrawImage(m_gpimage, dstrect, 0, 0, o_w, o_h, Gdiplus::UnitPixel);
    delete g;
    gdiimg = pTemp;
  } else {
    gdiimg = m_gpimage;
  }

  ULONG lQuality = 80;
  GUID jpeg_encoder_clsid;

  Gdiplus::EncoderParameters eps;
  eps.Count = 1;
  eps.Parameter[0].Guid = Gdiplus::EncoderQuality;
  eps.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
  eps.Parameter[0].NumberOfValues = 1;
  eps.Parameter[0].Value = &lQuality;

  GetEncoderClsid(L"image/jpeg", &jpeg_encoder_clsid);

  // Create output stream with 0 size
  IStream* pOutIStream = NULL;
  if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pOutIStream) != S_OK)
  {
    TRACE("Failed to create stream on global memory!\n");
    return;
  }

  Gdiplus::Status status = gdiimg->Save(pOutIStream, &jpeg_encoder_clsid, &eps);
  if (status != S_OK)
  {
    pOutIStream->Release();
    TRACE("Failed to save to stream!\n");
    return;
  }
  if (gdiimg != m_gpimage) {
    delete (Gdiplus::Bitmap *)gdiimg;
    gdiimg = NULL;
  }

  // get the size of the output stream
  LARGE_INTEGER lnOffset;
  ULARGE_INTEGER ulnSize;
  lnOffset.QuadPart = 0;
  if (pOutIStream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize) != S_OK)
  {
    pOutIStream->Release();
    TRACE("Failed to get size!\n");
    return;
  }

  // now move the pointer to the beginning of the stream
  //(the stream should now contain a complete JPG file, just as if it were on the hard drive)
  lnOffset.QuadPart = 0;
  if (pOutIStream->Seek(lnOffset, STREAM_SEEK_SET, NULL) != S_OK)
  {
    pOutIStream->Release();
    TRACE("Failed to move to end of stream!\n");
    return;
  }

  //copy the stream JPG to memory
  DWORD dwJpgSize = (DWORD)ulnSize.QuadPart;
  BYTE* pJPG = (BYTE *)malloc(dwJpgSize);
  if (pOutIStream->Read(pJPG, dwJpgSize, NULL) != S_OK)
  {
    delete pJPG;
    pOutIStream->Release();
    TRACE("Failed to read pBMP!\n");
    return;
  }

  pOutIStream->Release();

  //set to our buffer finally
  m_image_buffer.SetBufferLink(pJPG, dwJpgSize, dwJpgSize);
#endif
}

HANDLE CVerifyCode::Finish(LPCSTR vcodetext, bool destoryevent)
{
	if (vcodetext)
	{
		m_vcodetext = vcodetext;
	} else {
		m_vcodetext.clear();
	}

	//创建销毁事件
	if (destoryevent)
	{
		m_destory_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	HANDLE hdestory = m_destory_event;
	m_finish = true;

	if (m_finish_event)
	{
		::SetEvent(m_finish_event);
	}

	return hdestory;
}

void CVerifyCode::Wait()
{
	if (m_finish_event) ::WaitForSingleObject(m_finish_event, INFINITE);
}

bool CVerifyCode::animateShift()
{
#ifdef _TA_VCODE_USE_SKIA
	if (m_vcode_gif) {
		return m_vcode_gif->setTime(SkTime::GetMSecs() % m_vcode_gif->duration());
	}
#endif
	return false;
}

void CVerifyCode::Draw(HDC hdc, int x, int y, int width, int height)
{
#ifdef _TA_VCODE_USE_SKIA
	if (m_vcode_gif) {
		view::frame::SkiaPaint(hdc, x, y, width, height, m_vcode_gif->bitmap(), 0, 0);
	} else if (m_vcode_bitmap) {
		view::frame::SkiaPaint(hdc, x, y, width, height, *m_vcode_bitmap, 0, 0);
	}
#else
	if (m_succeed && m_gpimage)
	{
		Gdiplus::Graphics g(hdc);
		g.DrawImage(m_gpimage, x, y, width, height);
	}
#endif
}

void CVerifyCode::Draw(
#ifdef _TA_VCODE_USE_SKIA
		SkCanvas* canvas,
#else
		Gdiplus::Graphics* canvas,
#endif
		int x, int y, int width, int height, int srcwdith, int srcheight)
{
#ifdef _TA_VCODE_USE_SKIA
	if (m_vcode_bitmap || m_vcode_gif) {
		if (srcwdith == 0) {
			srcwdith = width;
		}
		if (srcheight == 0) {
			srcheight = height;
		}
		SkRect srcrect = {0, 0, srcwdith, srcheight};
		SkRect dstrect = {x, y, x + width, y + height};
		if (m_vcode_gif) {
			canvas->drawBitmapRectToRect(m_vcode_gif->bitmap(), &srcrect, dstrect, NULL);
		} else {
			canvas->drawBitmapRectToRect(*m_vcode_bitmap, &srcrect, dstrect, NULL);
		}
	}
#else
	if (m_gpimage) {
		if (srcwdith == 0) {
			srcwdith = width;
		}
		if (srcheight == 0) {
			srcheight = height;
		}
		Gdiplus::Rect dstrect(x, y, width, height);
		canvas->DrawImage(m_gpimage, dstrect, 0, 0, srcwdith, srcheight, Gdiplus::UnitPixel);
	}
#endif
}

//VerifyCodeQueue
CVerifyCodeQueue::CVerifyCodeQueue(int count)
	: m_count(count)
	, m_closed(false)
	, m_accumulate(false)
	, m_autoreplay_autoclean(false)
	, m_autorelease_event(NULL)
	, m_autoreplay_times(0)
	, m_autorelease_time(0)
	, m_release_times(0)
	, m_release_accu_count(0)
{
	//m_wait_event = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CVerifyCodeQueue::~CVerifyCodeQueue()
{
	m_closed = true;
	//SetEvent(m_wait_event);

	SetAutoReleaseTime(-1);

	AutoLock al(m_add_lock);

	for (unsigned long i = 0; i < m_verifycode.size(); i++)
	{
		if (m_verifycode[i]) delete m_verifycode[i];
	}
	m_verifycode.clear();
	m_count = 0;
	
	//CloseHandle(m_wait_event);
}

int CVerifyCodeQueue::Add(BYTE* picdata, int len, int type, int kind, OCRBase *ocr, bool unknowlen)
{
	if (m_closed || IsSkipType(type))
	{
		return -1;
	}

	AutoLock al(m_add_lock);
	/*if (m_closed || IsSkipType(type))
	{
		return -1;
	}*/

	CVerifyCode* pvcode = new CVerifyCode(picdata, len, type, kind, ocr, unknowlen);
	if (pvcode->Succeed())
	{
		//while (m_verifycode.size() > m_count)
		{
			//需要在这里暂停
			//::WaitForSingleObject(m_wait_event, INFINITE);
			//::ResetEvent(m_wait_event);

			if (m_closed || IsSkipType(type))
			{
				delete pvcode;
				return -1;
			}

		}

		m_verifycode.push_back(pvcode);
    OCRDetect();
		return m_verifycode.size() - 1;
	} else {
		delete pvcode;
		return -1;
	}
}

void CVerifyCodeQueue::Delete(int i, bool destory, bool checkaccumulate)
{
	AutoLock al(m_delete_lock);

	CVerifyCode* vcode;

	if (i >= 0 && i < m_verifycode.size())
	{
		vcode = m_verifycode[i];
	
		bool call_event = false;
		if (m_verifycode.size() >= m_count)
		{
			call_event = true;
		}
		m_verifycode.erase(m_verifycode.begin() + i);

		if (vcode && destory)
		{
			delete vcode;
		}

		//if (call_event) SetEvent(m_wait_event);

		Callback();
	}
}

void CVerifyCodeQueue::Delete(CVerifyCode* p, bool destory, bool checkaccumulate)
{
	AutoLock al(m_delete_lock);

	std::vector<CVerifyCode *>::iterator iter = m_verifycode.begin();
	CVerifyCode *pvcode = NULL;

	bool call_event = false;
	int delete_count = 0;
	if (m_verifycode.size() >= m_count)
	{
		call_event = true;
	}

	while (iter != m_verifycode.end())
	{
		pvcode = *iter;
		if (pvcode == p)
		{
			delete_count++;

			m_verifycode.erase(iter);

			if (destory) delete pvcode;
			break;
		}
		iter++;
	}

	//积攒验证码中的
	if (checkaccumulate)
	{
		std::vector<AccumulateVerifyCode>::iterator iter2 = m_accu_vcode.begin();
		while (iter2 != m_accu_vcode.end())
		{
			if (iter2->vcode == p)
			{
				pvcode = iter2->vcode;
				//delete_count++;
				//积攒验证码不唤醒事件
				m_accu_vcode.erase(iter2);

				if (destory) delete pvcode;
				break;
			}
			iter2++;
		}
	}

	/*if (delete_count > 0 && call_event)
	{
		SetEvent(m_wait_event);
	}*/
	if (delete_count > 0)
	{
    OCRDetect();
		Callback();
	}
}

void CVerifyCodeQueue::FinishType(int type)
{
	std::vector<CVerifyCode *>::iterator iter = m_verifycode.begin();
	CVerifyCode *pvcode = NULL;

	bool call_event = false;
	int delete_count = 0;
	if (m_verifycode.size() >= m_count)
	{
		call_event = true;
	}

	//std::vector<HANDLE> m_vec_event;
	std::vector<CVerifyCode *> m_vec_vcode;

	while (iter != m_verifycode.end())
	{
		pvcode = *iter;
		if (pvcode)
		{
			if (pvcode->GetType() == type)
			{
				delete_count++;
				m_vec_vcode.push_back(pvcode);
			}
		}
		iter++;
	}

	std::vector<AccumulateVerifyCode>::iterator iter2 = m_accu_vcode.begin();
	while (iter2 != m_accu_vcode.end())
	{
		if (iter2->vcode)
		{
			if (iter2->vcode->GetType() == type)
			{
				pvcode = iter2->vcode;
				m_vec_vcode.push_back(pvcode);
			}
		}
		iter2++;
	}

	if (delete_count > 0)
	{
		unsigned long vcode_count = m_vec_vcode.size();
		HANDLE *vec_handle = (HANDLE *)malloc(sizeof(HANDLE) * vcode_count);

		for (unsigned long i = 0; i < vcode_count; i++)
		{
      m_vec_vcode[i]->StopOCR();
			vec_handle[i] = m_vec_vcode[i]->Finish(NULL, true);
		}

		::WaitForMultipleObjects(vcode_count, vec_handle, TRUE, INFINITE);

		for (unsigned long i = 0; i < vcode_count; i++)
		{
			CloseHandle(vec_handle[i]);
		}

		free(vec_handle);

		/*if (call_event)
		{
			SetEvent(m_wait_event);
		}*/
	}
}

CVerifyCode* CVerifyCodeQueue::GetVerifyCode(int i)
{
	if (i >= 0 && i < m_verifycode.size())
	{
		return m_verifycode[i];
	}
	return NULL;
}

LPCSTR CVerifyCodeQueue::GetVcodeText(int i)
{
	if (i >= 0 && i < m_verifycode.size())
	{
		if (m_verifycode[i])
		{
			if (m_verifycode[i]->GetVcodeText().length() > 0)
			{
				return m_verifycode[i]->GetVcodeText().c_str();
			}
		}
	}
	return NULL;
}

void CVerifyCodeQueue::Draw(int i, HDC hdc, int x, int y, int width, int height)
{
	if (i >= 0 && i < m_verifycode.size())
	{
		if (m_verifycode[i])
		{
			m_verifycode[i]->Draw(hdc, x, y, width, height);
		}
	}
}

void CVerifyCodeQueue::Draw(int i, 
#ifdef _TA_VCODE_USE_SKIA
		SkCanvas* canvas,
#else
		Gdiplus::Graphics* canvas,
#endif
		 int x, int y, int width, int height, int srcwdith, int srcheight)
{
	if (i >= 0 && i < m_verifycode.size())
	{
		if (m_verifycode[i])
		{
			m_verifycode[i]->Draw(canvas, x, y, width, height, srcwdith, srcheight);
		}
	}
}

void CVerifyCodeQueue::JumpHead()
{
  if (m_verifycode.size() > 0)
  {
    if (m_verifycode[0]->HasOCR()) {
      m_verifycode[0]->StopOCR();
    }
    Finish(0, NULL);
  }
}

void CVerifyCodeQueue::Finish(int i, LPCSTR vcodetext)
{
	if (i >= 0 && i < m_verifycode.size())
	{
		if (m_verifycode[i])
		{
			if (vcodetext && m_accumulate)
			{
				//积攒验证码
				Accumulate(m_verifycode[i], vcodetext);
			} else {
				m_verifycode[i]->Finish(vcodetext);
			}
		}
	}
}

void CVerifyCodeQueue::Finish(CVerifyCode* p, LPCSTR vcodetext)
{
  if (p) {
    std::vector<CVerifyCode *>::iterator iter = m_verifycode.begin();
    CVerifyCode *pvcode = NULL;

    while (iter != m_verifycode.end()) {
      pvcode = *iter;
      if (pvcode == p) {
        //Has
        if (vcodetext && m_accumulate) {
          //积攒验证码
          Accumulate(p, vcodetext);
        } else {
          p->Finish(vcodetext);
        }
        break;
      }
      iter++;
    }
  }
}

bool CVerifyCodeQueue::GetVerifyCodeSize(int i, HDC hdc, SIZE& size)
{
	if (i >= 0 && i < m_verifycode.size())
	{
		if (m_verifycode[i])
		{
#ifdef _TA_VCODE_USE_SKIA
			if (m_verifycode[i]->animate() || m_verifycode[i]->isSkBitmap()) {
				size.cx = m_verifycode[i]->m_width;
				size.cy = m_verifycode[i]->m_height;
				return true;
			}
#else
			Gdiplus::Image *gpimage = m_verifycode[i]->GetGPImage();
			if (gpimage)
			{
				size.cx = gpimage->GetWidth();
				size.cy = gpimage->GetHeight();

				return true;
			}
#endif
		}
	}
	return false;
}

bool CVerifyCodeQueue::IsSkipType(int type)
{
	//检查重复
	bool bfind = false;
	for (unsigned long i = 0; i < m_skip_type.size(); i++)
	{
		if (m_skip_type[i] == type)
		{
			bfind = true;
			break;
		}
	}
	return bfind;
}

void CVerifyCodeQueue::AddSkipType(int type)
{
	if (!IsSkipType(type))
		m_skip_type.push_back(type);
}

void CVerifyCodeQueue::DeleteSkipType(int type)
{
	AutoLock al(m_delete_lock);

	for (unsigned long i = 0; i < m_skip_type.size(); i++)
	{
		if (m_skip_type[i] == type)
		{
			m_skip_type.erase(m_skip_type.begin() + i);
			break;
		}
	}
}

void CVerifyCodeQueue::ReleaseAccumulate()
{
	if (m_accumulate && m_accu_vcode.size() > 0)
	{
		std::vector<HANDLE> m_vec_event;
		std::vector<AccumulateVerifyCode> tmp_accu_vcode = m_accu_vcode;

		for (unsigned long i = 0; i < tmp_accu_vcode.size(); i++)
		{
			if (tmp_accu_vcode[i].vcode)
			{
				m_vec_event.push_back(
					tmp_accu_vcode[i].vcode->Finish(tmp_accu_vcode[i].vcodetext.c_str(), true));
			}
		}

		::WaitForMultipleObjects(m_vec_event.size(),
				 &m_vec_event[0], TRUE, INFINITE);

		for (unsigned long i = 0; i < m_vec_event.size(); i++)
		{
			CloseHandle(m_vec_event[i]);
		}

		m_accu_vcode.clear();

		if (m_autoreplay_times > 0)
		{
			if (!snow::m_replay.IsReplaying()) m_release_times++;
			if (m_release_times >= m_autoreplay_times)
			{
				m_release_times = 0;
				//重拨
#ifdef SNOW
				SnowReplayAsyn(m_autoreplay_autoclean, NULL, NULL);
#else
				//_SNOW_ZERO etc...
				SnowReplayAsyn(m_autoreplay_autoclean);
#endif
			}
		}
	}
}

int CVerifyCodeQueue::GetAccumulateCount()
{
	return m_accu_vcode.size();
}

void CVerifyCodeQueue::Accumulate(CVerifyCode* p, LPCSTR vcodetext)
{
	if (p)
	{
		AccumulateVerifyCode avc = {p, vcodetext};
		m_accu_vcode.push_back(avc);
		Delete(p, false, false);

		if (m_release_accu_count > 0)
		{
			if (GetAccumulateCount() >= m_release_accu_count)
			{
				//释放积攒验证码
				ReleaseAccumulate();
			}
		}

		Callback();
	}
}

//bool CVerifyCodeQueue::m_callbacking = false;
void CVerifyCodeQueue::Callback(BOOL bSetInfo)
{
	/*if (m_callbacking)
	{
		return;
	}

	m_callback_lock.Acquire();*/

	//m_callbacking = true;
	VcodeChangeCallback(bSetInfo);
	//m_callbacking = false;

	//m_callback_lock.Release();
}

void CVerifyCodeQueue::Lock()
{
	//m_add_lock.Acquire();
	m_delete_lock.Acquire();
}

void CVerifyCodeQueue::UnLock()
{
	//m_add_lock.Release();
	m_delete_lock.Release();
}

void CVerifyCodeQueue::SetAutoReleaseTime(int time)
{
	if (time == -1 || time == 0)
	{
		if (m_autorelease_event)
		{
			SetEvent(m_autorelease_event);
			CloseHandle(m_autorelease_event);
			m_autorelease_event = NULL;
		}
		/*if (m_ar_thread)
		{
			CloseHandle(m_ar_thread);
		}*/
		m_autorelease_time = 0;
	} else {
		if (m_autorelease_time != time)
		{
			SetAutoReleaseTime(-1);
		}
		m_autorelease_time = time;

		if (!m_autorelease_event)
		{
			m_autorelease_event = CreateEvent(NULL, TRUE, FALSE, NULL);
			HANDLE m_ar_thread = ::CreateThread(NULL, 0, AutoReleaseProc, this, NULL, NULL);
			CloseHandle(m_ar_thread);
		}
	}
}

void CVerifyCodeQueue::SetAutoReleaseCount(int count)
{
	if (count == -1 || count == 0)
	{
		m_release_accu_count = 0;
	} else {
		m_release_accu_count = count;
	}
}

void CVerifyCodeQueue::SetAutoReplay(int times, bool autoclean)
{
	if (times == -1 || times == 0)
	{
		m_autoreplay_times = 0;
		m_autoreplay_autoclean = false;
	} else {
		m_autoreplay_times = times;
		m_autoreplay_autoclean = autoclean;
	}
	m_release_times = 0;
}

DWORD WINAPI CVerifyCodeQueue::AutoReleaseProc(LPVOID lParam)
{
	CVerifyCodeQueue *pQueue = (CVerifyCodeQueue *)lParam;
	if (!pQueue) return 0;

	DWORD ret;
	while (true)
	{
		ret = ::WaitForSingleObject(pQueue->m_autorelease_event, pQueue->m_autorelease_time);
		if (ret == WAIT_TIMEOUT)
		{
			pQueue->ReleaseAccumulate();
			pQueue->Callback(TRUE);
		} else {
			break;
		}
	}

	return 0;
}

int CVerifyCodeQueue::SkipAll()
{
	std::vector<CVerifyCode *>::iterator iter = m_verifycode.begin();
	CVerifyCode *pvcode = NULL;

	bool call_event = false;
	int delete_count = 0;
	if (m_verifycode.size() >= m_count)
	{
		call_event = true;
	}

	//std::vector<HANDLE> m_vec_event;
	std::vector<CVerifyCode *> m_vec_vcode;

	while (iter != m_verifycode.end())
	{
		pvcode = *iter;
		if (pvcode)
		{
			delete_count++;
			m_vec_vcode.push_back(pvcode);
		}
		iter++;
	}

	std::vector<AccumulateVerifyCode>::iterator iter2 = m_accu_vcode.begin();
	while (iter2 != m_accu_vcode.end())
	{
		if (iter2->vcode)
		{
			pvcode = iter2->vcode;
			m_vec_vcode.push_back(pvcode);
		}
		iter2++;
	}

	if (delete_count > 0)
	{
		unsigned long vcode_count = m_vec_vcode.size();
		HANDLE *vec_handle = (HANDLE *)malloc(sizeof(HANDLE) * vcode_count);

		for (unsigned long i = 0; i < vcode_count; i++)
		{
			vec_handle[i] = m_vec_vcode[i]->Finish(NULL, true);
		}

		::WaitForMultipleObjects(vcode_count, vec_handle, TRUE, INFINITE);

		for (unsigned long i = 0; i < vcode_count; i++)
		{
			CloseHandle(vec_handle[i]);
		}

		free(vec_handle);

		/*if (call_event)
		{
			SetEvent(m_wait_event);
		}*/
	}

	return delete_count;
}

typedef struct _OCR_PROC_DATA {
  CVerifyCodeQueue* pqueue;
  CVerifyCode* pvc;
  OCRBase* ocr;
  OCRBase::OCRContentType oct;
} OCR_PROC_DATA;

void CVerifyCodeQueue::OCRDetect()
{
  if (!OCRBase::enable()) {
    return;
  }

  AutoLock al(m_delete_lock);

  int max_threads = *(int *)OCRBase::getValue(OCRBase::kOCRKMaxThreads);
  int ocrcount = 0;
  int i = 0;
  while (ocrcount < max_threads && i < m_verifycode.size()) {
    if (m_verifycode[i]->HasOCR()) {
      ocrcount++;
    } else {
      int ikind = m_verifycode[i]->GetKind();

      OCRBase::OCRContentType oct;
      if (ikind == _VCODE_FRIENDLY) {
        oct = OCRBase::kOCRCTTiebaSquared;
      } else if (ikind == _VCODE_FRIENDLY_WITH_PINYIN) {
        oct = OCRBase::kOCRCTTiebaSquaredWithPinYin;
      } else if (ikind == _VCODE_NORMAL) {
        oct = OCRBase::kOCRCT4NumberAndLetter;
      } else {
        oct = OCRBase::kOCRCT4Chinese;
      }
      OCRBase *ocr = m_verifycode[i]->EnableOCR();
      if (ocr) {
        
        OCR_PROC_DATA *opd = (OCR_PROC_DATA *)malloc(sizeof(OCR_PROC_DATA));
        opd->pqueue = this;
        opd->pvc = m_verifycode[i];
        opd->ocr = ocr;
        opd->oct = oct;

        HANDLE hThread = ::CreateThread(NULL, 0, OCRDetectProc, opd, NULL, NULL);
        CloseHandle(hThread);

        ocrcount++;
      }
    }
    i++;
  }
}

DWORD WINAPI CVerifyCodeQueue::OCRDetectProc(LPVOID lParam)
{
  static bool writed = false;
  OCR_PROC_DATA *opd = (OCR_PROC_DATA *)lParam;
  if (!opd) {
    return 0;
  }

  CBuffer *buffer = opd->pvc->GetImageBuffer();
  std::string ocr_result;
  char *ret_info = NULL;
  bool need_finish = false;

  int retry_times = 0;

  OCRBase::OCRResultCode ocr_code = OCRBase::kOCRErrorUnknown;

  //accq delete lock
  opd->ocr->lock();

  {
try_again_l:
    ocr_code = opd->ocr->detect(opd->oct, buffer->GetBuffer(), buffer->GetBufferLen(), &ocr_result, &ret_info);

    if (ret_info) {
      SAFE_FREE(ret_info);
    }

    if (ocr_code == OCRBase::kOCRErrorNetwork && retry_times < 3) {
      retry_times++;
      goto try_again_l;
    }

    if (!opd->ocr->isstop()) {
      need_finish = true;
    }
  }

  //destory() in VerifyRecognizeEnd
  opd->ocr->unlock();

  if (need_finish) {
    opd->pqueue->Finish(opd->pvc, ocr_code == OCRBase::kOCRSuccess ? ocr_result.c_str() : NULL);
    //opd->pqueue->Callback();
  }

  free(opd);
  return 0;
}
