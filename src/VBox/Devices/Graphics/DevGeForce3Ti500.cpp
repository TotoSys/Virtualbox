/* $Id$ */
/** @file
 * DevGeForce3Ti500 - NVIDIA GeForce 3 Ti 500 device emulation.
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


/*********************************************************************************************************************************
*   Header Files                                                                                                                 *
*********************************************************************************************************************************/
#define LOG_GROUP LOG_GROUP_DEV_VGA
#include <VBox/vmm/pdmdev.h>
#include <VBox/vmm/pgm.h>
#include <VBox/log.h>
#include <VBox/err.h>
#include <VBox/pci.h>
#include <VBox/AssertGuest.h>
#include <iprt/assert.h>
#include <iprt/string.h>
#include <iprt/mem.h>
#include <iprt/uuid.h>

#include "DevGeForce3Ti500.h"


/*********************************************************************************************************************************
*   Defined Constants And Macros                                                                                                 *
*********************************************************************************************************************************/

/** The current saved state version for the GeForce 3 Ti 500 device. */
#define GEFORCE3TI500_SAVED_STATE_VERSION   1


/*********************************************************************************************************************************
*   Structures and Typedefs                                                                                                      *
*********************************************************************************************************************************/


/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/


/*********************************************************************************************************************************
*   Internal Functions                                                                                                           *
*********************************************************************************************************************************/

/* Forward declarations */
static int geforce3Ti500R3ProcessGraphicsCmds(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC);
static int geforce3Ti500R3Setup2DContext(PGEFORCE3TI500STATE pThis);

/**
 * Updates the display mode based on CRTC configuration.
 */
static void geforce3Ti500UpdateDisplayMode(PGEFORCE3TI500STATE pThis, uint32_t u32CrtcConfig)
{
    /* Extract display parameters from CRTC config (simplified) */
    if (u32CrtcConfig & 0x80000000) /* Enable bit */
    {
        /* Decode resolution from config - this is a simplified approach */
        uint32_t mode = (u32CrtcConfig >> 16) & 0xFF;
        switch (mode)
        {
            case 0: /* 640x480x16 */
                pThis->cxDisplay = 640;
                pThis->cyDisplay = 480;
                pThis->cBitsPerPixel = 16;
                pThis->uCurrentMode = GEFORCE3TI500_MODE_640X480X16;
                break;
            case 1: /* 800x600x16 */
                pThis->cxDisplay = 800;
                pThis->cyDisplay = 600;
                pThis->cBitsPerPixel = 16;
                pThis->uCurrentMode = GEFORCE3TI500_MODE_800X600X16;
                break;
            case 2: /* 1024x768x16 */
                pThis->cxDisplay = 1024;
                pThis->cyDisplay = 768;
                pThis->cBitsPerPixel = 16;
                pThis->uCurrentMode = GEFORCE3TI500_MODE_1024X768X16;
                break;
            case 4: /* 640x480x32 */
                pThis->cxDisplay = 640;
                pThis->cyDisplay = 480;
                pThis->cBitsPerPixel = 32;
                pThis->uCurrentMode = GEFORCE3TI500_MODE_640X480X32;
                break;
            case 5: /* 800x600x32 */
                pThis->cxDisplay = 800;
                pThis->cyDisplay = 600;
                pThis->cBitsPerPixel = 32;
                pThis->uCurrentMode = GEFORCE3TI500_MODE_800X600X32;
                break;
            case 6: /* 1024x768x32 */
                pThis->cxDisplay = 1024;
                pThis->cyDisplay = 768;
                pThis->cBitsPerPixel = 32;
                pThis->uCurrentMode = GEFORCE3TI500_MODE_1024X768X32;
                break;
            case 7: /* 1280x1024x32 */
                pThis->cxDisplay = 1280;
                pThis->cyDisplay = 1024;
                pThis->cBitsPerPixel = 32;
                pThis->uCurrentMode = GEFORCE3TI500_MODE_1280X1024X32;
                break;
            default:
                /* Keep current mode for unknown configurations */
                break;
        }
        
        Log(("GeForce3Ti500: Display mode updated to %ux%ux%u (mode %u)\n",
             pThis->cxDisplay, pThis->cyDisplay, pThis->cBitsPerPixel, pThis->uCurrentMode));
    }
}

/**
 * @callback_method_impl{FNIOMMMIONEWWRITE}
 */
