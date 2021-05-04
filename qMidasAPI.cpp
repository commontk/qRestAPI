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
#include <QEventLoop>
#include <QUrl>
#include <QScopedPointer>
#include <QScriptEngine>
#include <QScriptValue>
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include <QUrlQuery>
#endif

// qMidasAPI includes
#include "qMidasAPI.h"
#include "qRestResult.h"

// --------------------------------------------------------------------------
// qMidasAPI methods

// --------------------------------------------------------------------------
qMidasAPI::qMidasAPI(QObject* _parent)
  : Superclass(_parent)
{
}

// --------------------------------------------------------------------------
qMidasAPI::~qMidasAPI()
{
}

// --------------------------------------------------------------------------
QUrl qMidasAPI::createUrl(const QString& method, const qRestAPI::Parameters& parameters)
{
  QString responseType = "json";
  QUrl url = Superclass::createUrl("/api/" + responseType, parameters);
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
  if (!method.isEmpty())
    {
    url.addQueryItem("method", method);
    }
#else
  QUrlQuery urlQuery(url);
  if (!method.isEmpty())
    {
    urlQuery.addQueryItem("method", method);
    }
  url.setQuery(urlQuery);
#endif
  return url;
}

// --------------------------------------------------------------------------
bool qMidasAPI::parseMidasResponse(const QByteArray& response, QList<QVariantMap>& result, QString& error)
{
  QScriptEngine scriptEngine;
  QScriptValue scriptValue = scriptEngine
                                .evaluate("JSON.parse")
                                .call(QScriptValue(),
                                      QScriptValueList() << QString(response));

  // e.g. {"stat":"ok","code":"0","message":"","data":[{"p1":"v1","p2":"v2",...}]}
  QScriptValue stat = scriptValue.property("stat");
  if (stat.toString() != "ok")
    {
    QString error = QString("Error while parsing outputs:") +
      " status: " + scriptValue.property("stat").toString() +
      " code: " + QString::number(scriptValue.property("code").toInteger()) +
      " msg: " + scriptValue.property("message").toString();
    return false;
    }

  QScriptValue data = scriptValue.property("data");
  if (!data.isObject())
    {
    if (data.toString().isEmpty())
      {
      error = "No data";
      }
    else
      {
      error = QString("Bad data: ") + data.toString();
      }
    return false;
    }

  if (data.isArray())
    {
    quint32 length = data.property("length").toUInt32();
    for(quint32 i = 0; i < length; ++i)
      {
      qRestAPI::appendScriptValueToVariantMapList(result, data.property(i));
      }
    }
  else
    {
    qRestAPI::appendScriptValueToVariantMapList(result, data);
    }
  return true;
}

// --------------------------------------------------------------------------
bool qMidasAPI::parseMidasResponse(qRestResult* restResult, const QByteArray& response)
{
  QList<QVariantMap> result;
  QString error;
  bool success = qMidasAPI::parseMidasResponse(response, result, error);
  if (success)
    {
    restResult->setResult(result);
    }
  else
    {
    restResult->setError(error, ResponseParseError);
    }
  return success;
}

// --------------------------------------------------------------------------
void qMidasAPI::parseResponse(qRestResult* restResult, const QByteArray& response)
{
  bool success = qMidasAPI::parseMidasResponse(restResult, response);
  if (success)
    {
    emit resultReceived(restResult->queryId(), restResult->results());
    }
  else
    {
    emit errorReceived(restResult->queryId(), restResult->error());
    }
}
