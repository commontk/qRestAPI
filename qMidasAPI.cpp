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
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include <QUrlQuery>
#endif

// qMidasAPI includes
#include "qMidasAPI.h"
#include "qMidasAPI_p.h"
#include "qRestResult.h"

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
void qMidasAPI::parseResponse(qRestResult* restResult, const QByteArray& response)
{
  Q_D(qMidasAPI);
  QScriptValue scriptValue = d->ScriptEngine
                                .evaluate("JSON.parse")
                                .call(QScriptValue(),
                                      QScriptValueList() << QString(response));

  QUuid queryId = restResult->queryId();

  // e.g. {"stat":"ok","code":"0","message":"","data":[{"p1":"v1","p2":"v2",...}]}
  QScriptValue stat = scriptValue.property("stat");
  if (stat.toString() != "ok")
    {
    QString error = QString("Error while parsing outputs:") +
      " status: " + scriptValue.property("stat").toString() +
      " code: " + QString::number(scriptValue.property("code").toInteger()) +
      " msg: " + scriptValue.property("message").toString();
    restResult->setError(error, ResponseParseError);
    emit errorReceived(queryId, error);
    }
  QScriptValue data = scriptValue.property("data");
  if (!data.isObject())
    {
    if (data.toString().isEmpty())
      {
      restResult->setError("No data", ResponseParseError);
      emit errorReceived(queryId, "No data");
      }
    else
      {
      restResult->setError(QString("Bad data: ") + data.toString(), ResponseParseError);
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
