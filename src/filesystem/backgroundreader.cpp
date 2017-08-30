#include "backgroundreader.h"
#include "constants.h"
#include "debug.h"

#include <QImageReader>
#include <QPixmap>

BackgroundReader::BackgroundReader (QObject *parent) :
  QObject (parent)
{

}

void BackgroundReader::readIcon (const QString &fileName)
{
  QImageReader reader (fileName);
  reader.setScaledSize ({constants::iconSize, constants::iconSize});
  const auto image = reader.read ();
  if (!image.isNull ())
  {
    const auto pixmap = QPixmap::fromImage (image);
    emit iconRead (fileName, pixmap);
  }
  else
  {
    LWARNING () << "Icon read error" << LARG (fileName) << LARG (reader.errorString ());
  }
}

#include "moc_backgroundreader.cpp"
