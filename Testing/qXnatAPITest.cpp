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

class qXnatAPITestCasePrivate
{
public:
  qXnatAPI* xnat;

  QString serverUrl;
  QString userName;
  QString password;

  QString project;
  QString subject;
  QString experiment;

  qRestAPI::Parameters xmlFormat;
};

// --------------------------------------------------------------------------
qXnatAPITestCase::qXnatAPITestCase()
: d_ptr(new qXnatAPITestCasePrivate())
{
}

// --------------------------------------------------------------------------
qXnatAPITestCase::~qXnatAPITestCase()
{
}

// --------------------------------------------------------------------------
void qXnatAPITestCase::initTestCase()
{
  Q_D(qXnatAPITestCase);

//  d->serverUrl = "http://localhost:8080/genfi";
//  d->userName = "admin";
//  d->password = "admin";
  d->serverUrl = "https://central.xnat.org";
  d->userName = "ctk";
  d->password = "ctk";

  QString id = QUuid::createUuid().toString().mid(1, 4);
  qDebug() << "ID --------------- :" << id;
  d->project = QString("qXnat_%1").arg(id);
  d->subject = QString("Subject-0001");
  d->experiment = QString("Experiment-0001");

  d->xmlFormat["format"] = "xml";

  d->xnat = new qXnatAPI();
  d->xnat->setServerUrl(d->serverUrl);
  d->xnat->setSuppressSslErrors(true);

  qXnatAPI::RawHeaders rawHeaders;
  rawHeaders["Authorization"] = "Basic " +
      QByteArray(QString("%1:%2").arg(d->userName).arg(d->password).toAscii()).toBase64();
  rawHeaders["User-Agent"] = "Qt";
  d->xnat->setDefaultRawHeaders(rawHeaders);
}

void qXnatAPITestCase::testProjectList()
{
  Q_D(qXnatAPITestCase);

  bool success = false;

  QString query("/REST/projects");
  QList<QVariantMap> result;
  success = d->xnat->sync(d->xnat->get(query), result);

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

  QVERIFY(result.size() > 0);

  query = "/REST/INVALID";
  success = d->xnat->sync(d->xnat->get(query));

  QVERIFY(!success);
}

void qXnatAPITestCase::testProject()
{
  Q_D(qXnatAPITestCase);

  QString projectRequest = QString("/REST/projects/%1").arg(d->project);
  QList<QVariantMap> response;
  QUuid requestId;
  bool success;

  // Query project:
  if (d->xnat->sync(d->xnat->get(projectRequest)))
  {
    // Delete project:
    QVERIFY(d->xnat->sync(d->xnat->del(projectRequest)));
    // Query project (expected not to exist):
    QVERIFY(!d->xnat->sync(d->xnat->get(projectRequest)));
  }

  // Create project:
  qDebug() << "CREATE PROJECT";
  qDebug() << projectRequest;
  QUuid queryId = d->xnat->put(projectRequest);
  success = d->xnat->sync(queryId, response);
  QVERIFY(success);
//  foreach (QVariantMap map, response)
//  {
//    foreach (QString key, map.keys())
//    {
//      qDebug() << key << "=" << map[key];
//    }
//  }

  // Query project:
  // TODO Without the format=xml parameter the central.xnat.org returns a HTML.
  // On other sites (v1.5 probably) a html response comes only if an error occurs.
  // The error detection should be corrected.
  requestId = d->xnat->get(projectRequest, d->xmlFormat);
  qDebug() << queryId << "QUERY";
  success = d->xnat->sync(requestId, response);
  QVERIFY(success);

  // Delete project:
  requestId = d->xnat->del(projectRequest);
  success = d->xnat->sync(requestId);
  QVERIFY(success);
}

void qXnatAPITestCase::testCreateProject()
{
  Q_D(qXnatAPITestCase);

  QString request = QString("/REST/projects/%1").arg(d->project);
  QList<QVariantMap> response;
  QUuid requestId = d->xnat->put(request);
  bool success = d->xnat->sync(requestId, response);

  QVERIFY(success);
}

void qXnatAPITestCase::testCreateSubject()
{
  Q_D(qXnatAPITestCase);

  QString request = QString("/REST/projects/%1/subjects/%2").arg(d->project, d->subject);
  QList<QVariantMap> response;
  QUuid requestId = d->xnat->put(request);
  bool success = d->xnat->sync(requestId, response);

  QVERIFY(success);
}

void qXnatAPITestCase::testCreateExperiment()
{
  Q_D(qXnatAPITestCase);

  // TODO Need PUT contents

  QString request = QString("/REST/projects/%1/subjects/%2/experiments/%3")
      .arg(d->project, d->subject, d->experiment);
  QList<QVariantMap> response;
  QUuid requestId = d->xnat->put(request);
  bool success = d->xnat->sync(requestId, response);

  QVERIFY(success);
}

void qXnatAPITestCase::testDeleteExperiment()
{
  Q_D(qXnatAPITestCase);

  QString request = QString("/REST/projects/%1/subjects/%2/experiments/%3")
      .arg(d->project, d->subject, d->experiment);
  QList<QVariantMap> response;
  QUuid requestId = d->xnat->del(request);
  bool success = d->xnat->sync(requestId, response);

  QVERIFY(success);
}

void qXnatAPITestCase::testDeleteSubject()
{
  Q_D(qXnatAPITestCase);

  QString request = QString("/REST/projects/%1/subjects/%2").arg(d->project, d->subject);
  QList<QVariantMap> response;
  QUuid requestId = d->xnat->del(request);
  bool success = d->xnat->sync(requestId, response);

  QVERIFY(success);
}

void qXnatAPITestCase::testDeleteProject()
{
  Q_D(qXnatAPITestCase);

  QString projectName("DEMO");
  QString request = QString("/REST/projects/%1").arg(d->project);
  QList<QVariantMap> response;
  QUuid requestId = d->xnat->del(request);
  bool success = d->xnat->sync(requestId, response);

  QVERIFY(success);
}

void qXnatAPITestCase::testDownloadScans()
{
  Q_D(qXnatAPITestCase);

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
  QUuid downloadQueryId = d->xnat->download(fileName, downloadQuery, parameters);

  QString projectQuery("/REST/projects");
  QUuid projectQueryId = d->xnat->get(projectQuery);

  bool ok = d->xnat->sync(downloadQueryId);
  QVERIFY(ok);
  QVERIFY(testFile.exists());
  QCOMPARE(testFile.size(), expectedSize);
  if (testFile.exists())
  {
    testFile.remove();
  }
  QVERIFY(!testFile.exists());
}

void qXnatAPITestCase::testWaitFor()
{
  Q_D(qXnatAPITestCase);

  bool ok = false;
  QList<QVariantMap> result;

  QString query("/REST/projects");
  QUuid queryId = d->xnat->get(query);
  ok = d->xnat->sync(queryId);
  QVERIFY(ok);
  ok = d->xnat->sync(d->xnat->get(query));
  QVERIFY(ok);
}

void qXnatAPITestCase::cleanupTestCase()
{
  Q_D(qXnatAPITestCase);

  delete d->xnat;
}

// --------------------------------------------------------------------------
int qXnatAPITest(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  qXnatAPITestCase test;
  return QTest::qExec(&test, argc, argv);
}