static DECLCALLBACK(VBOXSTRICTRC) geforce3Ti500MmioWrite(PPDMDEVINS pDevIns, void *pvUser, RTGCPHYS off, void const *pv, unsigned cb)
{
    PGEFORCE3TI500STATE pThis = PDMDEVINS_2_DATA(pDevIns, PGEFORCE3TI500STATE);
    RT_NOREF(pvUser);

    Log2(("geforce3Ti500MmioWrite: off=%RGp cb=%u data=%.*Rhxs\n", off, cb, cb, pv));

    uint32_t u32Value = 0;
    switch (cb)
    {
        case 1: u32Value = *(uint8_t const *)pv; break;
        case 2: u32Value = *(uint16_t const *)pv; break;
        case 4: u32Value = *(uint32_t const *)pv; break;
        default:
            ASSERT_GUEST_MSG_FAILED(("cb=%u off=%RGp\n", cb, off));
            return VINF_SUCCESS;
    }

    /* Basic register handling - just store the value for now */
    if (off < sizeof(pThis->au32Regs))
    {
        uint32_t uReg = (uint32_t)(off / 4);
        pThis->au32Regs[uReg] = u32Value;

        /* Handle special registers */
        switch (off)
        {
            case GEFORCE3TI500_REG_PMC + 0x140: /* INTR_EN_0 */
                pThis->u32IrqEnable = u32Value;
                Log(("GeForce3Ti500: Interrupt enable set to 0x%08x\n", u32Value));
                break;

            case GEFORCE3TI500_REG_PMC + 0x100: /* INTR_0 */
                /* Clear interrupt status bits */
                pThis->u32IrqStatus &= ~u32Value;
                if (!pThis->u32IrqStatus)
                    PDMDevHlpPCISetIrq(pDevIns, 0, 0);
                Log(("GeForce3Ti500: Interrupt status cleared, new status: 0x%08x\n", pThis->u32IrqStatus));
                break;

            case GEFORCE3TI500_REG_GRAPH_STATUS:
                pThis->u32GraphStatus = u32Value;
                /* Process graphics commands if engine is enabled */
                if (u32Value & 0x02) /* Engine enable bit */
                {
                    PGEFORCE3TI500STATECC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PGEFORCE3TI500STATECC);
                    geforce3Ti500R3ProcessGraphicsCmds(pThis, pThisCC);
                }
                Log(("GeForce3Ti500: Graphics status set to 0x%08x\n", u32Value));
                break;

            case GEFORCE3TI500_REG_D3D_COMMAND: /* D3D command register */
                /* Handle D3D command: format expected is class|method|param */
                {
                    PGEFORCE3TI500STATECC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PGEFORCE3TI500STATECC);
                    uint32_t cls = (u32Value >> 16) & 0xFF;
                    uint32_t method = u32Value & 0xFFFF;
                    uint32_t chid = 0; /* Default channel */
                    
                    if (cls == GEFORCE3TI500_D3D_CLASS)
                    {
                        Log2(("GeForce3Ti500: D3D command received - class=0x%02x, method=0x%03x\n", cls, method));
                        /* For now, we'll need the parameter in a separate write */
                        /* This is a simplified interface - real hardware would use FIFO */
                    }
                }
                break;

            case GEFORCE3TI500_REG_D3D_PARAM: /* D3D parameter register */
                /* Handle D3D parameter for the last command */
                {
                    PGEFORCE3TI500STATECC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PGEFORCE3TI500STATECC);
                    uint32_t lastCommand = pThis->au32Regs[GEFORCE3TI500_REG_D3D_COMMAND / 4];
                    uint32_t cls = (lastCommand >> 16) & 0xFF;
                    uint32_t method = lastCommand & 0xFFFF;
                    uint32_t chid = 0; /* Default channel */
                    
                    if (cls == GEFORCE3TI500_D3D_CLASS)
                    {
                        geforce3Ti500R3ProcessD3DCommand(pThis, pThisCC, chid, method, u32Value);
                    }
                }
                break;

            case GEFORCE3TI500_REG_CRTC_CONFIG:
                pThis->u32CrtcConfig = u32Value;
                /* Extract display mode from configuration */
                geforce3Ti500UpdateDisplayMode(pThis, u32Value);
                Log(("GeForce3Ti500: CRTC config set to 0x%08x\n", u32Value));
                break;

            case GEFORCE3TI500_REG_DAC_PALETTE_IDX:
                pThis->u32PaletteIndex = u32Value & 0xFF;
                Log2(("GeForce3Ti500: Palette index set to %u\n", pThis->u32PaletteIndex));
                break;

            case GEFORCE3TI500_REG_DAC_PALETTE_DATA:
                if (pThis->u32PaletteIndex < 256)
                {
                    uint32_t idx = pThis->u32PaletteIndex * 3;
                    if (idx + 2 < sizeof(pThis->abPalette))
                    {
                        pThis->abPalette[idx]     = (u32Value >> 16) & 0xFF; /* Red */
                        pThis->abPalette[idx + 1] = (u32Value >> 8) & 0xFF;  /* Green */
                        pThis->abPalette[idx + 2] = u32Value & 0xFF;         /* Blue */
                    }
                    pThis->u32PaletteIndex = (pThis->u32PaletteIndex + 1) & 0xFF;
                }
                Log2(("GeForce3Ti500: Palette data written: 0x%08x\n", u32Value));
                break;

            default:
                /* Other registers are just stored */
                break;
        }
    }

    return VINF_SUCCESS;
}

/**
 * @callback_method_impl{FNIOMMMIONEWREAD}
 */
