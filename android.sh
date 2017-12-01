#!/bin/bash
# Sets up de native part for android build, should be done by premake in the future

premake5 --platform=android

FILES=build/android/jni/*
# @todo This doesn't work, we need to only put blaze?? (should be launch)
# for f in $FILES; do
#   sed -i 's/LOCAL_STATIC_LIBRARIES/LOCAL_WHOLE_STATIC_LIBRARIES/' $f
# done

echo "include \$(call my-dir)/blaze_Application.mk" > build/android/jni/Application.mk
