#ifndef RMDLUI_HPP
#define RMDLUI_HPP

#include "RMDLFontLoader.h"
#include "RMDLMesh.hpp"
#include "RMDLMathUtils.hpp"
#include <../includes/NonCopyable.h>
#include "RMDLUtils.hpp"
#include "RMDLMeshUtils.hpp"

#include <memory>

constexpr uint64_t kFontAtlasTextureIndex = 0;
static constexpr size_t kMaxFramesInFlight = 3;

struct UIConfig
{
    NS::UInteger                            screenWidth;
    NS::UInteger                            screenHeight;
    NS::UInteger                            virtualCanvasWidth;
    NS::UInteger                            virtualCanvasHeight;
    FontAtlas                               fontAtlas;
    FiraCode                                firaCode;
    NS::SharedPtr<MTL::RenderPipelineState> uiPso;
};

struct UIRenderData
{
    //std::array<std::unique_ptr<BumpAllocator>, kMaxFramesInFlight> bufferAllocator;
    std::array<NS::SharedPtr<MTL::Heap>, kMaxFramesInFlight>       resourceHeaps;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight>     frameDataBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight>     highScorePositionBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight>     currentScorePositionBuf;
    NS::SharedPtr<MTL::Buffer>                                     textureTable;
    NS::SharedPtr<MTL::Buffer>                                     samplerTable;
    NS::SharedPtr<MTL::SamplerState>                               pSampler;
    NS::SharedPtr<MTL::ResidencySet>                               pResidencySet;
};

class UI : public NonCopyable
{
public:
    UI();
    ~UI();
    void initialize( const UIConfig& config, MTL::Device* pDevice, MTL::CommandQueue* pCommandQueue );
    void showHighScore( const char* label, int highscore, MTL::Device* pDevice );
    void showCurrentScore( const char* label, int score, MTL::Device* pDevice );
    void update(double targetTimestamp, uint8_t frameID);
    void draw( MTL::RenderCommandEncoder* pRenderCmd, uint8_t frameID );
private:
    void createBuffers( MTL::Device* pDevice );
    void createResidencySet( MTL::Device* pDevice, MTL::CommandQueue* pCommandQueue );
    
    UIConfig        _uiConfig;
    UIRenderData    _renderData;
    IndexedMesh     _highScoreMesh;
    IndexedMesh     _currentScoreMesh;
    
    simd::float4    _highScorePosition;
    simd::float4    _currentScorePosition;
    double          _lastTimestamp = 0.0;
    double          _bannerCountdownSecs = 0.0;
};

#endif // RMDLUI_HPP
