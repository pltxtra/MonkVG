#!/bin/bash

PROJECT_NAME="FontLoader"
PACKAGE_NAME=com.holidaystudios.fontloader
LIBFREETYPE=../../../precompiled/lib/libfreetype.a

# check if we have a Android project that is usable, if not, fix it automatically

if [ ! -f ".ANDROID_PROJECT_CREATED" ]; then
    if [ "$1" != "--update-android-project" ]; then
	echo "To build using this script you must have a properly updated Android project."
	echo " - If you have done that manually, please create the file .ANDROID_PROJECT_CREATED"
	echo " - If you have NOT done that and want this script to do it for you, run this script again like this:"
	echo "./BuildAPK.sh --update-android-project <android build target>"
	echo ""

	echo "To list available targets run:"
	echo "android list targets"
	
	exit 1;
    fi

    if [ "$2" = "" ]; then
	echo "You must supply an Android build target."
	echo "Available targets are:"
	android list targets
	exit 1;
    fi
    
    android update project --name "$PROJECT_NAME" --target $2 --path ./

    if [ $? -ne 0 ]; then
	echo "Android project NOT properly created!"
	exit -1
    fi
    touch .ANDROID_PROJECT_CREATED

    echo "Project created, re-run script to build."
    
    exit 0;
fi

# check if FreeType library exists
if [ -e "$LIBFREETYPE" ]; then
    echo ""
else
    echo "the FreeType library is required. It could not be found at $LIBFREETYPE. Please install it."
    exit 1
fi

# check if user wants to just uninstall package from device
if [ "$1" = "-ud" ]; then
    adb -d uninstall $PACKAGE_NAME
    if [ $? -ne 0 ]; then
	echo "Failed to uninstall - perhaps already uninstalled?"
	exit 1
    fi
    
    echo "Package uninstalled."
    exit 0
fi

# check if user wants to debug
if [ "$1" = "-gdb" ]; then
    $NDK_PATH/ndk-gdb
    exit 0
fi


# build native libraries, then run ant to build java stuff 'n create the .apk

RELEASE_OR_DEBUG=debug
if [ "$1" = "-release" ]; then
    RELEASE_OR_DEBUG=release
fi

$NDK_PATH/ndk-build -j9 && ant $RELEASE_OR_DEBUG

if [ $? -ne 0 ]; then
    echo "Build failed."
    exit -1
fi

if [ "$1" = "-release" ]; then
    echo "Release build finished."
    exit 0
fi

#if -ie (Install to Emulator == -ie)
if [ "$1" = "-ie" ]; then
    if [ "$2" = "-c" ]; then
	adb -e uninstall $PACKAGE_NAME
    fi
    adb -e install -r bin/FontLoader-debug.apk
fi

#if -id (Install to Device == -id)
if [ "$1" = "-id" ]; then
    if [ "$2" = "-c" ]; then
	adb -d uninstall $PACKAGE_NAME
    fi
    adb -d install -r bin/FontLoader-debug.apk
fi

echo "Build finished at:" `date`
