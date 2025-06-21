/* $Id$ */
/** @file
 * DevGeForce3Ti500 - NVIDIA GeForce 3 Ti 500 device emulation, internal header.
 */

/*
 * Copyright (C) 2024 Oracle and/or its affiliates.
 *
 * This file is part of VirtualBox base platform packages, as
 * available from https://www.virtualbox.org.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, in version 3 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 * --------------------------------------------------------------------
 *
 * This code is based on:
 *
 * Bochs-DX GeForce implementation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

#ifndef VBOX_INCLUDED_SRC_Graphics_DevGeForce3Ti500_h
#define VBOX_INCLUDED_SRC_Graphics_DevGeForce3Ti500_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <VBox/vmm/pdmdev.h>
#include <VBox/vmm/pdmifs.h>
#include <VBox/param.h>
#include <iprt/assert.h>

/** @defgroup grp_geforce3ti500  GeForce 3 Ti 500 Device
 * @{
 */

/** NVIDIA Vendor ID */
#define GEFORCE3TI500_PCI_VENDOR_ID         0x10de

/** GeForce 3 Ti 500 Device ID */
#define GEFORCE3TI500_PCI_DEVICE_ID         0x0201

/** Default VRAM size for GeForce 3 Ti 500 (64MB) */
#define GEFORCE3TI500_VRAM_DEFAULT          (64 * _1M)

/** Minimum VRAM size (16MB) */
#define GEFORCE3TI500_VRAM_MIN              (16 * _1M)

/** Maximum VRAM size (128MB) */
#define GEFORCE3TI500_VRAM_MAX              (128 * _1M)

/** Register space size */
#define GEFORCE3TI500_REG_SIZE              (16 * _1M)

/** Framebuffer aperture size */
#define GEFORCE3TI500_FB_SIZE               GEFORCE3TI500_VRAM_MAX

/** @name PCI Base Address Registers (BARs)
 * @{ */
#define GEFORCE3TI500_PCI_BAR_FB            0  /**< Framebuffer BAR */
#define GEFORCE3TI500_PCI_BAR_REG           1  /**< Register BAR */
/** @} */

/** @name GeForce 3 Ti 500 Register Offsets
 * @{ */
#define GEFORCE3TI500_REG_BOOT_0            0x000000
#define GEFORCE3TI500_REG_PMC               0x000000
#define GEFORCE3TI500_REG_PBUS              0x001000
#define GEFORCE3TI500_REG_PFIFO             0x002000
#define GEFORCE3TI500_REG_PRAMIN            0x004000
#define GEFORCE3TI500_REG_PTIMER            0x009000
#define GEFORCE3TI500_REG_PGRAPH            0x400000
#define GEFORCE3TI500_REG_PCRTC             0x600000
#define GEFORCE3TI500_REG_PRAMDAC           0x680000
#define GEFORCE3TI500_REG_PRMDIO            0x6C0000

/* PMC registers */
#define GEFORCE3TI500_PMC_BOOT_0            0x000000
#define GEFORCE3TI500_PMC_INTR_0            0x000100
#define GEFORCE3TI500_PMC_INTR_EN_0         0x000140
#define GEFORCE3TI500_PMC_ENABLE            0x000200

/* PBUS registers */
#define GEFORCE3TI500_PBUS_INTR_0           0x001100
#define GEFORCE3TI500_PBUS_INTR_EN_0        0x001140

/* PFIFO registers */
#define GEFORCE3TI500_PFIFO_INTR_0          0x002100
#define GEFORCE3TI500_PFIFO_INTR_EN_0       0x002140
#define GEFORCE3TI500_PFIFO_RAMHT           0x002210
#define GEFORCE3TI500_PFIFO_RAMFC           0x002214
#define GEFORCE3TI500_PFIFO_RAMRO           0x002218
#define GEFORCE3TI500_PFIFO_MODE            0x002504
#define GEFORCE3TI500_PFIFO_CACHE1_PUSH1    0x003204
#define GEFORCE3TI500_PFIFO_CACHE1_PUT      0x003210
#define GEFORCE3TI500_PFIFO_CACHE1_DMA_PUSH 0x003220
#define GEFORCE3TI500_PFIFO_CACHE1_DMA_INSTANCE 0x003224
#define GEFORCE3TI500_PFIFO_CACHE1_DMA_PUT  0x003240
#define GEFORCE3TI500_PFIFO_CACHE1_DMA_GET  0x003244
#define GEFORCE3TI500_PFIFO_CACHE1_REF_CNT  0x003248
#define GEFORCE3TI500_PFIFO_CACHE1_PULL0    0x003250
#define GEFORCE3TI500_PFIFO_CACHE1_SEMAPHORE 0x003260
#define GEFORCE3TI500_PFIFO_CACHE1_GET      0x003270
#define GEFORCE3TI500_PFIFO_GRCTX_INSTANCE  0x003280