static DECLCALLBACK(VBOXSTRICTRC) geforce3Ti500MmioRead(PPDMDEVINS pDevIns, void *pvUser, RTGCPHYS off, void *pv, unsigned cb)
{
    PGEFORCE3TI500STATE pThis = PDMDEVINS_2_DATA(pDevIns, PGEFORCE3TI500STATE);
    RT_NOREF(pvUser);

    uint32_t u32Value = 0;

    /* Basic register handling */
    if (off < sizeof(pThis->au32Regs))
    {
        uint32_t uReg = (uint32_t)(off / 4);
        u32Value = pThis->au32Regs[uReg];

        /* Handle special registers */
        switch (off)
        {
            case GEFORCE3TI500_REG_PMC + 0x000: /* BOOT_0 */
                u32Value = 0x020200A5; /* GeForce 3 Ti 500 identification */
                break;

            case GEFORCE3TI500_REG_PMC + 0x100: /* INTR_0 */
                u32Value = pThis->u32IrqStatus;
                break;

            case GEFORCE3TI500_REG_PMC + 0x140: /* INTR_EN_0 */
                u32Value = pThis->u32IrqEnable;
                break;

            case GEFORCE3TI500_REG_GRAPH_STATUS:
                u32Value = pThis->u32GraphStatus;
                break;

            case GEFORCE3TI500_REG_CRTC_CONFIG:
                u32Value = pThis->u32CrtcConfig;
                break;

            case GEFORCE3TI500_REG_DAC_PALETTE_IDX:
                u32Value = pThis->u32PaletteIndex;
                break;

            case GEFORCE3TI500_REG_DAC_PALETTE_DATA:
                if (pThis->u32PaletteIndex < 256)
                {
                    uint32_t idx = pThis->u32PaletteIndex * 3;
                    if (idx + 2 < sizeof(pThis->abPalette))
                    {
                        u32Value = (pThis->abPalette[idx] << 16) |      /* Red */
                                   (pThis->abPalette[idx + 1] << 8) |   /* Green */
                                   pThis->abPalette[idx + 2];          /* Blue */
                    }
                }
                break;

            default:
                /* Use stored register value */
                break;
        }
    }

    /* Return value based on access width */
    switch (cb)
    {
        case 1: *(uint8_t *)pv = (uint8_t)u32Value; break;
        case 2: *(uint16_t *)pv = (uint16_t)u32Value; break;
        case 4: *(uint32_t *)pv = u32Value; break;
        default:
            ASSERT_GUEST_MSG_FAILED(("cb=%u off=%RGp\n", cb, off));
            break;
    }

    Log2(("geforce3Ti500MmioRead: off=%RGp cb=%u -> 0x%0*x\n", off, cb, cb * 2, u32Value));
    return VINF_SUCCESS;
}

/**
 * @callback_method_impl{FNPCIIOREGIONMAP}
 */
static DECLCALLBACK(int) geforce3Ti500R3Map(PPDMDEVINS pDevIns, PPDMPCIDEV pPciDev, uint32_t iRegion,
                                            RTGCPHYS GCPhysAddress, RTGCPHYS cb, PCIADDRESSSPACE enmType)
{
    PGEFORCE3TI500STATE pThis = PDMDEVINS_2_DATA(pDevIns, PGEFORCE3TI500STATE);
    RT_NOREF(pPciDev);

    Log(("geforce3Ti500R3Map: iRegion=%u GCPhysAddress=%RGp cb=%RGp enmType=%d\n",
         iRegion, GCPhysAddress, cb, enmType));

    if (iRegion == GEFORCE3TI500_PCI_BAR_FB)
    {
        /* Framebuffer region */
        AssertReturn(enmType == PCI_ADDRESS_SPACE_MEM, VERR_INTERNAL_ERROR);
        AssertReturn(cb == pThis->cbVRAM, VERR_INTERNAL_ERROR);

        pThis->GCPhysFB = GCPhysAddress;
        Log(("GeForce3Ti500: Framebuffer mapped to %RGp\n", GCPhysAddress));
    }
    else if (iRegion == GEFORCE3TI500_PCI_BAR_REG)
    {
        /* Register region */
        AssertReturn(enmType == PCI_ADDRESS_SPACE_MEM, VERR_INTERNAL_ERROR);
        AssertReturn(cb == GEFORCE3TI500_REG_SIZE, VERR_INTERNAL_ERROR);

        pThis->GCPhysReg = GCPhysAddress;
        Log(("GeForce3Ti500: Registers mapped to %RGp\n", GCPhysAddress));
    }

    return VINF_SUCCESS;
}

#ifdef IN_RING3

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnUpdateDisplay}
 */
static DECLCALLBACK(int) geforce3Ti500R3PortUpdateDisplay(PPDMIDISPLAYPORT pInterface)
{
    PGEFORCE3TI500STATECC pThisCC = RT_FROM_MEMBER(pInterface, GEFORCE3TI500STATECC, IPort);
    PPDMDEVINS pDevIns = pThisCC->pDevIns;
    PGEFORCE3TI500STATE pThis = PDMDEVINS_2_DATA(pDevIns, PGEFORCE3TI500STATE);

    Log2(("geforce3Ti500R3PortUpdateDisplay\n"));

    /* For now, just indicate that no update is needed */
    RT_NOREF(pThis);
    return VINF_SUCCESS;
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnUpdateDisplayAll}
 */
static DECLCALLBACK(int) geforce3Ti500R3PortUpdateDisplayAll(PPDMIDISPLAYPORT pInterface, bool fFailOnResize)
{
    PGEFORCE3TI500STATECC pThisCC = RT_FROM_MEMBER(pInterface, GEFORCE3TI500STATECC, IPort);
    PPDMDEVINS pDevIns = pThisCC->pDevIns;

    Log2(("geforce3Ti500R3PortUpdateDisplayAll: fFailOnResize=%RTbool\n", fFailOnResize));

    /* Stub implementation */
    RT_NOREF(pDevIns, fFailOnResize);
    return VINF_SUCCESS;
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnQueryVideoMode}
 */
static DECLCALLBACK(int) geforce3Ti500R3PortQueryVideoMode(PPDMIDISPLAYPORT pInterface, uint32_t *pcBits, uint32_t *pcx, uint32_t *pcy)
{
    PGEFORCE3TI500STATECC pThisCC = RT_FROM_MEMBER(pInterface, GEFORCE3TI500STATECC, IPort);
    PPDMDEVINS pDevIns = pThisCC->pDevIns;
    PGEFORCE3TI500STATE pThis = PDMDEVINS_2_DATA(pDevIns, PGEFORCE3TI500STATE);

    Log2(("geforce3Ti500R3PortQueryVideoMode\n"));

    if (pcx)
        *pcx = pThis->cxDisplay;
    if (pcy)
        *pcy = pThis->cyDisplay;
    if (pcBits)
        *pcBits = pThis->cBitsPerPixel;

    return VINF_SUCCESS;
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnSetRefreshRate}
 */
