/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLCoordinatorBasic.cpp            +++     	**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 07/11/2025 13:35:49      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*#include "RMDLGameCoordinator.hpp"

static const float cubeVerts[] = {
        // pos             // color
        -1,-1,-1, 1,0,0,  1,-1,-1, 0,1,0,  1,1,-1, 0,0,1,
        -1,-1,-1, 1,0,0,  1,1,-1, 0,0,1,  -1,1,-1, 1,1,0
    };

void GameCoordinator::setupPipelineCamera()
{
    NS::Error* pError = nullptr;
    std::string metallibPath = "Shaders.metallib";
    NS::String* nsPath = NS::String::string(metallibPath.c_str(), NS::StringEncoding::UTF8StringEncoding);
    MTL::Library* pLibraryCamera = _pDevice->newLibrary(nsPath, &pError);
    MTL::Function* pVertexFnC = pLibrary->newFunction( NS::String::string("vertexMainCamera", NS::StringEncoding::UTF8StringEncoding) );
    MTL::Function* pFragFnC = pLibrary->newFunction( NS::String::string("fragmentMainCamera", NS::StringEncoding::UTF8StringEncoding) );
    MTL::RenderPipelineDescriptor* pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pDesc->setVertexFunction( pVertexFnC );
    pDesc->setFragmentFunction( pFragFnC );
    pDesc->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormat::PixelFormatRGBA16Float );
    _pCameraPSO = device->newRenderPipelineState(pDesc, &error);
    pVertexFnC->release();
    pFragFnC->release();
    pDesc->release();
}

void GameCoordinator::setupCube()
{
    vertexBuffer = device->newBuffer(cubeVerts, sizeof(cubeVerts), MTL::ResourceStorageModeManaged);
    uniformBuffer = device->newBuffer(sizeof(Uniforms), MTL::ResourceStorageModeManaged);
}*/

