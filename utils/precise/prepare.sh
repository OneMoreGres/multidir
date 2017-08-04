#!/bin/bash

# build dependencies

if [ -z "`which apt-add-repository`" ]; then
  echo "deb http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu precise main" > /etc/apt/sources.list.d/ubuntu-toolchain.list
  apt-key adv --keyserver keyserver.ubuntu.com --recv-keys BA9EF27F

  echo "deb http://ppa.launchpad.net/beineri/opt-qt562/ubuntu precise main" > /etc/apt/sources.list.d/qt.list
  apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E9977759
else
  apt-add-repository -y ppa:ubuntu-toolchain-r/test
  apt-add-repository -y ppa:beineri/opt-qt562
fi

apt-get -qq update
apt-get -y -qq install wget ca-certificates libfontconfig libgl1-mesa-dev make libxcb-util0-dev g++-5 qt56base qt56x11extras qt56imageformats qt56tools qt56translations


update-alternatives --force --install /usr/bin/gcc gcc /usr/bin/gcc-5 50 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-5
update-alternatives --set gcc /usr/bin/gcc-5
echo `g++ --version`

qt_path="/opt/qt56/"
qt_bin="/opt/qt56/bin"
alternatives="update-alternatives --force --install /usr/bin/qmake qt $qt_bin/qmake 56 "
for file in `find "$qt_bin" -maxdepth 1 -type f -executable`; do
    name=$(basename $file)
    if [ "$name" == "qmake" ]; then
	continue;
    fi
    alternatives="$alternatives --slave /usr/bin/$name $name $qt_bin/$name "
done
if $alternatives; then
    update-alternatives --set qt $qt_bin/qmake
else
    ln -s $qt_path /opt/qt
fi
echo `qmake --version`



# appimage dependencies
apt-get -y -qq install libfuse2 libxi6 libsm6 libxrender1 libegl1-mesa libjasper1 file

appimage_target="/usr/bin/linuxdeployqt"
appimage_dist="linuxdeployqt-continuous-x86_64.AppImage"
appimage_host="https://github.com/probonopd/linuxdeployqt/releases/download/continuous"
cache_host="http://172.17.0.1:8000"
if ! wget -q -t 1 --connect-timeout=5 "$cache_host/$appimage_dist" -O "$appimage_target"; then
  wget -cq "$appimage_host/$appimage_dist" -O "$appimage_target"
fi
chmod +x "$appimage_target"



# cleanup
apt-get clean
rm -rf /var/lib/apt/lists/* /usr/share/doc/* /usr/share/man/*



# rest
mkdir /build