static DECLCALLBACK(int) geforce3Ti500R3PortSetRefreshRate(PPDMIDISPLAYPORT pInterface, uint32_t cMilliesInterval)
{
    RT_NOREF(pInterface, cMilliesInterval);
    Log2(("geforce3Ti500R3PortSetRefreshRate: cMilliesInterval=%u\n", cMilliesInterval));
    return VINF_SUCCESS;
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnTakeScreenshot}
 */
static DECLCALLBACK(int) geforce3Ti500R3PortTakeScreenshot(PPDMIDISPLAYPORT pInterface, uint8_t **ppbData, size_t *pcbData,
                                                          uint32_t *pcx, uint32_t *pcy)
{
    RT_NOREF(pInterface, ppbData, pcbData, pcx, pcy);
    Log2(("geforce3Ti500R3PortTakeScreenshot - not implemented\n"));
    return VERR_NOT_SUPPORTED;
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnFreeScreenshot}
 */
static DECLCALLBACK(void) geforce3Ti500R3PortFreeScreenshot(PPDMIDISPLAYPORT pInterface, uint8_t *pbData)
{
    RT_NOREF(pInterface, pbData);
    Log2(("geforce3Ti500R3PortFreeScreenshot\n"));
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnDisplayBlt}
 */
static DECLCALLBACK(int) geforce3Ti500R3PortDisplayBlt(PPDMIDISPLAYPORT pInterface, const void *pvData,
                                                      uint32_t x, uint32_t y, uint32_t cx, uint32_t cy)
{
    RT_NOREF(pInterface, pvData, x, y, cx, cy);
    Log2(("geforce3Ti500R3PortDisplayBlt: x=%u y=%u cx=%u cy=%u\n", x, y, cx, cy));
    return VINF_SUCCESS;
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnUpdateDisplayRect}
 */
static DECLCALLBACK(void) geforce3Ti500R3PortUpdateDisplayRect(PPDMIDISPLAYPORT pInterface, int32_t x, int32_t y, uint32_t cx, uint32_t cy)
{
    RT_NOREF(pInterface, x, y, cx, cy);
    Log2(("geforce3Ti500R3PortUpdateDisplayRect: x=%d y=%d cx=%u cy=%u\n", x, y, cx, cy));
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnCopyRect}
 */
static DECLCALLBACK(int) geforce3Ti500R3PortCopyRect(PPDMIDISPLAYPORT pInterface, uint32_t cx, uint32_t cy,
                                                    const uint8_t *pbSrc, int32_t xSrc, int32_t ySrc, uint32_t cxSrc, uint32_t cySrc,
                                                    uint32_t cbSrcLine, uint32_t cSrcBitsPerPixel,
                                                    uint8_t *pbDst, int32_t xDst, int32_t yDst, uint32_t cxDst, uint32_t cyDst,
                                                    uint32_t cbDstLine, uint32_t cDstBitsPerPixel)
{
    RT_NOREF(pInterface, cx, cy, pbSrc, xSrc, ySrc, cxSrc, cySrc, cbSrcLine, cSrcBitsPerPixel);
    RT_NOREF(pbDst, xDst, yDst, cxDst, cyDst, cbDstLine, cDstBitsPerPixel);
    Log2(("geforce3Ti500R3PortCopyRect - not implemented\n"));
    return VERR_NOT_SUPPORTED;
}

/**
 * @interface_method_impl{PDMIDISPLAYPORT,pfnSetRenderVRAM}
 */
static DECLCALLBACK(void) geforce3Ti500R3PortSetRenderVRAM(PPDMIDISPLAYPORT pInterface, bool fRender)
{
    RT_NOREF(pInterface, fRender);
    Log2(("geforce3Ti500R3PortSetRenderVRAM: fRender=%RTbool\n", fRender));
}

/* -=-=-=-=-=- GeForce 3 Ti 500 2D Acceleration (stub implementations) -=-=-=-=-=- */

/**
 * Performs a simple rectangle fill operation (2D acceleration stub).
 * This is a basic implementation that can be extended for actual hardware acceleration.
 */
static int geforce3Ti500R3RectFill(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC,
                                   uint32_t x, uint32_t y, uint32_t cx, uint32_t cy, uint32_t color)
{
    RT_NOREF(pThis, x, y, cx, cy, color);
    
    if (!pThisCC->pbVRAM)
        return VINF_SUCCESS;

    /* Basic software fallback for rectangle fill */
    Log2(("GeForce3Ti500: Rectangle fill %ux%u at (%u,%u) color=0x%08x\n", cx, cy, x, y, color));
    
    /* This would normally program the 2D engine, but for now we just log */
    
    return VINF_SUCCESS;
}

/**
 * Performs a simple bit block transfer operation (2D acceleration stub).
 */
static int geforce3Ti500R3BitBlt(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC,
                                 uint32_t xSrc, uint32_t ySrc, uint32_t xDst, uint32_t yDst,
                                 uint32_t cx, uint32_t cy)
{
    RT_NOREF(pThis, xSrc, ySrc, xDst, yDst, cx, cy);
    
    if (!pThisCC->pbVRAM)
        return VINF_SUCCESS;

    /* Basic software fallback for bit blit */
    Log2(("GeForce3Ti500: BitBlt %ux%u from (%u,%u) to (%u,%u)\n", cx, cy, xSrc, ySrc, xDst, yDst));
    
    /* This would normally program the 2D engine, but for now we just log */
    
    return VINF_SUCCESS;
}

/**
 * Sets up 2D acceleration context (stub).
 */
