#include "ProcessExecutor.h"

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/buildconfiguration.h>

#include <QDebug>

ProcessExecutor::ProcessExecutor(QObject *parent) :
    Executor(parent),
    process(new QProcess(this))
{
    connect(process,SIGNAL(readyReadStandardOutput()),SLOT(readOutput()));
    connect(process,SIGNAL(readyReadStandardError()),SLOT(readError()));
    connect(process,SIGNAL(finished(int,QProcess::ExitStatus)),SLOT(handleCoverageResults(int,QProcess::ExitStatus)));
}

void ProcessExecutor::execute()
{
    using namespace ProjectExplorer;

    Project *project = ProjectExplorer::ProjectTree::currentProject();

    const QString &buildDir = project->activeTarget()->activeBuildConfiguration()->buildDirectory().toString();
    const QString &objectFilesDir = getObjectFilesDir(buildDir);

    const QString &rootDir = project->projectDirectory().toString();
    QDir dir(rootDir);
    dir.mkdir(QLatin1String("./coverage"));
    const QString outputFileName = rootDir + QLatin1Char('/') + QLatin1String("coverage/result.info");

    const QString program = QLatin1String("lcov");
    const QStringList arguments = {
        QLatin1String("--gcov-tool"),
        QLatin1String("/usr/bin/gcov-4.6"),
        QLatin1String("-d"),
        objectFilesDir,
        QLatin1String("-c"),
        QLatin1String("-o"),
        outputFileName,
        QLatin1String("-b"),
        buildDir
    };

    process->start(program, arguments);
}

void ProcessExecutor::readOutput()
{
}

void ProcessExecutor::readError()
{
}

void ProcessExecutor::handleCoverageResults(int code, QProcess::ExitStatus exitStatus)
{    
    if (code == 0 && exitStatus == QProcess::NormalExit)
        emit finished();
    else
        emit error();
}

//#TOTEST:
QString ProcessExecutor::getRunConfigurationPath(ProjectExplorer::ProjectNode *parent, ProjectExplorer::RunConfiguration *activeRunConfiguration) const
{
    using namespace ProjectExplorer;

    foreach (ProjectNode *projectNode, parent->subProjectNodes()) {
        foreach (RunConfiguration *runConfiguration, projectNode->runConfigurations())
            if (runConfiguration == activeRunConfiguration)
                return projectNode->filePath().toString();

        const QString &runConfigurationPath = getRunConfigurationPath(projectNode, activeRunConfiguration);
        if (!runConfigurationPath.isEmpty())
            return runConfigurationPath;
    }

    return QString();
}

//#TOTEST:
QString ProcessExecutor::getObjectFilesDir(const QString &buildDir) const
{
    QFile makeFile(buildDir + QLatin1String("/Makefile"));
    QTextStream out(&makeFile);

    if (makeFile.open(QIODevice::ReadOnly)) {
        while (!out.atEnd()) {
            const QString &line = out.readLine();

            QRegExp rx(QLatin1String("OBJECTS_DIR\\s*=\\s*([^\\s]*)$"));
            if (rx.indexIn(line) != -1) {
                const QString &objectsDir = rx.cap(1);
                return QString(QLatin1String("%1/%2")).arg(buildDir).arg(objectsDir);
            }
        }
    }

    return buildDir;
}
