/*==============================================================================

  Library: qRestAPI

  Copyright (c) 2021 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.

==============================================================================*/

// Qt includes
#include <QCoreApplication>
#include <QDebug>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QStringList>
#include <QTest>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QUuid>

// qCDashAPI includes
#include "qGirderAPI.h"
#include "qRestResult.h"


// --------------------------------------------------------------------------
class qGirderAPITester : public  QObject
{
  Q_OBJECT
private slots:
  void cleanup();
  void testTakeResult();
  void testSync();
  void testReturnArrayOfData();

  void testFinishedSignal_data();
  void testFinishedSignal();
private:
  QString LastTestResult;
};

// --------------------------------------------------------------------------
void qGirderAPITester::cleanup()
{
  if (QTest::currentTestFailed() && !this->LastTestResult.isEmpty())
    {
    qDebug() << QTest::currentTestFunction() << "result:" << this->LastTestResult;
    }
  this->LastTestResult.clear();
}

// --------------------------------------------------------------------------
void qGirderAPITester::testTakeResult()
{
  qGirderAPI girderAPI;
  girderAPI.setServerUrl("https://data.kitware.com/api/v1/system/version");

  QUuid queryUuid = girderAPI.get("");
  QVERIFY(!queryUuid.isNull());

  QScopedPointer<qRestResult> restResult(girderAPI.takeResult(queryUuid));

  QVERIFY(!restResult.isNull());
  QVERIFY(!restResult->response().isEmpty());
  QCOMPARE(restResult->results().size(), 1);
}

// --------------------------------------------------------------------------
void qGirderAPITester::testSync()
{
  qGirderAPI girderAPI;
  girderAPI.setServerUrl("https://data.kitware.com/api/v1/system/version");

  QUuid queryUuid = girderAPI.get("");
  QVERIFY(!queryUuid.isNull());

  QList<QVariantMap> result;
  QVERIFY(girderAPI.sync(queryUuid, result));
  this->LastTestResult = qGirderAPI::qVariantMapListToString(result);

  QCOMPARE(result.size(), 1);
}

// --------------------------------------------------------------------------
void qGirderAPITester::testReturnArrayOfData()
{
  qGirderAPI girderAPI;
  girderAPI.setServerUrl("https://data.kitware.com/api/v1/collection");

  qRestAPI::Parameters parameters;
  parameters["limit"] = "2";

  QUuid queryUuid = girderAPI.get("", parameters);
  QVERIFY(!queryUuid.isNull());

  QList<QVariantMap> result;
  QVERIFY(girderAPI.sync(queryUuid, result));
  this->LastTestResult = qGirderAPI::qVariantMapListToString(result);

  QCOMPARE(result.size(), 2);
}


// --------------------------------------------------------------------------
void qGirderAPITester::testFinishedSignal_data()
{
  QTest::addColumn<QString>("url");
  QTest::addColumn<bool>("syncResult");

  QTest::newRow("1") << "https://data.kitware.com/api/v1/system/version" << true;
  QTest::newRow("2") << "https://data.kitware.com/api/v1/unknown" << false;
}

// --------------------------------------------------------------------------
void qGirderAPITester::testFinishedSignal()
{
  QFETCH(QString, url);
  QFETCH(bool, syncResult);

  qGirderAPI girderAPI;
  girderAPI.setServerUrl(url);

  QSignalSpy finishedSpy(&girderAPI, SIGNAL(finished(QUuid)));

  QUuid queryUuid = girderAPI.get("");
  QCOMPARE(finishedSpy.count(), 0);

  QList<QVariantMap> result;
  QCOMPARE(girderAPI.sync(queryUuid, result), syncResult);
  QCOMPARE(finishedSpy.count(), 1);

  this->LastTestResult = qGirderAPI::qVariantMapListToString(result);
}

#define main qGirderAPITest
QTEST_MAIN(qGirderAPITester)
#undef main

#include "moc_qGirderAPITest.cpp"
