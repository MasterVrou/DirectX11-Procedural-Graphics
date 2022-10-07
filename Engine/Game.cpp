//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <cmath>


//toreorganise
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace ImGui;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{

	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	//setup imgui.  its up here cos we need the window handle too
	//pulled from imgui directx11 example
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);		//tie to our window
	ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());	//tie to directx

	m_fullscreenRect.left = 0;
	m_fullscreenRect.top = 0;
	m_fullscreenRect.right = 800;
	m_fullscreenRect.bottom = 600;

	m_CameraViewRect.left = 500;
	m_CameraViewRect.top = 0;
	m_CameraViewRect.right = 800;
	m_CameraViewRect.bottom = 240;

	//setup light
	m_light.setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	m_light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	m_light.setPosition(2.0f, 1.0f, 1.0f);
	m_light.setDirection(-1.0f, -1.0f, 0.0f);

	//setup camera
    cameraStart = Vector3(-67, 10, -67);
	m_mainCamera.setPosition(cameraStart);
    m_mainCamera.setRotation(Vector3(-90.0f, -45.0f, 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up. 

	
    postProcess = std::make_unique<BasicPostProcess>(m_deviceResources.get()->GetD3DDevice());

//#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);
    //load Audio
    m_pop = std::make_unique<SoundEffect>(m_audEngine.get(), L"pop.wav");
    m_boing = std::make_unique<SoundEffect>(m_audEngine.get(), L"boing.wav");

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

//#endif
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	//Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{	
    double deltaTime = timer.GetElapsedSeconds();

	//this is hacky,  i dont like this here.  
	auto device = m_deviceResources->GetD3DDevice();

    //float deltaTime = timer.GetElapsedSeconds();
    //note that currently.  Delta-time is not considered in the game object movement. 
    if (m_gameInputCommands.left)
    {
        Vector3 rotation = m_mainCamera.getRotation();
        rotation.y = rotation.y += m_mainCamera.getRotationSpeed();
        m_mainCamera.setRotation(rotation);
        //m_reflectionCamera.setRotation(rotation);
        m_postCamera.setRotation(rotation);
        rotLeft = true;
    }
    if (m_gameInputCommands.right)
    {
        Vector3 rotation = m_mainCamera.getRotation();
        rotation.y = rotation.y -= m_mainCamera.getRotationSpeed();
        m_mainCamera.setRotation(rotation);
        //m_reflectionCamera.setRotation(rotation);
        m_postCamera.setRotation(rotation);
        rotRight = true;
    }
    if (m_gameInputCommands.forward)
    {
        //Vector3 position = m_mainCamera.getPosition(); //get the position
        //position +=(m_mainCamera.getForward() * m_mainCamera.getMoveSpeed()) /2; //add the forward vector
        //m_mainCamera.setPosition(position);
        //m_reflectionCamera.setPosition(position);
        //m_postCamera.setPosition(position);
        Vector3 position = m_mainCamera.getPosition(); //get the position

        float steadyY = position.y;

        position += (m_mainCamera.getForward() * m_mainCamera.getMoveSpeed()) / 2; //add the forward vector
        m_mainCamera.setPosition(Vector3(position.x, steadyY, position.z));
        m_reflectionCamera.setPosition(position);
        m_postCamera.setPosition(position);

    }
    if (m_gameInputCommands.back)
    {
        Vector3 position = m_mainCamera.getPosition(); //get the position
        position -= (m_mainCamera.getForward() * m_mainCamera.getMoveSpeed()); //add the forward vector
        m_mainCamera.setPosition(position);
        //m_reflectionCamera.setPosition(position);
        m_postCamera.setPosition(position);
    }
    if (m_gameInputCommands.rotLeft)
    {
        Vector3 rotation = m_mainCamera.getRotation();
        rotation.y = rotation.y -= m_input.getDeltaX()/2;
        m_mainCamera.setRotation(rotation);
        //m_reflectionCamera.setRotation(rotation);
        m_postCamera.setRotation(rotation);
        rotLeft = true;
    }
    if (m_gameInputCommands.rotUp)
    {
        Vector3 rotation = m_mainCamera.getRotation();
        rotation.x = rotation.x += m_input.getDeltaY()/2;
        if (rotation.x < -180)
        {
            rotation.x = -179;
        }
        if (rotation.x > 0)
        {
            rotation.x = 1;
        }
        m_mainCamera.setRotation(rotation);
        //m_reflectionCamera.setRotation(rotation);
        m_postCamera.setRotation(rotation);
        rotUp = true;
    }
    if (m_gameInputCommands.jump)
    {
        pressedJump = true;

    }
	if (m_gameInputCommands.generate)
	{
		m_HighpointTerrain.GenerateGroundMap(device);
        terrainGenerated = true;
	}
    if (m_gameInputCommands.shoot)
    {
        shooting = true;
    }
    else 
    {
        shooting = false;
    }
    
    //Checking water platforms for collisions
    for (int i = 0; i < m_WaterTerrain.highPoints.size(); i++)
    {
        pA = DirectX::SimpleMath::Vector3(m_WaterTerrain.highPoints[i].x - 1, m_WaterTerrain.highPoints[i].y, m_WaterTerrain.highPoints[i].z-1);
        pB = DirectX::SimpleMath::Vector3(m_WaterTerrain.highPoints[i].x - 1, m_WaterTerrain.highPoints[i].y, m_WaterTerrain.highPoints[i].z + 1);
        pD = DirectX::SimpleMath::Vector3(m_WaterTerrain.highPoints[i].x + 1, m_WaterTerrain.highPoints[i].y, m_WaterTerrain.highPoints[i].z - 1);

        if (CameraPlaneIntersection(pA, pB, pD, DirectX::SimpleMath::Vector3(m_mainCamera.getPosition().x, m_mainCamera.getPosition().y - 3, m_mainCamera.getPosition().z)))
        {
            grounded = true;
            ridingPlatform = m_WaterTerrain.highPoints[i];
        }
    }

    //Checking ground platforms for collisions
    for (int i = 0; i < m_HighpointTerrain.highPoints.size(); i++)
    {
        pA = DirectX::SimpleMath::Vector3(m_HighpointTerrain.highPoints[i].x - 1 - 66, m_HighpointTerrain.highPoints[i].y, m_HighpointTerrain.highPoints[i].z - 1 - 66);
        pB = DirectX::SimpleMath::Vector3(m_HighpointTerrain.highPoints[i].x - 1 - 66, m_HighpointTerrain.highPoints[i].y, m_HighpointTerrain.highPoints[i].z + 1 - 66);
        pD = DirectX::SimpleMath::Vector3(m_HighpointTerrain.highPoints[i].x + 1 - 66, m_HighpointTerrain.highPoints[i].y, m_HighpointTerrain.highPoints[i].z - 1 - 66);

        if (CameraPlaneIntersection(pA, pB, pD, DirectX::SimpleMath::Vector3(m_mainCamera.getPosition().x, m_mainCamera.getPosition().y - 3, m_mainCamera.getPosition().z)))
        {
            //if its green
            if(std::count(shotGroundBalls.begin(), shotGroundBalls.end(), m_HighpointTerrain.highPoints[i]))
            {
                grounded = true;
                ridingPlatform = Vector3(m_HighpointTerrain.highPoints[i].x - 66, m_HighpointTerrain.highPoints[i].y, m_HighpointTerrain.highPoints[i].z - 66);
            }
            
        }
    }
        
    //first platform
    pA = DirectX::SimpleMath::Vector3(-68, 0, -68);
    pB = DirectX::SimpleMath::Vector3(-68, 0, -66);
    pD = DirectX::SimpleMath::Vector3(-66, 0, -68);
    if (CameraPlaneIntersection(pA, pB, pD, DirectX::SimpleMath::Vector3(m_mainCamera.getPosition().x, m_mainCamera.getPosition().y-3, m_mainCamera.getPosition().z)))//-----------1
    {
        grounded = true;
        ridingPlatform = DirectX::SimpleMath::Vector3(-67, 0, -67);
    }

    //Goal platform
    pA = DirectX::SimpleMath::Vector3(62, 0, 62);
    pB = DirectX::SimpleMath::Vector3(62, 0, 68);
    pD = DirectX::SimpleMath::Vector3(68, 0, 62);
    if (CameraPlaneIntersection(pA, pB, pD, DirectX::SimpleMath::Vector3(m_mainCamera.getPosition().x, m_mainCamera.getPosition().y - 3, m_mainCamera.getPosition().z)))//-----------1
    {
        grounded = true;
        ridingPlatform = DirectX::SimpleMath::Vector3(65, 0, 65);
        won = true;
    }

    //Gravity
    if (!grounded) 
    {
        gravity = -9 * 2 *deltaTime;
        pressedJump = false;
        once = false;
    }
    else
    {
        gravity = 0;
        if (pressedJump && !once) 
        {
            goingUp = 50;
            once = true;
            pressedJump = false;
            m_boing->Play();
        }
        if (goingUp > 0) 
        {
            gravity =  9 * 2 * deltaTime;
            goingUp--;
        }
        else 
        {
            grounded = false;
        }

    }
    
    //shooting water balls
    if (shooting == true) 
    {
        for (int i = 0; i < m_WaterTerrain.highPoints.size(); i++)
        {
            if (RaySphereIntersection(m_mainCamera.getPosition(), m_mainCamera.getLookAt(), Vector3(m_WaterTerrain.highPoints[i].x, m_WaterTerrain.highPoints[i].y + 4, m_WaterTerrain.highPoints[i].z), 0.7))
            {
                shotBalls.push_back(m_WaterTerrain.highPoints[i]);
            }
        }
    }
    
    //shooting ground balls
    if (shooting == true)
    {

        for (int i = 0; i < m_HighpointTerrain.highPoints.size(); i++)
        {
            if (RaySphereIntersection(m_mainCamera.getPosition(), m_mainCamera.getLookAt(), Vector3(m_HighpointTerrain.highPoints[i].x-66, m_HighpointTerrain.highPoints[i].y + 4, m_HighpointTerrain.highPoints[i].z-66), 0.7))
            {
                shotGroundBalls.push_back(m_HighpointTerrain.highPoints[i]);
                if (!playOnce) {
                    m_pop->Play();
                    playOnce = true;
                }
                
            }
        }
    }
    else
    {
        playOnce = false;
    }

    //player on platform
    if (gravity == 0 && !once)
    {

        m_mainCamera.setPosition(Vector3(ridingPlatform.x, ridingPlatform.y + 3, ridingPlatform.z));
        
    }
    

    //Water terrain setting the offset as the seconds that have passed
    m_WaterTerrain.SetOffset(timer.GetTotalSeconds()*10);
    m_WaterTerrain.GenerateWaterMap(device);

    if (timer.GetTotalSeconds() > 3) 
    {
        introOn = false;
    }



    m_mainCamera.setPosition(Vector3(m_mainCamera.getPosition().x, m_mainCamera.getPosition().y + gravity, m_mainCamera.getPosition().z));

    

    if (reset) 
    {
        reset = false;
        m_mainCamera.setPosition(cameraStart);
        countdown = -1;
    }

    if (m_mainCamera.getPosition().y < -4 && !died)
    {
        died = true;
        timeDead = m_timer.GetTotalSeconds();
    }

    m_mainCamera.Update();	//camera update.
    
    m_WaterTerrain.Update();
    m_HighpointTerrain.Update();

	m_mainCameraView = m_mainCamera.getCameraMatrix();
	m_world = Matrix::Identity;

    m_batchEffect->SetView(m_mainCameraView);
    m_batchEffect->SetWorld(m_world);

	/*create our UI*/
	SetupGUI();

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

  
	if (m_input.Quit())
	{
		ExitGame();
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	
	//context->RSSetState(m_states->Wireframe());

    postProcessTexture();


    postProcess->SetSourceTexture(m_postProcessingTexture->getShaderResourceView());
    postProcess->SetEffect(filter);
    postProcess->Process(context);

    // Draw Text to the screen
    m_sprites->Begin();

    if (!died && !won) 
    {
        if (!introOn)
        {
            m_font->DrawString(m_sprites.get(), L"O", XMFLOAT2(385, 288), Colors::Yellow);
        }



        if (introOn)
        {
            m_font->DrawString(m_sprites.get(), intro, XMFLOAT2(100, 300), Colors::Yellow);
            countdown = -1;
        }
        else 
        {
            std::wstring str = std::to_wstring(countdown);
            t1 = str.c_str();

            m_font->DrawString(m_sprites.get(), t1, XMFLOAT2(30, 30), Colors::Yellow);
        }

        
        
        if (timeInt != (int)m_timer.GetTotalSeconds()) 
        {
            timeInt = (int)m_timer.GetTotalSeconds();
            countdown += 1;

        }

        if (countdown == 30)
        {
            died = true;
            timeDead = m_timer.GetTotalSeconds();
        }
    }
    else if(died)
    {
        m_font->DrawString(m_sprites.get(), lost, XMFLOAT2(300, 300), Colors::Red);
        filter = BasicPostProcess::Monochrome;

        if (timeDead + 3 <= m_timer.GetTotalSeconds()) 
        {
            died = false;
            filter = BasicPostProcess::Copy;
            reset = true;
            countdown = -1;
            
        }
    }
    else 
    {
        m_font->DrawString(m_sprites.get(), victory, XMFLOAT2(300, 300), Colors::Aquamarine);
        filter = BasicPostProcess::Sepia;
    }
        
    m_sprites->End();


	//render our GUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	

    m_deviceResources->PIXEndEvent();
    // Show the new frame.
    m_deviceResources->Present();
}


void Game::postProcessTexture()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetRenderTargetView();
    auto depthTargetView = m_deviceResources->GetDepthStencilView();

    
    
    //Set Rendering states. 
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_states->CullClockwise());

    m_postProcessingTexture->setRenderTarget(context);

    m_postProcessingTexture->clearRenderTarget(context, 0.0f, 0.0f, 1.0f, 1.0f);

    
    

    //PERLIN WATER
    m_world = Matrix::Identity; //set world back to identity

    m_BasicShaderPairWater.EnableShader(context);
    m_BasicShaderPairWater.SetShaderParameters3(context, &m_world, &m_mainCameraView, &m_projection, &m_light, m_timer.GetTotalSeconds(), m_darkBlue.Get(), m_blue.Get(), m_lightBlue.Get());
    m_WaterTerrain.Render(context);
    
    //WATER PLATFORMS
    for (int i = 0; i < m_WaterTerrain.highPoints.size(); i++)
    {
        m_world = Matrix::Identity; //set world back to identity
        Matrix platformPosition = Matrix::CreateTranslation(m_WaterTerrain.highPoints[i].x, m_WaterTerrain.highPoints[i].y, m_WaterTerrain.highPoints[i].z);

        m_world = m_world * platformPosition;

        m_ball_platColor = m_orange;

        m_BasicShaderPair.EnableShader(context);
        m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_mainCameraView, &m_projection, &m_light, 1, m_ball_platColor.Get());
        m_platform.Render(context);
    }

    for (int i = 0; i < m_WaterTerrain.highPoints.size(); i++)
    {
        m_world = Matrix::Identity; //set world back to identity
        Matrix ballPosition = Matrix::CreateTranslation(m_WaterTerrain.highPoints[i].x, m_WaterTerrain.highPoints[i].y + 4, m_WaterTerrain.highPoints[i].z);

        m_world = m_world * ballPosition;

        m_ball_platColor = m_orange;

        m_BasicShaderPair.EnableShader(context);
        m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_mainCameraView, &m_projection, &m_light, 1, m_ball_platColor.Get());
        m_targetBall.Render(context);
    }

    //PERLIN TERRAIN
    m_world = Matrix::Identity;
    Matrix terrainPosition = Matrix::CreateTranslation(-66, 0, -66);
    Matrix terrainRotation = Matrix::CreateRotationX(3.14);
    m_world = m_world * terrainPosition;

    m_BasicShaderPair.EnableShader(context);
    m_BasicShaderPair.SetShaderParameters3(context, &m_world, &m_mainCameraView, &m_projection, &m_light, m_timer.GetTotalSeconds(), m_grass.Get(), m_slope.Get(), m_rock.Get());
    m_HighpointTerrain.Render(context);


    //GROUND PLATFORMS
    if (terrainGenerated)
    {
        for (int i = 0; i < m_HighpointTerrain.highPoints.size(); i++)
        {
            m_world = Matrix::Identity; //set world back to identity
            Matrix platformPosition = Matrix::CreateTranslation(m_HighpointTerrain.highPoints[i].x-66, m_HighpointTerrain.highPoints[i].y, m_HighpointTerrain.highPoints[i].z-66);

            m_world = m_world * platformPosition;

            if (std::count(shotGroundBalls.begin(), shotGroundBalls.end(), m_HighpointTerrain.highPoints[i]))
            {
                m_ball_platColor = m_orange;
            }
            else
            {
                m_ball_platColor = m_red;
            }

            m_BasicShaderPair.EnableShader(context);
            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_mainCameraView, &m_projection, &m_light, 1, m_ball_platColor.Get());
            m_platform.Render(context);
        }

        for (int i = 0; i < m_HighpointTerrain.highPoints.size(); i++)
        {
            m_world = Matrix::Identity; //set world back to identity
            Matrix ballPosition = Matrix::CreateTranslation(m_HighpointTerrain.highPoints[i].x-66, m_HighpointTerrain.highPoints[i].y + 4, m_HighpointTerrain.highPoints[i].z-66);

            m_world = m_world * ballPosition;

            if (std::count(shotGroundBalls.begin(), shotGroundBalls.end(), m_HighpointTerrain.highPoints[i]))
            {
                m_ball_platColor = m_orange;
                
            }
            else
            {
                m_ball_platColor = m_red;
            }

            m_BasicShaderPair.EnableShader(context);
            m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_mainCameraView, &m_projection, &m_light, 1, m_ball_platColor.Get());
            m_targetBall.Render(context);
        }
    }
    

    //first platform
    m_world = Matrix::Identity; //set world back to identity
    Matrix startPosition = Matrix::CreateTranslation(-67, 0, -67);
    m_world = m_world * startPosition;

    m_BasicShaderPair.EnableShader(context);
    m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_mainCameraView, &m_projection, &m_light, 1, m_orange.Get());
    m_platform.Render(context);

    //goal platform
    m_world = Matrix::Identity; //set world back to identity
    Matrix goalPosition = Matrix::CreateTranslation(65, 0, 65);
    Matrix goalScale = Matrix::CreateScale(5);

    m_world = m_world * goalScale * goalPosition;

    m_BasicShaderPair.EnableShader(context);
    m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_mainCameraView, &m_projection, &m_light, 1, m_orange.Get());
    m_platform.Render(context);
    

    ////target ball
    //m_world = Matrix::Identity;
    //Matrix targetPosition = Matrix::CreateTranslation(0,0,0);
    //m_world = m_world * targetPosition;

    //m_BasicShaderPair.EnableShader(context);
    //m_BasicShaderPair.SetShaderParameters3(context, &m_world, &m_mainCameraView, &m_projection, &m_light, m_red.Get(), m_red.Get(), m_red.Get());
    //m_targetBall.Render(context);


    m_skyboxEffect->SetView(m_mainCameraView);
    m_sky->Draw(m_skyboxEffect.get(), m_batchInputLayout.Get());

    //post
    context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
}

