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
    void loadGameSounds(const std::string& assetSearchPath, PhaseAudio* pAudioEngine);
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
    
    NS::SharedPtr<MTL::Texture>         _pBackbuffer;
    

    MTL::Device*                        _pDevice;
    MTL::CommandQueue*                  _pCommandQueue;

    float          _maxEDRValue;
    float          _brightness;
    float          _edrBias;
    
    int            _highScore;
    int            _prevScore;

    MTL::Texture* currentDrawableTexture( MTL::Drawable* pCurrentDrawable );
    MTL::CommandBuffer* beginDrawableCommands();
    int _frame;
    std::unique_ptr<PhaseAudio> _pAudioEngine;
    uint64_t m_frameNumber;
    simd::float4x4 m_projection_matrix;
    MTL::CommandQueue* m_pCommandQueue;
    dispatch_semaphore_t m_inFlightSemaphore;
};

#endif /* RMDLGAMECOORDINATOR_HPP */
