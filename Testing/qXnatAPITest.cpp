/*==============================================================================

  Library: qRestAPI

  Copyright (c) 2013 University College London, Centre for Medical Image Computing

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Original author: Miklos Espak <m.espak@ucl.ac.uk>

==============================================================================*/

#include "qXnatAPITest.h"

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

// --------------------------------------------------------------------------
void QXnatAPITest::initTestCase()
{
  QString serverUrl("http://localhost:8080/genfi");
  QString userName("admin");
  QString password("admin");

  this->xnat = new qXnatAPI();
  this->xnat->setServerUrl(serverUrl);

  qXnatAPI::RawHeaders rawHeaders;
  rawHeaders["Authorization"] = "Basic " +
      QByteArray(QString("%1:%2").arg(userName).arg(password).toAscii()).toBase64();
  this->xnat->setDefaultRawHeaders(rawHeaders);
}

void QXnatAPITest::testReplaceChild()
{
  TestObject* testObject1 = new TestObject();
  testObject1->id = "test 1";
  testObject1->name = "name 1";
  TestObject* testObject2 = new TestObject();
  testObject2->id = "test 2";
  testObject2->name = "name 2";
  TestList* testList = new TestList();
  testObject1->setParent(testList);
//  qDebug() << "name before replacement:" << qobject_cast<TestObject*>(testList->children()[0])->name;
//  const QObjectList& children = testList->children();
//  const TestObject* to1 = qobject_cast<TestObject*>(children[0]);
////  testList->children()[0] = testObject2;
////  *testList->children()[0] = *testObject2;
//  qDebug() << "name after replacement:" << qobject_cast<TestObject*>(testList->children()[0])->name;
}

void QXnatAPITest::testProjectList()
{
  bool success = false;

  QString query("/REST/projects");
  QList<QVariantMap> result;
  success = xnat->sync(xnat->get(query), result);

  QVERIFY(success);

//  int i = 0;
//  foreach (QVariantMap project, result)
//  {
//    qDebug() << "Project" << i << ":";
//    foreach (QString key, project.keys())
//    {
//      qDebug() << key << "=" << project[key];
//    }
//    ++i;
//  }

//  QCOMPARE(response.size(), 3);

  query = "/REST/INVALID";
  success = xnat->sync(xnat->get(query));

  QVERIFY(!success);
}

void QXnatAPITest::testProject()
{
  qDebug() << "QXnatAPITest::testProject()";
  QString projectName("DEMO3");
  QString projectRequest = QString("/REST/projects/%1").arg(projectName);
  QList<QVariantMap> response;
  QUuid requestId;
  bool success;

  // Query project:
  if (xnat->sync(xnat->get(projectRequest)))
  {
    // Delete project:
    QVERIFY(xnat->sync(xnat->del(projectRequest)));
    // Query project (expected not to exist):
    QVERIFY(!xnat->sync(xnat->get(projectRequest)));
  }

  // Create project:
  qDebug() << "CREATE PROJECT";
  QUuid queryId = xnat->put(projectRequest);
  success = xnat->sync(queryId, response);
  QVERIFY(success);
//  foreach (QVariantMap map, response)
//  {
//    foreach (QString key, map.keys())
//    {
//      qDebug() << key << "=" << map[key];
//    }
//  }

  // Query project:
  requestId = xnat->get(projectRequest);
  qDebug() << queryId << "QUERY";
  success = xnat->sync(requestId, response);
  QVERIFY(success);

  // Delete project:
  requestId = xnat->del(projectRequest);
  success = xnat->sync(requestId);
  QVERIFY(success);
}

void QXnatAPITest::testCreateSubject()
{
  QString projectName("DEMO");
  QString subjectName("GENFI_S01001");
  QString request = QString("/REST/projects/%1/subjects/%2").arg(projectName, subjectName);
  QList<QVariantMap> response;
  QUuid requestId = xnat->put(request);
  bool success = xnat->sync(requestId, response);

  QVERIFY(success);
}

void QXnatAPITest::testDeleteSubject()
{
  QString projectName("DEMO");
  QString subjectName("GENFI_S01001");
  QString request = QString("/REST/projects/%1/subjects/%2").arg(projectName, subjectName);
  QList<QVariantMap> response;
  QUuid requestId = xnat->del(request);
  bool success = xnat->sync(requestId, response);

  QVERIFY(success);
}

void QXnatAPITest::testDownloadScans()
{
  QString projectName("DEMO");
  QString subjectName("GENFI_S00001");
  QString experimentName("GENFI_E00007");
  qint64 expectedSize(130847650);

  QString fileName = "testFile.zip";
  QFile testFile(fileName);
  if (testFile.exists())
  {
    testFile.remove();
  }
  QVERIFY(!testFile.exists());

  QString downloadQuery = QString("/REST/projects/%1/subjects/%2/experiments/%3/scans/ALL/files").arg(projectName, subjectName, experimentName);
  qRestAPI::Parameters parameters;
  parameters["format"] = "zip";
  QVERIFY(!testFile.exists());
  QUuid downloadQueryId = this->xnat->download(fileName, downloadQuery, parameters);

  QString projectQuery("/REST/projects");
  QUuid projectQueryId = this->xnat->get(projectQuery);

  bool ok = this->xnat->sync(downloadQueryId);
  QVERIFY(ok);
  QVERIFY(testFile.exists());
  QCOMPARE(testFile.size(), expectedSize);
  if (testFile.exists())
  {
    testFile.remove();
  }
  QVERIFY(!testFile.exists());
}

void QXnatAPITest::testWaitFor()
{
  bool ok = false;
  QList<QVariantMap> result;

  QString query("/REST/projects");
  QUuid queryId = xnat->get(query);
  ok = xnat->sync(queryId);
  QVERIFY(ok);
  ok = xnat->sync(xnat->get(query));
  QVERIFY(ok);
}

void QXnatAPITest::cleanupTestCase()
{
  delete this->xnat;
}

// --------------------------------------------------------------------------
int qXnatAPITest(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QXnatAPITest test;
  return QTest::qExec(&test, argc, argv);
}
