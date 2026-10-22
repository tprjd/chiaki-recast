#!/bin/bash

set -xe
cd "`dirname $(readlink -f ${0})`"

podman build -t chiaki-appimage . -f Dockerfile.qt6
cd ..
podman run --rm \
	--volume "`pwd`:/build/chiaki:Z" \
	-w "/build/chiaki" \
	--device /dev/fuse \
	--cap-add SYS_ADMIN \
	-t chiaki-appimage \
	/bin/bash -c 'sudo -E env PATH="$PATH" scripts/build-appimage.sh /build/appdir'
