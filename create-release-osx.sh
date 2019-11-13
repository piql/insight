#!/bin/sh

rm -rf insight.app
rm -rf innsyn.app
rm *.dmg

#./create-icons-osx.sh

qmake || exit 1
make || exit 1
appname="innsyn.app"
mv insight.app $appname



qtver=`qmake -v | sed -e '2!d; s/.* \([0-9\.]*\) .*/\1/'`
deploytool=/usr/local/Cellar/qt/$qtver/bin/macdeployqt

$deploytool ./$appname


install_name_tool -change \
     /usr/local/Cellar/qt5/$qtver/lib/QtCore.framework/Versions/5/QtCore \
     @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
     $appname/Contents/Frameworks/QtGui.framework/QtGui

install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtCore.framework/Versions/5/QtCore \
    @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
    $appname/Contents/Frameworks/QtWidgets.framework/QtWidgets

install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtGui.framework/Versions/5/QtGui \
    @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui \
    $appname/Contents/Frameworks/QtWidgets.framework/QtWidgets 

install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtCore.framework/Versions/5/QtCore \
    @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
    $appname/Contents/Frameworks/QtXml.framework/QtXml

install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtCore.framework/Versions/5/QtCore \
    @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
    $appname/Contents/Frameworks/QtPrintSupport.framework/QtPrintSupport

install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtGui.framework/Versions/5/QtGui \
    @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui \
    $appname/Contents/Frameworks/QtPrintSupport.framework/QtPrintSupport

install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtWidgets.framework/Versions/5/QtWidgets \
    @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets \
    $appname/Contents/Frameworks/QtPrintSupport.framework/QtPrintSupport

install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtCore.framework/Versions/5/QtCore \
    @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
    $appname/Contents/Frameworks//QtSvg.framework/Versions/5/QtSvg

install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtGui.framework/Versions/5/QtGui \
    @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui \
    $appname/Contents/Frameworks//QtSvg.framework/Versions/5/QtSvg

Install_name_tool -change \
    /usr/local/Cellar/qt5/$qtver/lib/QtWidgets.framework/Versions/5/QtWidgets \
    @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets \
    $appname/Contents/Frameworks//QtSvg.framework/Versions/5/QtSvg




imgfmt="$appname/Contents/PlugIns/imageformats"
plat="$appname/Contents/PlugIns/platforms"
print="$appname/Contents/PlugIns//printsupport"
declare -a arr=("$imgfmt/libqicns.dylib" "$imgfmt/libqjpeg.dylib" "$imgfmt/libqsvg.dylib" "$imgfmt/libqtiff.dylib" "$imgfmt/libqwebp.dylib" "$imgfmt/libqgif.dylib" "$imgfmt/libqico.dylib" "$imgfmt/libqmacjp2.dylib" "$imgfmt/libqtga.dylib" "$imgfmt/libqwbmp.dylib" "$plat/libqcocoa.dylib" "$print/libcocoaprintersupport.dylib" ) ; \
declare -a arrc=("QtCore" "QtGui" "QtWidgets" "QtPrintSupport" "QtSvg" ); \
for a in "${arr[@]}"; do \
  for b in "${arrc[@]}"; do \
    install_name_tool -change \
     /usr/local/Cellar/qt5/$qtver/lib/$b.framework/Versions/5/$b \
     @executable_path/../Frameworks/$b.framework/Versions/5/$b \
     $a; \
  done ;\
done



$deploytool ./$appname -dmg 
