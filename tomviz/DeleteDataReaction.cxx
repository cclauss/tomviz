/******************************************************************************

  This source file is part of the tomviz project.

  Copyright Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/
#include "DeleteDataReaction.h"

#include "ActiveObjects.h"
#include "ModuleManager.h"

namespace tomviz {

DeleteDataReaction::DeleteDataReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  this->connect(&ActiveObjects::instance(),
                SIGNAL(dataSourceChanged(DataSource*)),
                SLOT(activeDataSourceChanged()));
  m_activeDataSource = ActiveObjects::instance().activeDataSource();
  this->updateEnableState();
}

DeleteDataReaction::~DeleteDataReaction()
{
}

void DeleteDataReaction::updateEnableState()
{
  bool enabled = (m_activeDataSource != nullptr);
  if (enabled) {
    enabled = !m_activeDataSource->isRunningAnOperator();
  }
  this->parentAction()->setEnabled(enabled);
}

void DeleteDataReaction::onTriggered()
{
  DataSource* source = ActiveObjects::instance().activeDataSource();
  Q_ASSERT(source);
  DeleteDataReaction::deleteDataSource(source);
  ActiveObjects::instance().renderAllViews();
}

void DeleteDataReaction::deleteDataSource(DataSource* source)
{
  Q_ASSERT(source);

  ModuleManager& mmgr = ModuleManager::instance();
  mmgr.removeAllModules(source);
  mmgr.removeDataSource(source);
}

void DeleteDataReaction::activeDataSourceChanged()
{
  DataSource* source = ActiveObjects::instance().activeDataSource();
  if (m_activeDataSource != source) {
    if (m_activeDataSource) {
      QObject::disconnect(m_activeDataSource.data(),
                          &DataSource::operatorStarted, this, nullptr);
      QObject::disconnect(m_activeDataSource.data(),
                          &DataSource::allOperatorsFinished, this, nullptr);
    }
    m_activeDataSource = source;
    if (m_activeDataSource) {
      QObject::connect(m_activeDataSource.data(), &DataSource::operatorStarted,
                       this, &DeleteDataReaction::updateEnableState);
      QObject::connect(m_activeDataSource.data(),
                       &DataSource::allOperatorsFinished, this,
                       &DeleteDataReaction::updateEnableState);
    }
  }
  updateEnableState();
}

} // end of namespace tomviz
