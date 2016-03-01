#include "ProjectExecutor.h"

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include "projectexplorer/runconfiguration.h"
#include <projectexplorer/projectconfiguration.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/buildmanager.h>
#include <projectexplorer/projecttree.h>

#include <QDebug>

ProjectExecutor::ProjectExecutor(QObject *parent) :
    Executor(parent)
{
    using namespace ProjectExplorer;
    connect(BuildManager::instance(),SIGNAL(buildQueueFinished(bool)),SLOT(buildingFinished(bool)));

    connect(ProjectExplorerPlugin::instance(),SIGNAL(runControlFinished(ProjectExplorer::RunControl*)),SIGNAL(finished()));
}

void ProjectExecutor::execute()
{
    using namespace ProjectExplorer;
    ProjectExplorerPlugin *projectExplorerPlugin = ProjectExplorerPlugin::instance();
    Project *startUpProject = ProjectExplorer::ProjectTree::currentProject();
    if (startUpProject)
        projectExplorerPlugin->runProject(startUpProject, Constants::NORMAL_RUN_MODE);
}

void ProjectExecutor::buildingFinished(bool successfully)
{
    if (!successfully)
        emit error();
}
