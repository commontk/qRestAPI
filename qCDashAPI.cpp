/*==============================================================================

  Program: 3D Slicer

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
#include <QScriptValueIterator>
#include <QStringList>
#include <QUuid>

// qCDashAPI includes
#include "qCDashAPI.h"
#include "qCDashAPI_p.h"

// --------------------------------------------------------------------------
// qCDashAPIPrivate methods

// --------------------------------------------------------------------------
qCDashAPIPrivate::qCDashAPIPrivate(qCDashAPI& object)
  :q_ptr(&object)
{
  this->LogLevel = qCDashAPI::SILENT;
}

// --------------------------------------------------------------------------
void qCDashAPIPrivate::init()
{
  QObject::connect(&this->NetworkManager, SIGNAL(finished(QNetworkReply*)),
                   this, SLOT(replyFinished(QNetworkReply*)));
}

// --------------------------------------------------------------------------
QUrl qCDashAPIPrivate::url(const QString& method, const QString& task)
{
  Q_ASSERT(!method.isEmpty() && !task.isEmpty());

  QUrl url(this->Url + "/api/");
  url.addQueryItem("method", method);
  url.addQueryItem("task", task);
  return url;
}

// --------------------------------------------------------------------------
QString qCDashAPIPrivate::query(const QUrl& url, ProcessingMethod processingMethod)
{
  QNetworkRequest request;
  request.setUrl(url);
  QNetworkReply * reply = this->NetworkManager.get(request);
  this->NetworkReplyToProcessingMethodMap.insert(reply, processingMethod);
  QString queryUuid = QUuid::createUuid();
  this->NetworkReplyToQueryUuidMap.insert(reply, queryUuid);
  if(this->LogLevel == qCDashAPI::LOW)
    {
    qDebug() << "url" << url << ", queryUuid:" << queryUuid;
    }
  return queryUuid;
}

// --------------------------------------------------------------------------
void qCDashAPIPrivate::processProjectFiles(qCDashAPIPrivate * self, const QString& queryUuid, const QScriptValue& scriptValue)
{
  Q_ASSERT(self);
  if(self->LogLevel == qCDashAPI::LOW)
    {
    qDebug() << "processProjectFiles - queryUuid:" << queryUuid;
    }
  QList<QVariantHash> result;
  //bool status = scriptValue.property("status").toBool();
  //QString message = scriptValue.property("message").toString();
  QScriptValue files = scriptValue.property("files");
  if (files.isArray())
    {
    int length = files.property("length").toUInt32();
    if(self->LogLevel == qCDashAPI::LOW)
      {
      qDebug() << "projectFiles length" << length;
      }
    for(quint32 i = 0; i < length; ++i)
      {
      QScriptValue file = files.property(i);
      result << file.toVariant().toHash();
      }
    }
  if(self->LogLevel == qCDashAPI::HIGH)
    {
    qDebug() << "projectFiles" << result;
    }
  emit self->q_func()->projectFilesReceived(queryUuid, result);
}

// --------------------------------------------------------------------------
void qCDashAPIPrivate::processProjectList(qCDashAPIPrivate * self, const QString& queryUuid, const QScriptValue& scriptValue)
{
  Q_ASSERT(self);
  if(self->LogLevel == qCDashAPI::LOW)
    {
    qDebug() << "processProjectList - queryUuid:" << queryUuid;
    }
  QList<QVariantHash> result;
  if (scriptValue.isArray())
    {
    int length = scriptValue.property("length").toUInt32();
    if(self->LogLevel == qCDashAPI::LOW)
      {
      qDebug() << "projectList length" << length;
      }
    for(quint32 i = 0; i < length; ++i)
      {
      QScriptValue project = scriptValue.property(i);;
      result << project.toVariant().toHash();
      }
    }
  if(self->LogLevel == qCDashAPI::HIGH)
    {
    qDebug() << "projectList" << result;
    }
  emit self->q_func()->projectListReceived(queryUuid, result);
}

// --------------------------------------------------------------------------
void qCDashAPIPrivate::replyFinished(QNetworkReply* reply)
{
  Q_Q(qCDashAPI);
  if(this->LogLevel == qCDashAPI::LOW)
    {
    if (reply->error() != QNetworkReply::NoError)
      {
      qCritical() << "error" << reply->error() << reply->errorString();
      }
    }
  QScriptValue sc = this->ScriptEngine.evaluate("(" + QString(reply->readAll()) + ")");
  ProcessingMethod processingMethod = this->NetworkReplyToProcessingMethodMap.take(reply);
  QString queryUuid = this->NetworkReplyToQueryUuidMap.take(reply);
  Q_ASSERT(processingMethod);
  (*processingMethod)(this, queryUuid, sc);
  reply->deleteLater();
}

// --------------------------------------------------------------------------
// qCDashAPI methods

// --------------------------------------------------------------------------
qCDashAPI::qCDashAPI(QObject* _parent):Superclass(_parent)
  , d_ptr(new qCDashAPIPrivate(*this))
{
  Q_D(qCDashAPI);
  d->init();
}

// --------------------------------------------------------------------------
qCDashAPI::~qCDashAPI()
{
}

// --------------------------------------------------------------------------
QString qCDashAPI::url()const
{
  Q_D(const qCDashAPI);
  return d->Url;
}

// --------------------------------------------------------------------------
void qCDashAPI::setUrl(const QString& newUrl)
{
  Q_D(qCDashAPI);
  d->Url = newUrl;
}

// --------------------------------------------------------------------------
qCDashAPI::LogLevel qCDashAPI::logLevel()const
{
  Q_D(const qCDashAPI);
  return d->LogLevel;
}

// --------------------------------------------------------------------------
void qCDashAPI::setLogLevel(qCDashAPI::LogLevel level)
{
  Q_D(qCDashAPI);
  d->LogLevel = level;
}

// --------------------------------------------------------------------------
QString qCDashAPI::queryProjectFiles(const QString& project, const QString& match)
{
  Q_D(qCDashAPI);

  QUrl url = d->url("project", "files");
  url.addQueryItem("project", project);
  if (!match.isEmpty())
    {
    url.addQueryItem("match", match);
    }
  return d->query(url, &qCDashAPIPrivate::processProjectFiles);
}

// --------------------------------------------------------------------------
QString qCDashAPI::queryProjectList()
{
  Q_D(qCDashAPI);
  QUrl url = d->url("project", "list");
  return d->query(url, &qCDashAPIPrivate::processProjectList);
}
