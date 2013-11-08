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

#include "qRestResult.h"

#include <QDebug>
#include <QEventLoop>
#include <QNetworkReply>
#include <QMetaProperty>

// --------------------------------------------------------------------------
// qRestAPIResult methods

// --------------------------------------------------------------------------
qRestResult::qRestResult(const QUuid& queryId, QObject* parent)
  : QObject(parent)
  , QueryId(queryId)
  , ErrorCode(qRestAPI::UnknownError)
  , done(false)
{
}

// --------------------------------------------------------------------------
qRestResult::~qRestResult()
{
}

// --------------------------------------------------------------------------
const QUuid& qRestResult::queryId() const
{
  return this->QueryId;
}

// --------------------------------------------------------------------------
const QList<QVariantMap>& qRestResult::results() const
{
  return this->Result;
}

// --------------------------------------------------------------------------
const QVariantMap qRestResult::result() const
{
  return this->Result[0];
}

// --------------------------------------------------------------------------
const QString& qRestResult::error() const
{
  return this->Error;
}

// --------------------------------------------------------------------------
qRestAPI::ErrorType qRestResult::errorType() const
{
  return this->ErrorCode;
}

// --------------------------------------------------------------------------
void qRestResult::setResult()
{
  this->done = true;
  emit ready();
}

// --------------------------------------------------------------------------
void qRestResult::setResult(const QList<QVariantMap>& result)
{
  this->Result.clear();
  for (int i = 0; i < result.size(); ++i)
    this->Result.push_back(result[i]);
//  this->Result = result;
  this->done = true;
  emit ready();
}

// --------------------------------------------------------------------------
void qRestResult::setError(qRestAPI::ErrorType errorType, const QString& error)
{
  this->ErrorCode = errorType;
  this->Error = error;
  this->done = true;
  emit ready();
}

// --------------------------------------------------------------------------
bool qRestResult::waitForDone()
{
  if (!done)
    {
    QEventLoop eventLoop;
    // Time out will fire an error which will quit the event loop.
    QObject::connect(this, SIGNAL(ready()),
                     &eventLoop, SLOT(quit()));
    eventLoop.exec();
    }
  // We require that the error code in case of an actual error is
  // never UnknownError for qRestResult objects.
  return this->ErrorCode != qRestAPI::UnknownError;
}

// --------------------------------------------------------------------------
void qRestResult::downloadReadyRead()
{
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());
  ioDevice->write(reply->readAll());
}

// --------------------------------------------------------------------------
void qRestResult::downloadFinished()
{
  ioDevice->close();
}

// --------------------------------------------------------------------------
void qRestResult::uploadReadyWrite()
{
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());
  Q_ASSERT(reply);
  if (!reply)
    {
    return;
    }
  reply->write(ioDevice->readAll());
}

// --------------------------------------------------------------------------
void qRestResult::uploadFinished()
{
  ioDevice->close();
}

QVariantMap qRestResult::qObjectToPropertyMap(QObject* object)
{
  QVariantMap propertyMap;
  const QMetaObject* metaobject = object->metaObject();
  int propertyCount = metaobject->propertyCount();
  for (int i = 0; i < propertyCount; ++i)
    {
    QMetaProperty metaProperty = metaobject->property(i);
    propertyMap[metaProperty.name()] = metaProperty.read(object);
    }
  foreach (QByteArray dynamicPropertyName, object->dynamicPropertyNames())
    {
    propertyMap[dynamicPropertyName] = object->property(dynamicPropertyName);
    }
  return propertyMap;
}
