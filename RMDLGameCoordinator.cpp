/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLGameCoordinator.cpp            +++    +++ **/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 27/10/2025 18:44:15      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTLFX_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>
#include <MetalFX/MetalFX.hpp>

#include <simd/simd.h>
#include <utility>
#include <variant>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <thread>
#include <sys/sysctl.h>
#include <stdlib.h>

#include "RMDLGameCoordinator.hpp"
#include "RMDLMathUtils.hpp"
#include "RMDLUtilities.h"

#define NUM_ELEMS(arr) (sizeof(arr) / sizeof(arr[0]))

static constexpr uint64_t kPerFrameBumpAllocatorCapacity = 1024;

struct Uniforms
{
    simd::float4x4 modelViewProjectionMatrix;
    simd::float4x4 modelMatrix;
};

static const float cubeVertices[] = {
    // positions          // colors
    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 0.0f
};

static const uint16_t cubeIndices[] = {
    0, 1, 2, 2, 3, 0, // front
    4, 5, 6, 6, 7, 4, // back
    0, 4, 7, 7, 3, 0, // left
    1, 5, 6, 6, 2, 1, // right
    3, 2, 6, 6, 7, 3, // top
    0, 1, 5, 5, 4, 0  // bottom
};

static const uint16_t indices[] = {
    0, 1, 2, 2, 3, 0, /* front */
    4, 5, 6, 6, 7, 4, /* right */
    8, 9, 10, 10, 11, 8, /* back */
    12, 13, 14, 14, 15, 12, /* left */
    16, 17, 18, 18, 19, 16, /* top */
    20, 21, 22, 22, 23, 20, /* bottom */
};

GameCoordinator::GameCoordinator(MTL::Device* pDevice,
                                 MTL::PixelFormat layerPixelFormat,
                                 NS::UInteger width,
                                 NS::UInteger height,
                                 NS::UInteger gameUICanvasSize,
                                 const std::string& assetSearchPath)
    : _layerPixelFormat(layerPixelFormat)
    , _pDevice(pDevice->retain())
    , _frame(0)
    , _maxEDRValue(1.0f)
    , _brightness(500)
    , _edrBias(0)
    , _highScore(0)
    , _prevScore(0)
    //, _pCubeVertexBuffer(nullptr)
{
    printf("GameCoordinator constructor called\n");
    unsigned int numThreads = std::thread::hardware_concurrency();
    std::cout << "number of Threads = std::thread::hardware_concurrency : " << numThreads << std::endl;
    _pCommandQueue = _pDevice->newCommandQueue();
    setupCamera();
    
    const NS::UInteger nativeWidth = (NS::UInteger)(width / 1.2);
    const NS::UInteger nativeHeight = (NS::UInteger)(height / 1.2);
    printf("GameCoordinator constructor completed\n");
}

GameCoordinator::~GameCoordinator()
{
    _pCommandQueue->release();
    if (m_inFlightSemaphore)
    {
        dispatch_release(m_inFlightSemaphore);
    }
    _pDevice->release();
}

// Get a drawable from the view (or hand back an offscreen drawable for buffer examination mode)
MTL::Texture* GameCoordinator::currentDrawableTexture( MTL::Drawable* pCurrentDrawable )
{
    if(pCurrentDrawable)
    {
        auto pMtlDrawable = static_cast< CA::MetalDrawable* >(pCurrentDrawable);
        return pMtlDrawable->texture();
    }

    return nullptr;
}

MTL::CommandBuffer* GameCoordinator::beginDrawableCommands()
{
    MTL::CommandBuffer* pCommandBuffer = m_pCommandQueue->commandBuffer();

    // Create a completed handler functor for Metal to execute when the GPU has fully finished
    // processing the commands encoded for this frame. This implenentation of the completed
    // handler signals the `m_inFlightSemaphore`, which indicates that the GPU is no longer
    // accessing the the dynamic buffer written to this frame. When the GPU no longer accesses the
    // buffer, the renderer can safely overwrite the buffer's contents with data for a future frame.

    pCommandBuffer->addCompletedHandler(MTL::HandlerFunction([this]( MTL::CommandBuffer* ){
        dispatch_semaphore_signal( m_inFlightSemaphore );
    }));

    return pCommandBuffer;
}

void GameCoordinator::setupCamera()
{
    _camera.initPerspectiveWithPosition(
        {0.0f, 0.0f, 5.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 1.0f, 0.0f},
        M_PI / 3.0f,
        1.0f,
        0.1f,
        100.0f
    );
}



void GameCoordinator::buildCubeBuffers()
{
}

