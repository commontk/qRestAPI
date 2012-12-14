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

#ifndef __qXnatAPI_p_h
#define __qXnatAPI_p_h

// qXnatAPI includes
#include "qXnatAPI.h"

#include <QList>
#include <QScriptEngine>
#include <QScriptValue>

// --------------------------------------------------------------------------
class qXnatAPIPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qXnatAPI);
  Q_OBJECT

  typedef QObject Superclass;

  qXnatAPI* const q_ptr;

  QScriptEngine ScriptEngine;

public:
  qXnatAPIPrivate(qXnatAPI* object);

  QList<QVariantMap> parseXmlResponse(qRestResult* restResult, const QByteArray& response);

  QList<QVariantMap> parseJsonResponse(qRestResult* restResult, const QByteArray& response);
};

#endif
