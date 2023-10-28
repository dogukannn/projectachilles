#include <iostream>

//#define DEBUG_CAMERA_LOCATION //uncomment to log camera data

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>
#include <fstream>
#include <vector>

#define GLM_DEPTH_ZERO_TO_ONE
#include <chrono>
#include <glm/matrix.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
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

// Global variables for the window and DirectX
SDL_Window* GWindow = nullptr;
SDL_Renderer* GWindowRenderer = nullptr;
HWND GWindowHandle = nullptr;
ID3D12Device* GDevice = nullptr;

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

	//SDL_SetRelativeMouseMode(SDL_TRUE);

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
    ID3D12Device* device = nullptr;
    ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));
    GDevice = device;

    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);
    
    std::cout << "DirectX12 device created" << std::endl;
    std::wcout << "Adapter: " << desc.Description << std::endl;
    std::cout << "Vendor ID: " << desc.VendorId << std::endl;
    std::cout << "Device ID: " << desc.DeviceId << std::endl;
    std::cout << "Dedicated Video Memory: " << desc.DedicatedVideoMemory << std::endl;

    // Create the command queue
    ID3D12CommandQueue* commandQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    ID3D12CommandAllocator* commandAllocator;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));

    UINT frameIndex = 0;
    HANDLE fenceEvent;
    ID3D12Fence* fence;
    UINT64 fenceValue = 0;

    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	 fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

    static const UINT backbufferCount = 2;
    UINT currentBuffer;
    ID3D12DescriptorHeap* renderTargetViewHeap;
    ID3D12Resource* renderTargets[backbufferCount];
    UINT rtvDescriptorSize;

    IDXGISwapChain1* swapchain1;
    IDXGISwapChain3* swapchain;

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

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.BufferCount = backbufferCount;
    swapchainDesc.Width = windowWidth;
    swapchainDesc.Height = windowHeight;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count = 1;

    ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue, GWindowHandle, &swapchainDesc, NULL, NULL, &swapchain1));

    ThrowIfFailed(swapchain1->QueryInterface(IID_PPV_ARGS(&swapchain)));

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = backbufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&renderTargetViewHeap)));

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT n = 0; n < backbufferCount; n++)
    {
        ThrowIfFailed(swapchain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
        device->CreateRenderTargetView(renderTargets[n], nullptr, rtvHandle);
        rtvHandle.ptr += (1 * rtvDescriptorSize);
    }

    ID3D12DescriptorHeap* sideRenderTargetViewHeap;
    ID3D12Resource* backDepthRenderTargets[backbufferCount];
    ID3D12Resource* frontDepthRenderTargets[backbufferCount];

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC sideRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
				DXGI_FORMAT_R32_FLOAT,
				windowWidth,
				windowHeight,
				1,
				1,
				1);
    sideRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE sideRTClearValue = {};
    sideRTClearValue.Format = DXGI_FORMAT_R32_FLOAT;
    sideRTClearValue.Color[0] = 0.0f;

    for(int i = 0; i < backbufferCount; i++)
    {
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&sideRTDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&sideRTClearValue,
			IID_PPV_ARGS(&backDepthRenderTargets[i])
		));
		backDepthRenderTargets[i]->SetName(L"back depth write targets");

		ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&sideRTDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&sideRTClearValue,
			IID_PPV_ARGS(&frontDepthRenderTargets[i])
		));
		frontDepthRenderTargets[i]->SetName(L"front depth write targets");
    }

    D3D12_DESCRIPTOR_HEAP_DESC sideRtvHeapDesc = {};
    sideRtvHeapDesc.NumDescriptors = backbufferCount * 2;
    sideRtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    sideRtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&sideRtvHeapDesc, IID_PPV_ARGS(&sideRenderTargetViewHeap)));

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE sideRtvHandle(sideRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT n = 0; n < backbufferCount; n++)
    {
        device->CreateRenderTargetView(backDepthRenderTargets[n], nullptr, sideRtvHandle);
        sideRtvHandle.ptr += (1 * rtvDescriptorSize);
    }
    for (UINT n = 0; n < backbufferCount; n++)
    {
        device->CreateRenderTargetView(frontDepthRenderTargets[n], nullptr, sideRtvHandle);
        sideRtvHandle.ptr += (1 * rtvDescriptorSize);
    }
    
	//create depth stencil
    ID3D12Resource* depthStencilBuffer;
    ID3D12DescriptorHeap* dsDescriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap)));

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
    ));
	dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    //Mesh mesh;
    //mesh.loadFromObj(device, "../Assets/graveyard.obj");

    Tilemap tilemap;
    tilemap.Initialize(9, 5.0f);

    Unit unit;
    unit.Initialize();

    Scene scene;
    scene.Objects.push_back(&tilemap);
    scene.Objects.push_back(&unit);


    Mesh cubeMesh;
    cubeMesh.loadFromObj(device, "../Assets/cube.obj");

    //vertices for fullscreen triangle
    Vertex a = { {-3.0f, -1.0f, 0.0f}, {3.f, 3.f, 3.f}, {3.f, 3.f, 3.f}, {3.f, 3.f} };
    Vertex b = { {1.0f, -1.0f, 0.0f}, {3.f, 3.f, 3.f}, {3.f, 3.f, 3.f}, {3.f, 3.f} };
    Vertex c = { {1.0f, 3.0f, 0.0f}, {3.f, 3.f, 3.f}, {3.f, 3.f, 3.f}, {3.f, 3.f} };
    std::vector<Vertex> tri = { a, b, c };
    Mesh triangle;
    triangle.loadFromVertices(device, tri);

    //init scene
    glm::vec3 StartEye = glm::vec3(7.5f, 30.0f, 0.0f);
    glm::vec3 StartEyeDir = normalize(glm::vec3(0.0f, -0.9f, 0.4f));
    glm::vec3 StartUp = glm::vec3(0.f, 1.f, 0.f);

    Camera cam;
    cam.Eye = StartEye;
    cam.EyeDir = StartEyeDir;
	cam.Up = StartUp;

    ConstantBuffer sceneBuffer;
    sceneBuffer.Initialize(device, sizeof(SceneCB));
    UINT8* sceneBufferMapped = sceneBuffer.Map();

    SceneCB sceneCB;
    sceneCB.VP = cam.GetVPMatrix();
    sceneCB.eye = cam.Eye;

	memcpy(sceneBufferMapped, &sceneCB, sizeof(sceneCB));

    VertexShader triangleVertexShader(L"../Assets/triangle.vert.hlsl");
    PixelShader trianglePixelShader(L"../Assets/triangle.px.hlsl");

	VertexShader noopVertexShader(L"../Assets/noop.vert.hlsl");

	Pipeline pipeline;
	pipeline.Initialize(device, &triangleVertexShader, &trianglePixelShader);

    Texture texture;
    texture.LoadFromFile(device, commandQueue, L"../Assets/lost_empire-RGBA.png");
    pipeline.BindTexture(device, "g_texture", &texture);

	ID3D12GraphicsCommandList* commandList;
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
											commandAllocator, pipeline.PipelineState,
											IID_PPV_ARGS(&commandList)));
    commandList->Close();

	std::chrono::time_point<std::chrono::system_clock> startTime;
	startTime = std::chrono::system_clock::now();

	frameIndex = swapchain->GetCurrentBackBufferIndex();
    SDL_Event event;
    bool quit = false;

    float speed = 0.03f;
    static const glm::vec3 forward(0.f,0.f,1.f);
    static const glm::vec3 right(-1.f,0.f,0.f);
    bool captureDir = false;

    while (!quit)
    {
		std::chrono::time_point<std::chrono::system_clock> currentTime;
		currentTime = std::chrono::system_clock::now();
        auto delta = currentTime - startTime;
        float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(delta).count() / 1000.f;
        startTime = currentTime;
        bool d = false;
        while (SDL_PollEvent(&event))
        {
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
                        captureDir = false;
						break;
	            }
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (!captureDir)
                    SDL_GetRelativeMouseState(nullptr, nullptr);
                captureDir = true;
            }
		}
        const Uint8* a = SDL_GetKeyboardState(NULL);

        if(a[SDL_SCANCODE_D])
			cam.Eye += normalize(glm::cross(cam.EyeDir, cam.Up)) * speed * deltaTime;
        if(a[SDL_SCANCODE_W])
			cam.Eye += normalize(cam.EyeDir) * speed * deltaTime;
        if(a[SDL_SCANCODE_S])
			cam.Eye -= normalize(cam.EyeDir) * speed * deltaTime;
        if(a[SDL_SCANCODE_A])
			cam.Eye -= normalize(glm::cross(cam.EyeDir, cam.Up)) * speed * deltaTime;

        if(captureDir && (SDL_GetWindowFlags(GWindow) & SDL_WINDOW_INPUT_FOCUS))
        {
			//int x = 0, y = 0;
   //     	static float pitch = glm::degrees(asin(cam.EyeDir.y));
   //         static float yaw = asin(cam.EyeDir.x / cos(pitch));
			//SDL_GetRelativeMouseState(&x, &y);
   //         yaw -= x * 0.2f;
   //         pitch -= y * 0.2f;
   //         glm::vec3 direction;
			//direction.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			//direction.y = sin(glm::radians(pitch));
			//direction.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
   //         cam.EyeDir = direction;

		   int xw, yw;
            SDL_GetMouseState(&xw, &yw);
            std::cout << xw <<  ", " << yw << std::endl;
            if(xw < 10)
            {
                auto right = normalize(glm::cross(cam.EyeDir, cam.Up)) * speed * deltaTime;
                cam.Eye -= glm::vec3(right.x, 0, right.z);
            }
            else if(xw > 790)
            {
                auto right = normalize(glm::cross(cam.EyeDir, cam.Up)) * speed * deltaTime;
                cam.Eye += glm::vec3(right.x, 0, right.z);
            }
            if(yw < 10)
            {
                auto forward = normalize(cam.EyeDir) * speed * deltaTime;
                cam.Eye += glm::vec3(forward.x, 0, forward.z);
            }
            else if(yw > 790)
            {
                auto forward = normalize(cam.EyeDir) * speed * deltaTime;
                cam.Eye -= glm::vec3(forward.x, 0, forward.z);
            }
        }

        sceneCB.VP = cam.GetVPMatrix();
        sceneCB.eye = cam.Eye;

		auto now = std::chrono::system_clock::now();
		std::chrono::duration<float, std::ratio<1,1>> diff = now - startTime;
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

        pipeline.SetPipelineState(commandAllocator, commandList);

        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		D3D12_CPU_DESCRIPTOR_HANDLE
			rtvHandle2(renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
		rtvHandle2.ptr = rtvHandle2.ptr + (frameIndex * rtvDescriptorSize);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        commandList->OMSetRenderTargets(1, &rtvHandle2, FALSE, &dsvHandle);

        commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                           D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &surfaceSize);
		commandList->ClearRenderTargetView(rtvHandle2, clearColor, 0, nullptr);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pipeline.BindConstantBuffer("scene", &sceneBuffer, commandList);

        scene.Update(deltaTime);
        scene.Draw(commandList, pipeline);

		//D3D12_CPU_DESCRIPTOR_HANDLE
		//	rtvHandle3(sideRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
		//rtvHandle3.ptr = rtvHandle3.ptr + (frameIndex * rtvDescriptorSize);

		//const float clearColorx[] = {0.0f, 0.0f, 0.0f, 0.0f};
		//commandList->ClearRenderTargetView(rtvHandle3, clearColorx, 0, nullptr);
  //      
  //      CopyTexturePlain(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, backDepthRenderTargets[frameIndex], D3D12_RESOURCE_STATE_DEPTH_WRITE, depthStencilBuffer);
		//
  //      commandList->OMSetRenderTargets(1, &rtvHandle3, FALSE, &dsvHandle);

  //      depthBackPipeline.SetPipelineState(commandAllocator, commandList);
  //  	depthBackPipeline.BindConstantBuffer("cb", &cubeBuffer, commandList);
		//commandList->IASetVertexBuffers(0, 1, &cubeMesh.vertexBufferView);
  //      commandList->DrawInstanced(cubeMesh._vertices.size(), 1, 0, 0);

		//D3D12_CPU_DESCRIPTOR_HANDLE
		//	rtvHandle4(sideRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
		//rtvHandle4.ptr = rtvHandle4.ptr + ((frameIndex + backbufferCount) * rtvDescriptorSize);

		//commandList->ClearRenderTargetView(rtvHandle4, clearColorx, 0, nullptr);
  //      
  //      commandList->OMSetRenderTargets(1, &rtvHandle4, FALSE, &dsvHandle);
  //      depthFrontPipeline.SetPipelineState(commandAllocator, commandList);
  //  	depthFrontPipeline.BindConstantBuffer("cb", &cubeBuffer, commandList);
		//commandList->IASetVertexBuffers(0, 1, &cubeMesh.vertexBufferView);
  //      commandList->DrawInstanced(cubeMesh._vertices.size(), 1, 0, 0);


		//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backDepthRenderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
		//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(frontDepthRenderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
  //      
  //      commandList->OMSetRenderTargets(1, &rtvHandle2, FALSE, &dsvHandle);
  //      commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
  //                                         D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

  //      volumetricPipeline.SetPipelineState(commandAllocator, commandList);
  //  	volumetricPipeline.BindConstantBuffer("cb", &cubeBuffer, commandList);
  //  	volumetricPipeline.BindTexture(device, "frontCulled", backDepthRenderTargets[frameIndex]);
  //  	volumetricPipeline.BindTexture(device, "backCulled", frontDepthRenderTargets[frameIndex]);
		//commandList->IASetVertexBuffers(0, 1, &triangle.vertexBufferView);
  //      commandList->DrawInstanced(triangle._vertices.size(), 1, 0, 0);
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