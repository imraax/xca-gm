# XCA-GM: X Certificate and Key management with Chinese GM/T (SM2/SM3/SM4)
# support, built against Tongsuo (https://github.com/Tongsuo-Project/Tongsuo)
# https://github.com/imraax/xca-gm
#
#   docker build -t xca-gm .
#   docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix xca-gm
#
# Dockerfile.openssl builds against the distribution's plain OpenSSL instead.

ARG REGISTRY_PREFIX=''
ARG CODENAME=noble

FROM ${REGISTRY_PREFIX}ubuntu:${CODENAME} as builder

ENV DEBIAN_FRONTEND noninteractive

RUN set -x \
	&& apt update \
	&& apt upgrade -y \
	&& apt install --yes --no-install-recommends \
		build-essential pkg-config ninja-build perl \
		python3-sphinxcontrib.qthelp git cmake locales ca-certificates \
		qt6-base-dev qt6-tools-dev

ARG TONGSUO_VERSION=8.4.0
ARG TONGSUO_PREFIX=/opt/tongsuo

# Build Tongsuo, the OpenSSL fork with the Chinese GM/T algorithms
RUN set -x \
	&& git clone --depth 1 -b ${TONGSUO_VERSION} \
		https://github.com/Tongsuo-Project/Tongsuo /tmp/tongsuo \
	&& cd /tmp/tongsuo \
	&& ./config --prefix=${TONGSUO_PREFIX} --libdir=lib \
		enable-ntls no-tests \
	&& make -j$(nproc) \
	&& make install_sw \
	&& rm -rf /tmp/tongsuo

ARG BUILD_DIR=/tmp/build

COPY . ${BUILD_DIR}
RUN set -x \
	&& cd ${BUILD_DIR} \
	&& cmake -B BUILD -G Ninja -DXCA_GM=ON \
		-DOPENSSL_ROOT_DIR=${TONGSUO_PREFIX} \
		-DCMAKE_PREFIX_PATH=${TONGSUO_PREFIX} \
		-DCMAKE_INSTALL_RPATH=${TONGSUO_PREFIX}/lib \
	&& cmake --build BUILD \
	&& cmake --install BUILD \
	&& cd \
	&& mv ${BUILD_DIR}/misc/docker_start.sh / \
	&& rm -rf ${BUILD_DIR}

RUN mkdir -p /home/user && chmod 0777 /home/user

ENTRYPOINT ["/docker_start.sh"]
