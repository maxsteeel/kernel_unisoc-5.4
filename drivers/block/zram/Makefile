# SPDX-License-Identifier: GPL-2.0-only
zram-y				:=	zcomp.o zram_drv.o
zram-$(CONFIG_ZRAM_DEDUP)	+=	zram_dedup.o

obj-y	+=	zram.o

obj-$(CONFIG_ZRAM)      += hybridswap/

ccflags-y += -O3
