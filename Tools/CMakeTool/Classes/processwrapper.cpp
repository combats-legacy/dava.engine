/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/


#include "processwrapper.h"
#include "filesystemhelper.h"
#include <QProgressDialog>
#include <QTimer>
#include <QRegularExpression>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QDirIterator>

ProcessWrapper::ProcessWrapper(QObject* parent)
    : QObject(parent)
{
    connect(&process, &QProcess::readyReadStandardOutput, this, &ProcessWrapper::OnReadyReadStandardOutput);
    connect(&process, &QProcess::readyReadStandardError, this, &ProcessWrapper::OnReadyReadStandardError);
    connect(&process, &QProcess::stateChanged, this, &ProcessWrapper::OnProcessStateChanged);
    connect(&process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &ProcessWrapper::OnProcessError);
}

void ProcessWrapper::LaunchCmake(const QString& command, bool needClean, const QString& buildFolder)
{
    taskQueue.enqueue({ command, needClean, buildFolder });
    if (process.state() == QProcess::NotRunning)
    {
        StartNextCommand();
    }
}

void ProcessWrapper::FindAndOpenProjectFile(const QString& buildFolder)
{
    QString suffix =
#if defined(Q_OS_WIN)
    "sln";
#elif defined(Q_OS_MAC)
    "xcodeproj";
#else 
#error "unsupported platform"
#endif //platform
    QDir sourceFolderDir(buildFolder);
    QDirIterator it(buildFolder);
    while (it.hasNext())
    {
        it.next();
        QFileInfo fileInfo(it.fileInfo());
#if defined(Q_OS_WIN)
        if (fileInfo.isFile())
#elif defined(Q_OS_MAC)
        if (fileInfo.isDir()) //xcodeproj is a directory
#else
#error "unsupported platform"
#endif //platform
        {
            if (fileInfo.suffix() == suffix)
            {
                if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath())))
                {
                    emit processStandardError(tr("Can not open project file!"));
                }
                return;
            }
        }
    }
    emit processStandardError(tr("Can not find project file!"));
}

void ProcessWrapper::OpenFolderInExplorer(const QString& folder)
{
    QFileInfo fileInfo(folder);
    if (!fileInfo.exists())
    {
        emit processStandardError(tr("build folder are not exists!"));
        return;
    }
    QString path = fileInfo.canonicalFilePath();
#ifdef Q_OS_MAC
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + path + "\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#endif
#ifdef Q_OS_WIN
    QString param;
    param = QLatin1String("/select,");
    param += QDir::toNativeSeparators(path);
    QString command = QString("explorer") + " " + param;
    QProcess::startDetached(command);
#endif
}

void ProcessWrapper::BlockingStopAllTasks()
{
    taskQueue.clear();
    KillProcess();
    QTimer performTimer;
    performTimer.setInterval(100);
    performTimer.setSingleShot(false);
    QProgressDialog progressDialog(tr("Finishing tasks"), tr("Exit"), 0, 0);
    connect(&performTimer, &QTimer::timeout, [&progressDialog, this]() {
        if (process.state() == QProcess::NotRunning)
        {
            progressDialog.accept();
        }
        else
        {
            if (progressDialog.wasCanceled())
            {
                exit(0);
            }
        }
    });
    if (process.state() == QProcess::NotRunning)
    {
        return;
    }
    performTimer.start();
    progressDialog.exec();
}

void ProcessWrapper::KillProcess()
{
    process.kill();
}

bool ProcessWrapper::IsRunning() const
{
    return running;
}

void ProcessWrapper::OnReadyReadStandardOutput()
{
    QString text = process.readAllStandardOutput();
    emit processStandardOutput(text);
}

void ProcessWrapper::OnReadyReadStandardError()
{
    QString text = process.readAllStandardError();
    emit processStandardError(text);
}

void ProcessWrapper::OnProcessStateChanged(QProcess::ProcessState newState)
{
    SetRunning(newState != QProcess::NotRunning);
    QString processState;
    switch (newState)
    {
    case QProcess::NotRunning:
        processState = "not running";
        break;
    case QProcess::Starting:
        processState = "starting";
        break;
    case QProcess::Running:
        processState = "running";
        break;
    }

    emit processStateChanged("cmake process is " + processState);
    if (newState == QProcess::NotRunning)
    {
        QMetaObject::invokeMethod(this, "StartNextCommand", Qt::QueuedConnection);
    }
}

void ProcessWrapper::OnProcessError(QProcess::ProcessError error)
{
    QString processError;
    switch (error)
    {
    case QProcess::FailedToStart:
        processError = "failed to start";
        break;
    case QProcess::Crashed:
        processError = "crashed";
        break;
    case QProcess::Timedout:
        processError = "timed out";
        break;
    case QProcess::ReadError:
        processError = "read error";
        break;
    case QProcess::WriteError:
        processError = "write error";
        break;
    case QProcess::UnknownError:
        processError = "unknown error";
        break;
    }
    emit processErrorChanged("process error: " + processError);
}

void ProcessWrapper::StartNextCommand()
{
    if (taskQueue.isEmpty())
    {
        return;
    }
    const Task& task = taskQueue.dequeue();
    const QString& buildFolder = task.buildFolder;
    if (!buildFolder.isEmpty())
    {
        if (!FileSystemHelper::IsDirExists(buildFolder))
        {
            if (FileSystemHelper::MkPath(buildFolder))
            {
                emit processStandardOutput(tr("created build folder %1").arg(buildFolder));
            }
            else
            {
                emit processStandardError(tr("can not create build folder %1").arg(buildFolder));
                QMetaObject::invokeMethod(this, "StartNextCommand", Qt::QueuedConnection);
                return;
            }
        }
        if (task.needClean)
        {
            CleanBuildFolder(task.buildFolder);
        }
    }
    Q_ASSERT(process.state() == QProcess::NotRunning);
    process.start(task.command);
}

bool ProcessWrapper::CleanBuildFolder(const QString& buildFolder) const
{
    QString keyFile = "CMakeCache.txt";
    FileSystemHelper::eErrorCode errCode = FileSystemHelper::ClearFolderIfKeyFileExists(buildFolder, keyFile);
    QString text = tr("Clear build folder: ");
    switch (errCode)
    {
    case FileSystemHelper::NO_ERRORS:
        text += tr("succesful");
        break;
    case FileSystemHelper::FOLDER_NAME_EMPTY:
        text += tr("path is empty!");
        break;
    case FileSystemHelper::FOLDER_NOT_EXISTS:
        text += tr("folder is not exists!");
        break;
    case FileSystemHelper::FOLDER_NOT_CONTAIN_KEY_FILE:
        text += tr("folder is not conain file %1, will not clear").arg(keyFile);
        break;
    case FileSystemHelper::CAN_NOT_REMOVE:
        text += tr("can not remove build dir recursively");
        break;
    case FileSystemHelper::CAN_NOT_CREATE_BUILD_FOLDER:
        text += tr("can not create build folder after recoursive removing");
        break;
    default:
        Q_ASSERT(false && "unhandled error code");
        break;
    }
    if (errCode == FileSystemHelper::NO_ERRORS)
    {
        emit processStandardOutput(text);
    }
    else
    {
        emit processStandardError(text);
    }
    return errCode == FileSystemHelper::NO_ERRORS;
}

void ProcessWrapper::SetRunning(bool running_)
{
    if (running_ != running)
    {
        running = running_;
        emit runningChanged(running);
    }
}
