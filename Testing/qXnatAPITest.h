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

class qXnatAPITestCasePrivate;

class qXnatAPITestCase: public QObject
{
  Q_OBJECT

  void wait(int msec);

public:
  explicit qXnatAPITestCase();
  virtual ~qXnatAPITestCase();

  void testCreateExperiment();
  void testDeleteExperiment();

  void testDownloadScans();

private slots:
  void initTestCase();

  void testProjectList();
  void testProject();

  void testCreateProject();
  void testCreateSubject();
  void testDeleteSubject();
  void testDeleteProject();

  void testWaitFor();

  void cleanupTestCase();

private:
  QScopedPointer<qXnatAPITestCasePrivate> d_ptr;

  Q_DECLARE_PRIVATE(qXnatAPITestCase);
  Q_DISABLE_COPY(qXnatAPITestCase);
};

// --------------------------------------------------------------------------
int qXnatAPITest(int argc, char* argv[]);

#endif
