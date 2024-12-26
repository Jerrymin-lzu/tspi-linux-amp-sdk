RK3568_RTT_BSP="$RTT_DIR/bsp/rockchip/rk3568-32"
AMP_IMGAE_PATH="$RK_FIRMWARE_DIR/amp"
RKBIN_DIR="$SDK_DIR/rkbin"
usage_hook()
{
	echo -e "rtthread                            \tbuild kernel"
}

clean_hook()
{
    cd "$RK3568_RTT_BSP"

	scons -c
    rm -rf $(pwd)/gcc_arm.ld $AMP_IMAGE_PATH/amp.img $AMP_IMAGE_PATH/cpu3.bin
}

init_hook()
{
    echo "mk-rtthread init"
    export RTT_EXEC_PATH=$SDK_DIR/prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux/bin/

    export RTT_PRMEM_BASE=0x03000000
    export RTT_PRMEM_SIZE=0x00800000
    export RTT_SHMEM_BASE=0x07800000
    export RTT_SHMEM_SIZE=0x00400000
    export LINUX_RPMSG_BASE=0x07c00000
    export LINUX_RPMSG_SIZE=0x00500000
}

BUILD_CMDS="rtthread"
build_hook()
{
	echo "=========================================="
	echo "          Start building RT-Thread"
	echo "=========================================="

    cd "$RK3568_RTT_BSP"

    #cp "$RK3568_RTT_BSP/tspi_defconfig" "$RK3568_RTT_BSP/.config"
    #scons --menuconfig
    scons -j8

    mkdir -p "$AMP_IMGAE_PATH"

    ln -rsf "$RK3568_RTT_BSP/rtthread.bin" "$AMP_IMGAE_PATH/cpu3.bin"
    ln -rsf "$CHIPS_DIR/rk3566_rk3568/tspi_amp.its" "$AMP_IMGAE_PATH/amp.its"
    cd "$AMP_IMGAE_PATH"
    $RKBIN_DIR/tools/mkimage -f amp.its -E -p 0xe00 amp.img
    ln -rsf "$AMP_IMGAE_PATH/amp.img" "$RK_FIRMWARE_DIR/amp.img"
    
    finish_build build_rtthread
}

# build_rtt
source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

init_hook $@
build_hook $@
