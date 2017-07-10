#include "CleanExecutor.h"

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/kitinformation.h>

#include <QDebug>
#include <QRegularExpression>

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
    QStringList arguments;
    if (ToolChain *tc = ToolChainKitInformation::toolChain(project->activeTarget()->kit(),
                                                           ProjectExplorer::Constants::CXX_LANGUAGE_ID)) {
        QRegularExpression compilerRegexp(QLatin1String("(gcc|g++|clang|clang++)"));
        QRegularExpressionMatch match = compilerRegexp.match(tc->compilerCommand().fileName());
        if (match.hasMatch()) {
            QString filename = tc->compilerCommand().fileName().replace(match.capturedStart(),
                                                                        match.capturedLength(),
                                                                        QLatin1String("gcov"));
            Utils::FileName gcovPath = tc->compilerCommand().parentDir().appendPath(filename);
            if (gcovPath.exists()) {
                arguments << QLatin1String("--gcov-tool")
                          << gcovPath.toString();
            }
        }
    }
    arguments << QLatin1String("-z")
              << QLatin1String("-d")
              << objectFilesDir;

    process->start(program, arguments);
}
