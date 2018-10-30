#!/bin/bash


echo "Sudo Chop!!!"

# Clean up old staging files from subversi0n and rebuild
rm -rf ddi.dmg
rm -rf dump.dylib
rm -rf patch.dylib
rm -rf crack.dylib

# Build dump.dylib
cd dump
rm -rf build
xcodebuild
cp build/Release-iphoneos/dump ../dump.dylib
cd ..

# Build patch.dylib
cd patch
rm -rf build
xcodebuild
cp build/Release-iphoneos/patch ../patch.dylib
cd ..

# Build crack.dylib
cd crack
rm -rf build
xcodebuild
cp build/Release-iphoneos/crack ../crack.dylib
cd ..

# Codesign everything
codesign -f -s "iPhone Developer" dump.dylib
codesign -f -s "iPhone Developer" patch.dylib
codesign -f -s "iPhone Developer" crack.dylib

# Mount our new image
sudo hdiutil detach /Volumes/DDI
sudo hdiutil mount udrw.dmg

# Remove old files
sudo rm -rf /Volumes/DDI/dump.dylib
sudo rm -rf /Volumes/DDI/patch.dylib
sudo rm -rf /Volumes/DDI/crack.dylib
sudo rm -rf /Volumes/DDI/Library/LaunchDaemons/com.sudo-sec.dump.plist
sudo rm -rf /Volumes/DDI/Library/LaunchDaemons/com.sudo-sec.patch.plist
sudo rm -rf /Volumes/DDI/Library/LaunchDaemons/com.sudo-sec.crack.plist
sudo rm -rf /Volumes/DDI/Library/LaunchDaemons/com.sudo-sec.panic.plist

# Copy over new dylibs
sudo cp ./dump.dylib /Volumes/DDI/dump.dylib
sudo cp ./patch.dylib /Volumes/DDI/patch.dylib
sudo cp ./crack.dylib /Volumes/DDI/crack.dylib

# Copy over LaunchServices
sudo cp ./dump/dump/com.sudo-sec.dump.plist /Volumes/DDI/Library/LaunchDaemons/com.sudo-sec.dump.plist
sudo cp ./patch/patch/com.sudo-sec.patch.plist /Volumes/DDI/Library/LaunchDaemons/com.sudo-sec.patch.plist
sudo cp ./crack/crack/com.sudo-sec.crack.plist /Volumes/DDI/Library/LaunchDaemons/com.sudo-sec.crack.plist

# Unmount and convert to UDRO
sudo hdiutil detach /Volumes/DDI
sudo hdiutil convert -format UDRO -o ddi.dmg udrw.dmg

echo "Done, copy ddi.dmg over to SudoPunch"
