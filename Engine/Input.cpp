#include "pch.h"
#include "Input.h"


Input::Input()
{
}

Input::~Input()
{
}

void Input::Initialise(HWND window)
{
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(window);
	m_quitApp = false;

	m_GameInput.forward		= false;
	m_GameInput.back		= false;
	m_GameInput.right		= false;
	m_GameInput.left		= false;
	m_GameInput.rotRight	= false;
	m_GameInput.rotLeft		= false;
	m_GameInput.rotUp		= false;
	m_GameInput.jump		= false;
	m_GameInput.shoot		= false;

	lastX = 0;
	deltaX = 0;
	lastY = 0;
	deltaY = 0;
}

void Input::Update()
{
	auto kb = m_keyboard->GetState();	//updates the basic keyboard state
	m_KeyboardTracker.Update(kb);		//updates the more feature filled state. Press / release etc. 
	auto mouse = m_mouse->GetState();   //updates the basic mouse state
	m_MouseTracker.Update(mouse);		//updates the more advanced mouse state. 

	if (kb.Escape)// check has escape been pressed.  if so, quit out. 
	{
		m_quitApp = true;
	}

	//A key
	if (kb.A)	m_GameInput.left = true;
	else		m_GameInput.left = false;
	
	//D key
	if (kb.D)	m_GameInput.right = true;
	else		m_GameInput.right = false;

	//W key
	if (kb.W)	m_GameInput.forward	 = true;
	else		m_GameInput.forward = false;

	//S key
	if (kb.S)	m_GameInput.back = true;
	else		m_GameInput.back = false;

	//SPACE key
	if(kb.Space)m_GameInput.jump = true;
	else		m_GameInput.jump = false;

	//generate ground
	if (kb.Q)	m_GameInput.generate = true;
	else		m_GameInput.generate = false;

	//shoot
	if (mouse.leftButton) m_GameInput.shoot = true;
	else                  m_GameInput.shoot = false;

	//mouse
	float currentX = mouse.x;
	float currentY = mouse.y;

	if (currentX != lastX)
	{
		m_GameInput.rotLeft = true;
	}
	else
	{
		m_GameInput.rotLeft = false;
	}
	if (lastX != 0)
	{
		if (lastX != currentX)
		{
			deltaX = currentX - lastX;
		}
	}
	if (currentY != lastY)
	{
		m_GameInput.rotUp = true;
	}
	else
	{
		m_GameInput.rotUp = false;
	}
	if (lastY != 0)
	{
		if (lastY != currentY)
		{
			deltaY = currentY - lastY;
		}
	}

	lastX = mouse.x;
	lastY = mouse.y;
}

bool Input::Quit()
{
	return m_quitApp;
}

InputCommands Input::getGameInput()
{
	return m_GameInput;
}

float Input::getDeltaX()
{
	return deltaX;
}

float Input::getDeltaY()
{
	return deltaY;
}