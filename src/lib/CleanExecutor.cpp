#include "CleanExecutor.h"

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/buildconfiguration.h>

#include <QDebug>

CleanExecutor::CleanExecutor(QObject *parent) :
    ProcessExecutor(parent)
{
}

void CleanExecutor::execute()
{
    using namespace ProjectExplorer;

    Project *project = ProjectExplorer::ProjectTree::currentProject();

    const QString &buildDir = project->activeTarget()->activeBuildConfiguration()->buildDirectory().toString();
    const QString &objectFilesDir = getObjectFilesDir(buildDir);

    const QString program = QLatin1String("lcov");
    const QStringList arguments = {
        QLatin1String("-z"),
        QLatin1String("-d"),
        objectFilesDir
    };

    process->start(program, arguments);
}
