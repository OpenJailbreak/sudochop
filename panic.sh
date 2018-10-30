#!/bin/bash

echo "Sudo Chop!!!"

# Clean up old staging files from subversi0n and rebuild
rm -rf ddi.dmg

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

sudo cp ./panic/com.sudo-sec.panic.plist /Volumes/DDI/Library/LaunchDaemons/com.sudo-sec.panic.plist

# Unmount and convert to UDRO
sudo hdiutil detach /Volumes/DDI
sudo hdiutil convert -format UDRO -o ddi.dmg udrw.dmg

echo "Done, copy ddi.dmg over to SudoPunch"
