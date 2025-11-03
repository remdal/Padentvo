/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLGame.hpp            +++     +++			**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 27/10/2025 22:20:05      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef GAME_HPP
#define GAME_HPP

#include <cstdint>
#include <vector>
#include <array>
#include <simd/simd.h>
#include <Metal/Metal.hpp>

#include "RMDLController.h"
#include "RMDLMeshUtils.hpp"
#include "RMDLPhaseAudio.hpp"
#include "RMDLBumpAllocator.hpp"

#include "RMDLConfig_Shared.h"

constexpr uint64_t kEnemyTextureIndex        = 0;
constexpr uint64_t kPlayerTextureIndex       = 1;
constexpr uint64_t kPlayerBulletTextureIndex = 2;
constexpr uint64_t kBackgroundTextureIndex   = 3;
constexpr uint64_t kExplosionTextureIndex    = 4;
constexpr uint64_t kNumTextures              = 5;

constexpr float kSpriteSize                  = 0.5f;
constexpr float kRumbleDurationSecs          = 0.1f;
constexpr float kRumbleIntensity             = 1.0f;

struct GameConfig
{
    uint32_t                                screenWidth;
    uint32_t                                screenHeight;
    
    PhaseAudio*                             pAudioEngine;


    uint8_t                                 enemyRows;
    uint8_t                                 enemyCols;
    NS::SharedPtr<MTL::Texture>             enemyTexture;
    NS::SharedPtr<MTL::Texture>             playerTexture;
    NS::SharedPtr<MTL::Texture>             playerBulletTexture;
    NS::SharedPtr<MTL::Texture>             backgroundTexture;
    NS::SharedPtr<MTL::Texture>             explosionTexture;
    NS::SharedPtr<MTL::Texture>             fontAtlasTexture;
    NS::SharedPtr<MTL::RenderPipelineState> spritePso;
    float                                   enemySpeed;
    float                                   enemyMoveDownStep;
    float                                   playerSpeed;
    float                                   playerFireCooldownSecs;
    uint8_t                                 maxPlayerBullets;
    uint8_t                                 maxExplosions;
    float                                   explosionDurationSecs;
};

/**
 * Holds resources the GPU accesses when drawing the game from GameRendering.cpp.
 * This structure triple-shadows all data that changes frame-to-frame to avoid
 * race conditions between the CPU and GPU as the CPU may update buffers while the
 * GPU reads from them.
 */
struct RenderData
{
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight> frameDataBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight> enemyPositionBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight> playerPositionBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight> playerBulletPositionBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight> backgroundPositionBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxFramesInFlight> explosionPositionBuf;
    
    NS::SharedPtr<MTL::Buffer>  textureTable;
    NS::SharedPtr<MTL::Buffer>  samplerTable;
    
    NS::SharedPtr<MTL::SamplerState> sampler;
    IndexedMesh                      spriteMesh;
    IndexedMesh                      backgroundMesh;
    
    std::array<std::unique_ptr<BumpAllocator>, kMaxFramesInFlight> bufferAllocator;
    std::array<NS::SharedPtr<MTL::Heap>, kMaxFramesInFlight>       resourceHeaps;
    
    NS::SharedPtr<MTL::ResidencySet> residencySet;
};

enum class EnemyDirection
{
    Right,
    Left,
    Down
};

enum class GameStatus
{
    Ongoing,
    PlayerWon,
    PlayerLost
};

struct GameState
{
    uint32_t                    enemiesAlive;
    uint8_t                     playerBulletsAlive;
    std::vector<simd::float4>   playerBulletPositions;
    float                       playerFireCooldownRemaining;
    std::vector<simd::float4>   explosionPositions;
    std::vector<float>          explosionCooldownsRemaining;
    simd::float4                playerPosition;
    // EnemyDirection              currentEnemyDirection;
    simd::float4                backgroundPosition;

    GameStatus                  gameStatus;
    float                       rumbleCountdownRemaining;
    float                       enemyMovedownRemaining;
    void                        reset();
    int                         playerScore;
};

class RMDLGame : public NonCopyable
{
public:
    RMDLGame();
    ~RMDLGame();
    
    void             initialize( const GameConfig& config, MTL::Device* pDevice, MTL::CommandQueue* pCommandQueue );
    void             restartGame(const GameConfig& config, float startingScore);
    const GameState* update(double targetTimestamp, uint8_t frameID);
    void             draw( MTL::RenderCommandEncoder* pRenderCmd, uint8_t frameID );
    void             drawUI( MTL::RenderCommandEncoder* pRenderCmd, uint8_t frameID, const FontAtlas&, const IndexedMesh& );

    
private:
    void initializeGameState(const GameConfig& config);
    void createBuffers( const GameConfig& config, MTL::Device* pDevice );
    void initializeResidencySet( const GameConfig& config, MTL::Device* pDevice, MTL::CommandQueue* pCommandQueue );
    void updateCollisions();

    GameController _gameController;
    GameConfig     _gameConfig;
    RenderData     _renderData;
    GameState      _gameState;
    
    uint32_t       _level;
    double         _prevTargetTimestamp;
    bool           _firstFrame;
};

#endif // GAME_HPP
