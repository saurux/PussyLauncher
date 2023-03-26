#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dinput.h>
#include <tchar.h>
#include <urlmon.h>
#include <iostream>
#include <math.h>
#include <random>

#include "Render/imgui.h"
#include "Render/imgui_impl_dx9.h"
#include "Render/imgui_impl_win32.h"

#include "Inputs/OPI.h"
#include "Inputs/Lucida.h"
#include "Inputs/Logo.h"
#include "Inputs/PatternScanner.h"
#include "Inputs/PatternScannerEU.h"

#include "define/stdafx.h"

#include "IconText.h"
#include <sstream>

#include <D3dx9tex.h>
#pragma comment(lib, "D3dx9")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "urlmon.lib")

#include <windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <memory>

#define APP_VERSION "PH-0.0.1"
HWND hWnd = NULL;

using namespace std;

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

ImFont* pDefaultFont;
ImFont* pFont;

ImTextureID        myImage = NULL;
LPDIRECT3DTEXTURE9 newTexture = NULL;

HRESULT CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void cSprite(IDirect3DDevice9* m_pD3Ddev, LPCSTR szFilePath);

int WindowX = 675;
int WindowY = 480;

bool bActivated = false;
bool _CheckPoint = false;
bool _AutoUpdate = true;

std::string ZeroTime;
std::string PussyTime;
std::string ApexTime;

std::string sGamePath;

HHOOK hHandle;