/* PTIMER registers */
#define GEFORCE3TI500_PTIMER_INTR_0         0x009100
#define GEFORCE3TI500_PTIMER_INTR_EN_0      0x009140
#define GEFORCE3TI500_PTIMER_NUMERATOR      0x009200
#define GEFORCE3TI500_PTIMER_DENOMINATOR    0x009210
#define GEFORCE3TI500_PTIMER_TIME_0         0x009400
#define GEFORCE3TI500_PTIMER_TIME_1         0x009410
#define GEFORCE3TI500_PTIMER_ALARM_0        0x009420

/* Graphics engine registers */
#define GEFORCE3TI500_REG_GRAPH_STATUS      0x400700
#define GEFORCE3TI500_REG_GRAPH_TRAPPED_ADDR 0x400704
#define GEFORCE3TI500_REG_GRAPH_TRAPPED_DATA 0x400708
#define GEFORCE3TI500_REG_GRAPH_SURFACE     0x400710
#define GEFORCE3TI500_PGRAPH_INTR           0x400100
#define GEFORCE3TI500_PGRAPH_NSOURCE        0x400108
#define GEFORCE3TI500_PGRAPH_INTR_EN        0x400140
#define GEFORCE3TI500_PGRAPH_CTX_SWITCH1    0x400160
#define GEFORCE3TI500_PGRAPH_CTX_SWITCH2    0x400164
#define GEFORCE3TI500_PGRAPH_CTX_SWITCH4    0x40016C
#define GEFORCE3TI500_PGRAPH_CTXCTL_CUR     0x400170
#define GEFORCE3TI500_PGRAPH_STATUS         0x400700
#define GEFORCE3TI500_PGRAPH_NOTIFY         0x400714
#define GEFORCE3TI500_PGRAPH_FIFO           0x400720
#define GEFORCE3TI500_PGRAPH_CHANNEL_CTX_TABLE 0x400780

/* D3D command interface registers */
#define GEFORCE3TI500_REG_D3D_COMMAND       0x400800  /* D3D command register */
#define GEFORCE3TI500_REG_D3D_PARAM         0x400804  /* D3D parameter register */

/* Display controller registers */
#define GEFORCE3TI500_REG_CRTC_START        0x600800
#define GEFORCE3TI500_REG_CRTC_CONFIG       0x600804
#define GEFORCE3TI500_REG_CRTC_PIXEL        0x600808
#define GEFORCE3TI500_REG_CRTC_H_SYNC       0x600830
#define GEFORCE3TI500_REG_CRTC_V_SYNC       0x600834
#define GEFORCE3TI500_PCRTC_INTR_0          0x600100
#define GEFORCE3TI500_PCRTC_INTR_EN_0       0x600140
#define GEFORCE3TI500_PCRTC_START           0x600800
#define GEFORCE3TI500_PCRTC_CONFIG          0x600804
#define GEFORCE3TI500_PCRTC_CURSOR_CONFIG   0x600810

/* RAMDAC registers */
#define GEFORCE3TI500_REG_DAC_PIXEL_MASK    0x680000
#define GEFORCE3TI500_REG_DAC_PALETTE_IDX   0x680008
#define GEFORCE3TI500_REG_DAC_PALETTE_DATA  0x68000C
#define GEFORCE3TI500_PRAMDAC_CU_START_POS  0x680300
#define GEFORCE3TI500_PRAMDAC_VPLL          0x680508
#define GEFORCE3TI500_PRAMDAC_VPLL_B        0x680578
#define GEFORCE3TI500_PRAMDAC_PLL_SELECT    0x68050C
#define GEFORCE3TI500_PRAMDAC_GENERAL_CONTROL 0x680600

