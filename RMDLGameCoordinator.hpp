/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLGameCoordinator.hpp           +++     +++	**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 27/10/2025 15:45:19      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RMDLGAMECOORDINATOR_HPP
#define RMDLGAMECOORDINATOR_HPP

#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>
#include <MetalFX/MetalFX.hpp>

#include <string>
#include <unordered_map>

#include "NonCopyable.h"
#include "RMDLCamera.hpp"
#include "RMDLMesh.hpp"
#include "RMDLUI.hpp"
#include "RMDLFontLoader.h"
#include "RMDLGame.hpp"

constexpr uint8_t MaxFramesInFlight = 3;
static const uint32_t NumLights = 256;

class GameCoordinator : public NonCopyable
{
public:
    GameCoordinator(MTL::Device* pDevice,
                    MTL::PixelFormat layerPixelFormat,
                    NS::UInteger width,
                    NS::UInteger height,
                    NS::UInteger gameUICanvasSize,
                    const std::string& assetSearchPath);
    ~GameCoordinator();

    void buildRenderPipelines( const std::string& shaderSearchPath );
    void buildComputePipelines( const std::string& shaderSearchPath );
    void buildRenderTextures(NS::UInteger nativeWidth, NS::UInteger nativeHeight,
                             NS::UInteger presentWidth, NS::UInteger presentHeight);
    void loadGameTextures( const std::string& textureSearchPath );
    void loadGameSounds( const std::string& assetSearchPath, PhaseAudio* pAudioEngine );
    void buildSamplers();
    void buildMetalFXUpscaler(NS::UInteger inputWidth, NS::UInteger inputHeight,
                              NS::UInteger outputWidth, NS::UInteger outputHeight);

    void presentTexture( MTL::RenderCommandEncoder* pRenderEnc, MTL::Texture* pTexture );
    void draw( CA::MetalDrawable* pDrawable, double targetTimestamp );

    void resizeDrawable(float width, float height);
    void setMaxEDRValue(float value)     { _maxEDRValue = value; }
    void setBrightness(float brightness) { _brightness = brightness; }
    void setEDRBias(float edrBias)       { _edrBias = edrBias; }

    enum class HighScoreSource {
        Local,
        Cloud
    };
    
    void setHighScore(int highScore, HighScoreSource scoreSource);
    int highScore() const                { return _highScore; }

    void setupCamera();
    void moveCamera( simd::float3 translation );
    void rotateCamera(float deltaYaw, float deltaPitch);
    void setCameraAspectRatio(float aspectRatio);
    
    float _rotationAngle;
    void buildCubeBuffers();
    
private:
    RMDLCamera                          _camera;
    RMDLGame                            _game;
    RMDLUI                              _ui;
    FontAtlas                           _fontAtlas;
    
    GameConfig                          standardGameConfig();

    MTL::PixelFormat                    _layerPixelFormat;
    MTL::Buffer*                        _pUniformBuffer;
    MTL::Device*                        _pDevice;
    MTL::CommandQueue*                  _pCommandQueue;
    
    NS::SharedPtr<MTL::Texture>         _pBackbuffer;
    NS::SharedPtr<MTL::Texture>         _pUpscaledbuffer;
    NS::SharedPtr<MTL::Texture>         _pBackbufferAdapter;
    NS::SharedPtr<MTL::Texture>         _pUpscaledbufferAdapter;

    float          _maxEDRValue;
    float          _brightness;
    float          _edrBias;
    simd::float4x4 _presentOrtho;

    int            _highScore;
    int            _prevScore;
    
    NS::SharedPtr<MTL::SharedEvent>     _pPacingEvent;
    uint64_t                            _pacingTimeStampIndex;

    MTL::RenderPipelineState*           _pPresentPipeline;
    MTL::RenderPipelineState*           _pInstancedSpritePipeline;
    NS::SharedPtr<MTLFX::SpatialScaler> _pSpatialScaler;

    // Assets:
    MTL::SamplerState*          _pSampler;
    IndexedMesh                 _quadMesh;
    IndexedMesh                 _screenMesh;
    std::unordered_map<std::string, NS::SharedPtr<MTL::Texture>> _textureAssets;

    std::array<std::unique_ptr<BumpAllocator>, kMaxFramesInFlight> _bufferAllocator;

    int _frame;

    std::unique_ptr<PhaseAudio> _pAudioEngine;
};

#endif /* RMDLGAMECOORDINATOR_HPP */
