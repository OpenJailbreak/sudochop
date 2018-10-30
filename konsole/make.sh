#!/bin/bash

rm -rf build
xcodebuild
codesign -f -s "iPhone Developer" --entitlements ./konsole/debug.plist build/Release-iphoneos/konsole
scp build/Release-iphoneos/konsole iphone:/usr/bin/konsole