/* Hardware acceleration classes */
#define GEFORCE3TI500_CLASS_CLIP            0x19
#define GEFORCE3TI500_CLASS_M2MF            0x39
#define GEFORCE3TI500_CLASS_ROP             0x43
#define GEFORCE3TI500_CLASS_PATT            0x44
#define GEFORCE3TI500_CLASS_GDI             0x4A
#define GEFORCE3TI500_CLASS_CHROMA          0x57
#define GEFORCE3TI500_CLASS_IMAGEBLIT       0x5F
#define GEFORCE3TI500_CLASS_IFC             0x61
#define GEFORCE3TI500_CLASS_SURF2D          0x62
#define GEFORCE3TI500_CLASS_IIFC            0x64
#define GEFORCE3TI500_CLASS_SIFC            0x65
#define GEFORCE3TI500_CLASS_BETA            0x72
#define GEFORCE3TI500_CLASS_SIFM            0x89
#define GEFORCE3TI500_CLASS_D3D             0x97

/* D3D command class and methods */
#define GEFORCE3TI500_D3D_CLASS             0x97
#define GEFORCE3TI500_D3D_METHOD_SEMAPHORE_OBJ       0x069
#define GEFORCE3TI500_D3D_METHOD_CLIP_HORIZONTAL     0x080
#define GEFORCE3TI500_D3D_METHOD_CLIP_VERTICAL       0x081
#define GEFORCE3TI500_D3D_METHOD_SURFACE_FORMAT      0x082
#define GEFORCE3TI500_D3D_METHOD_SURFACE_PITCH       0x083
#define GEFORCE3TI500_D3D_METHOD_SURFACE_COLOR_OFFSET 0x084
#define GEFORCE3TI500_D3D_METHOD_SEMAPHORE_OFFSET    0x75b
#define GEFORCE3TI500_D3D_METHOD_SEMAPHORE_WRITE     0x75c
#define GEFORCE3TI500_D3D_METHOD_COLOR_CLEAR_VALUE   0x764
#define GEFORCE3TI500_D3D_METHOD_CLEAR_SURFACE       0x765

/* Constants */
#define GEFORCE3TI500_CHANNEL_COUNT         32
#define GEFORCE3TI500_SUBCHANNEL_COUNT      8
#define GEFORCE3TI500_CACHE1_SIZE           64
/** @} */

/** @name Interrupt Status Flags
 * @{ */
#define GEFORCE3TI500_IRQ_DISPLAY           RT_BIT(24)
#define GEFORCE3TI500_IRQ_FIFO              RT_BIT(8)
#define GEFORCE3TI500_IRQ_GRAPH             RT_BIT(12)
/** @} */

/** Graphics subchannel state */
typedef struct GEFORCE3TI500SUBCHANNEL
{
    uint32_t                object;
    uint8_t                 engine;
    uint32_t                notifier;
} GEFORCE3TI500SUBCHANNEL;

