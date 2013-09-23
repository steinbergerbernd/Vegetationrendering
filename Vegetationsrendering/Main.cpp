#include "DXUT.h"
#include "SDKmisc.h"
#include "DXUTgui.h"
#include <DXGI.h>
#include "VegetationRendering.h"

VegetationRendering* vegetationRendering;

CDXUTDialogResourceManager dialogResourceManager;
CDXUTTextHelper textHelper;

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
									   DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
									  void* pUserContext )
{
	vegetationRendering = new VegetationRendering();

	Window::setHandle(DXUTGetHWND());

	ID3D11DeviceContext* deviceContext = DXUTGetD3D11DeviceContext();
	dialogResourceManager.OnD3D11CreateDevice(pd3dDevice, deviceContext);
	textHelper = CDXUTTextHelper(pd3dDevice, deviceContext, &dialogResourceManager, 30);

	vegetationRendering->init(pd3dDevice);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										  const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	//pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	vegetationRendering->onResetDevice(pd3dDevice);

	dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	vegetationRendering->update(GameTime(fElapsedTime, fTime));
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
								  double fTime, float fElapsedTime, void* pUserContext )
{
	// Clear render target and the depth stencil 
	float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	vegetationRendering->draw(pd3dDevice, GameTime(fElapsedTime, fTime));

	std::wstringstream ws;

	ws << DXUTGetFPS();
	
	textHelper.Begin();
	textHelper.SetInsertionPos(5, 5);
	textHelper.SetForegroundColor(vegetationRendering->isInCaptureMode() ? D3DXCOLOR(1,0,0,1) : D3DXCOLOR(1, 1, 0, 1));
	textHelper.DrawTextLine(ws.str().c_str());
	textHelper.End();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	vegetationRendering->onLostDevice();
	dialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	dialogResourceManager.OnD3D11DestroyDevice();

	vegetationRendering->release();
	delete vegetationRendering;
	vegetationRendering = 0;
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						  bool* pbNoFurtherProcessing, void* pUserContext )
{
	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	Keyboard::setKey(nChar, bKeyDown);
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
					   bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
					   int xPos, int yPos, void* pUserContext )
{
	POINT pos;

	GetCursorPos(&pos);

	Mouse::setState(MouseState(bLeftButtonDown, bRightButtonDown, nMouseWheelDelta, Point(pos.x, pos.y)));
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// DXUT will create and use the best device (either D3D9 or D3D11) 
	// that is available on the system depending on which D3D callbacks are set below

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMouse( OnMouse, true );
	//DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetHotkeyHandling(true, true, true);
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"VegetationRendering" );

	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 1024, 768 );
	DXUTMainLoop(); // Enter into the DXUT ren  der loop

	return DXUTGetExitCode();
}


