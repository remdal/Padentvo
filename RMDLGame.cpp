/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLGame.cpp            +++     +++			**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 28/10/2025 12:41:46      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "RMDLGame.hpp"

#include "RMDLMathUtils.hpp"

#define IR_RUNTIME_METALCPP
#define IR_PRIVATE_IMPLEMENTATION
#include <metal_irconverter_runtime/metal_irconverter_runtime.h>

#include <TargetConditionals.h>

bool deviceSupportsResidencySets( MTL::Device* pDevice )
{
    static bool result = false;
    static std::once_flag flag;
    std::call_once(flag, [&](){
        NS::OperatingSystemVersion minimumVersion;
#if TARGET_OS_OSX
        minimumVersion.majorVersion = 15;
        minimumVersion.minorVersion = 0;
        minimumVersion.patchVersion = 0;
#elif TARGET_OS_IOS
        minimumVersion.majorVersion = 18;
        minimumVersion.minorVersion = 0;
        minimumVersion.patchVersion = 0;
#endif
        result = NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(minimumVersion) && pDevice->supportsFamily(MTL::GPUFamilyApple6);
    });
    
    return (result);
}

RMDLGame::RMDLGame()
: _gameConfig()
, _level(0)
, _prevTargetTimestamp(0.0f)
, _firstFrame(true)
{
}

RMDLGame::~RMDLGame()
{
}

void RMDLGame::initializeGameState(const GameConfig& config)
{
    auto sw = config.screenWidth;
    auto sh = config.screenHeight;
    const float canvasW = 10;
    const float canvasH = canvasW * sh / (float)sw;
    const float spriteSize = kSpriteSize;
    for (uint8_t i = 0; i < kMaxFramesInFlight; ++i)
    {
        assert(_renderData.frameDataBuf[i]);
        auto pFrameData = (FrameData *)_renderData.frameDataBuf[i]->contents();
        pFrameData->projectionMatrix = math::makeOrtho(-canvasW/2, canvasW/2, canvasH/2, -canvasH/2, -1, 1);
    }
    _gameState.playerPosition = simd_make_float4(0, -canvasH/2 + spriteSize * 2, 0, 1);
}

void RMDLGame::createBuffers( const GameConfig& config, MTL::Device* pDevice )
{
    _renderData.spriteMesh = mesh_utils::newScreenQuad(pDevice, kSpriteSize, kSpriteSize);
    _renderData.backgroundMesh = mesh_utils::newScreenQuad(pDevice, 10 * 1920 / 1080.0, 10);
    const size_t playerPositionBufSize       = sizeof(simd::float4);
    const size_t frameDataBufSize            = sizeof(FrameData);
    const size_t playerBulletPositionBufSize = sizeof(simd::float4) * config.maxPlayerBullets;
    const size_t backgroundPositionBufSize   = sizeof(simd::float4);
    
    auto pHeapDesc = NS::TransferPtr( MTL::HeapDescriptor::alloc()->init() );
    pHeapDesc->setSize(playerPositionBufSize +
                       frameDataBufSize +
                       playerBulletPositionBufSize +
                       backgroundPositionBufSize);
    pHeapDesc->setResourceOptions(MTL::ResourceStorageModeShared);
    pHeapDesc->setHazardTrackingMode(MTL::HazardTrackingModeUntracked);
    
    for (uint8_t i = 0u; i < kMaxFramesInFlight; ++i)
    {
        auto pHeap = NS::TransferPtr(pDevice->newHeap(pHeapDesc.get()));
        _renderData.resourceHeaps[i] = pHeap;
        
        _renderData.playerPositionBuf[i] = NS::TransferPtr(pHeap->newBuffer(playerPositionBufSize, MTL::ResourceStorageModeShared));
        _renderData.playerPositionBuf[i]->setLabel(MTLSTR("playerPositionBuf"));
        
        _renderData.frameDataBuf[i] = NS::TransferPtr(pHeap->newBuffer(frameDataBufSize, MTL::ResourceStorageModeShared));
        _renderData.frameDataBuf[i]->setLabel(MTLSTR("frameDataBuf"));
        
        _renderData.playerBulletPositionBuf[i] = NS::TransferPtr(pHeap->newBuffer(playerBulletPositionBufSize, MTL::ResourceStorageModeShared));
        _renderData.playerBulletPositionBuf[i]->setLabel(MTLSTR("playerBulletPositionBuf"));
        
        _renderData.backgroundPositionBuf[i] = NS::TransferPtr(pHeap->newBuffer(backgroundPositionBufSize, MTL::ResourceStorageModeShared));
        _renderData.backgroundPositionBuf[i]->setLabel(MTLSTR("backgroundPositionBuf"));
        
        constexpr uint64_t bumpAllocatorCapacity = 1024;
        _renderData.bufferAllocator[i] = std::make_unique<BumpAllocator>(pDevice, bumpAllocatorCapacity, MTL::ResourceStorageModeShared);
    }
    
    const size_t textureTableBufSize         = sizeof(IRDescriptorTableEntry) * kNumTextures;
    const size_t samplerTableBufSize         = sizeof(IRDescriptorTableEntry) * 1;

    _renderData.textureTable = NS::TransferPtr(pDevice->newBuffer(textureTableBufSize, MTL::ResourceStorageModeShared));
    _renderData.textureTable->setLabel(MTLSTR("Sprite Texture Table"));
    auto pTextureTableContents = (IRDescriptorTableEntry *)_renderData.textureTable->contents();
    
    IRDescriptorTableSetTexture(&(pTextureTableContents[kEnemyTextureIndex]), config.enemyTexture.get(), 0, 0);
    IRDescriptorTableSetTexture(&(pTextureTableContents[kPlayerTextureIndex]), config.playerTexture.get(), 0, 0);
    IRDescriptorTableSetTexture(&(pTextureTableContents[kPlayerBulletTextureIndex]), config.playerBulletTexture.get(), 0, 0);
    IRDescriptorTableSetTexture(&(pTextureTableContents[kBackgroundTextureIndex]), config.backgroundTexture.get(), 0, 0);
    IRDescriptorTableSetTexture(&(pTextureTableContents[kExplosionTextureIndex]), config.explosionTexture.get(), 0, 0);
    
    // Sampler table:
    auto pSamplerDesc = NS::TransferPtr(MTL::SamplerDescriptor::alloc()->init());
    pSamplerDesc->setSupportArgumentBuffers(true);
    pSamplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    pSamplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    pSamplerDesc->setRAddressMode(MTL::SamplerAddressModeClampToEdge);
    pSamplerDesc->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    pSamplerDesc->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    
    _renderData.sampler = NS::TransferPtr(pDevice->newSamplerState(pSamplerDesc.get()));
    _renderData.samplerTable = NS::TransferPtr(pDevice->newBuffer(samplerTableBufSize, MTL::ResourceStorageModeShared));
    _renderData.samplerTable->setLabel(MTLSTR("Sprite Sampler Table"));
    
    // Set LOD bias to to account for image scaling by MetalFX.
    auto pSamplerTableContents = (IRDescriptorTableEntry *)_renderData.samplerTable->contents();
    IRDescriptorTableSetSampler(pSamplerTableContents, _renderData.sampler.get(), -0.5);
}

