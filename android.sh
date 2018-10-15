./flint.py --version dev -p android libgame
cd ./modules/android/; ./gradlew assembleDebug; cd ../..
adb install -r -t .flint/build/android/debug/bin/blaze.apk && adb shell am start nl.mtgames.blaze/.Bootstrap
