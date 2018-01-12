
#include "stdafx.h"
#include "ocr_ruokuai.h"

#include <base/md5.h>
#include <base/json/values.h>
#include <base/json/json_reader.h>
#include <base/string/stringprintf.h>
#include <base/string/string_number_conversions.h>
#include <common/Urlcode.h>
#include <common/strconv.h>
#include <net/http/TANetBase.h>

#include <dlg/vcode/VerifyCode.h>

#include <curlhelper.h>


#define RUOKUAI_API_URL "http://api.ruokuai.com/create.json"
#define RUOKUAI_API_REPORT_ERROR_URL "http://api.ruokuai.com/reporterror.json"

#if defined(SNOW)
#define RUOKUAI_SOFTID "20448"
#define RUOKUAI_SOFTKEY "5ceeb8735f86b751d74cb802a221aeb9"
#elif defined(_SNOW_ZERO)
#define RUOKUAI_SOFTID "20516"
#define RUOKUAI_SOFTKEY "85b952247d072b5c312c890b948291ee"
#elif defined(_TB_FANTASY)
#define RUOKUAI_SOFTID "21723"
#define RUOKUAI_SOFTKEY "be276ee175a74cf4a820a0e2929dd773"
#endif

char* OCRRuokuai::sz_ruokuai_typeid[kOCRCTCount] = {
  "1040",
  "2040",
  "3040",
  "4040",
  "6103", //Ìù°É¾Å¹¬¸ñ
  "6104"
};

OCRRuokuai::OCRRuokuai()
  : OCRBase()
  , url_username(NULL)
  , md5_password(NULL)
  , m_stop(true)
  , m_socket(NULL)
{
  if (m_value.username.length() > 0) {
    char *utf8_username = NULL;
    lo_W2Utf8(&utf8_username, m_value.username.c_str());
    if (utf8_username) {
      lo_UrlEncodeA(&url_username, utf8_username, lstrlenA(utf8_username));
      free(utf8_username);
    }
  }
  if (m_value.password.length() > 0) {
    char *utf8_password = NULL;
    lo_W2Utf8(&utf8_password, m_value.password.c_str());
    if (utf8_password) {
      md5_password = _strdup(base::MD5String(utf8_password).c_str());
      //lo_UrlEncodeA(&md5_password, utf8_password, lstrlenA(utf8_password));
      free(utf8_password);
    }
  }
}

OCRRuokuai::~OCRRuokuai()
{
  if (url_username) free(url_username);
  if (md5_password) free(md5_password);
}

void OCRRuokuai::destory()
{

  m_lock.Acquire();
  m_event.Wait();

  m_lock.Release();

  //final delete
  delete this;
}

void OCRRuokuai::transformTiebaSquaredResult(std::string& in, std::string *out)
{
  const int CHAR_COUNT = 9;
  static char* input_str_map[CHAR_COUNT] = {
    "00000000", "00010000", "00020000",
    "00000001", "00010001", "00020001",
    "00000002", "00010002", "00020002"
  };

  std::string input_str;

  int indexs = 0;
  base::StringToInt(in, &indexs);

  int index[4] = {
    (indexs / 1000),
    (indexs % 1000 / 100),
    (indexs % 100 / 10),
    (indexs % 10),
  };

  for (int i = 0; i < 4; i++) {
    //here is from 1 to 9
    int inputIndex = index[i] - 1;
    if (inputIndex <= 0 || inputIndex >= 9) {
      inputIndex = 0;
    }
    input_str += input_str_map[inputIndex];
  }

  *out = input_str;
}

