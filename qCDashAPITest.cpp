/*==============================================================================

  Program: 3D Slicer

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
#include <QStringList>
#include <QTimer>
#include <QUuid>

// qCDashAPI includes
#include "qCDashAPI.h"

// STD includes
#include <cstdlib>
#include <iostream>

int qCDashAPITest(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  QString query;
  if (app.arguments().count() > 1)
    {
    query = app.arguments().at(1);
    }

  qCDashAPI cdashAPI;
  cdashAPI.setLogLevel(qCDashAPI::LOW);
  cdashAPI.setUrl("http://www.cdash.org/slicer4");

  QString queryUuid;
  if (query.isEmpty() || query == "ProjectFiles")
    {
    queryUuid = cdashAPI.queryProjectFiles("Slicer4");
    }
  if (query.isEmpty() || query == "ProjectList")
    {
    queryUuid = cdashAPI.queryProjectList();
    }

  if (QUuid(queryUuid).isNull())
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with query() - Return Uuid is  not valid." << std::endl;
    return EXIT_FAILURE;
    }

  QTimer::singleShot(1000, qApp, SLOT(quit()));

  return app.exec();
}
