#!/usr/bin/env sh

VERSION=0.1.0
LINES_INSTALL_PREFIX=${LINES_INSTALL_PREFIX:-/usr/local}

abort() {
    echo "$1" >&2
    exit 1
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || abort "$1 is required, but not found"
}

download() {
    url="$1"
    file="$2"

    require_command curl

    curl -fsSL "$url" -o "$file" || abort "Download failed"
}

check_sudo() {
    if ! [ -w "$LINES_INSTALL_PREFIX" ]; then
       SUDO="sudo"
       require_command sudo
    else
       SUDO=""
    fi
}

check_system() {
    case "$OS" in
        Linux|macOS)
            ;;
        *)
            return 1
            ;;
    esac
    case "$ARCH" in
        x86_64)
            if [ "$OS" = "macOS" ]; then
               return 1
            fi
            ;;
        aarch64)
            ;;
        *)
            return 1
            ;;
    esac
}

detect_system_info() {
    echo "Detecting system info..."
    OS=$(uname -s)
    ARCH=$(uname -m)
    if [ "$OS" = "Darwin" ]; then
       OS=macOS
    fi
    if [ "$ARCH" = "arm64" ]; then
       ARCH=aarch64
    fi
    check_system || abort "System $OS $ARCH is unsupported"
    echo "System identifier is $OS $ARCH"
}

make_url() {
    if [ "$OS" = "macOS" ]; then
        ARCHIVE="lines-cli-$VERSION-$OS"
    else
        ARCHIVE="lines-cli-$VERSION-$OS-$ARCH"
    fi
    URL="https://github.com/ihateyouall-dev/lines-cli/releases/download/v$VERSION/$ARCHIVE.tar.gz"
}

prepare_tmp() {
    TMP_DIR=$(mktemp -d) || abort "Cannot create temporary directory for download"
    trap 'rm -rf "$TMP_DIR"' EXIT
}

untar() {
    tar -xzf "$1" -C "$2" || abort "Cannot unpack archive $1"
}

main() {
    echo "Installing Lines CLI ${VERSION}..."
    detect_system_info
    make_url
    prepare_tmp

    echo "Downloading archive..."
    download "$URL" "$TMP_DIR/lines-cli-install.tar.gz"
    echo "Extracting archive..."
    untar "$TMP_DIR/lines-cli-install.tar.gz" $TMP_DIR

    ls -A "$TMP_DIR/$ARCHIVE" >/dev/null 2>&1 || abort "Archive is empty or missing, cannot continue installation"

    check_sudo
    echo "Installing..."
    $SUDO mv "$TMP_DIR/$ARCHIVE/"* "$LINES_INSTALL_PREFIX"

    echo "Installation successful"
    return 0
}

main "$@"
