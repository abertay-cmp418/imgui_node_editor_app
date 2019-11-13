#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include <input/touch_input_manager.h>
#include <graphics/sprite.h>
#include "load_texture.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "Application.h"
#include <platform/d3d11/system/platform_d3d11.h>
#include <platform/d3d11/input/keyboard_d3d11.h>

ImTextureID Application_LoadTexture(const char* path)
{
	return ImGui_LoadTexture(path);
}

ImTextureID Application_CreateTexture(const void* data, int width, int height)
{
	return ImGui_CreateTexture(data, width, height);
}

void Application_DestroyTexture(ImTextureID texture)
{
	ImGui_DestroyTexture(texture);
}

int Application_GetTextureWidth(ImTextureID texture)
{
	return ImGui_GetTextureWidth(texture);
}

int Application_GetTextureHeight(ImTextureID texture)
{
	return ImGui_GetTextureHeight(texture);
}

// Our state
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	input_manager_(NULL),
	font_(NULL)
{
}

void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	InitFont();

	// initialise input manager
	input_manager_ = gef::InputManager::Create(platform_);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	gef::PlatformD3D11& platform_d3d = static_cast<gef::PlatformD3D11&>(platform_);

	ImGui_ImplWin32_Init(platform_d3d.hwnd());
	ImGui_ImplDX11_Init(platform_d3d.device(), platform_d3d.device_context());

	Application_Initialize();
}

void SceneApp::CleanUp()
{
	Application_Finalize();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete input_manager_;
	input_manager_ = NULL;

	CleanUpFont();

	delete sprite_renderer_;
	sprite_renderer_ = NULL;
}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;


	input_manager_->Update();

	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[0] = input_manager_->touch_manager()->is_button_down(0);
		io.MouseDown[1] = input_manager_->touch_manager()->is_button_down(1);
		io.MouseWheel += input_manager_->touch_manager()->mouse_rel().z() / WHEEL_DELTA;

		gef::KeyboardD3D11* keyboard_d3d11 = (gef::KeyboardD3D11*)input_manager_->keyboard();


		io.KeyShift = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LSHIFT) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RSHIFT);
		io.KeyCtrl = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LCONTROL) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RCONTROL);
		io.KeyAlt = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LALT) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RALT);


		for (int key_code_num = 0; key_code_num < gef::Keyboard::NUM_KEY_CODES; ++key_code_num)
		{
			gef::Keyboard::KeyCode key_code = (gef::Keyboard::KeyCode)key_code_num;
			int scan_code = keyboard_d3d11->GetScanCode(key_code);

			int vk_code = MapVirtualKey(scan_code, MAPVK_VSC_TO_VK_EX);

			io.KeysDown[vk_code] = keyboard_d3d11->IsKeyDown(key_code);
			if (keyboard_d3d11->IsKeyPrintable(key_code) && keyboard_d3d11->IsKeyPressed(key_code))
			{
				if (!io.KeyShift && vk_code >= 'A' && vk_code <= 'Z')
					vk_code = 'a' + (vk_code - 'A');
				io.AddInputCharacter(vk_code);
			}
		}
	}

	return true;
}




void SceneApp::Render()
{
	sprite_renderer_->Begin();

	DrawHUD();
	sprite_renderer_->End();

	ImGuiRender();
}

void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() - 120.0f, platform_.height() - 40.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);
	}
}

void SceneApp::ImGuiRender()
{
	ImGuiIO& io = ImGui::GetIO();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(io.DisplaySize);

	ImGui::Begin("Content", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus);

	Application_Frame();

	ImGui::End();

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void SceneApp::UpdateImGuiIO()
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[0] = input_manager_->touch_manager()->is_button_down(0);
	io.MouseDown[1] = input_manager_->touch_manager()->is_button_down(1);

	gef::KeyboardD3D11* keyboard_d3d11 = (gef::KeyboardD3D11*)input_manager_->keyboard();


	io.KeyShift = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LSHIFT) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RSHIFT);
	io.KeyCtrl = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LCONTROL) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RCONTROL);
	io.KeyAlt = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LALT) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RALT);


	for (int key_code_num = 0; key_code_num < gef::Keyboard::NUM_KEY_CODES; ++key_code_num)
	{
		gef::Keyboard::KeyCode key_code = (gef::Keyboard::KeyCode)key_code_num;
		int scan_code = keyboard_d3d11->GetScanCode(key_code);

		int vk_code = MapVirtualKey(scan_code, MAPVK_VSC_TO_VK_EX);

		io.KeysDown[vk_code] = keyboard_d3d11->IsKeyDown(key_code);
		if (keyboard_d3d11->IsKeyPrintable(key_code) && keyboard_d3d11->IsKeyPressed(key_code))
		{
			if (!io.KeyShift && vk_code >= 'A' && vk_code <= 'Z')
				vk_code = 'a' + (vk_code - 'A');
			io.AddInputCharacter(vk_code);
		}
	}
}