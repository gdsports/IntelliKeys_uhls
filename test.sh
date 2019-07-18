#!/bin/bash
IDEVER="1.8.9"
WORKDIR="/tmp/autobuild_$$"
mkdir -p ${WORKDIR}
# Install Ardino IDE in work directory
wget -O arduino.tar.xz https://downloads.arduino.cc/arduino-${IDEVER}-linux64.tar.xz
tar xf arduino.tar.xz -C ${WORKDIR}
rm arduino.tar.xz
# Create portable sketchbook and library directories
IDEDIR="${WORKDIR}/arduino-${IDEVER}"
LIBDIR="${IDEDIR}/portable/sketchbook/libraries"
mkdir -p "${LIBDIR}"
export PATH="${IDEDIR}:${PATH}"
cd ${IDEDIR}
which arduino
arduino --install-boards "arduino:samd"
arduino --pref "boardsmanager.additional.urls=https://adafruit.github.io/arduino-board-index/package_adafruit_index.json" --save-prefs
arduino --install-boards "adafruit:samd"
cd $LIBDIR
# Install USB Host Library SAMD
git clone https://github.com/gdsports/USB_Host_Library_SAMD
# For production
BOARD="adafruit:samd:adafruit_trinket_m0"
CC="arduino --verify --board ${BOARD}"
arduino --board "${BOARD}" --save-prefs
git clone https://github.com/gdsports/IntelliKeys_uhls.git
# ikevent.ino should work.
cd ${LIBDIR}/IntelliKeys_uhls/examples
(find . -name '*.ino' -print0 | xargs -0 -n 1 $CC >/tmp/samd_$$.txt 2>&1)&
