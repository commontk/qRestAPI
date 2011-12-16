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
#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>
#include <QStringList>
#include <QTimer>
#include <QUuid>

// qCDashAPI includes
#include "qMidasAPI.h"

// STD includes
#include <cstdlib>
#include <iostream>

// --------------------------------------------------------------------------
void wait(int msec)
{
  QEventLoop eventLoop;
  QTimer::singleShot(msec, &eventLoop, SLOT(quit()));
  eventLoop.exec();
}

// --------------------------------------------------------------------------
int qMidasAPITest(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  QString query;
  if (app.arguments().count() > 1)
    {
    query = app.arguments().at(1);
    }

  QString midasUrl("http://slicer.kitware.com/midas3");
  qMidasAPI midasAPI;
  midasAPI.setMidasUrl(midasUrl);

  if (midasAPI.midasUrl() != midasUrl)
    {
    std::cout << "Failed to set Midas Url" << std::endl;
    return EXIT_FAILURE;
    }

  // --------------------------------------------------------------------------
  // Successfull query: midas.version
  // --------------------------------------------------------------------------
  QSignalSpy errorSpy(&midasAPI, SIGNAL(errorReceived(QString)));
  QSignalSpy receivedSpy(&midasAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)));
  QUuid queryUuid = midasAPI.query("midas.version");

  // Give 5 seconds for the server to answer
  wait(5000);

  if (queryUuid.isNull() ||
      errorSpy.count() > 0||
      receivedSpy.count() != 1)
    {
    std::cerr << "Failed to query 'midas.version': "
              << errorSpy.count() << " errors,"
              << receivedSpy.count() << " results." << std::endl;
    return EXIT_FAILURE;
    }
  errorSpy.clear();
  receivedSpy.clear();

  // --------------------------------------------------------------------------
  // Fail query: midas.notafunction
  // --------------------------------------------------------------------------
  queryUuid = midasAPI.query("midas.notafunction");

  // Give 5 seconds for the server to answer
  wait(5000);

  /// Even if errors are received, an empty result is sent
  if (queryUuid.isNull() ||
      errorSpy.count() == 0 ||
      receivedSpy.count() != 1)
    {
    std::cerr << "Failed to query 'midas.notafunction': "
              << errorSpy.count() << " errors,"
              << receivedSpy.count() << " results." << std::endl;
    return EXIT_FAILURE;
    }

  // --------------------------------------------------------------------------
  // Synchronous query: midas.info
  // --------------------------------------------------------------------------
  bool ok = false;
  QList<QVariantMap> result  =
    qMidasAPI::synchronousQuery(ok, midasUrl, "midas.info");
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
  result= qMidasAPI::synchronousQuery(ok, midasUrl,"midas.notafunction");
  std::cout << "result: " <<
    qPrintable(qMidasAPI::qVariantMapListToString(result))<< std::endl;
  if (ok || 
      result.size() != 1 ||
      result.at(0)["queryError"].isNull())
    {
    std::cout << "Failed to query 'midas.info'."
              << result.size() << " results."
              << std::endl;
    return EXIT_FAILURE;
    }

  // --------------------------------------------------------------------------
  // Synchronous fail query: midas.login (wrong credentials)
  // --------------------------------------------------------------------------
  qMidasAPI::ParametersType wrongParameters;
  wrongParameters["appname"] = "qMidasAPITest";
  wrongParameters["email"] = "john.doe@mail.com";
  wrongParameters["apikey"] = "123456789";
  result= qMidasAPI::synchronousQuery(ok, midasUrl,"midas.login", wrongParameters);
  std::cout << "result: " <<
    qPrintable(qMidasAPI::qVariantMapListToString(result))<< std::endl;
  if (ok || 
      result.size() != 1 ||
      result.at(0)["queryError"].isNull())
    {
    std::cout << "Failed to query 'midas.login'."
              << result.size() << " results."
              << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
