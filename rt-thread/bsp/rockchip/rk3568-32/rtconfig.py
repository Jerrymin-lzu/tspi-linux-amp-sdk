import os
import sys

from building import *

# toolchains options
ARCH        ='arm'
CPU         ='cortex-a'
CROSS_TOOL  ='gcc'

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')
else:
    RTT_ROOT = r'../../..'

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

PLATFORM    = 'gcc'
EXEC_PATH   = RTT_ROOT + '/../prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux/bin/'

BUILD = 'release'

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

if PLATFORM == 'gcc':
    # toolchains
    # PREFIX = 'arm-none-eabi-'
    PREFIX = 'arm-none-eabi-'
    CC      = PREFIX + 'gcc'
    CXX     = PREFIX + 'g++'
    AS      = PREFIX + 'gcc'
    AR      = PREFIX + 'ar'
    LINK    = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE    = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY  = PREFIX + 'objcopy'

    DEVICE = ' -mcpu=cortex-a55+crypto -mfloat-abi=hard -marm -ffast-math'
    CFLAGS = DEVICE + ' -std=gnu99 -Wall -g -Wno-stringop-truncation'
    AFLAGS = DEVICE + ' -c -x assembler-with-cpp -D__ASSEMBLY__'
    LINK_SCRIPT = 'gcc_arm.ld.S'
    LFLAGS  = DEVICE + ' -lm -lgcc -lc' + ' -nostartfiles -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,system_vectors -T gcc_arm.ld'
    CPATH   = ''
    LPATH   = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2'
        AFLAGS += ' -gdwarf-2'
    else:
        CFLAGS += ' -O2'

    if os.getenv('RTT_PRMEM_BASE'):
        RTT_PRMEM_BASE = os.getenv('RTT_PRMEM_BASE')
    else:
        RTT_PRMEM_BASE = 0x01800000

    if os.getenv('RTT_PRMEM_SIZE'):
        RTT_PRMEM_SIZE = os.getenv('RTT_PRMEM_SIZE')
    else:
        RTT_PRMEM_SIZE = 0x00800000

    if os.getenv('RTT_SHMEM_BASE'):
        RTT_SHMEM_BASE = os.getenv('RTT_SHMEM_BASE')
    else:
        RTT_SHMEM_BASE = 0x07800000

    if os.getenv('RTT_SHMEM_SIZE'):
        RTT_SHMEM_SIZE = os.getenv('RTT_SHMEM_SIZE')
    else:
        RTT_SHMEM_SIZE = 0x00400000

    if os.getenv('LINUX_RPMSG_BASE'):
        LINUX_RPMSG_BASE = os.getenv('LINUX_RPMSG_BASE')
    else:
        LINUX_RPMSG_BASE = 0x07c00000

    if os.getenv('LINUX_RPMSG_SIZE'):
        LINUX_RPMSG_SIZE = os.getenv('LINUX_RPMSG_SIZE')
    else:
        LINUX_RPMSG_SIZE = 0x00400000

    CFLAGS += ' -DFIRMWARE_BASE={a} -DDRAM_SIZE={b} -DSHMEM_BASE={c} -DSHMEM_SIZE={d} -DLINUX_RPMSG_BASE={e} -DLINUX_RPMSG_SIZE={f}'.format(a=RTT_PRMEM_BASE, b=RTT_PRMEM_SIZE, c=RTT_SHMEM_BASE, d=RTT_SHMEM_SIZE, e=LINUX_RPMSG_BASE, f=LINUX_RPMSG_SIZE)

    CXXFLAGS = CFLAGS

DUMP_ACTION = OBJDUMP + ' -D -S $TARGET > rtt.asm\n'
POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'

