#pragma once

#include <QWidget>

class SearchResultsModel;
class Searcher;

class QLabel;
class QLineEdit;
class QTreeView;
class QCheckBox;
class QDialogButtonBox;
class QSettings;

class SearchWidget : public QWidget
{
public:
  explicit SearchWidget (QWidget *parent = nullptr);
  ~SearchWidget ();

  void setDefaultDir (const QString &path);

private:
  void saveState (QSettings &settings) const;
  void restoreState (QSettings &settings);

  void setRunning (bool isRunning);
  void start ();
  void abort ();
  void finished ();

  QLineEdit *dir_;
  QLineEdit *filePattern_;
  QLineEdit *text_;
  QCheckBox *recursive_;
  QDialogButtonBox *buttons_;
  QTreeView *results_;

  SearchResultsModel *model_;
  Searcher *searcher_;
};