/** Channel state for command processing */
typedef struct GEFORCE3TI500CHANNEL
{
    /** Subroutine return address */
    uint32_t                subr_return;
    /** Subroutine active flag */
    bool                    subr_active;
    
    /** DMA state */
    struct {
        uint32_t            mthd;
        uint32_t            subc;
        uint32_t            mcnt;
        bool                ni;
    } dma_state;
    
    /** Subchannel state */
    GEFORCE3TI500SUBCHANNEL schs[GEFORCE3TI500_SUBCHANNEL_COUNT];

    /** Notification state */
    bool                    notify_pending;
    uint32_t                notify_type;

    /** Surface 2D state */
    uint32_t                s2d_img_src;
    uint32_t                s2d_img_dst;
    uint32_t                s2d_color_fmt;
    uint32_t                s2d_color_bytes;
    uint32_t                s2d_pitch;
    uint32_t                s2d_ofs_src;
    uint32_t                s2d_ofs_dst;

    /** Image From CPU state */
    bool                    ifc_color_key_enable;
    uint32_t                ifc_operation;
    uint32_t                ifc_color_fmt;
    uint32_t                ifc_color_bytes;
    uint32_t                ifc_yx;
    uint32_t                ifc_dhw;
    uint32_t                ifc_shw;
    uint32_t                ifc_words_ptr;
    uint32_t                ifc_words_left;
    uint32_t               *ifc_words;

    /** Indexed Image From CPU state */
    uint32_t                iifc_palette;
    uint32_t                iifc_palette_ofs;
    uint32_t                iifc_operation;
    uint32_t                iifc_color_fmt;
    uint32_t                iifc_color_bytes;
    uint32_t                iifc_bpp4;
    uint32_t                iifc_yx;
    uint32_t                iifc_dhw;
    uint32_t                iifc_shw;
    uint32_t                iifc_words_ptr;
    uint32_t                iifc_words_left;
    uint32_t               *iifc_words;

    /** Stretched Image From CPU state */
    uint32_t                sifc_operation;
    uint32_t                sifc_color_fmt;
    uint32_t                sifc_color_bytes;
    uint32_t                sifc_shw;
    uint32_t                sifc_dxds;
    uint32_t                sifc_dydt;
    uint32_t                sifc_syx;

    /** Scaled Image From Memory state */
    uint32_t                sifm_operation;
    uint32_t                sifm_color_fmt;
    uint32_t                sifm_color_bytes;
    uint32_t                sifm_shw;
    uint32_t                sifm_syx;

    /** D3D state */
    uint32_t                d3d_semaphore_obj;
    uint32_t                d3d_semaphore_offset;
    uint32_t                d3d_clip_horizontal;
    uint32_t                d3d_clip_vertical;
    uint32_t                d3d_surface_format;
    uint32_t                d3d_surface_pitch;
    uint32_t                d3d_surface_color_offset;
    uint32_t                d3d_color_clear_value;
    uint32_t                d3d_clear_surface;
    uint32_t                d3d_color_bytes;
} GEFORCE3TI500CHANNEL;

/**
 * GeForce 3 Ti 500 device state structure.
 */
