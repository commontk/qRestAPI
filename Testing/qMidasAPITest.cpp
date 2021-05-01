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
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSignalSpy>
#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QUuid>

// qCDashAPI includes
#include "qMidasAPI.h"
#include "qRestResult.h"

// STD includes
#include <cstdlib>
#include <iostream>

// --------------------------------------------------------------------------
int qMidasAPITest(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  qMidasAPI midasAPI;

  QUuid queryUuid;
  bool ok = false;
  QList<QVariantMap> result;

  qRestResult* restResult = nullptr;

  // --------------------------------------------------------------------------
  // Check that query associated with local file release file handle
  // --------------------------------------------------------------------------
  std::cout << "\n=== Check that query associated with local file release file handle ===" << std::endl;
  QDir tmp = QDir::temp();
  QString temporaryDirName =
        QString("qMidasAPITest1-queryFile.%1").arg(QTime::currentTime().toString("hhmmsszzz"));
  tmp.mkdir(temporaryDirName);
  tmp.cd(temporaryDirName);
  tmp.mkdir("api");
  tmp.cd("api");

  QFile fileReply(tmp.filePath("json"));
  if (!fileReply.open(QFile::WriteOnly))
    {
    std::cerr << "Line " << __LINE__ << " - Failed to create temporary file." 
              << qPrintable(tmp.filePath("json")) << std::endl;
    return EXIT_FAILURE;
    }

  fileReply.write(
    QString("{\"stat\":\"ok\",\"code\":\"0\",\"message\":\"\",\"data\":{"
            "\"quote\":\"If a day goes past and you do not learn anything, it is a waste of a day.\","
            "\"author\" : \"Mike Horn\"}}").toLatin1());
            
  fileReply.close();

  midasAPI.setServerUrl(QUrl::fromLocalFile(QDir::temp().filePath(temporaryDirName)).toString());
  queryUuid = midasAPI.get("midas.quote.of.the.day");
  ok  = midasAPI.sync(queryUuid, result);
  std::cout << "result: " <<
    qPrintable(qMidasAPI::qVariantMapListToString(result)) << std::endl;
  if (!ok || result.size() == 0)
    {
    std::cout << "Failed to query 'midas.quote.of.the.day'." << std::endl;
    return EXIT_FAILURE;
    }

  // Attempt to delete 'queryFile'
  if (!QFile::remove(tmp.filePath("json")))
    {
    std::cout << "Failed to remove queryFile. [" << qPrintable(tmp.filePath("json")) << "]" << std::endl;
    return EXIT_FAILURE;
    }

  // --------------------------------------------------------------------------
  QString midasUrl("http://slicer.kitware.com/midas3");
  midasAPI.setMidasUrl(midasUrl);

  if (midasAPI.midasUrl() != midasUrl)
    {
    std::cout << "Failed to set Midas Url" << std::endl;
    return EXIT_FAILURE;
    }

  // --------------------------------------------------------------------------
  // Successful query: midas.version
  // --------------------------------------------------------------------------
  std::cout << "\n=== Successful query: midas.version ===" << std::endl;
  QSignalSpy errorSpy(&midasAPI, SIGNAL(errorReceived(QUuid,QString)));
  QSignalSpy receivedSpy(&midasAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)));
  queryUuid = midasAPI.get("midas.version");

  restResult = midasAPI.takeResult(queryUuid);

  if (queryUuid.isNull() ||
      errorSpy.count() > 0||
      receivedSpy.count() != 1)
    {
    std::cerr << "Failed to query 'midas.version': "
              << errorSpy.count() << " errors, "
              << receivedSpy.count() << " results." << std::endl;
    return EXIT_FAILURE;
    }
  errorSpy.clear();
  receivedSpy.clear();

  // --------------------------------------------------------------------------
  // Fail query: midas.notafunction
  // --------------------------------------------------------------------------
  std::cout << "\n=== Fail query: midas.notafunction ===" << std::endl;
  queryUuid = midasAPI.get("midas.notafunction");

  restResult = midasAPI.takeResult(queryUuid);

  if (queryUuid.isNull() ||
      errorSpy.count() == 0 ||
      receivedSpy.count() != 1)
    {
    std::cerr << "Failed to query 'midas.notafunction': "
              << errorSpy.count() << " errors, "
              << receivedSpy.count() << " results." << std::endl;
    return EXIT_FAILURE;
    }

  // --------------------------------------------------------------------------
  // Synchronous query: midas.info
  // --------------------------------------------------------------------------
  std::cout << "\n=== Synchronous query: midas.info ===" << std::endl;
  queryUuid = midasAPI.get("midas.info");
  ok = midasAPI.sync(queryUuid, result);
  std::cout << "result: " <<
    qPrintable(qMidasAPI::qVariantMapListToString(result))<< std::endl;
  if (!ok || result.size() == 0)
    {
    std::cout << "Failed to query 'midas.info'."
              << std::endl;
    return EXIT_FAILURE;
    }

  // --------------------------------------------------------------------------
  // Synchronous fail query: midas.notafunction
  // --------------------------------------------------------------------------
  std::cout << "\n=== Synchronous fail query: midas.notafunction ===" << std::endl;
  queryUuid = midasAPI.get("midas.notafunction");
  ok = midasAPI.sync(queryUuid, result);
  std::cout << "result: " <<
    qPrintable(qMidasAPI::qVariantMapListToString(result))<< std::endl;
  if (ok ||
      result.size() != 1 ||
      result.at(0)["queryError"].isNull())
    {
    std::cout << "Failed to query 'midas.info'. Got "
              << result.size() << " results."
              << std::endl;
    return EXIT_FAILURE;
    }

  // --------------------------------------------------------------------------
  // Synchronous fail query: midas.login (wrong credentials)
  // --------------------------------------------------------------------------
  std::cout << "\n=== Synchronous fail query: midas.login (wrong credentials) ===" << std::endl;
  qRestAPI::Parameters wrongParameters;
  wrongParameters["appname"] = "qMidasAPITest";
  wrongParameters["email"] = "john.doe@mail.com";
  wrongParameters["apikey"] = "123456789";
  queryUuid = midasAPI.get("midas.login", wrongParameters);
  ok = midasAPI.sync(queryUuid, result);
  std::cout << "result: " <<
    qPrintable(qMidasAPI::qVariantMapListToString(result))<< std::endl;
  if (ok ||
      result.size() != 1 ||
      result.at(0)["queryError"].isNull())
    {
    std::cout << "Failed to query 'midas.login'. Got "
              << result.size() << " results."
              << std::endl;
    return EXIT_FAILURE;
    }
    
  // --------------------------------------------------------------------------
  // Synchronous query: midas.community.list (return array of data)
  // --------------------------------------------------------------------------
  std::cout << "\n=== Synchronous query: midas.community.list (return array of data) ===" << std::endl;
  queryUuid = midasAPI.get("midas.community.list");
  ok = midasAPI.sync(queryUuid, result);
  std::cout << "result: " <<
    qPrintable(qMidasAPI::qVariantMapListToString(result))<< std::endl;
  if (!ok || result.size() == 0)
    {
    std::cout << "Failed to query 'midas.community.list'." << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