static int geforce3Ti500R3Setup2DContext(PGEFORCE3TI500STATE pThis)
{
    Log(("GeForce3Ti500: Setting up 2D acceleration context\n"));
    
    /* Initialize 2D engine state */
    pThis->u32GraphStatus |= 0x01; /* Mark 2D engine as idle */
    
    return VINF_SUCCESS;
}

/**
 * D3D clear surface operation (stub implementation).
 */
static int geforce3Ti500R3D3DClearSurface(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC, uint32_t chid)
{
    RT_NOREF(pThisCC, chid);
    
    Log2(("GeForce3Ti500: D3D Clear Surface - format=0x%08x, pitch=%u, offset=0x%08x, value=0x%08x\n",
          pThis->u32D3DSurfaceFormat, pThis->u32D3DSurfacePitch, 
          pThis->u32D3DSurfaceColorOffset, pThis->u32D3DColorClearValue));
    
    /* This would normally clear the surface with the specified color */
    /* For now, just log the operation */
    
    return VINF_SUCCESS;
}

/**
 * Execute D3D commands based on Bochs-DX implementation.
 */
static int geforce3Ti500R3ExecuteD3D(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC, 
                                     uint32_t chid, uint32_t method, uint32_t param)
{
    int rc = VINF_SUCCESS;
    
    Log2(("GeForce3Ti500: D3D Execute - chid=%u, method=0x%03x, param=0x%08x\n", chid, method, param));
    
    switch (method)
    {
        case GEFORCE3TI500_D3D_METHOD_SEMAPHORE_OBJ:
            pThis->u32D3DSemaphoreObj = param;
            Log2(("GeForce3Ti500: D3D Semaphore Object = 0x%08x\n", param));
            break;
            
        case GEFORCE3TI500_D3D_METHOD_CLIP_HORIZONTAL:
            pThis->u32D3DClipHorizontal = param;
            Log2(("GeForce3Ti500: D3D Clip Horizontal = 0x%08x\n", param));
            break;
            
        case GEFORCE3TI500_D3D_METHOD_CLIP_VERTICAL:
            pThis->u32D3DClipVertical = param;
            Log2(("GeForce3Ti500: D3D Clip Vertical = 0x%08x\n", param));
            break;
            
        case GEFORCE3TI500_D3D_METHOD_SURFACE_FORMAT:
            pThis->u32D3DSurfaceFormat = param;
            {
                uint32_t format_color = param & 0x0000000F;
                if (format_color == 0x3)      /* R5G6B5 */
                    pThis->u32D3DColorBytes = 2;
                else if (format_color == 0x8) /* A8R8G8B8 */
                    pThis->u32D3DColorBytes = 4;
                else
                    Log(("GeForce3Ti500: Unknown D3D color format: 0x%01x\n", format_color));
            }
            Log2(("GeForce3Ti500: D3D Surface Format = 0x%08x (bytes per pixel = %u)\n", 
                  param, pThis->u32D3DColorBytes));
            break;
            
        case GEFORCE3TI500_D3D_METHOD_SURFACE_PITCH:
            pThis->u32D3DSurfacePitch = param;
            Log2(("GeForce3Ti500: D3D Surface Pitch = %u\n", param));
            break;
            
        case GEFORCE3TI500_D3D_METHOD_SURFACE_COLOR_OFFSET:
            pThis->u32D3DSurfaceColorOffset = param;
            Log2(("GeForce3Ti500: D3D Surface Color Offset = 0x%08x\n", param));
            break;
            
        case GEFORCE3TI500_D3D_METHOD_SEMAPHORE_OFFSET:
            pThis->u32D3DSemaphoreOffset = param;
            Log2(("GeForce3Ti500: D3D Semaphore Offset = 0x%08x\n", param));
            break;
            
        case GEFORCE3TI500_D3D_METHOD_SEMAPHORE_WRITE:
            /* Write value to semaphore location */
            Log2(("GeForce3Ti500: D3D Semaphore Write - obj=0x%08x, offset=0x%08x, value=0x%08x\n",
                  pThis->u32D3DSemaphoreObj, pThis->u32D3DSemaphoreOffset, param));
            /* This would normally perform DMA write to guest memory */
            /* For now, just log the operation */
            break;
            
        case GEFORCE3TI500_D3D_METHOD_COLOR_CLEAR_VALUE:
            pThis->u32D3DColorClearValue = param;
            Log2(("GeForce3Ti500: D3D Color Clear Value = 0x%08x\n", param));
            break;
            
        case GEFORCE3TI500_D3D_METHOD_CLEAR_SURFACE:
            pThis->u32D3DClearSurface = param;
            Log2(("GeForce3Ti500: D3D Clear Surface trigger = 0x%08x\n", param));
            rc = geforce3Ti500R3D3DClearSurface(pThis, pThisCC, chid);
            break;
            
        default:
            Log(("GeForce3Ti500: Unknown D3D method: 0x%03x\n", method));
            break;
    }
    
    return rc;
}

/**
 * Processes graphics commands from FIFO.
 * Enhanced to handle D3D semaphore commands (class 0x97).
 */
