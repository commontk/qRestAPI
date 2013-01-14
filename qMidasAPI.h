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

#ifndef __qMidasAPI_h
#define __qMidasAPI_h

#include "qRestAPI.h"

#include "qRestAPI_Export.h"

class qMidasAPIPrivate;

/// qMidasAPI is a simple interface class to communicate with a Midas3 public
/// API.
/// Queries are posted to the server and answers reported back.
/// qMidasAPI works in synchronous or unsynchronous way.
/// Usage:
/// <code>
/// qMidasAPI midas;
/// midas.setMidasUrl("http://slicer.kitware.com/midas3");
/// connect(&midas, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
///         myApp, SLOT(processResult(QUuid,QList<QVariantMap>)));
/// midas.query("midas.version");
/// ...
/// </code>
class qRestAPI_EXPORT qMidasAPI : public qRestAPI
{
  Q_OBJECT
  /// Url of the Midas server. e.g. "http://slicer.kitware.com/midas3"
  /// @deprecated Use the serverUrl property
  Q_PROPERTY(QString midasUrl READ midasUrl WRITE setMidasUrl)

  typedef qRestAPI Superclass;

public:
  explicit qMidasAPI(QObject*parent = 0);
  virtual ~qMidasAPI();

  /// @deprecated Use qRestAPI::Parameters.
  typedef Parameters ParametersType;

  QString midasUrl()const;
  void setMidasUrl(const QString& newMidasUrl);

  /// Utility function that waits \a maxWaitingTimeInMSecs msecs for the result
  /// of the query. Returns the answer of the server or an empty map if the
  /// result failed.
  /// If an error is emitted, "queryError" is added to the output.
  /// Internally, a QEventLoop is used so it can have side effects on your
  /// application.
  /// @deprecated Use the non-static version from qRestAPI.
  static QList<QVariantMap> synchronousQuery(bool &ok,
    const QString& midasUrl,
    const QString& method,
    const ParametersType& parameters = ParametersType(),
    int maxWaitingTimeInMSecs = 2500);

signals:
  void errorReceived(QUuid queryId, QString error);
  void resultReceived(QUuid queryId, QList<QVariantMap> result);

protected:
  QUrl createUrl(const QString& method, const qRestAPI::Parameters& parameters);
  void parseResponse(qRestResult* restResult, const QByteArray& response);

private:
  QScopedPointer<qMidasAPIPrivate> d_ptr;

  Q_DECLARE_PRIVATE(qMidasAPI);
  Q_DISABLE_COPY(qMidasAPI);
};

#endif
