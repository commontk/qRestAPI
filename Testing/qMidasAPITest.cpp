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
#include <QTest>
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
class qMidasAPITester : public  QObject
{
  Q_OBJECT
private slots:
  void cleanup();
  void testQueryLocalFileReleaseFileHandle();
  void testServerURL();
  void testTakeResult();
  void testSync();
  void testReturnArrayOfData();
private:
  QString LastTestResult;
};

// --------------------------------------------------------------------------
void qMidasAPITester::cleanup()
{
  if (QTest::currentTestFailed() && !this->LastTestResult.isEmpty())
    {
    qDebug() << QTest::currentTestFunction() << "result:" << this->LastTestResult;
    }
  this->LastTestResult.clear();
}

// --------------------------------------------------------------------------
void qMidasAPITester::testQueryLocalFileReleaseFileHandle()
{
  QDir tmp = QDir::temp();
  QString temporaryDirName =
        QString("qMidasAPITest1-queryFile.%1").arg(QTime::currentTime().toString("hhmmsszzz"));
  tmp.mkdir(temporaryDirName);
  tmp.cd(temporaryDirName);
  tmp.mkdir("api");
  tmp.cd("api");

  QFile fileReply(tmp.filePath("json"));
  QVERIFY2(fileReply.open(QFile::WriteOnly), qPrintable(QString("Failed to create temporary file. %1").arg(tmp.filePath("json"))));

  fileReply.write(
    QString("{\"stat\":\"ok\",\"code\":\"0\",\"message\":\"\",\"data\":{"
            "\"quote\":\"If a day goes past and you do not learn anything, it is a waste of a day.\","
            "\"author\" : \"Mike Horn\"}}").toLatin1());
            
  fileReply.close();

  qMidasAPI midasAPI;
  midasAPI.setServerUrl(QUrl::fromLocalFile(QDir::temp().filePath(temporaryDirName)).toString());

  QUuid queryUuid = midasAPI.get("midas.quote.of.the.day");
  QVERIFY(!queryUuid.isNull());

  QList<QVariantMap> result;
  QVERIFY(midasAPI.sync(queryUuid, result));
  this->LastTestResult = qMidasAPI::qVariantMapListToString(result);

  QCOMPARE(result.size(), 1);

  // Attempt to delete 'queryFile'
  QVERIFY2(QFile::remove(tmp.filePath("json")), qPrintable(QString("Failed to remove queryFile. [%1]").arg(tmp.filePath("json"))));
}

// --------------------------------------------------------------------------
void qMidasAPITester::testServerURL()
{
  qMidasAPI midasAPI;
  QString midasUrl("http://slicer.kitware.com/midas3");
  midasAPI.setServerUrl(midasUrl);
  QCOMPARE(midasAPI.serverUrl(), midasUrl);
}

// --------------------------------------------------------------------------
void qMidasAPITester::testTakeResult()
{
  qMidasAPI midasAPI;
  midasAPI.setServerUrl("http://slicer.kitware.com/midas3");

  QSignalSpy errorSpy(&midasAPI, SIGNAL(errorReceived(QUuid,QString)));
  QSignalSpy receivedSpy(&midasAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)));

  // Successful query: midas.version
  {
  QUuid queryUuid = midasAPI.get("midas.version");
  QVERIFY(!queryUuid.isNull());

  QScopedPointer<qRestResult> restResult(midasAPI.takeResult(queryUuid));

  QVERIFY(!restResult.isNull());
  QCOMPARE(restResult->results().size(), 1);
  QCOMPARE(errorSpy.count(), 0);
  QCOMPARE(receivedSpy.count(), 1);
  }

  errorSpy.clear();
  receivedSpy.clear();
  this->LastTestResult.clear();

  // Fail query: midas.notafunction
  {
  QUuid queryUuid = midasAPI.get("midas.notafunction");
  QVERIFY(!queryUuid.isNull());

  QScopedPointer<qRestResult> restResult(midasAPI.takeResult(queryUuid));

  QVERIFY(restResult.isNull());
  // TODO: Fix qMidasAPI::parseResponse to ensure it emits only the relevant signals
  QCOMPARE(errorSpy.count(), 2);
  QCOMPARE(receivedSpy.count(), 1);
  }
}

// --------------------------------------------------------------------------
void qMidasAPITester::testSync()
{
  qMidasAPI midasAPI;
  midasAPI.setServerUrl("http://slicer.kitware.com/midas3");

  // Synchronous query: midas.info
  {
  QUuid queryUuid = midasAPI.get("midas.info");
  QVERIFY(!queryUuid.isNull());

  QList<QVariantMap> result;
  QVERIFY(midasAPI.sync(queryUuid, result));
  this->LastTestResult = qMidasAPI::qVariantMapListToString(result);

  QCOMPARE(result.size(), 1);
  }

  // Synchronous fail query: midas.notafunction
  {
  QUuid queryUuid = midasAPI.get("midas.notafunction");
  QVERIFY(!queryUuid.isNull());

  QList<QVariantMap> result;
  QVERIFY(!midasAPI.sync(queryUuid, result));
  this->LastTestResult = qMidasAPI::qVariantMapListToString(result);

  QCOMPARE(result.size(), 1);
  QVERIFY(result.at(0).contains("queryError"));
  }

  // Synchronous fail query: midas.login (wrong credentials)
  {
  qRestAPI::Parameters wrongParameters;
  wrongParameters["appname"] = "qMidasAPITest";
  wrongParameters["email"] = "john.doe@mail.com";
  wrongParameters["apikey"] = "123456789";

  QUuid queryUuid = midasAPI.get("midas.login", wrongParameters);
  QVERIFY(!queryUuid.isNull());

  QList<QVariantMap> result;
  QVERIFY(!midasAPI.sync(queryUuid, result));
  this->LastTestResult = qMidasAPI::qVariantMapListToString(result);

  QCOMPARE(result.size(), 1);
  QVERIFY(result.at(0).contains("queryError"));
  }
}

// --------------------------------------------------------------------------
void qMidasAPITester::testReturnArrayOfData()
{
  qMidasAPI midasAPI;
  midasAPI.setServerUrl("http://slicer.kitware.com/midas3");

  // Synchronous query: midas.community.list (return array of data)
  QUuid queryUuid = midasAPI.get("midas.community.list");
  QVERIFY(!queryUuid.isNull());

  QList<QVariantMap> result;
  QVERIFY(midasAPI.sync(queryUuid, result));
  this->LastTestResult = qMidasAPI::qVariantMapListToString(result);

  QVERIFY(result.size() > 0);
}

#define main qMidasAPITest
QTEST_MAIN(qMidasAPITester)
#undef main

#include "moc_qMidasAPITest.cpp"
