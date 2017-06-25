#include "openwith.h"
#include "debug.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMultiHash>
#include <QDir>
#include <QTextStream>
#include <QUrl>
#include <QProcess>
#include <QMenu>
#include <QSettings>
#include <QFileIconProvider>
#include <QtConcurrentRun>

namespace
{

class ExternalApp
{
public:
  QString command;
  QString name;
  QIcon icon;

  bool isValid () const
  {
    return !name.isEmpty () && !command.isEmpty ();
  }

  bool operator== (const ExternalApp &r) const
  {
    return name == r.name;
  }

};

}

#ifdef Q_OS_LINUX

namespace
{
QMultiHash<QString, ExternalApp> allApps;
std::atomic_bool isInited {false};

void loadDesktop (const QString &fileName)
{
  // not use qsettings because it ignores text after ;
  QFile f (fileName);
  if (!f.open (QFile::ReadOnly))
  {
    WARNING () << "Failed to open desktop file" << LARG (fileName);
    return;
  }

  QStringList mimeTypes;
  ExternalApp app;
  const auto readValue = [](const QString &in) {
                           return in.mid (in.indexOf (QLatin1Char ('=')) + 1);
                         };
  QTextStream s (&f);
  auto desktop = false;
  while (!s.atEnd ())
  {
    const auto line = s.readLine ();
    if (!desktop)
    {
      desktop = (line.startsWith (QLatin1String ("[Desktop Entry]")));
      continue;
    }
    else if (line.startsWith (QLatin1Char ('['))) // other section
    {
      break;
    }

    if (line.startsWith (QLatin1String ("Exec=")))
    {
      app.command = readValue (line);
    }
    else if (line.startsWith (QLatin1String ("Name=")))
    {
      app.name = readValue (line);
    }
    else if (line.startsWith (QLatin1String ("Icon=")))
    {
      app.icon = QIcon::fromTheme (readValue (line));
      if (app.icon.isNull ())
      {
        app.icon = QIcon (QLatin1String ("/usr/share/app-install/icons/") + readValue (line));
      }
    }
    else if (line.startsWith (QLatin1String ("MimeType=")))
    {
      mimeTypes = readValue (line).split (QLatin1Char (';'));
    }
  }

  if (!mimeTypes.isEmpty () && app.isValid ())
  {
    for (const auto &type: mimeTypes)
    {
      if (!type.isEmpty ())
      {
        allApps.insertMulti (type, app);
      }
    }
  }
}

void loadDesktopsDir (const QString &name)
{
  for (const auto &i: QDir (name).entryInfoList (QDir::Dirs | QDir::NoDotAndDotDot))
  {
    loadDesktopsDir (i.absoluteFilePath ());
  }
  for (const auto &i: QDir (name).entryInfoList ({QLatin1String ("*.desktop")}))
  {
    loadDesktop (i.absoluteFilePath ());
  }
}

void fillApps ()
{
  QStringList desktopDirs {QLatin1String ("/usr/share/applications/"),
                           QLatin1String ("/usr/local/share/applications"),
                           QDir::home ().absolutePath () +
                           QLatin1String ("/.local/share/applications/")};
  for (const auto &i: desktopDirs)
  {
    loadDesktopsDir (i);
  }
  isInited = true;
}

}

void OpenWith::init ()
{
  QtConcurrent::run (fillApps);
}

QList<ExternalApp> applications (const QFileInfo &file)
{
  if (!isInited)
  {
    ERROR () << "External apps not initialized";
    return {};
  }
  QMimeDatabase db;
  const auto mime = db.mimeTypeForFile (file.fileName (), QMimeDatabase::MatchExtension);
  return allApps.values (mime.name ());
}

