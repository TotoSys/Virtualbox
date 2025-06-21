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
#define GEFORCE3TI500_REG_PGRAPH            0x400000
#define GEFORCE3TI500_REG_PCRTC             0x600000
#define GEFORCE3TI500_REG_PRAMDAC           0x680000
#define GEFORCE3TI500_REG_PRMDIO            0x6C0000
/** @} */

/** @name Interrupt Status Flags
 * @{ */
#define GEFORCE3TI500_IRQ_DISPLAY           RT_BIT(24)
#define GEFORCE3TI500_IRQ_FIFO              RT_BIT(8)
#define GEFORCE3TI500_IRQ_GRAPH             RT_BIT(12)
/** @} */

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

/** @} */

#endif /* !VBOX_INCLUDED_SRC_Graphics_DevGeForce3Ti500_h */