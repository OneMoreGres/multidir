#include "debug.h"

#include <QFile>
#include <QDir>
#include <QMutex>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>


namespace
{

QtMessageHandler original = nullptr;
QMutex mutex;
QFile file;
QTextStream stream;


void handler (QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  const auto typeName = QMap<QtMsgType, QByteArray> {
    {QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"},
    {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}
  }.value (type);

  auto message = QDateTime::currentDateTime ().toString ("yyyy.MM.dd hh:mm:ss.zzz").toUtf8 () +
                 ' ' + QByteArray (context.file) + ':' + QByteArray::number (context.line) +
                 ' ' + typeName + ": " + msg.toUtf8 () + '\n';

  ASSERT (original);
  original (type, context, msg);

  QMutexLocker locker (&mutex);
  file.write (message);
}
}

namespace debug
{
#ifdef DEVELOPMENT
std::atomic_bool isOn {true};
#else
std::atomic_bool isOn {false};
#endif

void setDebugMode (bool isOn)
{
  if (isOn)
  {
    ASSERT (!original);
    file.setFileName (QDir::home ().absoluteFilePath (QString ("multidir-%1.log")
                                                      .arg (QDateTime::currentDateTime ().toString ("yyyy-MM-dd-hh-mm-ss"))));
    if (!file.open (QFile::WriteOnly))
    {
      QMessageBox::information (nullptr, {}, QObject::tr ("Failed to create log file: %1")
                                .arg (file.fileName ()));
      return;
    }

    original = qInstallMessageHandler (handler);
    QMessageBox::information (nullptr, {}, QObject::tr ("Started logging to file: %1")
                              .arg (file.fileName ()));
  }
  else
  {
    file.close ();
    QDesktopServices::openUrl (QUrl::fromLocalFile (file.fileName ()));
    original = nullptr;
    qInstallMessageHandler (nullptr);
  }
  debug::isOn = isOn;
}

}
