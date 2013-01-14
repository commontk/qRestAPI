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

#ifndef __qXnatAPI_h
#define __qXnatAPI_h

#include "qRestAPI.h"

#include "qRestAPI_Export.h"

class qXnatAPIPrivate;

/// qXnatAPI is a simple interface class to communicate with an XNAT
/// server through its REST API.
class qRestAPI_EXPORT qXnatAPI : public qRestAPI
{
  Q_OBJECT

  typedef qRestAPI Superclass;

public:
  explicit qXnatAPI(QObject*parent = 0);
  virtual ~qXnatAPI();

  virtual QUuid get(const QString& resource,
    const Parameters& parameters = Parameters(),
    const RawHeaders& rawHeaders = RawHeaders());

protected:
  void parseResponse(qRestResult* restResult, const QByteArray& result);

private:
  QScopedPointer<qXnatAPIPrivate> d_ptr;

  Q_DECLARE_PRIVATE(qXnatAPI);
  Q_DISABLE_COPY(qXnatAPI);
};

#endif
