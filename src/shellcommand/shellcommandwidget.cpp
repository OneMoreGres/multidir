#include "shellcommandwidget.h"
#include "debug.h"
#include "shellcommand.h"
#include "shortcutmanager.h"
#include "styleoptionsproxy.h"

#include <QTextEdit>
#include <QBoxLayout>
#include <QSplitter>
#include <QSettings>
#include <QCloseEvent>
#include <QMenuBar>

namespace
{
const QString qs_size = "command/size";
}

ShellCommandWidget::ShellCommandWidget (QWidget *parent) :
  QWidget (parent),
  process_ (new QProcess (this)),
  log_ (new QTextEdit (this)),
  userInput_ (new QTextEdit (this)),
  errorColor_ ()
{
  // self
  setObjectName ("command");
  setAttribute (Qt::WA_DeleteOnClose);


  // widgets
  log_->setUndoRedoEnabled (false);

  auto menuBar = new QMenuBar (this);
  auto fileMenu = menuBar->addMenu (tr ("File"));


  // actions
  auto processInput = ShortcutManager::create (this, Qt::CTRL | Qt::Key_Return);
  connect (processInput, &QAction::triggered, this, &ShellCommandWidget::processUserInput);

  auto esc = ShortcutManager::create (this, Qt::Key_Escape);
  connect (esc, &QAction::triggered, this, &ShellCommandWidget::close);

  auto eot = ShortcutManager::create (this, Qt::CTRL | Qt::Key_D);
  connect (eot, &QAction::triggered, this, &ShellCommandWidget::processUserInputWithEot);

  auto terminate = fileMenu->addAction (tr ("Terminate"));
  connect (terminate, &QAction::triggered, this, &ShellCommandWidget::terminate);
  connect (terminate, &QAction::triggered, this, &ShellCommandWidget::close);

  connect (process_, &QProcess::started, this, &ShellCommandWidget::handleStart);
  connect (process_, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
           this, &ShellCommandWidget::handleFinish);
  connect (process_, &QProcess::readyReadStandardError, this, &ShellCommandWidget::readError);
  connect (process_, &QProcess::readyReadStandardOutput, this, &ShellCommandWidget::readOut);


  // layout
  auto splitter = new QSplitter (Qt::Vertical, this);
  splitter->addWidget (log_);
  splitter->addWidget (userInput_);
  splitter->setStretchFactor (0, 5);
  splitter->setStretchFactor (1, 1);

  auto layout = new QVBoxLayout (this);
  layout->addWidget (menuBar);
  layout->addWidget (splitter);


  // initialize
  userInput_->setFocus ();

  connect (&StyleOptionsProxy::instance (), &StyleOptionsProxy::changed,
           this, &ShellCommandWidget::updateStyle);
  updateStyle ();

  QSettings settings;
  const auto size = settings.value (qs_size).toSize ();
  if (!size.isEmpty ())
  {
    resize (size);
  }
}

ShellCommandWidget::~ShellCommandWidget ()
{
  QSettings settings;
  settings.setValue (qs_size, size ());

  hide ();
  terminate ();
}

bool ShellCommandWidget::run (const ShellCommand &command)
{
  log_->append (tr ("<b>running \"%1\" in \"%2\"</b>").arg (command.command (),
                                                            command.workDir ()));
  const auto result = command.run (*process_);
  setWindowTitle (name ());
  return result;
}

QString ShellCommandWidget::name () const
{
  return process_->program ();
}

bool ShellCommandWidget::isRunning () const
{
  return process_->state () == QProcess::Running;
}

bool ShellCommandWidget::isFailed () const
{
  return process_->state () == QProcess::NotRunning &&
         (process_->exitStatus () == QProcess::CrashExit ||  process_->exitCode () != 0);
}

void ShellCommandWidget::handleStart ()
{
  log_->append (tr ("<b>started</b>"));
  emit stateChanged (this);
}

void ShellCommandWidget::handleFinish (int exitCode, QProcess::ExitStatus exitStatus)
{
  log_->append (tr ("<b>%1 with code %2</b>")
                .arg (exitStatus == QProcess::NormalExit ? tr ("Finished") : tr ("Crashed"))
                .arg (exitCode));
  userInput_->setEnabled (false);
  setFocus ();
  emit stateChanged (this);
}

void ShellCommandWidget::readError ()
{
  process_->setReadChannel (QProcess::StandardError);

  const auto color = log_->textColor ();
  log_->setTextColor (errorColor_);

  log_->append (QString::fromLocal8Bit (process_->readAll ()));

  log_->setTextColor (color);
}

void ShellCommandWidget::readOut ()
{
  process_->setReadChannel (QProcess::StandardOutput);
  log_->append (QString::fromLocal8Bit (process_->readAll ()));
}

void ShellCommandWidget::processUserInput ()
{
  auto text = userInput_->toPlainText ();
  if (text.isEmpty () || !isRunning ())
  {
    return;
  }

  process_->write (text.toUtf8 () + '\n');
  log_->append (tr ("<b>user input \"%1\"</b>").arg (text.replace ('\n', "<br>")));

  userInput_->clear ();
}

void ShellCommandWidget::processUserInputWithEot ()
{
  processUserInput ();
  if (isRunning ())
  {
    process_->closeWriteChannel ();
    log_->append (tr ("<b>closed write channel</b>"));
  }
}

void ShellCommandWidget::terminate ()
{
  if (isRunning ())
  {
    process_->terminate ();
    const auto timeout = 5000;
    process_->waitForFinished (timeout);
    process_->kill ();
  }
}

void ShellCommandWidget::updateStyle ()
{
  const auto &options = StyleOptionsProxy::instance ();
  errorColor_ = options.stdErrOutputColor ();
}

void ShellCommandWidget::closeEvent (QCloseEvent *event)
{
  if (!isRunning ())
  {
    event->accept ();
  }
  else
  {
    event->ignore ();
    hide ();
  }
}

#include "moc_shellcommandwidget.cpp"