typedef struct GEFORCE3TI500STATE
{
    /** VRAM size in bytes. */
    uint32_t                cbVRAM;
    
    /** Current display width. */
    uint32_t                cxDisplay;
    /** Current display height. */
    uint32_t                cyDisplay;
    /** Current display bits per pixel. */
    uint32_t                cBitsPerPixel;
    
    /** Framebuffer base address. */
    RTGCPHYS                GCPhysFB;
    /** Register base address. */
    RTGCPHYS                GCPhysReg;
    
    /** MMIO handle for registers. */
    IOMMMIOHANDLE           hMmioReg;
    /** MMIO2 handle for framebuffer. */
    PGMMMIO2HANDLE          hMmio2FB;
    
    /** Interrupt enable flags. */
    uint32_t                u32IrqEnable;
    /** Interrupt status flags. */
    uint32_t                u32IrqStatus;
    
    /** Register values for basic emulation. */
    uint32_t                au32Regs[0x1000];
    
    /** Device capabilities flags. */
    uint32_t                fCapabilities;

    /** Current video mode. */
    uint32_t                uCurrentMode;
    
    /** Graphics engine state. */
    uint32_t                u32GraphStatus;
    
    /** Display configuration. */
    uint32_t                u32CrtcConfig;
    
    /** Palette state. */
    uint32_t                u32PaletteIndex;
    uint8_t                 abPalette[768]; /* 256 colors * 3 components */

    /* Master Control registers */
    uint32_t                mc_intr_en;
    uint32_t                mc_enable;

    /* Bus Control registers */
    uint32_t                bus_intr;
    uint32_t                bus_intr_en;

    /* FIFO registers */
    uint32_t                fifo_intr;
    uint32_t                fifo_intr_en;
    uint32_t                fifo_ramht;
    uint32_t                fifo_ramfc;
    uint32_t                fifo_ramro;
    uint32_t                fifo_mode;
    uint32_t                fifo_cache1_push1;
    uint32_t                fifo_cache1_put;
    uint32_t                fifo_cache1_dma_push;
    uint32_t                fifo_cache1_dma_instance;
    uint32_t                fifo_cache1_dma_put;
    uint32_t                fifo_cache1_dma_get;
    uint32_t                fifo_cache1_ref_cnt;
    uint32_t                fifo_cache1_pull0;
    uint32_t                fifo_cache1_semaphore;
    uint32_t                fifo_cache1_get;
    uint32_t                fifo_grctx_instance;
    uint32_t                fifo_cache1_method[GEFORCE3TI500_CACHE1_SIZE];
    uint32_t                fifo_cache1_data[GEFORCE3TI500_CACHE1_SIZE];

    /* Memory mapping registers */
    uint32_t                rma_addr;

    /* Timer registers */
    uint32_t                timer_intr;
    uint32_t                timer_intr_en;
    uint32_t                timer_num;
    uint32_t                timer_den;
    uint64_t                timer_inittime1;
    uint64_t                timer_inittime2;
    uint32_t                timer_alarm;

    /* Hardware straps */
    uint32_t                straps0_primary;
    uint32_t                straps0_primary_original;

    /* Graphics engine registers */
    uint32_t                graph_intr;
    uint32_t                graph_nsource;
    uint32_t                graph_intr_en;
    uint32_t                graph_ctx_switch1;
    uint32_t                graph_ctx_switch2;
    uint32_t                graph_ctx_switch4;
    uint32_t                graph_ctxctl_cur;
    uint32_t                graph_status;
    uint32_t                graph_trapped_addr;
    uint32_t                graph_trapped_data;
    uint32_t                graph_notify;
    uint32_t                graph_fifo;
    uint32_t                graph_channel_ctx_table;

    /* Display controller registers */
    uint32_t                crtc_intr;
    uint32_t                crtc_intr_en;
    uint32_t                crtc_start;
    uint32_t                crtc_config;
    uint32_t                crtc_cursor_offset;
    uint32_t                crtc_cursor_config;

    /* RAMDAC registers */
    uint32_t                ramdac_cu_start_pos;
    uint32_t                ramdac_vpll;
    uint32_t                ramdac_vpll_b;
    uint32_t                ramdac_pll_select;
    uint32_t                ramdac_general_control;

    /* Channel state */
    GEFORCE3TI500CHANNEL    chs[GEFORCE3TI500_CHANNEL_COUNT];

    /* Hardware acceleration state */
    bool                    acquire_active;

    /** The PCI device. */
    PDMPCIDEV               PciDev;
    
} GEFORCE3TI500STATE;
/** Pointer to the GeForce 3 Ti 500 device state. */
typedef GEFORCE3TI500STATE *PGEFORCE3TI500STATE;

/**
 * GeForce 3 Ti 500 device state for the current context.
 */
typedef struct GEFORCE3TI500STATECC
{
    /** Pointer to the device instance. */
    PPDMDEVINS              pDevIns;
    
    /** Pointer to base of VRAM. */
    R3PTRTYPE(uint8_t *)    pbVRAM;
    
    /** LUN\#0: The display port base interface. */
    PDMIBASE                IBase;
    /** LUN\#0: The display port interface. */
    PDMIDISPLAYPORT         IPort;
    
    /** Pointer to display connector interface. */
    R3PTRTYPE(PPDMIBASE) pDrvBase;
    
} GEFORCE3TI500STATECC;
/** Pointer to the current context GeForce 3 Ti 500 state. */
typedef GEFORCE3TI500STATECC *PGEFORCE3TI500STATECC;

/** @name Capability flags
 * @{ */
#define GEFORCE3TI500_CAP_2D_ACCEL          RT_BIT(0)
#define GEFORCE3TI500_CAP_3D_ACCEL          RT_BIT(1)
#define GEFORCE3TI500_CAP_OVERLAY           RT_BIT(2)
#define GEFORCE3TI500_CAP_CURSOR            RT_BIT(3)
/** @} */

/** @name Common display modes for GeForce 3 Ti 500
 * @{ */
