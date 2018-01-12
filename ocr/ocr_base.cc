
#include "stdafx.h"
#include "ocr_base.h"

#include <base/json-config.h>

#include "ocr_ruokuai.h"

base::CJSONConfig* OCRBase::m_config = NULL;
OCR_CONFIG_VALUE OCRBase::m_value;
OCR_CONFIG_VALUE OCRBase::m_value_tmp;

const char* OCRBase::sz_key_name[kOCRKCount] = {
  "type",
  "platform",
  "username",
  "password",
  "timeout",
  "max-threads",
  "enable",
};

OCRBase::OCRBase()
  : m_last_result(kOCRErrorUnknown)
  , m_pvc(NULL)
{
}

OCRBase::~OCRBase()
{
}

OCRBase* OCRBase::create()
{
  if (m_value.benable) {
    switch (m_value.platform) {
    case kSNOCRRuokuai:
      OCRRuokuai *pruokuai = new OCRRuokuai();
      return pruokuai;
      //return new OCRRuokuai();
      break;
    }
  }
  return NULL;
}

void OCRBase::lock()
{
  m_lock.Acquire();
}

void OCRBase::unlock()
{
  m_lock.Release();
}

bool OCRBase::enable()
{
  return m_value.benable;
}

void OCRBase::save()
{
  m_value = m_value_tmp;

  m_config->SetString(sz_key_name[kOCRKType], m_value.type);

  m_config->SetInteger(sz_key_name[kOCRKPlatform], m_value.platform);

  m_config->SetString(sz_key_name[kOCRKUsername], m_value.username);
  m_config->SetString(sz_key_name[kOCRKPassword], m_value.password);

  m_config->SetInteger(sz_key_name[kOCRKTimeout], m_value.timeout);
  m_config->SetInteger(sz_key_name[kOCRKMaxThreads], m_value.max_threads);

  m_config->SetBoolean(sz_key_name[kOCRKEnable], m_value.benable);

  m_config->Save();
}

bool OCRBase::dirty()
{
  if (m_value_tmp.type == m_value.type
    && m_value_tmp.platform == m_value.platform
    && m_value_tmp.username == m_value.username
    && m_value_tmp.password == m_value.password
    && m_value_tmp.timeout == m_value.timeout
    && m_value_tmp.max_threads == m_value.max_threads
    && m_value_tmp.benable == m_value.benable) {
    return false;
  }
  return true;
}

void OCRBase::undirty()
{
  m_value_tmp = m_value;
}

bool OCRBase::setValue(OCRKey key, const void *value)
{
  switch (key) {
  case kOCRKType:
    m_value_tmp.type = (LPCWSTR)value;
    break;
  case kOCRKPlatform:
    if ((*(int *)value) < 0 || (*(int *)value) >= kSNOCRCount) {
      return false;
    }
    m_value_tmp.platform = *(int *)value;
    break;
  case kOCRKUsername:
    m_value_tmp.username = (LPCWSTR)value;
    break;
  case kOCRKPassword:
    m_value_tmp.password = (LPCWSTR)value;
    break;
  case kOCRKTimeout:
    if ((*(int *)value) <= 0) {
      return false;
    }
    m_value_tmp.timeout = *(int *)value;
    break;
  case kOCRKMaxThreads:
    if ((*(int *)value) <= 0) {
      return false;
    }
    m_value_tmp.max_threads = *(int *)value;
    break;
  case kOCRKEnable:
    m_value_tmp.benable = *(bool *)value;
    break;
  }
  return true;
}

const void* OCRBase::getValue(OCRKey key)
{
  switch (key) {
  case kOCRKType:
    return (void *)m_value.type.c_str();
    break;
  case kOCRKPlatform:
    return (void *)&m_value.platform;
    break;
  case kOCRKUsername:
    return (void *)m_value.username.c_str();
    break;
  case kOCRKPassword:
    return (void *)m_value.password.c_str();
    break;
  case kOCRKTimeout:
    return (void *)&m_value.timeout;
    break;
  case kOCRKMaxThreads:
    return (void *)&m_value.max_threads;
    break;
  case kOCRKEnable:
    return (void *)&m_value.benable;
    break;
  }
  return NULL;
}

void OCRBase::load()
{
  m_config->GetString(sz_key_name[kOCRKType], &m_value.type, L"net");

  m_config->GetInteger(sz_key_name[kOCRKPlatform], &m_value.platform, 0);
  if (m_value.platform < 0 || m_value.platform >= kSNOCRCount) {
    m_value.platform = 0;
  }

  m_config->GetString(sz_key_name[kOCRKUsername], &m_value.username, L"");
  m_config->GetString(sz_key_name[kOCRKPassword], &m_value.password, L"");

  m_config->GetInteger(sz_key_name[kOCRKTimeout], &m_value.timeout, 90);
  if (m_value.timeout <= 0) {
    m_value.timeout = 90;
  }

  m_config->GetInteger(sz_key_name[kOCRKMaxThreads], &m_value.max_threads, 5);
  if (m_value.max_threads <= 0) {
    m_value.max_threads = 5;
  }

  m_config->GetBoolean(sz_key_name[kOCRKEnable], &m_value.benable, false);

  m_value_tmp = m_value;
}

void OCRBase::stop()
{
}

bool OCRBase::isstop()
{
  return false;
}

void OCRBase::InitOCRSettings(LPCTSTR config_file)
{
  m_config = new base::CJSONConfig;
  m_config->SetPath(config_file);

  load();
}

void OCRBase::Uninit()
{
  delete m_config;
}