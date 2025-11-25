#!/bin/bash

# make sure to change :
# 1. ROOTFS_DIR
# 2. IMG_DIR
# 3. IMG_SUFFIX
# 4. (optional), see the build directory in general (e.g., PREFIX)
# 5. if built on debug mode, then also check where the compiled version is located. 
#     (now we assume in release mode)

export IMG_DIR="/home/ardhi/kbuild"
export IMG_SUFFIX="agsys-s-cdh"

# compile kata-agent
pushd ~/kata-containers
make -C src/agent/ SECCOMP=no AGENT_POLICY=yes
#INIT=yes BUILD_TYPE=debug
export ROOTFS_DIR="$(realpath ~/kata-containers/tools/osbuilder/rootfs-builder/rootfs-ag)"

# copy IMA
sudo mkdir -p $ROOTFS_DIR/etc/ima/
sudo install -o root -g root -m 0755 ~/kata-containers/ima-policy $ROOTFS_DIR/etc/ima/ima-policy

# if agent=yes, to /sbin/init
# sudo install -o root -g root -m 0755 src/agent/target/x86_64-unknown-linux-musl/release/kata-agent $ROOTFS_DIR/sbin/init
# else, to /usr/bin/kata-agent
sudo install -o root -g root -m 0755 src/agent/target/x86_64-unknown-linux-musl/release/kata-agent $ROOTFS_DIR/usr/bin/kata-agent
# this is debug release - agent=no
# sudo install -o root -g root -m 0755 src/agent/target/x86_64-unknown-linux-musl/debug/kata-agent $ROOTFS_DIR/usr/bin/kata-agent

# attestation agent
pushd ~/kata-containers/guest-components/attestation-agent/
make ATTESTER=snp-attester LIBC=musl ttrpc=true
sudo install -o root -g root -m 0755 ../target/x86_64-unknown-linux-musl/release/attestation-agent $ROOTFS_DIR/usr/local/bin
popd

pushd ~/kata-containers/guest-components/attestation-agent/attestation-agent/
cargo build --release --no-default-features --features "kbs bin,ttrpc snp-attester,rust-crypto" --bin ttrpc-aa-client --target x86_64-unknown-linux-musl 
sudo install -o root -g root -m 0755 ../../target/x86_64-unknown-linux-musl/release/ttrpc-aa-client $ROOTFS_DIR/usr/local/bin
popd

# CDH
pushd ~/kata-containers/guest-components/confidential-data-hub/
make RESOURCE_PROVIDER=kbs RCP=ttrpc LIBC=musl
sudo install -o root -g root -m 0755 ../target/x86_64-unknown-linux-musl/release/confidential-data-hub $ROOTFS_DIR/usr/local/bin
popd

# compiling runtime
pushd ~/kata-containers/src/runtime
PREFIX=~/kbuild/ make
cp kata-runtime /home/ardhi/kbuild/bin
cp containerd-shim-kata-v2 /home/ardhi/kbuild/bin
popd

# testmain for DICE
# pushd ~/kata-containers/src/agent
# RUSTFLAGS=" --deny warnings" cargo build --target x86_64-unknown-linux-musl --release --features "default-pull" --bin testmain
# sudo install -o root -g root -m 0755 target/x86_64-unknown-linux-musl/release/testmain $ROOTFS_DIR/usr/local/bin/testmain
# popd

# building initrd and image
pushd ~/kata-containers/tools/osbuilder/initrd-builder
#AGENT_INIT=yes 
./initrd_builder.sh -o ${IMG_DIR}/kata-containers-initrd-${IMG_SUFFIX}.img "${ROOTFS_DIR}"
popd

pushd ~/kata-containers/tools/osbuilder/image-builder
#AGENT_INIT=yes 
USE_DOCKER=yes ./image_builder.sh -o ${IMG_DIR}/kata-containers-image-${IMG_SUFFIX}.img -f ext4 "${ROOTFS_DIR}"
popd

popd