void OCRRuokuai::setRequestParams(curl_httppost **pformpost, OCRContentType content_type, uint8 *image_data, uint32 image_length)
{
  struct curl_httppost *lastptr = NULL;

  curl_formadd(pformpost,
    &lastptr,
    CURLFORM_COPYNAME, "username",
    CURLFORM_PTRCONTENTS, url_username,
    CURLFORM_END);

  curl_formadd(pformpost,
    &lastptr,
    CURLFORM_COPYNAME, "password",
    CURLFORM_PTRCONTENTS, md5_password,
    CURLFORM_END);

  curl_formadd(pformpost,
    &lastptr,
    CURLFORM_COPYNAME, "typeid",
    CURLFORM_COPYCONTENTS, sz_ruokuai_typeid[content_type],
    CURLFORM_END);

  /*curl_formadd(pformpost,
    &lastptr,
    CURLFORM_COPYNAME, "timeout",
    CURLFORM_COPYCONTENTS, sztimeout,
    CURLFORM_END);*/

  curl_formadd(pformpost,
    &lastptr,
    CURLFORM_COPYNAME, "softid",
    CURLFORM_COPYCONTENTS, RUOKUAI_SOFTID,
    CURLFORM_END);

  curl_formadd(pformpost,
    &lastptr,
    CURLFORM_COPYNAME, "softkey",
    CURLFORM_COPYCONTENTS, RUOKUAI_SOFTKEY,
    CURLFORM_END);

  curl_formadd(pformpost,
    &lastptr,
    CURLFORM_COPYNAME, "image",
    CURLFORM_BUFFER, "vcode.jpg",
    CURLFORM_BUFFERPTR, image_data,
    CURLFORM_BUFFERLENGTH, image_length,
    CURLFORM_CONTENTTYPE, "image/jpeg",
    CURLFORM_END);

  //fix end bug
  curl_formadd(pformpost,
    &lastptr,
    CURLFORM_COPYNAME, "submit",
    CURLFORM_COPYCONTENTS, "Submit",
    CURLFORM_END);
}

curl_socket_t OCRRuokuai::curlOpenSocket(SOCKET *data, curlsocktype purpose, struct curl_sockaddr *addr)
{
  SOCKET sock = socket(addr->family, addr->socktype, addr->protocol);
  *data = sock;
  return sock;
}

int OCRRuokuai::curl_progress_callback(OCRRuokuai *that, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
  if (that->m_stop) {
    return CURLE_ABORTED_BY_CALLBACK;
  }
  return CURLE_OK;
}