bool Game::RayTriIntersect(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 dir, DirectX::SimpleMath::Vector3 p0, DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 p2)
{
    //compute plane's normal
    DirectX::SimpleMath::Vector3 p0p1 = p1 - p0;
    DirectX::SimpleMath::Vector3 p0p2 = p2 - p0;

    DirectX::SimpleMath::Vector3 N = p0p1.Cross(p0p2);

    float area2 = N.Length();

    //finding P

    //check if ray and plane are parallel
    float NdotRayDirection = N.Dot(dir);

    float epsilon = pow(10, -5);

    if (abs(NdotRayDirection) < epsilon)
    {
        return false;
    }

    //compute d parameter using equation 2
    float d = -N.Dot(p0);

    //compute t (equation 3)
    float t = -(N.Dot(origin) + d) / NdotRayDirection;

    if (t < 0)
    {
        return false;//the triangle is behind
    }

    //compute the intersection point using equation 1
    DirectX::SimpleMath::Vector3 P = origin + t * dir;

    //Step 2:inside-outside test
    DirectX::SimpleMath::Vector3 C; //vector perpedincular to triangles plane

    //edge 0
    DirectX::SimpleMath::Vector3 edge0 = p1 - p0;
    DirectX::SimpleMath::Vector3 vp0 = P - p0;
    C = edge0.Cross(vp0);

    if (N.Dot(C) < 0)
    {
        return false; // P is on the right side
    }

    //edge 1
    DirectX::SimpleMath::Vector3 edge1 = p2 - p1;
    DirectX::SimpleMath::Vector3 vp1 = P - p1;
    C = edge1.Cross(vp1);

    if (N.Dot(C) < 0)
    {
        return false; //P is on the right side
    }

    //edge 2
    DirectX::SimpleMath::Vector3 edge2 = p0 - p2;
    DirectX::SimpleMath::Vector3 vp2 = P - p2;
    C = edge2.Cross(vp2);

    if (N.Dot(C) < 0)
    {
        return false; // P is on the right side
    }

    return true;// ray hits triangle
}

