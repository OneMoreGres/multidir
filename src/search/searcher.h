#pragma once

#include <QObject>
#include <QVector>
#include <QStringMatcher>

#include <atomic>

class QTextDecoder;

class SearchResultsModel;

class Searcher : public QObject
{
Q_OBJECT
public:
  explicit Searcher (QObject *parent = nullptr);
  ~Searcher ();

  void setRecursive (bool isOn);
  void setFilePatterns (const QStringList &filePatterns);
  void setText (const QString &text, Qt::CaseSensitivity caseSeisitivity,
                bool wordOnly);

  void startAsync (const QStringList &dirs);
  void abort ();

signals:
  void finished ();
  void foundFile (const QString &file);
  void foundText (const QString &file, const QMap<int, QString> &occurrences);

private:
  struct Options
  {
    QVector<QRegExp> filePatterns;
    QStringMatcher text;
    bool recursive{true};
    bool wordOnly{false};
    int textLength{0};
    int sideContextLength{50};
    int maxOccurenceLength{0};
    QTextDecoder *textDecoder{nullptr};
  };

  void searchFiles (QStringList dirs, Options options, int depth);
  void searchText (const QString &fileName, Options options);

  std::atomic_bool isAborted_;
  Options options_;
};