#define GEFORCE3TI500_MODE_640X480X16       0
#define GEFORCE3TI500_MODE_800X600X16       1
#define GEFORCE3TI500_MODE_1024X768X16      2
#define GEFORCE3TI500_MODE_1280X1024X16     3
#define GEFORCE3TI500_MODE_640X480X32       4
#define GEFORCE3TI500_MODE_800X600X32       5
#define GEFORCE3TI500_MODE_1024X768X32      6
#define GEFORCE3TI500_MODE_1280X1024X32     7
/** @} */

#ifdef IN_RING3
/* Core register access functions */
uint8_t  geforce3Ti500RegisterRead8(PGEFORCE3TI500STATE pThis, uint32_t address);
uint32_t geforce3Ti500RegisterRead32(PGEFORCE3TI500STATE pThis, uint32_t address);
void     geforce3Ti500RegisterWrite8(PGEFORCE3TI500STATE pThis, uint32_t address, uint8_t value);
void     geforce3Ti500RegisterWrite32(PGEFORCE3TI500STATE pThis, uint32_t address, uint32_t value);

/* Memory access functions */
uint8_t  geforce3Ti500VramRead8(PGEFORCE3TI500STATE pThis, uint32_t address);
uint16_t geforce3Ti500VramRead16(PGEFORCE3TI500STATE pThis, uint32_t address);
uint32_t geforce3Ti500VramRead32(PGEFORCE3TI500STATE pThis, uint32_t address);
void     geforce3Ti500VramWrite8(PGEFORCE3TI500STATE pThis, uint32_t address, uint8_t value);
void     geforce3Ti500VramWrite16(PGEFORCE3TI500STATE pThis, uint32_t address, uint16_t value);
void     geforce3Ti500VramWrite32(PGEFORCE3TI500STATE pThis, uint32_t address, uint32_t value);
void     geforce3Ti500VramWrite64(PGEFORCE3TI500STATE pThis, uint32_t address, uint64_t value);

uint8_t  geforce3Ti500RaminRead8(PGEFORCE3TI500STATE pThis, uint32_t address);
uint32_t geforce3Ti500RaminRead32(PGEFORCE3TI500STATE pThis, uint32_t address);
void     geforce3Ti500RaminWrite8(PGEFORCE3TI500STATE pThis, uint32_t address, uint8_t value);
void     geforce3Ti500RaminWrite32(PGEFORCE3TI500STATE pThis, uint32_t address, uint32_t value);

uint8_t  geforce3Ti500PhysicalRead8(PGEFORCE3TI500STATE pThis, uint32_t address);
uint16_t geforce3Ti500PhysicalRead16(PGEFORCE3TI500STATE pThis, uint32_t address);
uint32_t geforce3Ti500PhysicalRead32(PGEFORCE3TI500STATE pThis, uint32_t address);
void     geforce3Ti500PhysicalWrite8(PGEFORCE3TI500STATE pThis, uint32_t address, uint8_t value);
void     geforce3Ti500PhysicalWrite16(PGEFORCE3TI500STATE pThis, uint32_t address, uint16_t value);
void     geforce3Ti500PhysicalWrite32(PGEFORCE3TI500STATE pThis, uint32_t address, uint32_t value);
void     geforce3Ti500PhysicalWrite64(PGEFORCE3TI500STATE pThis, uint32_t address, uint64_t value);

/* DMA operations */
uint8_t  geforce3Ti500DmaRead8(PGEFORCE3TI500STATE pThis, uint32_t object, uint32_t address);
uint16_t geforce3Ti500DmaRead16(PGEFORCE3TI500STATE pThis, uint32_t object, uint32_t address);
uint32_t geforce3Ti500DmaRead32(PGEFORCE3TI500STATE pThis, uint32_t object, uint32_t address);
void     geforce3Ti500DmaWrite8(PGEFORCE3TI500STATE pThis, uint32_t object, uint32_t address, uint8_t value);
void     geforce3Ti500DmaWrite16(PGEFORCE3TI500STATE pThis, uint32_t object, uint32_t address, uint16_t value);
void     geforce3Ti500DmaWrite32(PGEFORCE3TI500STATE pThis, uint32_t object, uint32_t address, uint32_t value);
void     geforce3Ti500DmaWrite64(PGEFORCE3TI500STATE pThis, uint32_t object, uint32_t address, uint64_t value);
void     geforce3Ti500DmaCopy(PGEFORCE3TI500STATE pThis, uint32_t dst_obj, uint32_t dst_addr,
                             uint32_t src_obj, uint32_t src_addr, uint32_t byte_count);