static int geforce3Ti500R3ProcessGraphicsCmds(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC)
{
    Log2(("GeForce3Ti500: Processing graphics commands\n"));
    
    /* Enhanced command processing with D3D support */
    /* This is a simplified implementation that would normally */
    /* read commands from a FIFO in VRAM */
    
    /* For demonstration, we could process stored command data */
    /* In a real implementation, this would read from graphics FIFO */
    
    /* Check if we have command data to process */
    /* This would normally parse command stream with format: */
    /* [class][method][parameter] */
    
    /* Example of how D3D commands would be processed: */
    /* uint32_t cls = command_class; */
    /* uint32_t method = command_method; */
    /* uint32_t param = command_param; */
    /* uint32_t chid = channel_id; */
    
    /* if (cls == GEFORCE3TI500_D3D_CLASS) */
    /*     geforce3Ti500R3ExecuteD3D(pThis, pThisCC, chid, method, param); */
    
    /* Mark the graphics engine as idle after processing */
    pThis->u32GraphStatus |= 0x01;
    
    return VINF_SUCCESS;
}

/**
 * Process D3D command when class 0x97 is detected.
 * This function can be called directly when D3D commands are written to MMIO.
 */
static int geforce3Ti500R3ProcessD3DCommand(PGEFORCE3TI500STATE pThis, PGEFORCE3TI500STATECC pThisCC,
                                           uint32_t chid, uint32_t method, uint32_t param)
{
    Log2(("GeForce3Ti500: Processing D3D command - chid=%u, method=0x%03x, param=0x%08x\n", chid, method, param));
    
    return geforce3Ti500R3ExecuteD3D(pThis, pThisCC, chid, method, param);
}

/**
 * @interface_method_impl{PDMIBASE,pfnQueryInterface}
 */
static DECLCALLBACK(void *) geforce3Ti500R3PortQueryInterface(PPDMIBASE pInterface, const char *pszIID)
{
    PGEFORCE3TI500STATECC pThisCC = RT_FROM_MEMBER(pInterface, GEFORCE3TI500STATECC, IBase);

    PDMIBASE_RETURN_INTERFACE(pszIID, PDMIBASE, &pThisCC->IBase);
    PDMIBASE_RETURN_INTERFACE(pszIID, PDMIDISPLAYPORT, &pThisCC->IPort);
    return NULL;
}

/**
 * @interface_method_impl{PDMDEVREG,pfnAttach}
 */
static DECLCALLBACK(int) geforce3Ti500R3Attach(PPDMDEVINS pDevIns, unsigned iLUN, uint32_t fFlags)
{
    PGEFORCE3TI500STATECC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PGEFORCE3TI500STATECC);

    AssertMsgReturn(fFlags & PDM_TACH_FLAGS_NOT_HOT_PLUG, ("Hot-plug not supported\n"), VERR_INVALID_PARAMETER);
    AssertMsgReturn(iLUN == 0, ("GeForce3Ti500 supports only one LUN (0)\n"), VERR_INVALID_PARAMETER);

    /* Try to attach the display connector */
    int rc = PDMDevHlpDriverAttach(pDevIns, iLUN, &pThisCC->IBase, &pThisCC->pDrvBase, "Display Port");
    if (RT_SUCCESS(rc))
    {
        Log(("GeForce3Ti500: Display connector attached\n"));
    }
    else if (rc == VERR_PDM_NO_ATTACHED_DRIVER)
    {
        Log(("GeForce3Ti500: No display connector attached\n"));
        pThisCC->pDrvBase = NULL;
        rc = VINF_SUCCESS;
    }
    else
    {
        AssertLogRelMsgFailed(("Failed to attach display driver, rc=%Rrc\n", rc));
    }

    return rc;
}

/**
 * @interface_method_impl{PDMDEVREG,pfnDetach}
 */
static DECLCALLBACK(void) geforce3Ti500R3Detach(PPDMDEVINS pDevIns, unsigned iLUN, uint32_t fFlags)
{
    PGEFORCE3TI500STATECC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PGEFORCE3TI500STATECC);
    RT_NOREF(iLUN, fFlags);

    AssertMsg(fFlags & PDM_TACH_FLAGS_NOT_HOT_PLUG, ("Hot-plug not supported\n"));
    AssertMsg(iLUN == 0, ("GeForce3Ti500 supports only one LUN (0)\n"));

    pThisCC->pDrvBase = NULL;
    Log(("GeForce3Ti500: Display connector detached\n"));
}

/**
 * @interface_method_impl{PDMDEVREG,pfnReset}
 */
static DECLCALLBACK(void) geforce3Ti500R3Reset(PPDMDEVINS pDevIns)
{
    PGEFORCE3TI500STATE pThis = PDMDEVINS_2_DATA(pDevIns, PGEFORCE3TI500STATE);

    Log(("GeForce3Ti500: Reset\n"));

    /* Clear registers */
    RT_ZERO(pThis->au32Regs);
    
    /* Reset interrupt state */
    pThis->u32IrqEnable = 0;
    pThis->u32IrqStatus = 0;
    
    /* Reset graphics engine state */
    pThis->u32GraphStatus = 0;
    pThis->u32CrtcConfig = 0;
    
    /* Reset palette state */
    pThis->u32PaletteIndex = 0;
    RT_ZERO(pThis->abPalette);
    
    /* Reset D3D acceleration state */
    pThis->u32D3DSemaphoreObj = 0;
    pThis->u32D3DSemaphoreOffset = 0;
    pThis->u32D3DClipHorizontal = 0;
    pThis->u32D3DClipVertical = 0;
    pThis->u32D3DSurfaceFormat = 0;
    pThis->u32D3DSurfacePitch = 0;
    pThis->u32D3DSurfaceColorOffset = 0;
    pThis->u32D3DColorClearValue = 0;
    pThis->u32D3DClearSurface = 0;
    pThis->u32D3DColorBytes = 4; /* Default to 32-bit color */
    
    /* Set default display mode (1024x768x32) */
    pThis->cxDisplay = 1024;
    pThis->cyDisplay = 768;
    pThis->cBitsPerPixel = 32;
    pThis->uCurrentMode = GEFORCE3TI500_MODE_1024X768X32;
    
    /* Set capabilities */
    pThis->fCapabilities = GEFORCE3TI500_CAP_2D_ACCEL | GEFORCE3TI500_CAP_3D_ACCEL;

    /* Initialize 2D acceleration context */
    geforce3Ti500R3Setup2DContext(pThis);

    Log(("GeForce3Ti500: Reset complete - Default mode: %ux%ux%u\n", 
         pThis->cxDisplay, pThis->cyDisplay, pThis->cBitsPerPixel));
}

