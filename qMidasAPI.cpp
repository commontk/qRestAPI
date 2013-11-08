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

// qMidasAPI includes
#include "qMidasAPI.h"
#include "qMidasAPI_p.h"
#include "qRestResult.h"

// --------------------------------------------------------------------------
void qMidasAPIResult::setResult(const QUuid& queryUuid, const QList<QVariantMap>& result)
{
  this->QueryUuid = queryUuid;
  this->Result = result;
}

// --------------------------------------------------------------------------
void qMidasAPIResult::setError(const QUuid& queryUuid, const QString& error)
{
  this->Error += error;
}

// --------------------------------------------------------------------------
// qMidasAPIPrivate methods

// --------------------------------------------------------------------------
qMidasAPIPrivate::qMidasAPIPrivate(qMidasAPI* object)
  : Superclass(object)
  , q_ptr(object)
{
  this->ResponseType = "json";
}

// --------------------------------------------------------------------------
// qMidasAPI methods

// --------------------------------------------------------------------------
qMidasAPI::qMidasAPI(QObject* _parent)
//  : Superclass(new qMidasAPIPrivate(this), _parent)
  : Superclass(_parent)
  , d_ptr(new qMidasAPIPrivate(this))
{
}

// --------------------------------------------------------------------------
qMidasAPI::~qMidasAPI()
{
}

// --------------------------------------------------------------------------
QString qMidasAPI::midasUrl()const
{
  return Superclass::serverUrl();
}

// --------------------------------------------------------------------------
void qMidasAPI::setMidasUrl(const QString& newMidasUrl)
{
  Superclass::setServerUrl(newMidasUrl);
}

// --------------------------------------------------------------------------
QList<QVariantMap> qMidasAPI::synchronousQuery(
  bool &ok,
  const QString& serverUrl,
  const QString& method,
  const ParametersType& parameters,
  int maxWaitingTimeInMSecs)
{
  qMidasAPI restAPI;
  restAPI.setServerUrl(serverUrl);
  restAPI.setTimeOut(maxWaitingTimeInMSecs);
  QUuid queryUuid = restAPI.get(method, parameters);
  qMidasAPIResult queryResult;
  QObject::connect(&restAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   &queryResult, SLOT(setResult(QUuid,QList<QVariantMap>)));
  QObject::connect(&restAPI, SIGNAL(errorReceived(QUuid,QString)),
                   &queryResult, SLOT(setError(QUuid,QString)));
  QEventLoop eventLoop;
  QObject::connect(&restAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   &eventLoop, SLOT(quit()));
  // Time out will fire an error which will quit the event loop.
  QObject::connect(&restAPI, SIGNAL(errorReceived(QUuid,QString)),
                   &eventLoop, SLOT(quit()));
  eventLoop.exec();
  ok = queryResult.Error.isNull();
  if (!ok)
    {
    QVariantMap map;
    map["queryError"] = queryResult.Error;
    queryResult.Result.push_front(map);
    }
  if (queryResult.Result.count() == 0)
    {
    // \tbd
    Q_ASSERT(queryResult.Result.count());
    QVariantMap map;
    map["queryError"] = tr("Unknown error");
    queryResult.Result.push_front(map);
    }
  return queryResult.Result;
}

// --------------------------------------------------------------------------
QUrl qMidasAPI::createUrl(const QString& method, const qRestAPI::Parameters& parameters)
{
  QString responseType = "json";
  QUrl url = Superclass::createUrl("/api/" + responseType, parameters);
  if (!method.isEmpty())
    {
    url.addQueryItem("method", method);
    }
  return url;
}

// --------------------------------------------------------------------------
void qMidasAPI::parseResponse(qRestResult* restResult, const QByteArray& response)
{
  Q_D(qMidasAPI);
  QScriptValue scriptValue = d->ScriptEngine.evaluate("(" + QString(response) + ")");

  QUuid queryId = restResult->queryId();

  // e.g. {"stat":"ok","code":"0","message":"","data":[{"p1":"v1","p2":"v2",...}]}
  QScriptValue stat = scriptValue.property("stat");
  if (stat.toString() != "ok")
    {
    QString error = QString("Error while parsing outputs:") +
      " status: " + scriptValue.property("stat").toString() +
      " code: " + scriptValue.property("code").toInteger() +
      " msg: " + scriptValue.property("message").toString();
    restResult->setError(ResponseParseError, error);
    emit errorReceived(queryId, error);
    }
  QScriptValue data = scriptValue.property("data");
  if (!data.isObject())
    {
    if (data.toString().isEmpty())
      {
      restResult->setError(ResponseParseError, "No data");
      emit errorReceived(queryId, "No data");
      }
    else
      {
      restResult->setError(ResponseParseError, QString("Bad data: ") + data.toString());
      emit errorReceived(queryId, QString("Bad data: ") + data.toString());
      }
    }
  QList<QVariantMap> result;
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
  restResult->setResult(result);
  emit resultReceived(queryId, result);
}
