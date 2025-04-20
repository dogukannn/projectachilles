#include <iostream>

//#define DEBUG_CAMERA_LOCATION //uncomment to log camera data

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>
#include <fstream>
#include <vector>

#define GLM_DEPTH_ZERO_TO_ONE
#include <chrono>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_sdl2.h>
#include <glm/matrix.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

#include "ClientThread.h"
#include "ServerThread.h"

#include "ConstantBuffer.h"
#include "DynamicRootSignature.h"
#include "pch.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Tilemap.h"
#include "Unit.h"

#include "dxri.h"

// Global variables for the window and DirectX
SDL_Window* GWindow = nullptr;
SDL_Renderer* GWindowRenderer = nullptr;
HWND GWindowHandle = nullptr;

struct SceneCB
{
	glm::mat4 VP;
	glm::vec3 eye;
	float time;
};



// Create and initialize the window using SDL
bool InitializeWindow(int width, int height)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "Failed to initialize SDL" << std::endl;
        return false;
    }


    //SDL_ShowCursor(SDL_TRUE);

    // Create window
    GWindow = SDL_CreateWindow("DirectX12 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (GWindow == nullptr)
    {
        std::cout << "Failed to create SDL window" << std::endl;
        return false;
    }

	Uint32 render_flags = SDL_RENDERER_ACCELERATED; 

	// creates a renderer to render our images 
	GWindowRenderer = SDL_CreateRenderer(GWindow, -1, render_flags); 


    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(GWindow, &wmInfo);
    GWindowHandle = wmInfo.info.win.window;

    //SDL_ShowCursor(SDL_ENABLE);
    //SDL_Grab
    auto c = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    SDL_SetCursor(c);

    SDL_SetWindowGrab(GWindow, SDL_TRUE);


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForD3D(GWindow);

    
    //SDL_SetWindowGrab(GWindow, SDL_TRUE);
	return true;
}

void CopyTexturePlain(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES dstState, ID3D12Resource* dstTexture, D3D12_RESOURCE_STATES srcState, ID3D12Resource* srcTexture)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srcTexture, srcState, D3D12_RESOURCE_STATE_COPY_SOURCE));
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dstTexture, dstState, D3D12_RESOURCE_STATE_COPY_DEST));

	D3D12_BOX box;
	box.right = dstTexture->GetDesc().Width;
	box.bottom = dstTexture->GetDesc().Height;
	box.back = 1;
	box.front = 0;
	box.left = 0;
	box.top = 0;

	D3D12_TEXTURE_COPY_LOCATION destLoc = {};
	destLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destLoc.pResource = dstTexture;
	destLoc.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
	srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	srcLoc.pResource = srcTexture;
	srcLoc.SubresourceIndex = 0;

	commandList->CopyTextureRegion(&destLoc, 0, 0, 0, &srcLoc, &box);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srcTexture, D3D12_RESOURCE_STATE_COPY_SOURCE, srcState));
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dstTexture, D3D12_RESOURCE_STATE_COPY_DEST, dstState));
}

glm::vec3 rayCast(double xpos, double ypos, glm::mat4 projection, glm::mat4 view, Camera& cam) {
    // converts a position from the 2d xpos, ypos to a normalized 3d direction
    float x = (2.0f * xpos) / 800.f - 1.0f;
    float y = 1.0f - (2.0f * ypos) / 800.f;
    
    //borrowed from net
    //float z = 1.0f;
    //glm::vec3 ray_nds = glm::vec3(x, y, z);
    //glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
    //// eye space to clip we would multiply by projection so
    //// clip space to eye space is the inverse projection
    //glm::vec4 ray_eye = inverse(projection) * ray_clip;
    //// convert point to forwards
    //ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
    //// world space to eye space is usually multiply by view so
    //// eye space to world space is inverse view
    //glm::vec4 inv_ray_wor = (inverse(view) * ray_eye);
    //glm::vec3 ray_wor = glm::vec3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
    //ray_wor = normalize(ray_wor);
    //return ray_wor;

    //my implementation #####################
	float top = tan(cam.fov * 0.5) * cam.nearz;
	float bottom = -top;

	float left = cam.aspect * bottom;
	float right = cam.aspect * top;

    auto rightV = normalize(glm::cross(cam.EyeDir, cam.Up));
    auto upV = normalize(glm::cross(rightV, cam.EyeDir));

    auto pointOnNearPlane =
        + cam.EyeDir * cam.nearz
        + upV * y * top
        + rightV * x * right;
    auto dir = pointOnNearPlane;
    dir = normalize(dir);
    return dir;
	//#####################

}

