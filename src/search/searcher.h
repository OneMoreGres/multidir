#pragma once

#include <QObject>
#include <QVector>
#include <QByteArrayMatcher>

#include <atomic>

class SearchResultsModel;

class Searcher : public QObject
{
Q_OBJECT
public:
  explicit Searcher (QObject *parent = nullptr);
  ~Searcher ();

  void setRecursive (bool isOn);
  void setFilePatterns (const QStringList &filePatterns);
  void setText (const QString &text);

  void startAsync (const QStringList &dirs);
  void abort ();

signals:
  void finished ();
  void foundFile (const QString &file);
  void foundText (const QString &file, int byteOffset, const QString &line);

private:
  struct Options
  {
    QVector<QRegExp> filePatterns;
    QByteArrayMatcher text;
    bool recursive{true};
  };

  void searchFiles (QStringList dirs, Options options, int depth);
  void searchText (const QString &fileName, Options options);

  std::atomic_bool isAborted_;
  Options options_;
};