bool Game::RaySphereIntersection(DirectX::SimpleMath::Vector3 og, DirectX::SimpleMath::Vector3 dest, DirectX::SimpleMath::Vector3 center, float r)
{
    //D = B^2 - 4 * A * G 
    float A = pow(dest.x - og.x, 2) + pow(dest.y - og.y, 2) + pow(dest.z - og.z, 2);
    float B = 2 * ((dest.x - og.x) * (og.x - center.x) + (dest.y - og.y) * (og.y - center.y) + (dest.z - og.z) * (og.z - center.z));
    float G = pow(og.x - center.x, 2) + pow(og.y - center.y, 2) + pow(og.z - center.z, 2) - pow(r, 2);

    float D = pow(B, 2) - 4 * A * G;

    if (D >= 0) 
    {
        return true;
    }
    
    return false;
}

bool Game::CameraPlaneIntersection(DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B, DirectX::SimpleMath::Vector3 D, DirectX::SimpleMath::Vector3 cPosition)
{
    //sphere bottom point
    //DirectX::SimpleMath::Vector3 center = DirectX::SimpleMath::Vector3(center.x, center.y, center.z);
    
    float maxCx = cPosition.x + 3;
    float minCx = cPosition.x - 3;
    float maxCz = cPosition.z + 3;
    float minCz = cPosition.z - 3;


    if (cPosition.y >= A.y - 0.5 && cPosition.y <= A.y + 0.5)
    {
        if (maxCz >= A.z && minCz <= B.z)
        {
            if (maxCx >= A.x && minCx <= D.x)
            {
                return true;
            }
        }
    }

    return false;
}



// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{

    m_audEngine->Suspend();
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();


    m_audEngine->Resume();

}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_sky = GeometricPrimitive::CreateGeoSphere(context, 2.f, 3, false);
    m_skyboxEffect = std::make_unique<DX::SkyboxEffect>(device);
    m_sky->CreateInputLayout(m_skyboxEffect.get(), m_batchInputLayout.ReleaseAndGetAddressOf());

    m_batchEffect = std::make_unique<BasicEffect>(device);
    m_batchEffect->SetVertexColorEnabled(true);

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	//setup our terrain
    m_WaterTerrain.Initialize(device, 64, 64);
    m_HighpointTerrain.Initialize(device, 64, 64);
    m_platform.InitializeBox(device, 2, 0.1, 2);


	//setup our test model
	m_BasicModel.InitializeSphere(device);
	m_BasicModel2.InitializeModel(device,"drone.obj");
	m_BasicModel3.InitializeBox(device, 10.0f, 0.1f, 10.0f);	//box includes dimensions

    m_playerBall.InitializeSphere(device);
    m_targetBall.InitializeSphere(device);
    m_targetBall2.InitializeSphere(device);

	//load and set up our Vertex and Pixel Shaders
	m_BasicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
    m_BasicShaderPairWater.InitStandard(device, L"Water_vs.cso", L"Water_ps.cso");

	//load Textures
    CreateDDSTextureFromFile(device, L"grass.dds", nullptr, m_grass.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"rock.dds", nullptr, m_rock.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"slope.dds", nullptr,	m_slope.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"red.dds", nullptr, m_red.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"orange.dds", nullptr, m_orange.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"dark_blue.dds", nullptr, m_darkBlue.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"light_blue.dds", nullptr, m_lightBlue.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"rough.dds", nullptr, m_blue.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"skybox.dds", nullptr, m_skyboxTexture.ReleaseAndGetAddressOf());

    

    m_skyboxEffect->SetTexture(m_skyboxTexture.Get());

	//Initialise Render to texture
	m_FirstRenderPass = new RenderTexture(device, 800, 600, 1, 2);	//for our rendering, We dont use the last two properties. but.  they cant be zero and they cant be the same. 
    m_postProcessingTexture = new RenderTexture(device, 800, 600, 1, 2);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f
    );

    m_skyboxEffect->SetProjection(m_projection);
    m_batchEffect->SetProjection(m_projection);
}

void Game::SetupGUI()
{

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin(" ");
        ImGui::SliderFloat("Difficulty", m_WaterTerrain.GetScale(), 0.0f, 0.5f);
        //ImGui::InputInt("Offset", m_Terrain.GetOffset(), 0, 100000);
	ImGui::End();
}




void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
	m_testmodel.reset();
    m_batchInputLayout.Reset();
    m_sky.reset();
    m_skyboxEffect.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#pragma endregion