inline std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    bool exists = (bool)file;

    if (!exists || !file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
};

int main(int argc, char* argv[]) 
{
    MessageQueues<MoveEvent>* netThread;
	if(argc > 1)
	{
        auto clientThread = new ClientThread;
        clientThread->Start();
        netThread = clientThread;
	}
    else
    {
        auto serverThread = new ServerThread;
        serverThread->Start();
        netThread = serverThread;
    }

    const int windowWidth = 800;
    const int windowHeight = 800;

    if (!InitializeWindow(windowWidth, windowHeight))
    {
        return 1;
    }

	ID3D12Debug* debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();

    // Select DirectX12 Physical Adapter
    IDXGIFactory4* factory = nullptr;
    ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)));


    IDXGIAdapter1* adapter = nullptr;
    for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
        adapter->Release();
    }

   
    // Create DirectX12 device
    //ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));

    DXRI dxri;
    dxri.Initialize();
    ID3D12Device* device = dxri.Device;
	
	//init imgui
	ID3D12DescriptorHeap* g_pd3dSrvDescHeap = dxri.CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	ImGui_ImplDX12_Init(dxri.Device, 2,
			DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
			g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
			g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    // Create the command queue and allocator
    ID3D12CommandQueue* commandQueue = dxri.CreateCommandQueue();
    ID3D12CommandAllocator* commandAllocator = dxri.CreateCommandAllocator();

    UINT frameIndex = 0;
    HANDLE fenceEvent;
    ID3D12Fence* fence = dxri.CreateFence();
    UINT64 fenceValue = 0;

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

    static const UINT backbufferCount = 2;
    UINT currentBuffer;

    IDXGISwapChain3* swapchain = dxri.CreateSwapChain(windowWidth, windowHeight, backbufferCount, GWindowHandle, commandQueue);

    ID3D12DescriptorHeap* renderTargetViewHeap = dxri.CreateDescriptorHeap(backbufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    std::vector<ID3D12Resource*> renderTargets = dxri.CreateRenderTargets(renderTargetViewHeap, swapchain, backbufferCount);

	//create depth stencil
    ID3D12DescriptorHeap* dsDescHeap = dxri.CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    ID3D12Resource* depthStencilBuffer = dxri.CreateDepthBuffer(windowWidth, windowHeight, dsDescHeap);

    Tilemap tilemap;
    tilemap.Initialize(&dxri, 10, 5.0f, {-25,0,-25}, true);
    tilemap.name = "tilemap";

    Tilemap water;
    water.Initialize(&dxri, 200, 0.5, {-50, -3, -50});
    water.name = "water";

    Unit unit;
    unit.Initialize(&dxri);
    unit.location = glm::vec3(5, 1, 5);
    unit.Target = glm::vec3(5, 1, 5);
    unit.name = "unit";


    Unit unit2;
    unit2.Initialize(&dxri);
    unit2.location = glm::vec3(10, 1, 10);
    unit2.Target = glm::vec3(10, 1, 10);
    unit2.name = "unit2";


    Mesh cubeMesh;
    cubeMesh.loadFromObj(&dxri, "../Assets/cube.obj");

    //vertices for fullscreen triangle
    Vertex a = { {-3.0f, -1.0f, 0.0f}, {3.f, 3.f, 3.f}, {3.f, 3.f, 3.f}, {3.f, 3.f} };
    Vertex b = { {1.0f, -1.0f, 0.0f}, {3.f, 3.f, 3.f}, {3.f, 3.f, 3.f}, {3.f, 3.f} };
    Vertex c = { {1.0f, 3.0f, 0.0f}, {3.f, 3.f, 3.f}, {3.f, 3.f, 3.f}, {3.f, 3.f} };
    std::vector<Vertex> tri = { a, b, c };
    Mesh triangle;
    triangle.loadFromVertices(&dxri, tri);

    //init scene
    glm::vec3 StartEye = glm::vec3(7.5f, 30.0f, 0.0f);
    glm::vec3 StartEyeDir = normalize(glm::vec3(0.0f, -0.9f, 0.4f));
    glm::vec3 StartUp = glm::vec3(0.f, 1.f, 0.f);

    Camera cam;
    cam.Eye = StartEye;
    cam.EyeDir = StartEyeDir;
	cam.Up = StartUp;

    ConstantBuffer sceneBuffer;
    sceneBuffer.Initialize(&dxri, sizeof(SceneCB));
    UINT8* sceneBufferMapped = sceneBuffer.Map();

    SceneCB sceneCB;
    sceneCB.VP = cam.GetVPMatrix();
    sceneCB.eye = cam.Eye;

	memcpy(sceneBufferMapped, &sceneCB, sizeof(sceneCB));

    VertexShader triangleVertexShader(L"../Assets/triangle.vert.hlsl");
    PixelShader trianglePixelShader(L"../Assets/triangle.px.hlsl");

	VertexShader noopVertexShader(L"../Assets/noop.vert.hlsl");

    VertexShader waterVertexShader(L"../Assets/water.vert.hlsl");
    PixelShader waterPixelShader(L"../Assets/water.px.hlsl");

	Pipeline pipeline;
	pipeline.Initialize(&dxri, &triangleVertexShader, &trianglePixelShader);

	Pipeline waterPipeline;
	waterPipeline.Initialize(&dxri, &waterVertexShader, &waterPixelShader);

    Texture texture;
    texture.LoadFromFile(device, commandQueue, L"../Assets/lost_empire-RGBA.png");
    pipeline.BindTexture(device, "g_texture", &texture);

    Material defaultMaterial;
    defaultMaterial.Initialize(&pipeline);

    Material waterMaterial;
    waterMaterial.Initialize(&waterPipeline);


    Scene scene;
    scene.Objects.insert({ &tilemap, &defaultMaterial });
    scene.Objects.insert({ &unit, &defaultMaterial });
    scene.Objects.insert({ &unit2, &defaultMaterial });
    scene.Objects.insert({ &water, &waterMaterial });
  

	ID3D12GraphicsCommandList* commandList = dxri.CreateGraphicsCommandList(commandAllocator, pipeline.PipelineState);
    commandList->Close();

	std::chrono::time_point<std::chrono::system_clock> startTime;
	startTime = std::chrono::system_clock::now();

	std::chrono::time_point<std::chrono::system_clock> beginTime;
	beginTime = startTime;

	frameIndex = swapchain->GetCurrentBackBufferIndex();
    SDL_Event event;
    bool quit = false;

    static const glm::vec3 forward(0.f,0.f,1.f);
    static const glm::vec3 right(-1.f,0.f,0.f);
    bool drag = false;
    ImVec2 dragStart(0,0);
    ImVec2 dragEnd(0,0);
    bool dragFinished = false;

    D3D12_VIEWPORT viewport;
    D3D12_RECT surfaceSize;

    surfaceSize.left = 0;
    surfaceSize.top = 0;
    surfaceSize.right = windowWidth;
    surfaceSize.bottom = windowHeight;

    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = windowWidth;
    viewport.Height = windowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    
    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


    while (!quit)
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

		//ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        //bool open = true;
		//ImGui::ShowDemoWindow(&open);


		std::chrono::time_point<std::chrono::system_clock> currentTime;
		currentTime = std::chrono::system_clock::now();
        auto delta = currentTime - startTime;
        float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(delta).count() / 1000.f;
        startTime = currentTime;
        bool d = false;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
            {
                quit = true;
            }

			if (event.type == SDL_KEYDOWN)
            {
	            switch (event.key.keysym.sym)
	            {
					case SDLK_r:
						cam.Eye = StartEye;
						cam.EyeDir =  StartEyeDir;
						cam.Up = StartUp;
						break;
	            }
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if(event.button.button == SDL_BUTTON_RIGHT)
                {
					int x, y;
					SDL_GetMouseState(&x, &y);
                    auto dir = rayCast(x, y, cam.GetPerspectiveMatrix(), cam.GetViewMatrix(), cam);
					auto f = (1.0f - cam.Eye.y) / dir.y;
                    auto target = (cam.Eye + dir * f);
                    for(auto [unit,_] : scene.Objects)
                    {
	                    if(unit->selected)
	                    {
                            MoveEvent me;
                            me.UnitName = unit->name;
                            me.Target = target;
							netThread->PushOutgoing(me);
	                    }
                    }
                    scene.SetTargetOfSelectedUnits(target);
                }
                else
                {
					std::cout << "doiwnnn" << std::endl;
					int x, y;
					SDL_GetMouseState(&x, &y);
					dragStart = ImVec2(x, y);
					drag = true;
                }
            }
           

            if(event.type == SDL_MOUSEBUTTONUP)
            {
                if(event.button.button == SDL_BUTTON_RIGHT)
                {
	                
                }
                else
                {
					drag = false;
					dragFinished = true;
                }

            }
		}

        if(drag)
        {
			int x, y;
			SDL_GetMouseState(&x, &y);
			dragEnd = ImVec2(x, y);
        }


		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

        const Uint8* sdl_keyboard_state = SDL_GetKeyboardState(NULL);

        bool windowFocused = SDL_GetWindowFlags(GWindow) & SDL_WINDOW_INPUT_FOCUS;

        cam.HandleInput(sdl_keyboard_state, mouseX, mouseY, windowFocused, deltaTime);



        float speed = 0.33f;
        if(drag)
        {
			auto dlist = ImGui::GetForegroundDrawList();
            int xmin = std::min(dragStart.x, dragEnd.x);
            int xmax = std::max(dragStart.x, dragEnd.x);
            int ymin = std::min(dragStart.y, dragEnd.y);
            int ymax = std::max(dragStart.y, dragEnd.y);
			dlist->AddRectFilled(ImVec2(xmin,ymin), ImVec2(xmax,ymax), IM_COL32(50, 168, 141, 180));
			dlist->AddRect(ImVec2(xmin,ymin), ImVec2(xmax,ymax), IM_COL32(30, 120, 90, 180), 0, 0, 3);
        }

        MoveEvent msg;
        while(netThread->PopReceive(msg))
        {
			for(auto [unit,_] : scene.Objects)
			{
				if(unit->name == msg.UnitName)
				{
                    unit->SetTarget(msg.Target);
				}
			}
        }

        sceneCB.VP = cam.GetVPMatrix();
        sceneCB.eye = cam.Eye;

		auto now = std::chrono::system_clock::now();
		std::chrono::duration<float, std::ratio<1,1>> diff = now - beginTime;
		sceneCB.time = diff.count();

#ifdef DEBUG_CAMERA_LOCATION
		//std::cerr << "\r" << static_cast<int>((static_cast<double>(imageHeight - j) / imageHeight) * 100.0) << "% of file write is completed         " << std::flush;
        std::cout << "eye " << eye.x << " " << eye.y << " " << eye.z << std::endl;
        std::cout << "eye dir " << eye_dir.x << " " << eye_dir.y << " " << eye_dir.z << std::endl;
        std::cout << "up " << up.x << " " << up.y << " " << up.z << std::endl;
        std::cout << "==========================================================" << std::endl;
#endif
		memcpy(sceneBufferMapped, &sceneCB, sizeof(SceneCB));

		ThrowIfFailed(commandAllocator->Reset());

		ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));

        //pipeline.SetPipelineState(commandList);

        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		D3D12_CPU_DESCRIPTOR_HANDLE
			rtvHandle2(renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
		rtvHandle2.ptr = rtvHandle2.ptr + (frameIndex * rtvDescriptorSize);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescHeap->GetCPUDescriptorHandleForHeapStart());

        commandList->OMSetRenderTargets(1, &rtvHandle2, FALSE, &dsvHandle);

        commandList->ClearDepthStencilView(dsDescHeap->GetCPUDescriptorHandleForHeapStart(),
                                           D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &surfaceSize);
		commandList->ClearRenderTargetView(rtvHandle2, clearColor, 0, nullptr);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


        if(dragFinished)
        {
            scene.SelectUnits({dragStart.x / 799.f, dragStart.y / 799.f}, {dragEnd.x / 799.f, dragEnd.y / 799.f}, cam);
            dragFinished = false;
        }

        scene.Update(deltaTime);
        scene.Draw(commandList, pipeline, &sceneBuffer);

        ImGui::Render();
		commandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		ThrowIfFailed(commandList->Close());

		ID3D12CommandList* ppCommandLists[] = {commandList};
		commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		 swapchain->Present(0, 0);

		// Signal and increment the fence value.
		ThrowIfFailed(commandQueue->Signal(fence, fenceValue++));

		// Wait until the previous frame is finished.
		if (fence->GetCompletedValue() < fenceValue - 1)
		{
			ThrowIfFailed(fence->SetEventOnCompletion(fenceValue - 1, fenceEvent));
			WaitForSingleObject(fenceEvent, INFINITE);
		}

		frameIndex = swapchain->GetCurrentBackBufferIndex();

    }

    SDL_DestroyWindow(GWindow);
    SDL_Quit();

    // Cleanup
    commandQueue->Release();
    device->Release();
    adapter->Release();
    factory->Release();

    return 0;
}
