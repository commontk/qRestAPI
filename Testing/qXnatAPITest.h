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

#ifndef __qXnatAPITest_h
#define __qXnatAPITest_h

// Qt includes
#include <QObject>

#include "qXnatAPI.h"

class TestObject : public QObject
{
public:
  mutable QString id;
  mutable QString name;
  void operator=(const TestObject& other) const {
    id = other.id;
    name = other.name;
  }
};

class TestList : public QObject
{

};

class QXnatAPITest: public QObject
{
  Q_OBJECT

  void wait(int msec);

private slots:
  void initTestCase();

  void testReplaceChild();
  void testProjectList();
  void testProject();

  void testCreateSubject();
  void testDeleteSubject();

  void testDownloadScans();
  void testWaitFor();

  void cleanupTestCase();

private:
  qXnatAPI* xnat;
};

// --------------------------------------------------------------------------
int qXnatAPITest(int argc, char* argv[]);

#endif
