//
// Game.h
//
#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Shader.h"
#include "modelclass.h"
#include "Light.h"
#include "Input.h"
#include "Camera.h"
#include "RenderTexture.h"
#include "Terrain.h"
#include "PostProcess.h"
#include "SkyboxEffect.h"
#include "Collision.h"
#include <stdio.h>
#include <Audio.h>


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
#ifdef DXTK_AUDIO
    void NewAudioDevice();
#endif

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

    SimpleMath::Vector3* highpoint;

	
private:

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	}; 

    void Update(DX::StepTimer const& timer);
    void Render();
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
	void SetupGUI();

    void postProcessTexture();

    bool RayTriIntersect(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 dest, DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 p3);
    bool RaySphereIntersection(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 dest, DirectX::SimpleMath::Vector3 center, float r);
    bool CameraPlaneIntersection(DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B, DirectX::SimpleMath::Vector3 D, DirectX::SimpleMath::Vector3 cPosition);
        
    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

	//input manager. 
	Input									m_input;
	InputCommands							m_gameInputCommands;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;	
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

	// Scene Objects
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;
	std::unique_ptr<DirectX::GeometricPrimitive>                            m_testmodel;

    //Skybox
    std::unique_ptr<DX::SkyboxEffect>                                       m_skyboxEffect;
    std::unique_ptr<DirectX::GeometricPrimitive>                            m_sky;

	//lights
	Light																	m_light;

	//Cameras
    Camera                                                                  m_mainCamera;
    Camera                                                                  m_reflectionCamera;
    Camera                                                                  m_postCamera;

    //Audio
    std::unique_ptr<DirectX::SoundEffect>                                   m_pop;
    std::unique_ptr<DirectX::SoundEffect>                                   m_boing;
    bool                                                                    playOnce = false;

	//textures
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_grass;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_rock;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_slope;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_red;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_orange;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_blue;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_lightBlue;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_darkBlue;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_skyboxTexture;

	//Shaders
	Shader																	m_BasicShaderPair;
    Shader                                                                  m_BasicShaderPairWater;

	//Scene. 
	Terrain																	m_WaterTerrain;
    Terrain                                                                 m_HighpointTerrain;
    

    RenderTexture*                                                          m_postProcessingTexture;
    std::unique_ptr<BasicPostProcess>                                       postProcess;
    DirectX::BasicPostProcess::Effect                                       filter = BasicPostProcess::Copy;

    //player
    ModelClass                                                              m_playerBall;
    ModelClass                                                              m_targetBall;
    ModelClass                                                              m_targetBall2;
    bool                                                                    m_playerR = 0;

	ModelClass																m_BasicModel;
	ModelClass																m_BasicModel2;
	ModelClass																m_BasicModel3;

	//RenderTextures
	RenderTexture*															m_FirstRenderPass;
	RECT																	m_fullscreenRect;
	RECT																	m_CameraViewRect;
	
    //camera variables
    float                                                                   playerRotationX;
    float                                                                   playerRotationY;

    bool                                                                    rotUp = false;
    bool                                                                    rotDown = false;
    bool                                                                    rotLeft = false;
    bool                                                                    rotRight = false;
    bool                                                                    move = false;

    //gravity variables
    float                                                                   gravity = 0;
    int                                                                     goingUp = 0;
    bool                                                                    pressedJump = false;
    bool                                                                    once = false;
    bool                                                                    grounded = false;

    //shooting game variables
    int                                                                     score = 0;
    bool                                                                    shooting = false;
    float                                                                   startShooting;
    float                                                                   targetScaling = 0.01;
    float                                                                   hitboxCheating = 1.1;
    bool                                                                    onceCal = false;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_ball_platColor;
    std::vector<DirectX::SimpleMath::Vector3>                               shotBalls;
    std::vector<DirectX::SimpleMath::Vector3>                               shotGroundBalls;
    
    //platforms
    bool                                                                    terrainGenerated = false;
    double                                                                  startTime = 0;
    
    //Collision
    Collision                                                               collisions;

    ModelClass                                                              m_platform;
    DirectX::SimpleMath::Vector3                                            pA;
    DirectX::SimpleMath::Vector3                                            pB;
    DirectX::SimpleMath::Vector3                                            pD;

    DirectX::SimpleMath::Vector3                                            ridingPlatform;

    //countdown
    const wchar_t*                                                          t1 = L"1";
    wchar_t*                                                                lost = L"YOU DIED";
    wchar_t*                                                                victory = L"VICTORY";
    wchar_t*                                                                intro = L"REACH THE OTHER SIDE OF THE SEA IN 30 SECONDS";
    
    bool                                                                    introOn = true;
    bool                                                                    died = false;
    bool                                                                    won = false;
    bool                                                                    start = false;

    double                                                                  countdown = -1;
    int                                                                     timeInt = 0;
    double                                                                  timeDead;
    DirectX::SimpleMath::Vector3                                            cameraStart;
    bool                                                                    reset = false;



    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;

    


    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_mainCameraView;
    DirectX::SimpleMath::Matrix                                             m_projection;
};