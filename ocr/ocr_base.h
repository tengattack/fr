
#ifndef _SNOW_CUTE_OCR_BASE_H_
#define _SNOW_CUTE_OCR_BASE_H_ 1

#include <base/basictypes.h>
#include <base/common.h>
#include <base/lock.h>

#include <string>

enum SnowNetOCRPlatform {
  kSNOCRRuokuai = 0,
  kSNOCRCount
};

typedef struct _OCR_CONFIG_VALUE {
  std::wstring type;
  int platform;
  std::wstring username;
  std::wstring password;
  int timeout;
  int max_threads;
  bool benable;
} OCR_CONFIG_VALUE;

namespace base {
  class CJSONConfig;
};

class CVerifyCode;

class OCRBase {
protected:
  OCRBase();
  virtual ~OCRBase();

  static void load();

  static const char *sz_key_name[];
  static base::CJSONConfig *m_config;
  static OCR_CONFIG_VALUE m_value;
  static OCR_CONFIG_VALUE m_value_tmp;

  Lock m_lock;

public:

  enum OCRKey {
    kOCRKType = 0,
    kOCRKPlatform,
    kOCRKUsername,
    kOCRKPassword,
    kOCRKTimeout,
    kOCRKMaxThreads,
    kOCRKEnable,
    kOCRKCount
  };

  enum OCRContentType {
    kOCRCT4Number = 0,
    kOCRCT4Letter,
    kOCRCT4NumberAndLetter,
    kOCRCT4Chinese,
    kOCRCTTiebaSquared,
    kOCRCTTiebaSquaredWithPinYin,
    kOCRCTCount
  };

  enum OCRResultCode {
    kOCRSuccess = 0,
    kOCRErrorUnknown,
    kOCRErrorParams,
    kOCRErrorResult,
    kOCRErrorNetwork,
    kOCRErrorTimeout,
    kOCRErrorAborted,
    kOCRErrorUnsupported,
  };

  static OCRBase* create();
  virtual void destory() = 0;
  virtual OCRResultCode detect(OCRContentType content_type, uint8 *image_data, uint32 image_length, std::string *result, char **ret_info = NULL) = 0;
  virtual void stop();
  virtual bool isstop();
  virtual OCRResultCode markResultAsWrong() = 0;

  inline Lock& getLock() {
    return m_lock;
  };
  virtual void lock();
  virtual void unlock();

  inline void setVerifyCode(CVerifyCode *pvc) {
    m_pvc = pvc;
  };

  static bool enable();

  static void save();
  static bool dirty();
  static void undirty();
  static bool setValue(OCRKey key, const void *value);
  static const void* getValue(OCRKey key);

  static void InitOCRSettings(LPCTSTR config_file);
  static void Uninit();

  inline OCRResultCode getLastResult() {
    return m_last_result;
  }

  inline std::string& getLastErrorMessage() {
    return m_last_errmsg;
  }

protected:
  std::string m_last_errmsg;
  OCRResultCode m_last_result;

  CVerifyCode *m_pvc;
};

#endif