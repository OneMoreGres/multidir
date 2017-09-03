#include "searchwidget.h"
#include "searchresultsmodel.h"
#include "searcher.h"

#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QCheckBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QDir>
#include <QPushButton>
#include <QSettings>

namespace
{
const QString qs_geometry = "search/geometry";
}

SearchWidget::SearchWidget (QWidget *parent) :
  QWidget (parent),
  dir_ (new QLineEdit (this)),
  filePattern_ (new QLineEdit (this)),
  text_ (new QLineEdit (this)),
  recursive_ (new QCheckBox (tr ("Recursive"), this)),
  buttons_ (new QDialogButtonBox (QDialogButtonBox::Apply |
                                  QDialogButtonBox::Abort, this)),
  results_ (new QTreeView (this)),
  model_ (new SearchResultsModel (this)),
  searcher_ (new Searcher (this))
{
  setRunning (false);
  connect (buttons_->button (QDialogButtonBox::Apply), &QPushButton::clicked,
           this, &SearchWidget::start);
  connect (buttons_->button (QDialogButtonBox::Abort), &QPushButton::clicked,
           this, &SearchWidget::abort);

  dir_->setText (QDir::homePath ());

  filePattern_->setText (QLatin1String ("*"));

  recursive_->setChecked (true);

  results_->setModel (model_);
  results_->hideColumn (SearchResultsModel::ByteOffset);
  results_->setUniformRowHeights (true);

  //  searcher_->setModel (*model_);
  connect (searcher_, &Searcher::foundText,
           model_, &SearchResultsModel::addText);
  connect (searcher_, &Searcher::foundFile,
           model_, &SearchResultsModel::addFile);
  connect (searcher_, &Searcher::finished,
           this, &SearchWidget::finished);


  {
    auto layout = new QGridLayout (this);
    auto row = 0;
    layout->addWidget (new QLabel (tr ("Search in:")), row, 0);
    layout->addWidget (dir_, row, 1);

    ++row;
    layout->addWidget (new QLabel (tr ("File pattern:")), row, 0);
    layout->addWidget (filePattern_, row, 1);

    ++row;
    layout->addWidget (new QLabel (tr ("Search text:")), row, 0);
    layout->addWidget (text_, row, 1);

    ++row;
    layout->addWidget (recursive_, row, 0);

    ++row;
    layout->addWidget (buttons_, row, 0, 1, 2);

    ++row;
    layout->addWidget (new QLabel (tr ("Results")), row, 0, 1, 2);

    ++row;
    layout->addWidget (results_, row, 0, 1, 2);
  }


  QSettings settings;
  restoreState (settings);
}

SearchWidget::~SearchWidget ()
{
  QSettings settings;
  saveState (settings);
}

void SearchWidget::saveState (QSettings &settings) const
{
  settings.setValue (qs_geometry, saveGeometry ());
}

void SearchWidget::restoreState (QSettings &settings)
{
  restoreGeometry (settings.value (qs_geometry).toByteArray ());
}

void SearchWidget::setRunning (bool isRunning)
{
  buttons_->button (QDialogButtonBox::Apply)->setEnabled (!isRunning);
  buttons_->button (QDialogButtonBox::Abort)->setEnabled (isRunning);
}

void SearchWidget::setDefaultDir (const QString &path)
{
  dir_->setText (path);
}

void SearchWidget::start ()
{
  const auto dirs = dir_->text ().split (',');
  if (dirs.isEmpty ())
  {
    return;
  }

  model_->clear ();

  searcher_->setRecursive (recursive_->isChecked ());
  searcher_->setFilePatterns (filePattern_->text ().split (','));
  searcher_->setText (text_->text ());

  searcher_->startAsync (dirs);

  setRunning (true);
}

void SearchWidget::abort ()
{
  searcher_->abort ();
}

void SearchWidget::finished ()
{
  setRunning (false);
}
