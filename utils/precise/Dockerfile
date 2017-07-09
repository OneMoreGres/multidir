FROM ubuntu:precise

RUN apt-get -qq update && \
    apt-get -y -qq install git libgl1-mesa-dev fontconfig \
        build-essential clang-3.4 libxcb-util0-dev wget fuse libxi6 \
        libsm6 libxrender1 libegl1-mesa libjasper1 && \
    apt-get -qq clean


ENV QT_RUN qt-opensource-linux-x64-5.6.2.run
#dl from https://download.qt.io/official_releases/qt/5.6/5.6.2/qt-opensource-linux-x64-5.6.2.run
ENV APPIMAGE_RUN linuxdeployqt
#dl from https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage

ENV DL_HOST http://172.17.0.1:8000
ENV QT_QPA_PLATFORM minimal
ENV QT_PATH /opt/qt/5.6/gcc_64
RUN wget -c "$DL_HOST/$QT_RUN" && \
    wget -c "$DL_HOST/qt_install.qs" && \
    wget -c "$DL_HOST/$APPIMAGE_RUN" -O /usr/bin/$APPIMAGE_RUN && \
    chmod +x /$QT_RUN && \
    chmod +x /usr/bin/$APPIMAGE_RUN && \
    /$QT_RUN --script /qt_install.qs && \
    rm -rf $QT_RUN qt_install.qs /opt/qt/Docs /opt/qt/Examples /opt/qt/Tools /opt/qt/Maintenance* && \
    echo "$QT_PATH/lib" > /etc/ld.so.conf.d/qt.conf && \
    ldconfig

ENV QMAKESPEC linux-clang
ENV PATH "$QT_PATH/bin:$PATH"
