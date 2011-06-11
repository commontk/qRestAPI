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

#ifndef __qCDashAPI_p_h
#define __qCDashAPI_p_h

// Qt includes
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScriptEngine>

// qCDashAPI includes
#include "qCDashAPI.h"

// --------------------------------------------------------------------------
class qCDashAPIPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qCDashAPI);
  Q_OBJECT
protected:
  qCDashAPI* const q_ptr;
public:
  typedef qCDashAPIPrivate Self;
  qCDashAPIPrivate(qCDashAPI& object);

  virtual void init();

  typedef void(*ProcessingMethod)(qCDashAPIPrivate *, const QScriptValue&);

  QUrl url(const QString& method, const QString& task);
  void query(const QUrl& url, ProcessingMethod processingMethod);

  static void processProjectFiles(qCDashAPIPrivate * self, const QScriptValue& scriptValue);

  static void processProjectList(qCDashAPIPrivate * self, const QScriptValue& scriptValue);

public slots:
  void replyFinished(QNetworkReply*);

public:
  QString Url;
  qCDashAPI::LogLevel LogLevel;
  QNetworkAccessManager NetworkManager;
  QScriptEngine ScriptEngine;
  QHash<QNetworkReply*, ProcessingMethod> NetworkReplyToProcessingMethodMap;
};

#endif
