premake5 --platform=android

FILES=build/android/jni/*
for f in $FILES; do
  sed -i 's/LOCAL_STATIC_LIBRARIES/LOCAL_WHOLE_STATIC_LIBRARIES/' $f
done

echo "include \$(call my-dir)/blaze_Application.mk" > build/android/jni/Application.mk

export NDK_PROJECT_PATH=/home/tim/Projects/cpp/blaze/build/android
export ANDROID_HOME=/home/tim/Projects/Android/Sdk
~/android-ndk/ndk-build PM5_CONFIG=debug -j6
