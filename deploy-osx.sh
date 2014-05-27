cp /usr/lib/libcurl.4.dylib VirusTotalUploader.app/Contents/Frameworks/
cp /usr/lib/libz.1.dylib VirusTotalUploader.app/Contents/Frameworks/
install_name_tool -change /usr/lib/libz.1.dylib @executable_path/../Frameworks/libz.1.dylib VirusTotalUploader.app/Contents/MacOS/VirusTotalUploader
install_name_tool -change /usr/lib/libcurl.4.dylib @executable_path/../Frameworks/libcurl.4.dylib VirusTotalUploader.app/Contents/MacOS/VirusTotalUploader
macdeployqt VirusTotalUploader.app -dmg
