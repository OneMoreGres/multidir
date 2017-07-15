#!/bin/bash

# build dependencies

# rescent compiler
echo "deb http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu precise main" > /etc/apt/sources.list.d/ubuntu-toolchain.list
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys BA9EF27F

apt-get -qq update
apt-get -y -qq install wget libfontconfig libgl1-mesa-dev make libxcb-util0-dev g++-5

update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-5

# install qt
qt_dist="qt-opensource-linux-x64-5.6.2.run"
qt_host="https://download.qt.io/official_releases/qt/5.6/5.6.2"
cache_host="http://172.17.0.1:8000"
if ! wget -q "$cache_host/$qt_dist"; then
  wget -cq "$qt_host/$qt_dist"
fi
chmod +x "$qt_dist"

qt_script="qt_install.qs"
echo "
function Controller() {
  installer.autoRejectMessageBoxes();
  installer.installationFinished.connect(function() {gui.clickButton(buttons.NextButton);})
}
Controller.prototype.WelcomePageCallback = function() {gui.clickButton(buttons.NextButton);}
Controller.prototype.CredentialsPageCallback = function() {gui.clickButton(buttons.NextButton);}
Controller.prototype.IntroductionPageCallback = function() {gui.clickButton(buttons.NextButton);}
Controller.prototype.TargetDirectoryPageCallback = function() {
  gui.currentPageWidget().TargetDirectoryLineEdit.setText('/opt/qt');
  gui.clickButton(buttons.NextButton);
}
Controller.prototype.ComponentSelectionPageCallback = function() {
  var widget = gui.currentPageWidget(); widget.deselectAll();
  widget.selectComponent('qt.56.gcc_64');
  gui.clickButton(buttons.NextButton);
}
Controller.prototype.LicenseAgreementPageCallback = function() {
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton);
}
Controller.prototype.StartMenuDirectoryPageCallback = function() {gui.clickButton(buttons.NextButton);}
Controller.prototype.ReadyForInstallationPageCallback = function() {gui.clickButton(buttons.NextButton);}
Controller.prototype.FinishedPageCallback = function() {
  var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm;
  if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
    checkBoxForm.launchQtCreatorCheckBox.checked = false;
  }
  gui.clickButton(buttons.FinishButton);
}
" > "$qt_script"

QT_QPA_PLATFORM="minimal" ./$qt_dist --script "$qt_script"
qt_path="/opt/qt/5.6/gcc_64"
qt_bin="/opt/qt/5.6/gcc_64/bin"

alternatives="update-alternatives --install /usr/bin/qmake qt $qt_bin/qmake 56 "
for file in `find "$qt_bin" -maxdepth 1 -type f -executable`; do
    name=$(basename $file)
    if [ "$name" == "qmake" ]; then
	continue;
    fi
    alternatives="$alternatives --slave /usr/bin/$name $name $qt_bin/$name "
done
`$alternatives`



# appimage dependencies
apt-get -y -qq install libfuse2 libxi6 libsm6 libxrender1 libegl1-mesa libjasper1 file

appimage_target="/usr/bin/linuxdeployqt"
appimage_dist="linuxdeployqt-continuous-x86_64.AppImage"
appimage_host="https://github.com/probonopd/linuxdeployqt/releases/download/continuous"
cache_host="http://172.17.0.1:8000"
if ! wget -q "$cache_host/$appimage_dist" -O "$appimage_target"; then
  wget -cq "$appimage_host/$appimage_dist" -O "$appimage_target"
fi
chmod +x "$appimage_target"



# cleanup
rm -rf "$qt_dist" "$qt_script" /opt/qt/Docs /opt/qt/Examples /opt/qt/Tools /opt/qt/Maintenance*
rm /var/lib/apt/lists/* /usr/share/doc /usr/share/man
apt-get clean



# rest
mkdir build
