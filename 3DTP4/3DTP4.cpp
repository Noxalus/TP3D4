#include "stdafx.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "3DTP4.h"
#include <math.h>
#include "InputManager.h"

// Global Variables:
HINSTANCE hInst;			// current instance
HWND	    hWnd;				// windows handle used in DirectX initialization

// Forward declarations
bool				CreateWindows(HINSTANCE, int);
bool				CreateDevice();
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// Input Manager
InputManager* _inputManager;

// Height map
unsigned short m_sizeX;
unsigned short m_sizeZ;
float m_maxY = 50;
float* m_height;

// Structure d'un vertex
struct Vertex
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 TextCoord;
};


bool LoadRAW (const std::string& map)
{
	FILE  *file;
	fopen_s(&file, map.c_str (), "rb");
	if (!file)
		return false;
	fread(&m_sizeX, sizeof(unsigned short), 1, file);
	fread(&m_sizeZ, sizeof(unsigned short), 1, file);
	unsigned int size = m_sizeX * m_sizeZ;
	unsigned char *tmp = new unsigned char[size];
	m_height = new float[size];
	fread(tmp, sizeof(unsigned char), size, file);
	fclose(file);
	int i = 0;
	for (unsigned short z = 0; z < m_sizeZ; ++z)
		for (unsigned short x = 0; x < m_sizeX; ++x, ++i)
			m_height[i] = float ((m_maxY * tmp[i]) / 255.0f);
	delete[] tmp;
	return true;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG oMsg;

	if (!CreateWindows (hInstance, nCmdShow))
	{
		MessageBox(NULL, L"Erreur lors de la cr�ation de la fen�tre", L"Error", 0);
		return false;
	}
	if (!CreateDevice())
	{
		MessageBox(NULL, L"Erreur lors de la cr�ation du device DirectX 9", L"Error", 0);
		return false;
	}

	// Input manager
	_inputManager = new InputManager();
	_inputManager->Create(hInstance, hWnd);

	D3DCOLOR backgroundColor = D3DCOLOR_RGBA(0, 0, 0, 0);

	D3DXMATRIX WorldViewProj;
	D3DXMatrixIdentity(&WorldViewProj);

	// World matrix
	D3DXMATRIX World;

	D3DXMATRIX Position;
	D3DXMATRIX Rotation;

	D3DXMatrixIdentity(&Rotation);
	D3DXMatrixIdentity(&Position);

	World = Position * Rotation;

	// View matrix
	D3DXMATRIX View;

	D3DXVECTOR3 CameraPosition (0.f, 0.f, -1.f);

	D3DXVECTOR3 At (0.f, 0.f, 0.f);
	D3DXVECTOR3 Up (0.f, 1.f, 0.f);


	// Projection matrix
	D3DXMATRIX Projection;

	float fovy = D3DX_PI / 2; // pi / 2
	D3DXMatrixPerspectiveFovLH(&Projection, fovy, 1.3f, 0.1f, 1000.0f);



	// Cr�ation de l�interface DirectX 9
	LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	D3DDISPLAYMODE displayMode;
	D3DPRESENT_PARAMETERS pp;
	pD3D->GetAdapterDisplayMode (D3DADAPTER_DEFAULT, &displayMode);
	pp.Windowed = true; //Mode fen�tr� ou pas
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.BackBufferWidth  = 800;// Taille en x du Back Buffer
	pp.BackBufferHeight  = 600; // Taille en y du Back Buffer
	pp.BackBufferFormat  =  displayMode.Format; // Format du Back Buffer
	pp.BackBufferCount  = 1; // Nombre de Back Buffer
	pp.MultiSampleType = D3DMULTISAMPLE_NONE ; // Nombre de sample pour l�antialiasing
	pp.MultiSampleQuality  = 0; // Qualit� pour l�antialiasing
	pp.hDeviceWindow = hWnd; //Handle de la fen�tre
	pp.EnableAutoDepthStencil = true; // True si on veut un depth-stencil buffer
	pp.AutoDepthStencilFormat = D3DFMT_D24S8; // Le format du deth-stencil buffer
	pp.Flags = 0; // Voir le man
	pp.FullScreen_RefreshRateInHz = 0; //Voir le man
	pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT; // Autrement dit 0, voir le man

	IDirect3DDevice9 *device;
	pD3D->CreateDevice(0, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device);

	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;
	int num_card = 0;
	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings
	for (unsigned int adapter = 0; adapter < pD3D->GetAdapterCount (); 
		adapter++)
	{
		D3DADAPTER_IDENTIFIER9 identifier;
		HRESULT Res;
		Res = pD3D->GetAdapterIdentifier (adapter, 0, &identifier);
		if (strstr (identifier.Description, "PerfHUD") != 0)
		{
			num_card = adapter;
			DeviceType = D3DDEVTYPE_REF;
			break;
		}
	}

	//Create and fill other DirectX Stuffs like Vertex/Index buffer, shaders  

	// Vertex declaration
	D3DVERTEXELEMENT9 dwDecl3[] = 
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END() 
	};

	IDirect3DVertexDeclaration9 *pDecl;
	device->CreateVertexDeclaration(dwDecl3, &pDecl );


	// Culling ?
	//device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// Wireframe ?
	//device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	// Load texture
	LPCWSTR pMapTexture = L"../Resources/terraintexture.jpg";

	LPDIRECT3DTEXTURE9 pTexture;
	D3DXCreateTextureFromFile(device, pMapTexture, &pTexture);

	/** Vertex & Index buffers (Heightmap) **/
	// Load height map
	LoadRAW("../Resources/terrainheight.raw");

	/*
	m_sizeX = 10;
	m_sizeZ = 10;
	*/

	// Vertex buffer
	IDirect3DVertexBuffer9* pMapVertexBuffer;
	device->CreateVertexBuffer((m_sizeX * m_sizeZ) * sizeof(Vertex), 0, 0, D3DPOOL_DEFAULT, &pMapVertexBuffer, NULL);

	Vertex* pMapVertexData;

	pMapVertexBuffer->Lock(0, 0, (void**) &pMapVertexData, 0);

	for(int x = 0; x < m_sizeX; x++)
	{
		for(int y = 0; y < m_sizeZ; y++)
		{
			float heightValue = m_height[x + (m_sizeX * y)];

			pMapVertexData[x + (m_sizeX * y)].Position = D3DXVECTOR3(x, heightValue, y);

			pMapVertexData[x + (m_sizeX * y)].TextCoord = D3DXVECTOR2(((float)x / (float)m_sizeX), 1 - ((float)y / (float)m_sizeZ));
		}
	}

	pMapVertexBuffer->Unlock();

	// Index buffer
	IDirect3DIndexBuffer9* pMapIndexBuffer;
	device->CreateIndexBuffer(6 * (m_sizeX * m_sizeZ) * sizeof(int), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pMapIndexBuffer, NULL);

	int* pMapIndexData;
	pMapIndexBuffer->Lock(0, 0, (void**) &pMapIndexData, 0);


	int counter = 0;
	for(int y = 0; y < m_sizeZ - 1; y++)
	{
		for(int x = 0; x < m_sizeX - 1; x++)
		{
			// First triangle
			pMapIndexData[counter] = x + (m_sizeX * (y + 1));
			counter++;
			pMapIndexData[counter] = (x + 1) + (m_sizeX * y);
			counter++;
			pMapIndexData[counter] = x + (m_sizeX * y);
			counter++;

			// Second triangle
			pMapIndexData[counter] = (x + 1) + (m_sizeX * (y + 1));
			counter++;
			pMapIndexData[counter] = (x + 1) + (m_sizeX * y);
			counter++;
			pMapIndexData[counter] = x + (m_sizeX * (y + 1));
			counter++;
		}
	}

	pMapIndexBuffer->Unlock();

	// Shader
	LPCWSTR pFxFile = L"../Resources/shader.fx";
	LPD3DXEFFECT pEffect;
	LPD3DXBUFFER CompilationErrors;

	if (D3D_OK != D3DXCreateEffectFromFile(device, pFxFile, NULL, NULL, 0, NULL, &pEffect, &CompilationErrors))
	{
		MessageBoxA (NULL, (char *) 
			CompilationErrors->GetBufferPointer(), "Error", 0);
	}

	D3DXHANDLE hWorldViewProj = pEffect->GetParameterByName (NULL, "WorldViewProj");

	D3DXHANDLE hTexture = pEffect->GetParameterByName (NULL, "Texture");

	PeekMessage( &oMsg, NULL, 0, 0, PM_NOREMOVE );
	while ( oMsg.message != WM_QUIT )
	{
		if (PeekMessage( &oMsg, NULL, 0, 0, PM_REMOVE )) 
		{
			TranslateMessage( &oMsg );
			DispatchMessage( &oMsg );
		}
		else
		{
			// Update the input state
			_inputManager->Manage();
			
			// Up
			if (_inputManager->IsKeyDone(DIK_W))
			{
				CameraPosition.z += 0.1f;
			}
			// Left
			if (_inputManager->IsKeyDone(DIK_A))
			{
				CameraPosition.x += 0.1f;
			}
			// Down
			if (_inputManager->IsKeyDone(DIK_S))
			{
				CameraPosition.z -= 0.1f;
			}
			// Right
			if (_inputManager->IsKeyDone(DIK_D))
			{
				CameraPosition.x -= 0.1f;
			}

			D3DXMatrixLookAtLH(&View, &CameraPosition, &At, &Up);
			D3DXMatrixIdentity(&World);

			WorldViewProj = World * View * Projection;

			// Do a lot of thing like draw triangles with DirectX
			device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, backgroundColor, 1.0f, 0);
			device->BeginScene();

			// Set device vertex declaration
			device->SetVertexDeclaration(pDecl);

			// "Send" WorldViewProj matrix to shader
			pEffect->SetMatrix(hWorldViewProj, &WorldViewProj);

			// Set texture
			pEffect->SetTexture(hTexture, pTexture);

			unsigned int cPasses, iPass;


			// Draw map
			device->SetStreamSource(0, pMapVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pMapIndexBuffer);

			cPasses = 0, iPass = 0;
			pEffect->Begin(&cPasses, 0);
			for (iPass= 0; iPass< cPasses; ++iPass)
			{
				pEffect->BeginPass(iPass);
				pEffect->CommitChanges(); // que si on a chang� des �tats apr�s le BeginPass

				device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 2 * (m_sizeX * m_sizeZ), 0, 2 * ((m_sizeX * m_sizeZ) - 1));

				pEffect->EndPass();
			}

			pEffect->End();


			device->EndScene();
			device->Present(NULL, NULL, NULL, NULL); 
		}
	}

	//Release D3D objectssss
	pD3D->Release();
	device->Release();
	pDecl->Release();
	pMapVertexBuffer->Release();
	pMapIndexBuffer->Release();
	pTexture->Release();
	pEffect->Release();
	_inputManager->Destroy();

	return (int) oMsg.wParam;
}


bool CreateWindows(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	//
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style					= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc		= WndProc;
	wcex.hInstance			= hInstance;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= L"3DTPClassName";

	if ( RegisterClassEx(&wcex) == 0 )
		return false;

	hWnd = CreateWindow(L"3DTPClassName", L"This course is awesome", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return false;

	SetWindowPos(hWnd, NULL, 0, 0, 800, 600, 0);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return true;
}

bool CreateDevice()
{
	return true;
}

//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		break;
	case WM_KEYUP:
		{
			switch(wParam)
			{
			case VK_ESCAPE:
				{
					PostQuitMessage(0);
					break;
				}
			}
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}