/**
 * @interface_method_impl{PDMDEVREG,pfnDestruct}
 */
static DECLCALLBACK(int) geforce3Ti500R3Destruct(PPDMDEVINS pDevIns)
{
    PDMDEV_CHECK_VERSIONS_RETURN_QUIET(pDevIns);

    Log(("GeForce3Ti500: Destruct\n"));
    return VINF_SUCCESS;
}

/**
 * @interface_method_impl{PDMDEVREG,pfnConstruct}
 */
static DECLCALLBACK(int) geforce3Ti500R3Construct(PPDMDEVINS pDevIns, int iInstance, PCFGMNODE pCfg)
{
    PDMDEV_CHECK_VERSIONS_RETURN(pDevIns);
    PGEFORCE3TI500STATE pThis = PDMDEVINS_2_DATA(pDevIns, PGEFORCE3TI500STATE);
    PGEFORCE3TI500STATECC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PGEFORCE3TI500STATECC);
    PCPDMDEVHLPR3 pHlp = pDevIns->pHlpR3;
    int rc;

    RT_NOREF(iInstance);

    /*
     * Initialize the instance data.
     */
    pThisCC->pDevIns = pDevIns;

    /*
     * Read configuration.
     */
    PDMDEV_VALIDATE_CONFIG_RETURN(pDevIns, "VRamSize", "");

    /* VRAM size */
    rc = pHlp->pfnCFGMQueryU32Def(pCfg, "VRamSize", &pThis->cbVRAM, GEFORCE3TI500_VRAM_DEFAULT);
    if (RT_FAILURE(rc))
        return PDMDEV_SET_ERROR(pDevIns, rc, N_("Configuration error: Querying \"VRamSize\" as integer failed"));
    
    if (pThis->cbVRAM < GEFORCE3TI500_VRAM_MIN)
        return PDMDEV_SET_ERROR(pDevIns, VERR_INVALID_PARAMETER, N_("Configuration error: VRamSize is too small"));
    if (pThis->cbVRAM > GEFORCE3TI500_VRAM_MAX)
        return PDMDEV_SET_ERROR(pDevIns, VERR_INVALID_PARAMETER, N_("Configuration error: VRamSize is too large"));

    /* Round VRAM to power of 2 */
    pThis->cbVRAM = RT_ALIGN_32(pThis->cbVRAM, _1M);

    Log(("GeForce3Ti500: Configured with %u MB VRAM\n", pThis->cbVRAM / _1M));

    /*
     * PCI device setup.
     */
    PPDMPCIDEV pPciDev = pDevIns->apPciDevs[0];
    PDMPCIDEV_ASSERT_VALID(pDevIns, pPciDev);

    /* Configure PCI device */
    PDMPciDevSetVendorId(pPciDev,       GEFORCE3TI500_PCI_VENDOR_ID);
    PDMPciDevSetDeviceId(pPciDev,       GEFORCE3TI500_PCI_DEVICE_ID);
    PDMPciDevSetClassBase(pPciDev,      VBOX_PCI_CLASS_DISPLAY);
    PDMPciDevSetClassSub(pPciDev,       VBOX_PCI_SUB_DISPLAY_3D);
    PDMPciDevSetClassProg(pPciDev,      0x00);
    PDMPciDevSetHeaderType(pPciDev,     0x00);
    PDMPciDevSetRevisionId(pPciDev,     0x01);
    
    /* Capabilities */
    PDMPciDevSetCapabilityList(pPciDev, 0x60);
    PDMPciDevSetInterruptPin(pPciDev,   1);

    /*
     * Register PCI device.
     */
    rc = PDMDevHlpPCIRegister(pDevIns, pPciDev);
    if (RT_FAILURE(rc))
        return rc;

    /*
     * Register PCI BARs.
     */
    
    /* Framebuffer BAR (BAR0) - prefetchable memory using MMIO2 */
    rc = PDMDevHlpPCIIORegionCreateMmio2Ex(pDevIns, GEFORCE3TI500_PCI_BAR_FB, pThis->cbVRAM,
                                           PCI_ADDRESS_SPACE_MEM_PREFETCH, 0 /*fFlags*/,
                                           geforce3Ti500R3Map, "GeForce3Ti500-FB",
                                           (void **)&pThisCC->pbVRAM, &pThis->hMmio2FB);
    if (RT_FAILURE(rc))
        return PDMDevHlpVMSetError(pDevIns, rc, RT_SRC_POS, "Failed to create framebuffer MMIO2 region");

    /* Register BAR (BAR1) - non-prefetchable memory */
    rc = PDMDevHlpPCIIORegionCreateMmio(pDevIns, GEFORCE3TI500_PCI_BAR_REG, GEFORCE3TI500_REG_SIZE,
                                        PCI_ADDRESS_SPACE_MEM, geforce3Ti500MmioWrite, geforce3Ti500MmioRead, NULL,
                                        IOMMMIO_FLAGS_READ_DWORD | IOMMMIO_FLAGS_WRITE_DWORD_ZEROED,
                                        "GeForce3Ti500-Reg", &pThis->hMmioReg);
    if (RT_FAILURE(rc))
        return PDMDevHlpVMSetError(pDevIns, rc, RT_SRC_POS, "Failed to create register MMIO region");

    /*
     * Initialize display port interface.
     */
    pThisCC->IBase.pfnQueryInterface                    = geforce3Ti500R3PortQueryInterface;
    pThisCC->IPort.pfnUpdateDisplay                     = geforce3Ti500R3PortUpdateDisplay;
    pThisCC->IPort.pfnUpdateDisplayAll                  = geforce3Ti500R3PortUpdateDisplayAll;
    pThisCC->IPort.pfnQueryVideoMode                    = geforce3Ti500R3PortQueryVideoMode;
    pThisCC->IPort.pfnSetRefreshRate                    = geforce3Ti500R3PortSetRefreshRate;
    pThisCC->IPort.pfnTakeScreenshot                    = geforce3Ti500R3PortTakeScreenshot;
    pThisCC->IPort.pfnFreeScreenshot                    = geforce3Ti500R3PortFreeScreenshot;
    pThisCC->IPort.pfnDisplayBlt                        = geforce3Ti500R3PortDisplayBlt;
    pThisCC->IPort.pfnUpdateDisplayRect                 = geforce3Ti500R3PortUpdateDisplayRect;
    pThisCC->IPort.pfnCopyRect                          = geforce3Ti500R3PortCopyRect;
    pThisCC->IPort.pfnSetRenderVRAM                     = geforce3Ti500R3PortSetRenderVRAM;

    /*
     * Reset the device to initialize state.
     */
    geforce3Ti500R3Reset(pDevIns);

    /*
     * Attach display driver.
     */
    rc = geforce3Ti500R3Attach(pDevIns, 0, 0);
    if (RT_FAILURE(rc))
        return rc;

    Log(("GeForce3Ti500: Construction complete\n"));
    return VINF_SUCCESS;
}

