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

  typedef qRestAPI Superclass;

public:
  explicit qMidasAPI(QObject*parent = 0);
  virtual ~qMidasAPI();

  /// Parse a Midas JSON \a response
  ///
  /// Response is expected to be formated like `{"stat":"ok","code":"0","message":"","data":[{"p1":"v1","p2":"v2",...}]}` or
  /// `{"stat":"ok","code":"0","message":"","data":{"p1":"v1","p2":"v2",...}}`
  ///
  /// Returns \a False and set \a error in the following cases:
  /// * If `stat` attribute value is different from `ok`, sets \a error to `Error while parsing outputs: status: {stat} code: {code} msg: {message}`
  /// * If `data` attribute is empty, sets \a error to `No data`
  /// * If `data` attribute is not a valid JSON object, sets \a error to `Bad data: {data}`
  ///
  /// Returns \a True and set \a result a list of `QVariantMap` if `data` attribute is either set to
  /// an array of objects with attribute-value pairs or a single object with attribute-value pairs.
  static bool parseMidasResponse(const QByteArray& response, QList<QVariantMap>& result, QString& error);

  /// Parse a Midas JSON \a response
  ///
  /// If \a response is successfully parsed, set result and returns \a True otherwise
  /// set error and returns \a False.
  ///
  /// \sa parseMidasResponse(const QByteArray& response, QList<QVariantMap>& result, QString& error)
  /// \sa qRestResult::setResult(const QList<QVariantMap>& result)
  /// \sa qRestResult::setError(const QString& error, qRestAPI::ErrorType errorType)
  static bool parseMidasResponse(qRestResult* restResult, const QByteArray& response);

signals:
  void errorReceived(QUuid queryId, QString error);
  void resultReceived(QUuid queryId, QList<QVariantMap> result);

protected:
  QUrl createUrl(const QString& method, const qRestAPI::Parameters& parameters);
  void parseResponse(qRestResult* restResult, const QByteArray& response);

private:
  Q_DISABLE_COPY(qMidasAPI);
};

#endif
