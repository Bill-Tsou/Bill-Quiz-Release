#!/bin/sh
# Usage: ./compile.sh compile   - build QuizRelease
#        ./compile.sh clean     - remove intermediate object files

CXX="${CXX:-c++}"
BIN="QuizRelease"
CXXFLAGS="-g3 -Wall"
OBJS="QuizRelease.o Menu.o CreativeCommons.o QuizCommon.o"

usage() {
	echo "Usage: $0 compile|clean"
	echo "  compile  - build $BIN"
	echo "  clean    - remove intermediate object files"
	exit 1
}

build_error() {
	echo
	echo "[ERROR] Build failed."
	exit 1
}

do_compile() {
	if ! command -v "$CXX" >/dev/null 2>&1; then
		echo "[ERROR] C++ compiler $CXX was not found."
		echo "  macOS : xcode-select --install"
		echo "  Ubuntu: sudo apt install build-essential"
		exit 1
	fi

	if ! pkg-config --exists gtk+-3.0 2>/dev/null; then
		echo "[ERROR] GTK3 was not found by pkg-config."
		echo "  macOS : brew install gtk+3 pkg-config"
		echo "  Ubuntu: sudo apt install pkg-config libgtk-3-dev"
		exit 1
	fi
	GTK_CFLAGS=$(pkg-config --cflags gtk+-3.0)
	GTK_LIBS=$(pkg-config --libs gtk+-3.0)
	if [ "$(uname -s)" = "Darwin" ]; then
		# CoreGraphics is used to wait until the About window is removed from the screen,
		# AppKit (through the Objective-C runtime) brings the About window to the front
		GTK_LIBS="$GTK_LIBS -framework CoreGraphics -framework CoreFoundation -framework AppKit"
	fi

	echo "Compiling QuizRelease.cpp ..."
	$CXX -c QuizRelease.cpp -o QuizRelease.o $CXXFLAGS || build_error

	echo "Compiling Menu.cpp ..."
	$CXX -c Menu.cpp -o Menu.o $CXXFLAGS || build_error

	echo "Compiling CreativeCommons.cpp ..."
	$CXX -c CreativeCommons.cpp -o CreativeCommons.o $CXXFLAGS $GTK_CFLAGS || build_error

	echo "Compiling QuizCommon.cpp ..."
	$CXX -c QuizCommon.cpp -o QuizCommon.o $CXXFLAGS || build_error

	echo "Linking $BIN ..."
	$CXX $OBJS -o $BIN $GTK_LIBS || build_error

	echo
	echo "Build succeeded: $BIN"
}

do_clean() {
	echo "Cleaning intermediate build files ..."
	rm -f $OBJS
	echo "Done."
}

case "$1" in
	compile) do_compile ;;
	clean) do_clean ;;
	*) usage ;;
esac