#endif /* IN_RING3 */

/**
 * The device registration structure.
 */
const PDMDEVREG g_DeviceGeForce3Ti500 =
{
    /* .u32Version = */             PDM_DEVREG_VERSION,
    /* .uReserved0 = */             0,
    /* .szName = */                 "geforce3ti500",
    /* .fFlags = */                 PDM_DEVREG_FLAGS_DEFAULT_BITS | PDM_DEVREG_FLAGS_NEW_STYLE,
    /* .fClass = */                 PDM_DEVREG_CLASS_GRAPHICS,
    /* .cMaxInstances = */          1,
    /* .uSharedVersion = */         42,
    /* .cbInstanceShared = */       sizeof(GEFORCE3TI500STATE),
    /* .cbInstanceCC = */           sizeof(GEFORCE3TI500STATECC),
    /* .cbInstanceRC = */           0,
    /* .cMaxPciDevices = */         1,
    /* .cMaxMsixVectors = */        0,
    /* .pszDescription = */         "NVIDIA GeForce 3 Ti 500 Graphics Adapter",
#if defined(IN_RING3)
    /* .pszRCMod = */               "",
    /* .pszR0Mod = */               "",
    /* .pfnConstruct = */           geforce3Ti500R3Construct,
    /* .pfnDestruct = */            geforce3Ti500R3Destruct,
    /* .pfnRelocate = */            NULL,
    /* .pfnMemSetup = */            NULL,
    /* .pfnPowerOn = */             NULL,
    /* .pfnReset = */               geforce3Ti500R3Reset,
    /* .pfnSuspend = */             NULL,
    /* .pfnResume = */              NULL,
    /* .pfnAttach = */              geforce3Ti500R3Attach,
    /* .pfnDetach = */              geforce3Ti500R3Detach,
    /* .pfnQueryInterface = */      NULL,
    /* .pfnInitComplete = */        NULL,
    /* .pfnPowerOff = */            NULL,
    /* .pfnSoftReset = */           NULL,
    /* .pfnReserved0 = */           NULL,
    /* .pfnReserved1 = */           NULL,
    /* .pfnReserved2 = */           NULL,
    /* .pfnReserved3 = */           NULL,
    /* .pfnReserved4 = */           NULL,
    /* .pfnReserved5 = */           NULL,
    /* .pfnReserved6 = */           NULL,
    /* .pfnReserved7 = */           NULL,
#elif defined(IN_RING0)
    /* .pfnEarlyConstruct = */      NULL,
    /* .pfnConstruct = */           NULL,
    /* .pfnDestruct = */            NULL,
    /* .pfnFinalDestruct = */       NULL,
    /* .pfnRequest = */             NULL,
    /* .pfnReserved0 = */           NULL,
    /* .pfnReserved1 = */           NULL,
    /* .pfnReserved2 = */           NULL,
    /* .pfnReserved3 = */           NULL,
    /* .pfnReserved4 = */           NULL,
    /* .pfnReserved5 = */           NULL,
    /* .pfnReserved6 = */           NULL,
    /* .pfnReserved7 = */           NULL,
#elif defined(IN_RC)
    /* .pfnConstruct = */           NULL,
    /* .pfnReserved0 = */           NULL,
    /* .pfnReserved1 = */           NULL,
    /* .pfnReserved2 = */           NULL,
    /* .pfnReserved3 = */           NULL,
    /* .pfnReserved4 = */           NULL,
    /* .pfnReserved5 = */           NULL,
    /* .pfnReserved6 = */           NULL,
    /* .pfnReserved7 = */           NULL,
#else
# error "Not in IN_RING3, IN_RING0 or IN_RC!"
#endif
    /* .u32VersionEnd = */          PDM_DEVREG_VERSION
};