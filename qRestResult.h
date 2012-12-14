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

#ifndef __qRestResult_h
#define __qRestResult_h

// qRestAPI includes
#include "qRestAPI.h"

class QIODevice;

// --------------------------------------------------------------------------
class qRestResult : public QObject
{
  Q_OBJECT

  friend class qRestAPI;
  friend class qRestAPIPrivate;

  QUuid QueryId;
  QList<QVariantMap> Result;
  QString Error;

  bool done;
  QIODevice* ioDevice;

public:
  qRestResult(const QUuid& queryId, QObject* parent = 0);
  virtual ~qRestResult();

  const QUuid& queryId() const;

  void waitForDone();

  const QList<QVariantMap>& results() const;
  const QVariantMap result() const;
  const QString& error() const;

  template <class Q>
  QList<Q*> results() const;

  template <class Q>
  Q* result() const;

public slots:
  void setResult();
  void setResult(const QList<QVariantMap>& result);
  void setError(const QString& error);

  void downloadReadyRead();
  void downloadFinished();
  void uploadReadyWrite();
  void uploadFinished();

signals:
  void ready();

private:
  static QVariantMap qObjectToPropertyMap(QObject* object);
  template <class Q>
  static Q* propertyMapToQObject(QVariantMap propertyMap);
};

// --------------------------------------------------------------------------
template <class Q>
Q* qRestResult::result() const
{
  QVariantMap propertyMap = this->Result[0];
  Q* object = qRestResult::propertyMapToQObject<Q>(propertyMap);
  return object;
}

// --------------------------------------------------------------------------
template <class Q>
QList<Q*> qRestResult::results() const
{
  QList<Q*> results;
  foreach (QVariantMap propertyMap, this->Result)
  {
    results.push_back(propertyMapToQObject<Q>(propertyMap));
  }
  return results;
}

// --------------------------------------------------------------------------
template <class Q>
Q* qRestResult::propertyMapToQObject(QVariantMap propertyMap)
{
  Q* object = new Q();
  QMapIterator<QString, QVariant> it(propertyMap);
  while (it.hasNext())
  {
    it.next();
    object->setProperty(it.key().toAscii().data(), it.value());
  }
  return object;
}

#endif