void RMDLGame::initializeResidencySet( const GameConfig& config, MTL::Device* pDevice, MTL::CommandQueue* pCommandQueue )
{
    if(deviceSupportsResidencySets(pDevice))
    {
        NS::Error* pError = nullptr;
        auto pResidencySetDesc = NS::TransferPtr(MTL::ResidencySetDescriptor::alloc()->init());
        pResidencySetDesc->setLabel(MTLSTR("Game Residency Set"));
        
        _renderData.residencySet = NS::TransferPtr(pDevice->newResidencySet(pResidencySetDesc.get(), &pError));
        
        if(_renderData.residencySet)
        {
            _renderData.residencySet->requestResidency();
            pCommandQueue->addResidencySet(_renderData.residencySet.get());
            
            for (uint8_t i = 0u; i < kMaxFramesInFlight; ++i)
            {
                _renderData.residencySet->addAllocation(_renderData.resourceHeaps[i].get());
                _renderData.residencySet->addAllocation(_renderData.enemyPositionBuf[i].get());
                _renderData.residencySet->addAllocation(_renderData.playerPositionBuf[i].get());
                _renderData.residencySet->addAllocation(_renderData.frameDataBuf[i].get());
                _renderData.residencySet->addAllocation(_renderData.playerBulletPositionBuf[i].get());
                _renderData.residencySet->addAllocation(_renderData.backgroundPositionBuf[i].get());
                _renderData.residencySet->addAllocation(_renderData.explosionPositionBuf[i].get());
                _renderData.residencySet->addAllocation(_renderData.bufferAllocator[i]->baseBuffer());
            }
            
            _renderData.residencySet->addAllocation(config.enemyTexture.get());
            _renderData.residencySet->addAllocation(config.playerTexture.get());
            _renderData.residencySet->addAllocation(config.playerBulletTexture.get());
            _renderData.residencySet->addAllocation(config.backgroundTexture.get());
            _renderData.residencySet->addAllocation(config.explosionTexture.get());
            
            _renderData.residencySet->addAllocation(_renderData.textureTable.get());
            _renderData.residencySet->addAllocation(_renderData.samplerTable.get());
            
            _renderData.residencySet->commit();
        }
        else
        {
            printf("Error creating residency set: %s\n", pError->localizedDescription()->utf8String());
            assert(_renderData.residencySet);
        }
    }
}

void RMDLGame::initialize( const GameConfig& config, MTL::Device* pDevice, MTL::CommandQueue* pCommandQueue )
{
    createBuffers(config, pDevice);
    initializeResidencySet(config, pDevice, pCommandQueue);
}

void GameState::reset()
{
    enemiesAlive                = 0;
    playerBulletsAlive          = 0;
    playerFireCooldownRemaining = 0;
    explosionsAlive             = 0;
    playerPosition              = simd_make_float4(0, 0, 0, 1);
    currentEnemyDirection       = EnemyDirection::Right;
    nextEnemyDirection          = EnemyDirection::Right;
    backgroundPosition          = simd_make_float4(0, 0, 0, 1);
    gameStatus                  = GameStatus::Ongoing;
    rumbleCountdownRemaining    = 0;
    enemyMovedownRemaining      = 0;
}

void RMDLGame::restartGame(const GameConfig &config, float startingScore)
{
    assert(_renderData.spriteMesh.pIndices || !"Attempt to restart game without calling initialize() first");
    
    _gameConfig = config;
    _gameConfig.enemySpeed *= (1 + _level * 0.25f);
    
    const uint32_t cols = _gameConfig.enemyCols;
    const uint32_t rows = _gameConfig.enemyRows;
    
    _gameState.reset();
    _gameState.enemiesAlive = rows * cols;
    _gameState.enemyPositions.resize(_gameState.enemiesAlive);
    _gameState.gameStatus = GameStatus::Ongoing;
    _gameState.playerScore = startingScore;
    
    initializeGameState(config);
}

const GameState* RMDLGame::update(double targetTimestamp, uint8_t frameID)
{
    assert(frameID < kMaxFramesInFlight);

    return (&_gameState);
}