namespace Tools
{
	string GetMyDocumentsPath()
	{
		char my_documents[MAX_PATH];
		HRESULT result = SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);
		return my_documents;
	}

	ImVec4 RGBAToImVec4(float r, float g, float b, float a)
	{
		return ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
	}

	namespace FuckingTools
	{
		vector<string> explode(const string& delimiter, const string& str)
		{
			vector<string> arr;

			int strleng = str.length();
			int delleng = delimiter.length();
			if (delleng == 0)
				return arr;//no change

			int i = 0;
			int k = 0;
			while (i < strleng)
			{
				int j = 0;
				while (i + j < strleng && j < delleng && str[i + j] == delimiter[j])
					j++;
				if (j == delleng)//found delimiter
				{
					arr.push_back(str.substr(k, i - k));
					i += delleng;
					k = i;
				}
				else
				{
					i++;
				}
			}
			arr.push_back(str.substr(k, i - k));
			return arr;
		}

		DWORD GetProcessID(HWND hWindow)
		{
			DWORD dwProcessId;
			GetWindowThreadProcessId(hWindow, &dwProcessId);

			return dwProcessId;
		}

		wstring ToWChar(string lpTarget)
		{
			wstring lpBuffer(lpTarget.begin(), lpTarget.end());
			return lpBuffer;
		}

		string ToAChar(wstring lpTarget)
		{
			string lpBuffer(lpTarget.begin(), lpTarget.end());
			return lpBuffer;
		}

		string GetMyDocumentsPath()
		{
			char my_documents[MAX_PATH];
			HRESULT result = SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);
			return my_documents;
		}

		string GetRandomStr(int length = rand() & 16)
		{
			std::string random(("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

			std::random_device rd;
			std::mt19937 generator(rd());

			std::shuffle(random.begin(), random.end(), generator);

			return random.substr(0, length);
		}

		string GetCurrentPath()
		{
			char buffer[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, buffer);

			return buffer;
		}

		string GetTemporaryPath()
		{
			char buffer[MAX_PATH];
			GetTempPathA(MAX_PATH, buffer);

			return buffer;
		}

		bool ExtractFile(string PATH, BYTE* FILE, DWORD dwSize)
		{
			HANDLE hNewFile = CreateFileA(PATH.c_str(), GENERIC_ALL, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

			if (GetLastError() == ERROR_FILE_EXISTS)
			{
				CloseHandle(hNewFile);
				return true;
			}

			if (hNewFile == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			DWORD NumberOfBytesWritten = 0;
			if (!WriteFile(hNewFile, FILE, dwSize, &NumberOfBytesWritten, nullptr))
				return false;

			CloseHandle(hNewFile);
			return true;
		}
	}
}

namespace ImGui {


}

namespace Console
{
	void ProgressBar()
	{
		system("cls");
		printf("Loading software...\n");
		for (int i = 0; i <= 100; i++)
		{
			Sleep(100);
			printf("%c", 219);
			//cout << i << " %" << endl;
			//system("cls");
		}
		printf("\nLoaded...");
	}
}

bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
	// Load texture from disk
	PDIRECT3DTEXTURE9 texture;
	HRESULT hr = D3DXCreateTextureFromFileA(g_pd3dDevice, filename, &texture);
	if (hr != S_OK)
		return false;

	// Retrieve description of the texture surface so we can access its size
	D3DSURFACE_DESC my_image_desc;
	texture->GetLevelDesc(0, &my_image_desc);
	*out_texture = texture;
	*out_width = (int)my_image_desc.Width;
	*out_height = (int)my_image_desc.Height;
	return true;
}

bool init = false;

int iTab = -1;

int main()
{

	bool bRename;
	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), LoadIcon(GetModuleHandle(NULL), IDC_ICON), NULL, NULL, NULL, Tools::FuckingTools::ToWChar(Tools::FuckingTools::GetRandomStr(6)).c_str(), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindowEx(/*WS_EX_TOPMOST | */WS_EX_LAYERED, wc.lpszClassName, Tools::FuckingTools::ToWChar(Tools::FuckingTools::GetRandomStr(6)).c_str(), WS_POPUP, (desktop.right / 2) - (WindowX / 2), (desktop.bottom / 2) - (WindowY / 2), WindowX, WindowY, NULL, NULL, wc.hInstance, NULL);

	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);

	if (CreateDeviceD3D(hwnd) < 0)
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	//SetWindowLong(hwnd, GWL_STYLE, 0); //remove all window styles, check MSDN for details

	if (!bRename) ::ShowWindow(hwnd, SW_SHOWDEFAULT);
	if (!bRename)::UpdateWindow(hwnd);

	//SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	if (!bRename) ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	//Console::ProgressBar();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//io.Fonts->AddFontFromMemoryTTF(Mayor, sizeof(Mayor), 20, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	io.Fonts->AddFontFromMemoryTTF(Lucida, sizeof(Lucida), 17, nullptr, io.Fonts->GetGlyphRangesCyrillic());

	ImFontConfig config;
	config.MergeMode = true;
	//config.GlyphMinAdvanceX = -13.0f; // Use if you want to make the icon monospaced
	//config.PixelSnapH = true;
	static const ImWchar icon_ranges[] = { ICON_MIN, ICON_MAX, 0 };
	io.Fonts->AddFontFromMemoryTTF(OPI, sizeof(OPI), 17, &config, icon_ranges);
	ImGui::GetIO().IniFilename = NULL;// "pussy.ini";

	ImGui::GetStyle().FrameRounding = 1.5f;
	ImGui::GetStyle().GrabRounding = 1.5f;
	ImGui::GetStyle().WindowRounding = 1.0f;
	ImGui::GetStyle().WindowBorderSize = 2.5f;
	ImGui::GetStyle().ChildBorderSize = 2.5f;
	ImGui::GetStyle().PopupBorderSize = 2.5f;
	ImGui::GetStyle().WindowTitleAlign.x = 0.50f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = Tools::RGBAToImVec4(255.f, 255.f, 255.f, 255.f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = Tools::RGBAToImVec4(53.f, 0.f, 138.f, 50.f);
	colors[ImGuiCol_ChildBg] = Tools::RGBAToImVec4(0.f, 0.f, 0.f, 56.f);
	colors[ImGuiCol_PopupBg] = Tools::RGBAToImVec4(53.f, 0.f, 138.f, 50.f);
	colors[ImGuiCol_Border] = Tools::RGBAToImVec4(156.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 255.00f);
	colors[ImGuiCol_FrameBg] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.77f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 0.80f);
	colors[ImGuiCol_TitleBg] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_TitleBgActive] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_TitleBgCollapsed] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_MenuBarBg] = Tools::RGBAToImVec4(217.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_ScrollbarBg] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_ScrollbarGrab] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_ScrollbarGrabHovered] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_ScrollbarGrabActive] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_CheckMark] = Tools::RGBAToImVec4(255.f, 255.f, 255.f, 255.f);
	colors[ImGuiCol_SliderGrab] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_SliderGrabActive] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_Button] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 190.f);
	colors[ImGuiCol_ButtonHovered] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 140.f);
	colors[ImGuiCol_ButtonActive] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_Header] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_HeaderHovered] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_HeaderActive] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_Separator] = Tools::RGBAToImVec4(53.f, 0.f, 138.f, 50.f);
	colors[ImGuiCol_SeparatorHovered] = Tools::RGBAToImVec4(53.f, 0.f, 138.f, 50.f);
	colors[ImGuiCol_SeparatorActive] = Tools::RGBAToImVec4(53.f, 0.f, 138.f, 50.f);
	colors[ImGuiCol_ResizeGrip] = Tools::RGBAToImVec4(0.f, 0.f, 0.f, 0.f);
	colors[ImGuiCol_ResizeGripHovered] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_ResizeGripActive] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_Tab] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 190.f);
	colors[ImGuiCol_TabHovered] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 140.f);
	colors[ImGuiCol_TabActive] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_TabUnfocused] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_TabUnfocusedActive] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_PlotLines] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_PlotLinesHovered] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_PlotHistogram] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_PlotHistogramHovered] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_TextSelectedBg] = Tools::RGBAToImVec4(54.f, 54.f, 54.f, 95.f);
	colors[ImGuiCol_DragDropTarget] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_NavHighlight] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_NavWindowingHighlight] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_NavWindowingDimBg] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);
	colors[ImGuiCol_ModalWindowDimBg] = Tools::RGBAToImVec4(157.f, 0.f, 255.f, 255.f);

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	//ImGui::SetNextWindowPos(ImVec2(0, 0));
	//ImVec4 clear_color = Tools::RGBAToImVec4(25.f, 25.f, 25.f, 255.f);

	DeleteFileA(string(Tools::FuckingTools::GetTemporaryPath() + "\\mrac.log").c_str());
	DeleteFileA(string(Tools::FuckingTools::GetTemporaryPath() + "\\mracdrv.log").c_str());

	int my_image_width = 0;
	int my_image_height = 0;
	bool bStarted = false;
	PDIRECT3DTEXTURE9 my_texture = NULL;

	Tools::FuckingTools::ExtractFile(Tools::FuckingTools::GetTemporaryPath() + "\\Logo.png", Logo, sizeof(Logo));
	bool ret = LoadTextureFromFile(std::string(Tools::FuckingTools::GetTemporaryPath() + "\\Logo.png").c_str(), &my_texture, &my_image_width, &my_image_height);
	IM_ASSERT(ret);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	static bool bIsOpen = true;
	static bool PussyHack, ApexHack, ZeroHack;

	ImVec4 ImageColor(255, 255, 255, 0.25f);
	ImVec4 ImageColorNet(255, 255, 255, 255);

	while (msg.message != WM_QUIT)
	{

		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}
		if (bIsOpen == false) ExitProcess(EXIT_SUCCESS);
		//Sleep(50);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(22.5f, 1.5f));
		ImGui::SetNextWindowSize(ImVec2(WindowX - 23, WindowY - 55));
		DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		//ImGui::PushFont(pDefaultFont);


		if (ImGui::Begin(u8"\ue04b" u8" Zero - Launcher [Beta Version]", &bIsOpen, dwFlag))
		{
			if (!bActivated)
			{
				ImGui::BeginChild(u8"Menu", ImVec2(WindowX - (WindowX / 2) - 150, WindowY - 96), true);

				ImGui::TextColored(ImVec4(255.f, 255.f, 255.f, 255.f), u8"Warface"); // ImGui::SameLine();
				//ImGui::SetCursorPos(ImVec2(160, 10));
				//ImGui::Text(u8"\ue092");
				if (ImGui::Button(u8"Pussy Hack", ImVec2(172, 45)))
					iTab == 0 ? iTab = -1 : iTab = 0;
				//if (ImGui::Button(u8"ZEROWARE", ImVec2(172, 45)))
					//iTab == 1 ? iTab = -1 : iTab = 1;

				ImGui::TextColored(ImVec4(255.f, 255.f, 255.f, 255.f), u8"Fortnite");  //ImGui::SameLine();
				if (ImGui::Button(u8"Fortnite-Zero", ImVec2(172, 45)))
					iTab == 2 ? iTab = -1 : iTab = 2;

				//ImGui::Spacing();
				ImGui::TextColored(ImVec4(255.f, 255.f, 255.f, 255.f), u8"Apex Legends"); // ImGui::SameLine();
				//ImGui::SetCursorPos(ImVec2(160, 181));
				//ImGui::Text(u8"\ue092");
				if (ImGui::Button(u8"Apex-Zero", ImVec2(172, 45)))
					iTab == 3 ? iTab = -1 : iTab = 3;

				//ImGui::Spacing();
				ImGui::TextColored(ImVec4(255.f, 255.f, 255.f, 255.f), u8"Launcher"); // ImGui::SameLine();
				//ImGui::SetCursorPos(ImVec2(160, 255));
				//ImGui::Text(u8"\ue0bc");
				if (ImGui::Button(u8"\ue0bc" u8" Настройки", ImVec2(172, 45)))
					iTab == 4 ? iTab = -1 : iTab = 4;

				//ImGui::TabItemButton(u8"Pussy Hack");
				//ImGui::TabItemButton(u8"ZEROWARE");
				//ImGui::TabItemButton(u8"Apex Legends");

				//ImGui::TabItemButton(u8"Free Hack");
				ImGui::SetCursorPos(ImVec2(35, 345));
				ImGui::TextColored(ImVec4(255.f, 0.f, 155.f, 255.f), u8"Версия: 0.1");

				ImGui::EndChild();

				static char szKeyBuffer[20] = u8"Лицензия";

				//switch (iTab)
				//{
				//case 0:
				if (iTab == -1)
				{
					ImGui::SetCursorPos(ImVec2(265, 65));
					ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColorNet);
				}

				else if (iTab == 0)
				{
					if (PussyHack == false)
					{
						ImGui::SetCursorPos(ImVec2(265, 65));
						ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColor);

						ImGui::SetCursorPos(ImVec2(210, 35));
						ImGui::Text(u8"\ue030" u8" Pussy Hack");
						ImGui::SetCursorPos(ImVec2(210, 65));
						ImGui::InputText(u8"", szKeyBuffer, sizeof(szKeyBuffer));
						ImGui::SetCursorPos(ImVec2(210, 91));
						if (ImGui::Button(u8"\ue0ac" u8" Активировать", ImVec2(423, 40)))
						{
							//if (Network->Authorization(szKeyBuffer, "Pussy.hack", "Pussy") == 0x1)
							//{
								//MessageBoxA(0, "Activation Successfuly!", 0, MB_ICONINFORMATION);
								//exit(EXIT_SUCCESS);
							//}
							//else
							//{
								MessageBoxA(0, "Error!", 0, MB_ICONINFORMATION);
								ImGui::SetCursorPos(ImVec2(210, 133));
								ImGui::TextColored(ImVec4(255, 0, 0, 255), u8"Не удалось активировать!");
							//}
						}
						ImGui::SetCursorPos(ImVec2(210, 134));
						if (ImGui::Button(u8"\ue0c2" u8" Приобрести", ImVec2(423, 40)))
						{
							//ShellExecuteA(0, "open", "https://zeroware.ru/shop", 0, 0, 0);
						}
					}
					else
					{
						ImGui::SetCursorPos(ImVec2(265, 65));
						ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColor);

						// Activated
						///if (!bStarted)
						//{
							//CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Online, 0, 0, 0);
							//bStarted = true;
						//}
						// Create Session
						ImGui::SetCursorPos(ImVec2(210, 35));
						ImGui::Text(u8"\ue030" u8" Pussy Hack"); ImGui::SameLine();
						ImGui::SetCursorPos(ImVec2(500, 35));
						ImGui::Text(std::string(u8"\ue027" u8" " + PussyTime).c_str());

						ImGui::SetCursorPos(ImVec2(210, 60));

						if (ImGui::Button(u8"\ue0a9" u8" Запустить", ImVec2(423, 40)))
						{
							//ImGui::OpenPopup(u8"Внедрение");		
							//ImGui::CloseCurrentPopup();
						}

						if (ImGui::BeginPopupModal(u8"Внедрение"))
						{
							ImGui::TextColored(ImVec4(255, 255, 255, 255), u8"Ожидание запуска игры!");
							//Sleep(2500);

							//HANDLE pThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)FindWarface, 0, 0, 0);

							if (ImGui::Button(u8"Закрыть", ImVec2(225, 25)))
							{
								//CloseHandle(pThread);
								ImGui::CloseCurrentPopup();
							}

							ImGui::EndPopup();
						}

						ImGui::SetCursorPos(ImVec2(210, 105));
						ImGui::Text(u8"\ue034" u8" Вы можете запустить чит\n до запуска или после запуска Warface");
						ImGui::SetCursorPos(ImVec2(210, 157));
						ImGui::Text(u8"\ue004" u8" Работает на DirectX 11");

					}
				}
				//case 1:
				else if (iTab == 1)
				{
					if (ZeroHack == false)
					{
						ImGui::SetCursorPos(ImVec2(265, 65));
						ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColor);

						ImGui::SetCursorPos(ImVec2(210, 35));
						ImGui::Text(u8"ZEROWARE - Warface");
						ImGui::SetCursorPos(ImVec2(210, 65));
						ImGui::InputText(u8"", szKeyBuffer, sizeof(szKeyBuffer));
						ImGui::SetCursorPos(ImVec2(210, 91));
						if (ImGui::Button(u8"\ue0ac" u8" Активировать", ImVec2(423, 40)))
						{
							//if (Network->Authorization(szKeyBuffer, "zero.hack", "Warface") == 0x1)
							//{
								MessageBoxA(0, "Activation Successfuly!", 0, MB_ICONINFORMATION);
								exit(EXIT_SUCCESS);
							//}
							//else
							//{
								//MessageBoxA(0, "Error!", 0, MB_ICONINFORMATION);
							//}
						}
						ImGui::SetCursorPos(ImVec2(210, 134));
						if (ImGui::Button(u8"\ue0c2" u8" Приобрести", ImVec2(423, 40)))
						{
							//ShellExecuteA(0, "open", "https://zeroware.ru/shop", 0, 0, 0);
						}
					}
					else
					{
						ImGui::SetCursorPos(ImVec2(265, 65));
						ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColor);

						// Activated
						// Network->Online("zero.hack");
						// Create Session
						ImGui::SetCursorPos(ImVec2(210, 35));
						ImGui::Text(u8"\ue030" u8" ZEROWARE - Warface"); ImGui::SameLine();
						ImGui::SetCursorPos(ImVec2(500, 35));
						ImGui::Text(std::string(u8"\ue027" u8" " + ZeroTime).c_str());
						ImGui::SetCursorPos(ImVec2(210, 60));
						if (ImGui::Button(u8"\ue0a9" u8" Запустить [RU]", ImVec2(423, 40)))
						{
						}
						ImGui::SetCursorPos(ImVec2(210, 103));
						if (ImGui::Button(u8"\ue0a9" u8" Запустить [EU]", ImVec2(423, 40)))
						{
						}

						ImGui::SetCursorPos(ImVec2(210, 160));
						ImGui::Text(u8"\ue034" u8" Вы можете запустить чит\n до запуска или после запуска Warface");
						ImGui::SetCursorPos(ImVec2(210, 205));
						ImGui::Text(u8"\ue004" u8" Объязательно запустите версию игры\nDirectX9 а также поставьте оконный режим\nбез рамки");
					}
				}
				//case 2:
				else if (iTab == 2)
				{
					ImGui::SetCursorPos(ImVec2(265, 65));
					ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColor);
					// Fortnite
					ImGui::SetCursorPos(ImVec2(210, 40));
					if (ImGui::Button(u8"\ue054" u8" Coming soon", ImVec2(423, 40)))
					{

					}
					ImGui::SetCursorPos(ImVec2(210, 85));
					//ImGui::Text(u8"Пароль - \"zeroware.ru\" или \"zero\" \nбез кавычек!");
				}
				//}
				else if (iTab == 3)
				{
					if (ApexHack == false)
					{
						ImGui::SetCursorPos(ImVec2(265, 65));
						ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColor);

						ImGui::SetCursorPos(ImVec2(210, 35));
						ImGui::Text(u8"ZEROWARE - Apex Legends");
						ImGui::SetCursorPos(ImVec2(210, 65));
						ImGui::InputText(u8"", szKeyBuffer, sizeof(szKeyBuffer));
						ImGui::SetCursorPos(ImVec2(210, 91));
						if (ImGui::Button(u8"\ue0ac" u8" Активировать", ImVec2(423, 40)))
						{
							//Network->Authorization(szKeyBuffer, "zeroware.bin", "Apex");
						}
						ImGui::SetCursorPos(ImVec2(210, 134));
						if (ImGui::Button(u8"\ue0c2" u8" Приобрести", ImVec2(423, 40)))
						{
							//ShellExecuteA(0, "open", "https://zeroware.ru/shop", 0, 0, 0);
						}
					}
					else
					{
						ImGui::SetCursorPos(ImVec2(265, 65));
						ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColor);

						// Activated
						//Network->Online("apex.hack");
						// Create Session
						ImGui::SetCursorPos(ImVec2(210, 35));
						ImGui::Text(u8"ZEROWARE - Apex Legends");
						ImGui::SetCursorPos(ImVec2(210, 60));
						if (ImGui::Button(u8"Запустить", ImVec2(423, 40)))
						{

						}
					}
				}

				else if (iTab == 4)
				{
					ImGui::SetCursorPos(ImVec2(265, 65));
					ImGui::Image((void*)my_texture, ImVec2(300, 300), ImVec2(0, 0), ImVec2(1, 1), ImageColor);

					ImGui::SetCursorPos(ImVec2(210, 40));
					ImGui::Checkbox(u8"Авто-обновление", &_AutoUpdate);
					ImGui::SetCursorPos(ImVec2(210, 75));
					ImGui::Button(u8"Проверить на наличие обновлении", ImVec2(423, 40));
				}
			}

			else
			{

				if (ImGui::Button(u8"\ue0a9" u8" Запустить", ImVec2(WindowX - 40, 30)))
				{

				}

			}


			ImGui::End();
			//}
		}

		ImGui::EndFrame();

		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) ResetDevice();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}

