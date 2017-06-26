#!/bin/bash

if [ -z "$QT_BIN" ]; then
  #QT_BIN=/opt/qt/5.9/gcc_64/bin
	QT_BIN=$HOME/Qt/Online/5.9/gcc_64/bin
fi
export PATH="$QT_BIN:$PATH"

