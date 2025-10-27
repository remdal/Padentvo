/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLBufferFormats.h            +++     +++	**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 19/09/2025 20:15:02      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once    

#import <Metal/Metal.h>

namespace BufferFormats
{
    static const MTLPixelFormat gBuffer0Format      = MTLPixelFormatBGRA8Unorm_sRGB;
    static const MTLPixelFormat gBuffer1Format      = MTLPixelFormatRGBA8Unorm;
    static const MTLPixelFormat depthFormat         = MTLPixelFormatDepth32Float;
    static const MTLPixelFormat shadowDepthFormat   = MTLPixelFormatDepth32Float;
    static const NSUInteger     count               = 1;
    static const MTLPixelFormat backBufferformat    = MTLPixelFormatBGRA8Unorm_sRGB;
}