void cSprite(IDirect3DDevice9* m_pD3Ddev, LPCSTR szFilePath)
{
	D3DXIMAGE_INFO pInfo;
	ZeroMemory(&pInfo, sizeof(D3DXIMAGE_INFO));

	if (FAILED(D3DXGetImageInfoFromFileA(szFilePath, &pInfo)))
		MessageBoxA(NULL, "Cannot get image info", "Error", MB_ICONERROR);

	if (FAILED(D3DXCreateTextureFromFileExA(m_pD3Ddev, szFilePath, pInfo.Width, pInfo.Height,
		pInfo.MipLevels, NULL, pInfo.Format, D3DPOOL_MANAGED, D3DX_DEFAULT, NULL, NULL,
		NULL, NULL, &newTexture)))
		MessageBoxA(NULL, "Cannot create image texture", "Error", MB_ICONERROR);

}

HRESULT CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) return E_FAIL;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0) return E_FAIL;

	return S_OK;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL) IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) return 0;// Disable ALT application menu
		break;

	case WM_QUIT:
		//onexit();
		PostQuitMessage(0);
	case WM_CLOSE:
		//onexit();
		DestroyWindow(hWnd);
		break;
	case WM_NCHITTEST:
	{
		ImVec2 Shit = ImGui::GetMousePos();
		if (Shit.y < 25 && Shit.x < WindowX - 25)
		{
			LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
			if (hit == HTCLIENT) hit = HTCAPTION;
			return hit;
		}
		else break;
	}
	case WM_DESTROY:
		//onexit();
		PostQuitMessage(0);
		return 0;
	default:
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
