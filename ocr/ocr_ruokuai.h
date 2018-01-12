
#ifndef _SNOW_CUTE_OCR_RUOKUAI_H_
#define _SNOW_CUTE_OCR_RUOKUAI_H_ 1

#include "ocr_base.h"
#include <curlhelper.h>
#include <base/event.h>
#include <base/lock.h>

class OCRRuokuai : public OCRBase {
public:
  OCRRuokuai();
  ~OCRRuokuai();

protected:
  char *url_username;
  char *md5_password;
  bool m_stop;

  Event m_event;

  std::string lastId;
  static char *sz_ruokuai_typeid[];

  static void transformTiebaSquaredResult(std::string& in, std::string *out);

  void setRequestParams(curl_httppost **pformpost, OCRContentType content_type, uint8 *image_data, uint32 image_length);
  static curl_socket_t curlOpenSocket(SOCKET *data, curlsocktype purpose, struct curl_sockaddr *addr);
  static int curl_progress_callback(OCRRuokuai *that, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
  SOCKET m_socket;

public:
  virtual void destory();
  virtual OCRResultCode detect(OCRContentType content_type, uint8 *image_data, uint32 image_length, std::string *result, char **ret_info);
  virtual void stop();
  virtual bool isstop();
  virtual OCRResultCode markResultAsWrong();

};

#endif