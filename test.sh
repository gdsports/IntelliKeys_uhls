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
# Install the latest Arduino JSON library
arduino --install-library "ArduinoJson"
arduino --install-library "Adafruit DotStar"
git clone https://github.com/adafruit/Adafruit_TinyUSB_Arduino
git clone https://github.com/adafruit/Adafruit_SPIFlash
git clone https://github.com/adafruit/SdFat
# Install USB Host Library SAMD
git clone https://github.com/gdsports/USB_Host_Library_SAMD
# Install IntelliKeys library
git clone https://github.com/gdsports/IntelliKeys_uhls.git
cd ${LIBDIR}/IntelliKeys_uhls/examples
# For production
BUILDBOARDS_M4="adafruit_feather_m4 adafruit_metro_m4 adafruit_grandcentral_m4 adafruit_itsybitsy_m4 adafruit_feather_m4"
BUILDBOARDS_M0="adafruit_trinket_m0 adafruit_metro_m0 adafruit_feather_m0_express adafruit_circuitplayground_m0 adafruit_circuitplayground_m0"

for BOARDNAME in $BUILDBOARDS_M0 $BUILDBOARDS_M4
do
    echo "BOARDNAME=${BOARDNAME}"
    if [[ $BOARDNAME == *"m4"* ]]
    then
        UF2OFFSET="0x4000"
    else
        UF2OFFSET="0x2000"
    fi
    echo "UF2OFFSET=${UF2OFFSET}"
    BOARD="adafruit:samd:${BOARDNAME}"
    arduino --pref "custom_debug=${BOARDNAME}_off" \
            --pref "custom_cache=${BOARDNAME}_on" \
            --pref "custom_debug=${BOARDNAME}_off" \
            --pref "custom_maxqspi=${BOARDNAME}_50" \
            --pref "custom_opt=${BOARDNAME}_small" \
            --pref "custom_speed=${BOARDNAME}_120" --save-prefs
    BUILDPATH=/tmp/mybuilddir_$$/${BOARDNAME}
    CC="arduino --verify --buildpath $BUILDPATH --preserve-temp-files --board ${BOARD}"
    echo "BUILDPATH=$BUILDPATH"
    rm -rf $BUILDPATH
    mkdir -p $BUILDPATH
    for EXAMPLE in $(find . -name '*.ino')
    do
        echo "EXAMPLE=$EXAMPLE"
        if [[ ${EXAMPLE} == *"ikrawevent_ard"* ]]
        then
            arduino --pref "custom_usbstack=${BOARDNAME}_tinyusb" --save-prefs
        else
            arduino --pref "custom_usbstack=${BOARDNAME}_arduino" --save-prefs
        fi
        if $CC ${EXAMPLE}
        then
            inoname="$(basename -- ${EXAMPLE})"
            uf2conv.py -b ${UF2OFFSET} -c $BUILDPATH/${inoname}.bin -o ${inoname}.${BOARDNAME}.bin.uf2
        fi
    done
done