/* RAMFC operations */
uint32_t geforce3Ti500RamfcAddress(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t offset);
void     geforce3Ti500RamfcWrite32(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t offset, uint32_t value);
uint32_t geforce3Ti500RamfcRead32(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t offset);

/* Hash table operations */
void     geforce3Ti500RamhtLookup(PGEFORCE3TI500STATE pThis, uint32_t handle, uint32_t chid, 
                                 uint32_t *object, uint8_t *engine);

/* FIFO and command processing */
void     geforce3Ti500FifoProcess(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC, uint32_t chid);
bool     geforce3Ti500ExecuteCommand(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC,
                                    uint32_t chid, uint32_t subc, uint32_t method, uint32_t param);

/* Graphics acceleration methods */
void     geforce3Ti500ExecuteClip(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteM2mf(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t subc, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteRop(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecutePatt(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteGdi(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteChroma(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteImageblit(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteIfc(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteSurf2d(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteIifc(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteSifc(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteBeta(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteSifm(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);
void     geforce3Ti500ExecuteD3d(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t method, uint32_t param);

/* 2D operations */
void     geforce3Ti500GdiFillrect(PGEFORCE3TI500STATE pThis, uint32_t chid, bool clipped);
void     geforce3Ti500GdiBlit(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t type);
void     geforce3Ti500Ifc(PGEFORCE3TI500STATE pThis, uint32_t chid);
void     geforce3Ti500Iifc(PGEFORCE3TI500STATE pThis, uint32_t chid);
void     geforce3Ti500Sifc(PGEFORCE3TI500STATE pThis, uint32_t chid);
void     geforce3Ti500Copyarea(PGEFORCE3TI500STATE pThis, uint32_t chid);
void     geforce3Ti500M2mf(PGEFORCE3TI500STATE pThis, uint32_t chid);
void     geforce3Ti500Sifm(PGEFORCE3TI500STATE pThis, uint32_t chid);

/* 3D operations */
void     geforce3Ti500D3dClearSurface(PGEFORCE3TI500STATE pThis, uint32_t chid);

/* Pixel operations */
uint32_t geforce3Ti500GetPixel(PGEFORCE3TI500STATE pThis, uint32_t obj, uint32_t ofs, uint32_t x, uint32_t cb);
void     geforce3Ti500PutPixel(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t ofs, uint32_t x, uint32_t value);
void     geforce3Ti500PixelOperation(PGEFORCE3TI500STATE pThis, uint32_t chid, uint32_t op,
                                    uint32_t *dstcolor, const uint32_t *srccolor, uint32_t cb, uint32_t px, uint32_t py);

/* Color management */
void     geforce3Ti500UpdateColorBytesIfc(PGEFORCE3TI500STATE pThis, uint32_t chid);
void     geforce3Ti500UpdateColorBytesSifc(PGEFORCE3TI500STATE pThis, uint32_t chid);
void     geforce3Ti500UpdateColorBytesIifc(PGEFORCE3TI500STATE pThis, uint32_t chid);
void     geforce3Ti500UpdateColorBytes(PGEFORCE3TI500STATE pThis, uint32_t s2d_color_fmt, uint32_t color_fmt, uint32_t *color_bytes);

/* Interrupt handling */
void     geforce3Ti500SetIrqLevel(PGEFORCE3TI500STATE pThis, bool level);
uint32_t geforce3Ti500GetMcIntr(PGEFORCE3TI500STATE pThis);
void     geforce3Ti500UpdateIrqLevel(PGEFORCE3TI500STATE pThis);

/* Timing */
uint64_t geforce3Ti500GetCurrentTime(void);

/* Legacy D3D command processing function declaration */
int geforce3Ti500R3ProcessD3DCommand(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC,
                                     uint32_t chid, uint32_t method, uint32_t param);
#endif

/** @} */

#endif /* !VBOX_INCLUDED_SRC_Graphics_DevGeForce3Ti500_h */