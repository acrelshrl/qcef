// Copyright (c) 2017 LiuLang. All rights reserved.
// Use of this source is governed by General Public License that can be found
// in the LICENSE file.

#include "core/qcef_scheme_handler_factory.h"

#include <QDebug>
#include <QFile>
#include <QMimeDatabase>
#include <QUrl>

namespace {

CefStreamResourceHandler* CreateQFileStreamResourceHandler(
    const QString& path) {
  if (!QFile::exists(path)) {
    qWarning() << __FUNCTION__ << "File not found:" << path;
    return nullptr;
  }
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
    qWarning() << __FUNCTION__ << "Failed to open file: " << path;
    return nullptr;
  }
  // TODO(LiuLang): Cache mime database object.
  QMimeDatabase mime_database;
  const QString mime_type = mime_database.mimeTypeForData(&file).name();
  const auto content = file.readAll();
  const char* content_bytes = content.constData();
  CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForData(
          static_cast<void*>(const_cast<char*>(content_bytes)),
          (size_t)(file.size()));

  return new CefStreamResourceHandler(mime_type.toStdString(), stream);
};

}  // namespace

CefRefPtr<CefResourceHandler> QCefSchemeHandlerFactory::Create(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& scheme_name,
    CefRefPtr<CefRequest> request) {
  (void) browser;
  (void) frame;

  const QString url(QString::fromStdString(request->GetURL().ToString()));
  if (scheme_name == "qrc") {
    // Handle qrc protocol.
    const QString qrc_file(url.mid(3));
    return CreateQFileStreamResourceHandler(qrc_file);
  }
  if (custom_scheme_handler_ != nullptr) {
    const QString filepath = custom_scheme_handler_(QUrl(url));
    return CreateQFileStreamResourceHandler(filepath);
  }
  return nullptr;
}

void QCefSchemeHandlerFactory::setCustomSchemeHandler(
    QCefSchemeHandler handler) {
  custom_scheme_handler_ = handler;
}
