/*==============================================================================

  Program: qMidasAPI

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
#include <QScriptValueIterator>
#include <QStringList>
#include <QTimer>
#include <QUuid>

// qMidasAPI includes
#include "qMidasAPI.h"
#include "qMidasAPI_p.h"

// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
QVariantMap scriptValueToMap(const QScriptValue& value)
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

} // end of anonymous namespace

// --------------------------------------------------------------------------
void qMidasAPIResult::setResult(QUuid queryUuid, const QList<QVariantMap>& result)
{
  this->QueryUuid = queryUuid;
  this->Result = result;
}

// --------------------------------------------------------------------------
void qMidasAPIResult::setError(const QString& error)
{
  this->Error += error;
}

// --------------------------------------------------------------------------
// qMidasAPIPrivate methods

// --------------------------------------------------------------------------
qMidasAPIPrivate::qMidasAPIPrivate(qMidasAPI& object)
  :q_ptr(&object)
{
  this->ResponseType = "json";
}

// --------------------------------------------------------------------------
void qMidasAPIPrivate::init()
{
  Q_Q(qMidasAPI);
  this->NetworkManager = new QNetworkAccessManager(q);
  QObject::connect(this->NetworkManager, SIGNAL(finished(QNetworkReply*)),
                   this, SLOT(processReply(QNetworkReply*)));
#if 0
  QObject::connect(q, SIGNAL(errorReceived(QString)),
                   this, SLOT(print(QString)));
  QObject::connect(q, SIGNAL(infoReceived(QString)),
                   this, SLOT(print(QString)));
#endif
}

// --------------------------------------------------------------------------
QUrl qMidasAPIPrivate
::createUrl(const QString& method, const qMidasAPI::ParametersType& parameters)
{
  QUrl url(this->MidasUrl + "/api/" + this->ResponseType);
  if (!method.isEmpty())
    {
    url.addQueryItem("method", method);
    }
  foreach(const QString& parameter, parameters.keys())
    {
    url.addQueryItem(parameter, parameters[parameter]);
    }
  return url;
}

// --------------------------------------------------------------------------
QUuid qMidasAPIPrivate::postQuery(const QUrl& url)
{
  Q_Q(qMidasAPI);
  QNetworkRequest queryRequest;
  queryRequest.setUrl(url);
  QUuid queryUuid = QUuid::createUuid();
  qDebug() << "query: " << url.toString();
  q->emit infoReceived("Post query: " + url.toString());
  QNetworkReply * queryReply = this->NetworkManager->get(queryRequest);
  queryReply->setProperty("uuid", queryUuid.toString());
  return queryUuid;
}

namespace
{
// --------------------------------------------------------------------------
void appendScriptValueToVariantMapList(QList<QVariantMap>& result, const QScriptValue& data)
{
  QVariantMap map = scriptValueToMap(data);
  if (!map.isEmpty())
    {
    result << scriptValueToMap(data);
    }
}
}

// --------------------------------------------------------------------------
QList<QVariantMap> qMidasAPIPrivate::parseResult(const QScriptValue& scriptValue)
{
  Q_Q(qMidasAPI);
  // e.g. {"stat":"ok","code":"0","message":"","data":[{"p1":"v1","p2":"v2",...}]}
  QList<QVariantMap> result;
  QScriptValue stat = scriptValue.property("stat");
  if (stat.toString() != "ok")
    {
    QString error = QString("Error while parsing outputs:") +
      " status: " + scriptValue.property("stat").toString() + 
      " code: " + scriptValue.property("code").toInteger() + 
      " msg: " + scriptValue.property("message").toString();
    q->emit errorReceived(error);
    }
  QScriptValue data = scriptValue.property("data");
  if (!data.isObject())
    {
    if (data.toString().isEmpty())
      {
      q->emit errorReceived("No data");
      }
    else
      {
      q->emit errorReceived( QString("Bad data: ") + data.toString());
      }
    }
  if (data.isArray())
    {
    quint32 length = data.property("length").toUInt32();
    for(quint32 i = 0; i < length; ++i)
      {
      appendScriptValueToVariantMapList(result, data.property(i));
      }
    }
  else
    {
    appendScriptValueToVariantMapList(result, data);
    }
  
  return result;
}

// --------------------------------------------------------------------------
void qMidasAPIPrivate::processReply(QNetworkReply* reply)
{
  Q_Q(qMidasAPI);
  QUuid uuid(reply->property("uuid").toString());
  if (reply->error() != QNetworkReply::NoError)
    {
    q->emit errorReceived(uuid.toString() + ": "  + 
                  reply->error() + ": " +
                  reply->errorString());
    }
  QScriptValue scriptResult =
    this->ScriptEngine.evaluate("(" + QString(reply->readAll()) + ")");
  q->emit infoReceived(QString("Parse results for ") + uuid);
  QList<QVariantMap> result = this->parseResult(scriptResult);
  q->emit infoReceived(QString("Results for ") + uuid.toString()
                       + ": " + q->qVariantMapListToString(result));
  q->emit resultReceived(uuid, result);
  reply->deleteLater();
}

// --------------------------------------------------------------------------
void qMidasAPIPrivate::print(const QString& msg)
{
  qDebug() << msg;
}

// --------------------------------------------------------------------------
// qMidasAPI methods

// --------------------------------------------------------------------------
qMidasAPI::qMidasAPI(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qMidasAPIPrivate(*this))
{
  Q_D(qMidasAPI);
  d->init();
  qRegisterMetaType<QUuid>("QUuid");
  qRegisterMetaType<QList<QVariantMap> >("QList<QVariantMap>");
}

// --------------------------------------------------------------------------
qMidasAPI::~qMidasAPI()
{
}

// --------------------------------------------------------------------------
QString qMidasAPI::midasUrl()const
{
  Q_D(const qMidasAPI);
  return d->MidasUrl;
}

// --------------------------------------------------------------------------
void qMidasAPI::setMidasUrl(const QString& newMidasUrl)
{
  Q_D(qMidasAPI);
  d->MidasUrl = newMidasUrl;
}

// --------------------------------------------------------------------------
QUuid qMidasAPI::query(const QString& method, const ParametersType& parameters)
{
  Q_D(qMidasAPI);
  QUrl url = d->createUrl(method, parameters);
  return d->postQuery(url);
}

// --------------------------------------------------------------------------
QList<QVariantMap> qMidasAPI::synchronousQuery(
  bool &ok,
  const QString& midasUrl,const QString& method,
  const ParametersType& parameters,
  int maxWaitingTimeInMSecs)
{
  qMidasAPI midasAPI;
  midasAPI.setMidasUrl(midasUrl);
  midasAPI.query(method, parameters);
  qMidasAPIResult queryResult;
  QObject::connect(&midasAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   &queryResult, SLOT(setResult(QUuid,QList<QVariantMap>)));
  QObject::connect(&midasAPI, SIGNAL(errorReceived(QString)),
                   &queryResult, SLOT(setError(QString)));
  QEventLoop eventLoop;
  QObject::connect(&midasAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   &eventLoop, SLOT(quit()));
  QTimer::singleShot(maxWaitingTimeInMSecs, &eventLoop, SLOT(quit()));
  eventLoop.exec();
  ok = queryResult.Error.isNull();
  if (!ok)
    {
    QVariantMap map;
    map["queryError"] = queryResult.Error;
    queryResult.Result.push_front(map);
    }
  return queryResult.Result;
}

// --------------------------------------------------------------------------
QString qMidasAPI::qVariantMapListToString(const QList<QVariantMap>& list)
{
  QStringList values;
  foreach(const QVariantMap& map, list)
    {
    foreach(const QString& key, map.keys())
      {
      QString value;
      value += key;
      value += ": ";
      value += map[key].toString();
      values << value;
      }
    }
  return values.join("\n");
}