void openWith (const QFileInfo &file, const ExternalApp &app)
{
  if (app.command.isEmpty ())
  {
    ERROR () << "External app has not command" << LARG (app.name);
    return;
  }

  auto command = app.command;
  const auto path = file.absoluteFilePath ();
  const auto quoted = [](const QString &in) {
                        return QLatin1Char ('"') + in + QLatin1Char ('"');
                      };
  if (command.contains ('%'))
  {
    command.replace ("%i", QLatin1String (""));
    command.replace ("%c", QLatin1String ("-"));
    command.replace ("%k", QLatin1String (""));
    command.replace ("%f", quoted (path));
    command.replace ("%F", quoted (path));
    command.replace ("%u", quoted (QUrl::fromLocalFile (path).toString ()));
    command.replace ("%U", quoted (QUrl::fromLocalFile (path).toString ()));
  }
  else
  {
    command += " " + quoted (path);
  }
  auto ok = QProcess::startDetached (command);
  ERROR_IF (!ok) << "Failed to start external app" << LARG (app.name) << LARG (command);
}


#endif


#ifdef Q_OS_WIN

#  include <array>
#  include "windows.h"
#  pragma comment(lib, "version.lib")

namespace
{

QHash<QString, ExternalApp> appForName;
QMultiHash<QString, ExternalApp> appsForExtension;

const QString hkcu = "HKEY_CURENT_USER\\";
const QString hklm = "HKEY_LOCAL_MACHINE\\";
const QString classes = "SOFTWARE\\Classes\\";
const QString explorer = "Software\\Microsoft\\Windows\\"
                         "CurrentVersion\\Explorer\\FileExts\\";
const QString associations = "HKEY_CLASSES_ROOT\\SystemFileAssociations\\";
const QString appPaths = "Software\\Microsoft\\Windows\\"
                         "CurrentVersion\\App Paths\\";
const QString applications = "HKEY_CLASSES_ROOT\\Applications\\";
const QString openCommand = "\\shell\\open\\command";
const QString editCommand = "\\shell\\edit\\command";


QString appName (const QFileInfo &info)
{
  auto path = QDir::toNativeSeparators (info.absoluteFilePath ()).toStdWString ();
  auto size = GetFileVersionInfoSize (path.c_str (), NULL);
  if (size <= 0)
  {
    return {};
  }

  std::vector<PLONG> buffer (size);
  if (!GetFileVersionInfo (path.c_str (), NULL, size, buffer.data ()))
  {
    return {};
  }

  struct LANGANDCODEPAGE
  {
    WORD wLanguage;
    WORD wCodePage;
  } *langs;
  UINT langCount;
  if (!VerQueryValue (buffer.data (), L"\\VarFileInfo\\Translation",
                      (LPVOID *)&langs, &langCount))
  {
    return {};
  }

  for (auto i = 0; i < int(langCount / sizeof(struct LANGANDCODEPAGE)); ++i)
  {
    std::array<wchar_t, 256> paramName;
    wchar_t *paramValue;
    UINT paramSize;
    wsprintf (paramName.data (),
              L"\\StringFileInfo\\%04x%04x\\FileDescription",
              langs[i].wLanguage, langs[i].wCodePage);
    if (VerQueryValue (buffer.data (), (LPCWSTR)paramName.data (),
                       (LPVOID *)&paramValue, &paramSize))
    {
      return QString::fromWCharArray (paramValue, paramSize - 1);
    }
  }
  return {};
}


QString replaceVariables (const QString &string)
{
  QString result;
  QRegExp rx ("%.*%");
  rx.setMinimal (true);

  auto pos = 0;
  auto oldPos = 0;
  while ((pos = rx.indexIn (string, pos)) != -1)
  {
    result.append (string.mid (oldPos, pos - oldPos));
    auto var = string.mid (pos + 1, rx.matchedLength () - 2);
    auto val = QString::fromLocal8Bit (qgetenv (var.toLocal8Bit ().constData ()));
    // In case if variable has other variables in it, we replace them too.
    result.append (replaceVariables (val));
    pos += rx.matchedLength ();
    oldPos = pos;
  }
  result.append (string.mid (oldPos));
  return result;
}

QString command (const QString &name)
{
  QStringList paths {hkcu + appPaths + name,
                     hklm + appPaths + name,
                     hkcu + classes + name + openCommand,
                     hkcu + classes + name + editCommand,
                     hklm + classes + name + openCommand,
                     hklm + classes + name + editCommand,
                     applications + name + openCommand,
                     applications + name + editCommand};
  for (const auto &i: paths)
  {
    QSettings reg (i, QSettings::NativeFormat);
    const auto value = reg.value (".").toString ();
    if (!value.isEmpty ())
    {
      return replaceVariables (value);
    }
  }
  return {};
}

QString pathFromCommand (const QString &command)
{
  QString result;
  if (command.startsWith (QLatin1Char ('"')))
  {
    result = command.mid (1, command.indexOf (QLatin1Char ('"'), 1) - 1);
  }
  else
  {
    result = command.left (command.indexOf (QLatin1Char (' ')));
  }

  QFileInfo info (result);
  if (info.isAbsolute ())
  {
    return result;
  }

  for (const auto &path: QString::fromLocal8Bit (qgetenv ("PATH")).split (';'))
  {
    QString absolutePath = path + QLatin1Char ('/') + result;
    if (QFile::exists (absolutePath))
    {
      return absolutePath;
    }
  }
  return {};
}


ExternalApp loadByName (const QString &name)
{
  if (appForName.contains (name))
  {
    return appForName.value (name);
  }

  if (name.isEmpty () || name == QLatin1String (".") ||
      name == QLatin1String ("Default"))
  {
    return {};
  }

  auto command = ::command (name);
  if (command.isEmpty ())
  {
    return {};
  }

  QFileInfo info (pathFromCommand (command));
  if (!info.exists ())
  {
    return {};
  }

  ExternalApp app;
  app.command = command;
  app.icon = QFileIconProvider ().icon (info);
  app.name = appName (info);
  if (app.name.isEmpty ())
  {
    app.name = info.baseName ();
  }

  return *appForName.insert (name, app);
}

void loadApps (const QString &regPath,const QString &extension)
{
  QSettings reg (regPath + extension, QSettings::NativeFormat);

  QList<ExternalApp> result;
  reg.beginGroup (QStringLiteral ("OpenWithList"));
  for (const auto &i: reg.childGroups ())
  {
    result << loadByName (i);
  }
  for (const auto &i: reg.value (QStringLiteral ("MRUList")).toString ())
  {
    result << loadByName (reg.value (i).toString ());
  }
  reg.endGroup ();

  reg.beginGroup (QStringLiteral ("OpenWithProgids"));
  for (const auto &id: reg.childKeys ())
  {
    result << loadByName (id);
  }
  reg.endGroup ();

  auto contentType = reg.value ("PerceivedType").toString ();
  if (!contentType.isEmpty ())
  {
    QSettings reg (associations + contentType, QSettings::NativeFormat);
    reg.beginGroup ("OpenWithList");
    for (const auto &i: reg.childGroups ())
    {
      result <<  loadByName (i);
    }
  }
  for (const auto &i: result)
  {
    if (i.isValid ())
    {
      appsForExtension.insertMulti (extension, i);
    }
  }
}

}