void GameCoordinator::buildRenderPipelines(const std::string& shaderSearchPath)
{
    NS::Error* pError = nullptr;
    std::string metallibPath = shaderSearchPath + "/Shaders.metallib";
    NS::String* nsPath = NS::String::string(metallibPath.c_str(), NS::StringEncoding::UTF8StringEncoding);
    MTL::Library* pLibrary = _pDevice->newLibrary(nsPath, &pError);
    if ( !pLibrary )
    {
        __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
        assert( false );
    }
    MTL::Function* pVertexFn = pLibrary->newFunction(NS::String::string("vertex_main", NS::StringEncoding::UTF8StringEncoding));
    MTL::Function* pFragmentFn = pLibrary->newFunction(NS::String::string("fragment_main", NS::StringEncoding::UTF8StringEncoding));
    MTL::RenderPipelineDescriptor* pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pDesc->setVertexFunction(pVertexFn);
    pDesc->setFragmentFunction(pFragmentFn);
    pDesc->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB );
    pDesc->setDepthAttachmentPixelFormat( MTL::PixelFormat::PixelFormatDepth16Unorm );

    MTL::VertexDescriptor* pVertexDesc = MTL::VertexDescriptor::alloc()->init();

    // Position (attribute 0)
    pVertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(0)->setOffset(0);
    pVertexDesc->attributes()->object(0)->setBufferIndex(0);

    // Couleur (attribute 1)
    pVertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(1)->setOffset(12); // 3 floats * 4 bytes = 12
    pVertexDesc->attributes()->object(1)->setBufferIndex(0);

    // Layout du buffer
    pVertexDesc->layouts()->object(0)->setStride(24); // 6 floats * 4 bytes = 24
    pVertexDesc->layouts()->object(0)->setStepRate(1);
    pVertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    pDesc->setVertexDescriptor(pVertexDesc);

    MTL::DepthStencilDescriptor* pDepthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    pDepthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    pDepthDesc->setDepthWriteEnabled(true);
    
    pDepthDesc->release();
    pVertexDesc->release();
    pDesc->release();
    pVertexFn->release();
    pFragmentFn->release();
    pLibrary->release();
}

void GameCoordinator::buildComputePipelines(const std::string& shaderSearchPath)
{
    // Build any compute pipelines
}

void GameCoordinator::buildRenderTextures(NS::UInteger nativeWidth, NS::UInteger nativeHeight,
                                          NS::UInteger presentWidth, NS::UInteger presentHeight)
{
}

void GameCoordinator::moveCamera(simd::float3 translation)
{
    simd::float3 newPosition = _camera.position() + translation;
    _camera.setPosition(newPosition);
}

void GameCoordinator::rotateCamera(float deltaYaw, float deltaPitch)
{
    _camera.rotateOnAxis({0.0f, 1.0f, 0.0f}, deltaYaw);
    _camera.rotateOnAxis(_camera.right(), deltaPitch);
}

void GameCoordinator::setCameraAspectRatio(float aspectRatio)
{
    _camera.setAspectRatio(aspectRatio);
}

void GameCoordinator::draw( CA::MetalDrawable* pDrawable, double targetTimestamp )
{
    assert(pDrawable);
    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();
    
    //dispatch_semaphore_wait(m_inFlightSemaphore, DISPATCH_TIME_FOREVER);
    
    MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
    
    //updateWorldState(false);

    _rotationAngle += 0.008f;
    if (_rotationAngle > 2 * M_PI)
    {
        _rotationAngle -= 2 * M_PI;
    }
    Uniforms* pUniforms = (Uniforms *)_pUniformBuffer->contents();
    simd::float4x4 modelMatrix =
    {
        simd::float4{ cosf(_rotationAngle), 0.0f, sinf(_rotationAngle), 0.0f },
        simd::float4{ 0.0f, 1.0f, 0.0f, 0.0f },
        simd::float4{ -sinf(_rotationAngle), 0.0f, cosf(_rotationAngle), 0.0f },
        simd::float4{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    RMDLCameraUniforms cameraUniforms = _camera.uniforms();
    pUniforms->modelMatrix = modelMatrix;
    pUniforms->modelViewProjectionMatrix = cameraUniforms.viewProjectionMatrix * modelMatrix;
    
    MTL::RenderPassDescriptor* pRenderPass = MTL::RenderPassDescriptor::renderPassDescriptor();
    pRenderPass->colorAttachments()->object(0)->setTexture(pDrawable->texture());
    pRenderPass->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    pRenderPass->colorAttachments()->object(0)->setClearColor(MTL::ClearColor::Make(0.1, 0.1, 0.1, 1.0));
    pRenderPass->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    pCmd->presentDrawable(pDrawable);
    pCmd->commit();
    pPool->release();
}