OCRBase::OCRResultCode OCRRuokuai::detect(OCRContentType content_type, uint8 *image_data, uint32 image_length, std::string *result, char **ret_info)
{
  m_last_errmsg.clear();
  m_last_result = kOCRErrorParams;

  if (!image_data) {
    return kOCRErrorParams;
  }

  if (content_type < 0 || content_type >= kOCRCTCount) {
    return kOCRErrorParams;
  }

  if (!(md5_password && url_username)) {
    return kOCRErrorParams;
  }

  if (sz_ruokuai_typeid[content_type] == NULL) {
    return kOCRErrorUnsupported;
  }

  m_socket = NULL;
  m_stop = false;
  m_event.Create(false, false);

  OCRBase::OCRResultCode ocr_result = kOCRErrorUnknown;
  CTANetBase netbase;
  CBuffer response, bheader;
  CURL *curl;

  struct curl_httppost *formpost = NULL;
  struct curl_slist *headerlist = NULL;

  setRequestParams(&formpost, content_type, image_data, image_length);
  headerlist = netbase.addheader(headerlist);

  curl = curl_easy_init();

  if (curl) {

    curl_easy_setopt(curl, CURLOPT_OPENSOCKETFUNCTION, curlOpenSocket);
    curl_easy_setopt(curl, CURLOPT_OPENSOCKETDATA, &m_socket);
    //curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    //curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curl_progress_callback);
    //curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);

    netbase.setopt(curl);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, m_value.timeout * 1000);

    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, RUOKUAI_API_URL);

    netbase.CurlSetWriteFunction(curl, response, &bheader);

    //curl_easy_setopt(curl, CURLOPT_READDATA, this);
    //curl_easy_setopt(curl, CURLOPT_READFUNCTION, curlReadPayload);

    /* initalize custom header list (stating that Expect: 100-continue is not
    wanted */
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    CURLcode retcode = curl_easy_perform(curl);

    if (m_stop) {
      ocr_result = kOCRErrorAborted;
    } else if (retcode == CURLE_OK) {
      bheader.WriteZeroByte();
      HttpProcessContent(bheader, response);

      if (response.GetBufferLen() > 0) {
        response.WriteZeroByte();
        std::string __result = (LPCSTR)response.GetBuffer();
        Value *result_json = base::JSONReader::Read(__result, false);
        if (result_json && result_json->IsType(Value::TYPE_DICTIONARY)) {
          DictionaryValue *dv = (DictionaryValue *)result_json;
          if (!dv->HasKey("Error")) {
            dv->GetString("Id", &lastId);
            if (result) {
              if (content_type == kOCRCTTiebaSquared || content_type == kOCRCTTiebaSquaredWithPinYin) {
                std::string _tmp_result;
                dv->GetString("Result", &_tmp_result);
                transformTiebaSquaredResult(_tmp_result, result);
              } else {
#ifdef NO_UTF8
                std::string _tmp_result;
                dv->GetString("Result", &_tmp_result);
                char *c_out = NULL;
                lo_Utf82C(&c_out, _tmp_result.c_str());
                *result = c_out;
                free(c_out);
#else
                dv->GetString("Result", result);
#endif
              }
            }
            ocr_result = kOCRSuccess;
          } else {
            if (ret_info) {
              std::string errmsg;
              dv->GetString("Error", &errmsg);
#ifdef NO_UTF8
              char *c_out = NULL;
              lo_Utf82C(&c_out, errmsg.c_str());
              if (c_out) {
                m_last_errmsg = c_out;
                if (ret_info) {
                  *ret_info = c_out;
                } else {
                  free(c_out);
                }
              }
#else
              m_last_errmsg = errmsg;
              if (ret_info) *ret_info = _strdup(errmsg.c_str());
#endif
            }
            ocr_result = kOCRErrorResult;
          }
        } else {
          ocr_result = kOCRErrorResult;
        }
        if (result_json) delete result_json;
      } else {
        ocr_result = kOCRErrorNetwork;
      }
    } else if (retcode == CURLE_OPERATION_TIMEDOUT) {
      ocr_result = kOCRErrorTimeout;
    } else {
      const char *sz_errmsg = curl_easy_strerror(retcode);
      m_last_errmsg = sz_errmsg;
      if (ret_info) *ret_info = _strdup(sz_errmsg);
      ocr_result = kOCRErrorNetwork;
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
  }


  /* then cleanup the formpost chain */
  curl_formfree(formpost);

  /* free slist */
  curl_slist_free_all(headerlist);

  m_socket = NULL;

  m_last_result = ocr_result;

  m_event.Set();
  m_event.Close();

  return ocr_result;
}

void OCRRuokuai::stop()
{
  m_stop = true;

  if (m_socket) {
    shutdown(m_socket, SD_BOTH);
    closesocket(m_socket);

    m_socket = NULL;
  }
}

bool OCRRuokuai::isstop()
{
  return m_stop;
}

OCRBase::OCRResultCode OCRRuokuai::markResultAsWrong()
{
  OCRBase::OCRResultCode ocr_result = kOCRErrorUnknown;
  if (m_last_result == kOCRSuccess && lastId.length() > 0) {
    //m_stop = false;
    m_event.Create(false, false);

    CTANetBase netbase;
    CBuffer res;
    std::string postdata;
    base::SStringPrintf(&postdata, "username=%s&password=%s&softid=%s&softkey=%s&id=%s",
        url_username, md5_password, RUOKUAI_SOFTID, RUOKUAI_SOFTKEY, lastId.c_str());
    if (netbase.CurlRequestPost(RUOKUAI_API_REPORT_ERROR_URL, postdata.c_str(), res, NULL) == TA_NET_OK) {
      lastId.clear();
      ocr_result = kOCRSuccess;
    } else {
      ocr_result = kOCRErrorNetwork;
    }

    m_event.Set();
    m_event.Close();
  }
  return ocr_result;
}