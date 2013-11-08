/*==============================================================================

  Library: qRestAPI

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QEventLoop>
#include <QIODevice>
#include <QSslSocket>
#include <QStringList>
#include <QTimer>
#include <QUuid>

// qRestAPI includes
#include "qRestAPI.h"
#include "qRestAPI_p.h"

#include "qRestResult.h"

// --------------------------------------------------------------------------
// Static file local error messages
static QString unknownErrorStr = "Unknown error";
static QString unknownUuidStr = "Unknown uuid %1";
static QString timeoutErrorStr = "Request timed out";

// --------------------------------------------------------------------------
// qRestAPIPrivate methods

qRestAPIPrivate::StaticInit qRestAPIPrivate::_staticInit;

// --------------------------------------------------------------------------
qRestAPIPrivate::qRestAPIPrivate(qRestAPI* object)
  : q_ptr(object)
  , NetworkManager(NULL)
  , TimeOut(0)
  , SuppressSslErrors(true)
  , ErrorCode(qRestAPI::UnknownError)
  , ErrorString(unknownErrorStr)
{
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::staticInit()
{
  qRegisterMetaType<QUuid>("QUuid");
  qRegisterMetaType<QList<QVariantMap> >("QList<QVariantMap>");
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::init()
{
  Q_Q(qRestAPI);
  this->NetworkManager = new QNetworkAccessManager(q);
  QObject::connect(this->NetworkManager, SIGNAL(finished(QNetworkReply*)),
                   this, SLOT(processReply(QNetworkReply*)));
#ifndef QT_NO_OPENSSL
  if (QSslSocket::supportsSsl())
    {
    QObject::connect(this->NetworkManager, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)),
          this, SLOT(onSslErrors(QNetworkReply*, QList<QSslError>)));
    }
#endif
}

// --------------------------------------------------------------------------
QNetworkReply* qRestAPI::sendRequest(QNetworkAccessManager::Operation operation,
    const QUrl& url,
    const qRestAPI::RawHeaders& rawHeaders)
{
  Q_D(qRestAPI);
  QNetworkRequest queryRequest;
  queryRequest.setUrl(url);

  for (QMapIterator<QByteArray, QByteArray> it(d->DefaultRawHeaders); it.hasNext();)
    {
    it.next();
    queryRequest.setRawHeader(it.key(), it.value());
    }

  for (QMapIterator<QByteArray, QByteArray> it(rawHeaders); it.hasNext();)
    {
    it.next();
    queryRequest.setRawHeader(it.key(), it.value());
    }

  QNetworkReply* queryReply;
  switch (operation)
    {
    case QNetworkAccessManager::GetOperation:
      queryReply = d->NetworkManager->get(queryRequest);
      break;
    case QNetworkAccessManager::DeleteOperation:
      queryReply = d->NetworkManager->deleteResource(queryRequest);
      break;
    case QNetworkAccessManager::PutOperation:
      queryReply = d->NetworkManager->put(queryRequest, QByteArray());
      break;
    case QNetworkAccessManager::PostOperation:
      queryReply = d->NetworkManager->post(queryRequest, QByteArray());
      break;
    default:
      // TODO
      return 0;
    }

  if (d->TimeOut > 0)
    {
    QTimer* timeOut = new QTimer(queryReply);
    timeOut->setSingleShot(true);
    QObject::connect(timeOut, SIGNAL(timeout()),
                     d, SLOT(queryTimeOut()));
    timeOut->start(d->TimeOut);
    QObject::connect(queryReply, SIGNAL(downloadProgress(qint64,qint64)),
                     d, SLOT(queryProgress(qint64, qint64)));
    QObject::connect(queryReply, SIGNAL(uploadProgress(qint64,qint64)),
                     d, SLOT(queryProgress(qint64, qint64)));
    }

  QUuid queryId = QUuid::createUuid();
  queryReply->setProperty("uuid", queryId.toString());

  qRestResult* result = new qRestResult(queryId);
  d->results[queryId] = result;
//  QObject::connect(this, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
//                   result, SLOT(setResult(QList<QVariantMap>)));
//  QObject::connect(this, SIGNAL(errorReceived(QUuid,QString)),
//                   result, SLOT(setError(QString)));

  return queryReply;
}

// --------------------------------------------------------------------------
QVariantMap qRestAPI::scriptValueToMap(const QScriptValue& value)
{
#if QT_VERSION >= 0x040700
  return value.toVariant().toMap();
#else
  QVariantMap result;
  for (QScriptValueIterator it(value); it.hasNext();)
    {
    it.next();
    result.insert(it.name(), it.value().toVariant());
    }
  return result;
#endif
}

// --------------------------------------------------------------------------
void qRestAPI::appendScriptValueToVariantMapList(QList<QVariantMap>& result, const QScriptValue& data)
{
  QVariantMap map = scriptValueToMap(data);
  if (!map.isEmpty())
    {
    result << scriptValueToMap(data);
    }
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::processReply(QNetworkReply* reply)
{
  Q_Q(qRestAPI);
  QUuid queryId(reply->property("uuid").toString());

  qRestResult* restResult = results[queryId];

  if (reply->error() != QNetworkReply::NoError)
    {
    qRestAPI::ErrorType errorCode = qRestAPI::NetworkError;
    switch (reply->error())
      {
    case QNetworkReply::TimeoutError:
      errorCode = qRestAPI::NetworkError;
      break;
    case QNetworkReply::SslHandshakeFailedError:
      errorCode = qRestAPI::SslError;
      break;
    default:
      ;
      }
    restResult->setError(errorCode, queryId.toString() + ": "  +
                         reply->error() + ": " +
                         reply->errorString());
    }
  QByteArray response = reply->readAll();
  q->parseResponse(restResult, response);
  reply->close();
  reply->deleteLater();
}

#ifndef QT_NO_OPENSSL
void qRestAPIPrivate::onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors)
{
  if (!this->SuppressSslErrors)
    {
    QString errorString;
    foreach (const QSslError& error, errors)
      {
      if (!errorString.isEmpty())
        {
        errorString.append(", ");
        }
      errorString.append(error.errorString());
      }

    QString plural(errors.empty() ? " has" : "s have");
    QString error = QString("SSL error%1 occurred: %2").arg(plural).arg(errorString);

    QUuid queryId(reply->property("uuid").toString());
    qRestResult* restResult = results[queryId];

    restResult->setError(qRestAPI::SslError, error);
    }
  else
    {
    reply->ignoreSslErrors();
    }
}
#endif

// --------------------------------------------------------------------------
void qRestAPIPrivate::queryProgress(qint64 bytesTransmitted, qint64 bytesTotal)
{
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());
  // We received some progress so we postpone the timeout if any.
  QTimer* timer = reply->findChild<QTimer*>();
  if (timer)
    {
    timer->start();
    }
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  Q_Q(qRestAPI);
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());
  QUuid queryId(reply->property("uuid").toString());
  double progress = static_cast<double>(bytesReceived) / bytesTotal;
  q->emit progress(queryId, progress);
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
  Q_Q(qRestAPI);
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());
  Q_ASSERT(reply);
  if (!reply)
    {
    return;
    }
  QUuid queryId(reply->property("uuid").toString());
  double progress = static_cast<double>(bytesSent) / bytesTotal;
  q->emit progress(queryId, progress);
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::queryTimeOut()
{
  QTimer* timer = qobject_cast<QTimer*>(this->sender());
  Q_ASSERT(timer);
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(timer->parent());
  Q_ASSERT(reply);
  reply->abort();
  //reply->setError(QNetworkReply::TimeoutError,
  //   q->tr("Time out: No progress for %1 seconds.").arg(timer->interval()));
}

// --------------------------------------------------------------------------
// qRestAPI methods

// --------------------------------------------------------------------------
qRestAPI::qRestAPI(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qRestAPIPrivate(this))
{
  Q_D(qRestAPI);
  d->init();
}

// --------------------------------------------------------------------------
qRestAPI::~qRestAPI()
{
}

// --------------------------------------------------------------------------
QString qRestAPI::serverUrl()const
{
  Q_D(const qRestAPI);
  return d->ServerUrl;
}

// --------------------------------------------------------------------------
void qRestAPI::setServerUrl(const QString& serverUrl)
{
  Q_D(qRestAPI);
  d->ServerUrl = serverUrl;
}

// --------------------------------------------------------------------------
int qRestAPI::timeOut()const
{
  Q_D(const qRestAPI);
  return d->TimeOut;
}

// --------------------------------------------------------------------------
void qRestAPI::setTimeOut(int msecs)
{
  Q_D(qRestAPI);
  d->TimeOut = msecs;
}

// --------------------------------------------------------------------------
qRestAPI::RawHeaders qRestAPI::defaultRawHeaders()const
{
  Q_D(const qRestAPI);
  return d->DefaultRawHeaders;
}

// --------------------------------------------------------------------------
void qRestAPI::setDefaultRawHeaders(const qRestAPI::RawHeaders& defaultRawHeaders)
{
  Q_D(qRestAPI);
  d->DefaultRawHeaders = defaultRawHeaders;
}

// --------------------------------------------------------------------------
bool qRestAPI::suppressSslErrors()const
{
  Q_D(const qRestAPI);
  return d->SuppressSslErrors;
}

// --------------------------------------------------------------------------
void qRestAPI::setSuppressSslErrors(bool suppressSslErrors)
{
  Q_D(qRestAPI);
  d->SuppressSslErrors = suppressSslErrors;
}

// --------------------------------------------------------------------------
QUrl qRestAPI::createUrl(const QString& resource, const qRestAPI::Parameters& parameters)
{
  Q_D(qRestAPI);
  QUrl url(d->ServerUrl + resource);
  foreach(const QString& parameter, parameters.keys())
    {
    url.addQueryItem(parameter, parameters[parameter]);
    }
  return url;
}

// --------------------------------------------------------------------------
void qRestAPI::parseResponse(qRestResult* restResult, const QByteArray& response)
{
  QList<QVariantMap> result;
  restResult->setResult(result);
}

// --------------------------------------------------------------------------
QUuid qRestAPI::get(const QString& resource, const Parameters& parameters, const qRestAPI::RawHeaders& rawHeaders)
{
  QUrl url = createUrl(resource, parameters);
  QNetworkReply* queryReply = sendRequest(QNetworkAccessManager::GetOperation, url, rawHeaders);
  QUuid queryId = queryReply->property("uuid").toString();
  return queryId;
}

// --------------------------------------------------------------------------
QUuid qRestAPI::get(QIODevice* output, const QString& resource, const Parameters& parameters, const qRestAPI::RawHeaders& rawHeaders)
{
  Q_D(qRestAPI);

  QUrl url = createUrl(resource, parameters);

  if (!output->open(QIODevice::WriteOnly))
    {
    QUuid queryId;
    // TODO How to raise the error? We do not have a qRestResult object yet...
    //    emit errorReceived(queryId, "Cannot open device for writing.");
    return queryId;
    }

  QNetworkReply* queryReply = sendRequest(QNetworkAccessManager::GetOperation, url, rawHeaders);
  QUuid queryId = QUuid(queryReply->property("uuid").toString());
  qRestResult* result = new qRestResult(queryId, queryReply);
  result->ioDevice = output;

  connect(queryReply, SIGNAL(downloadProgress(qint64,qint64)),
          d, SLOT(downloadProgress(qint64,qint64)));
  connect(queryReply, SIGNAL(readyRead()),
          result, SLOT(downloadReadyRead()));
  connect(queryReply, SIGNAL(finished()),
          result, SLOT(downloadFinished()));

  return queryId;
}

// --------------------------------------------------------------------------
QUuid qRestAPI::download(const QString& fileName, const QString& resource, const Parameters& parameters, const qRestAPI::RawHeaders& rawHeaders)
{
  QIODevice* output = new QFile(fileName);

  QUuid queryId = get(output, resource, parameters);

  return queryId;
}

// --------------------------------------------------------------------------
QUuid qRestAPI::del(const QString& resource, const Parameters& parameters, const qRestAPI::RawHeaders& rawHeaders)
{
  QUrl url = createUrl(resource, parameters);
  QNetworkReply* queryReply = sendRequest(QNetworkAccessManager::DeleteOperation, url, rawHeaders);
  QUuid queryId = queryReply->property("uuid").toString();
  return queryId;
}

// --------------------------------------------------------------------------
QUuid qRestAPI::post(const QString& resource, const Parameters& parameters, const qRestAPI::RawHeaders& rawHeaders)
{
  QUrl url = createUrl(resource, parameters);
  QNetworkReply* queryReply = sendRequest(QNetworkAccessManager::PostOperation, url, rawHeaders);
  QUuid queryId = queryReply->property("uuid").toString();
  return queryId;
}

// --------------------------------------------------------------------------
QUuid qRestAPI::put(const QString& resource, const Parameters& parameters, const qRestAPI::RawHeaders& rawHeaders)
{
  QUrl url = createUrl(resource, parameters);
  QNetworkReply* queryReply = sendRequest(QNetworkAccessManager::PutOperation, url, rawHeaders);
  QUuid queryId = queryReply->property("uuid").toString();
  return queryId;
}

// --------------------------------------------------------------------------
QUuid qRestAPI::upload(const QString& fileName, const QString& resource, const Parameters& parameters, const qRestAPI::RawHeaders& rawHeaders)
{
  Q_D(qRestAPI);

  QUuid queryId = QUuid::createUuid();

  QUrl url = createUrl(resource, parameters);
  QIODevice* input = new QFile(fileName);
  if (!input->open(QIODevice::ReadOnly))
    {
//    delete input;
    QString error =
        QString("Cannot open file '%1' for reading to upload '%2'.").arg(
              fileName,
              url.toEncoded().constData());

//    emit errorReceived(queryId, error);
    return queryId;
    }

  QNetworkReply* queryReply = sendRequest(QNetworkAccessManager::PutOperation, url, rawHeaders);

  qRestResult* result = new qRestResult(queryId, queryReply);
  result->ioDevice = input;
  result->ioDevice->setParent(result);

  connect(queryReply, SIGNAL(uploadProgress(qint64,qint64)),
          d, SLOT(uploadProgress(qint64,qint64)));
  connect(queryReply, SIGNAL(finished()),
          result, SLOT(uploadFinished()));
  connect(queryReply, SIGNAL(readyWrite()),
          result, SLOT(uploadReadyWrite()));

  queryReply->setProperty("uuid", queryId.toString());
  return queryId;
}

bool qRestAPI::sync(const QUuid& queryId)
{
  QList<QVariantMap> result;
  return sync(queryId, result);
}

bool qRestAPI::sync(const QUuid& queryId, QList<QVariantMap>& result)
{
  Q_D(qRestAPI);
  result.clear();
  if (d->results.contains(queryId))
    {
    qRestResult* queryResult = d->results.take(queryId);
    bool ok = true;
    if (!queryResult->waitForDone())
      {
      ok = false;
      QVariantMap map;
      map["queryError"] = queryResult->Error;
      queryResult->Result.push_front(map);
      d->ErrorCode = queryResult->errorType();
      d->ErrorString = queryResult->error();
      }
    result = queryResult->Result;
    return ok;
    }
  d->ErrorCode = UnknownUuidError;
  d->ErrorString = unknownUuidStr;
  return false;
}

// --------------------------------------------------------------------------
QString qRestAPI::qVariantMapListToString(const QList<QVariantMap>& list)
{
  QStringList values;
  foreach(const QVariantMap& map, list)
    {
    foreach(const QString& key, map.keys())
      {
      values << QString("%1: %2").arg(key, map[key].toString());
      }
    }
  return values.join("\n");
}

// --------------------------------------------------------------------------
qRestResult* qRestAPI::takeResult(const QUuid& queryId)
{
  Q_D(qRestAPI);
  if (d->results.contains(queryId))
    {
    qRestResult* result = d->results.take(queryId);
    if (result->waitForDone())
      {
      return result;
      }
    else
      {
      d->ErrorCode = result->errorType();
      d->ErrorString = result->error();
      return NULL;
      }
    }
  d->ErrorCode = UnknownUuidError;
  d->ErrorString = UnknownUuidError;
  return NULL;
}

// --------------------------------------------------------------------------
qRestAPI::ErrorType qRestAPI::error() const
{
  Q_D(const qRestAPI);
  return d->ErrorCode;
}

// --------------------------------------------------------------------------
QString qRestAPI::errorString() const
{
  Q_D(const qRestAPI);
  return d->ErrorString;
}

//// --------------------------------------------------------------------------
//void qRestAPIPrivate::onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
//{
//  Q_D(qRestAPI);
//  authenticator->setUser(d->userName);
//  authenticator->setPassword(d->password);
//}