void OpenWith::init ()
{
}

QList<ExternalApp> applications (const QFileInfo &file)
{
  const auto extension = QLatin1Char ('.') + file.suffix ();
  if (!appsForExtension.contains (extension))
  {
    loadApps (hkcu + classes, extension);
    loadApps (hklm + classes, extension);
    loadApps (hkcu + explorer, extension);
    loadApps (hklm + explorer, extension);
  }
  return appsForExtension.values (extension);
}

void openWith (const QFileInfo &file, const ExternalApp &app)
{
  auto command = app.command;
  auto arg = QLatin1Char ('\"') +
             QDir::toNativeSeparators (file.absoluteFilePath ()) +
             QLatin1Char ('\"');
  command.replace (",", "");
  if (command.contains ("%1"))
  {
    command.replace ("%1", arg);
  }
  else
  {
    command += " " + arg;
  }
  QProcess::startDetached (command);
}


#endif


void OpenWith::popupateMenu (QMenu &menu, const QFileInfo &file)
{
  menu.clear ();
  for (const auto &app: ::applications (file))
  {
    auto action = menu.addAction (app.icon, app.name);
    QObject::connect (action, &QAction::triggered,
                      [file, app] {openWith (file, app);});
  }
  if (menu.isEmpty ())
  {
    menu.setEnabled (false);
  }
}
