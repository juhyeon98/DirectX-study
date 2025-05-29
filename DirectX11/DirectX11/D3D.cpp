#include "D3D.h"

D3D::D3D()
{
	mSwapChain = 0;
	mDevice = 0;
	mDeviceContext = 0;
	mRenderTargetView = 0;
	mDapthStencilBuffer = 0;
	mDepthStencilState = 0;
	mDepthStencilView = 0;
	mResterState = 0;
}

D3D::D3D(const D3D& other)
{
}

D3D::~D3D()
{
}

// 렌더링 파이프라인
/*
* 모든 그래픽은 삼각형(폴리곤)으로 이루어져 있음
* 렌더링 파이프라인은 이 폴리곤 데이터들을 화면에 출력하기까지의 전 과정
* 첫 시작은 폴리곤 데이터(정점 데이터)에서 시작
* 1. Input assembler : GPU 메모리에 정점 버퍼, 인덱스 버퍼 등의 모델 데이터를 입력
* 2. Vertex shader : 3D 공간에 정의된 정점들의 위치를 카메라 시점, 원근감, 투영 등을 고려해 2D 화면 공간의 적절한 위치로 변환
* 3. Hull shader : Tessellation 이전에 폴리곤 분할을 어떻게, 얼마나 할 것인지 결정
* 4. Tessellation : 적은 수의 폴리곤 모델을 GPU에서 실시간으로 더 많은 폴리곤으로 세분화해 디테일을 높인다.
* 5. Domain shader : Tessellation으로 생성된 폴리곤을 화면상의 위치로 변환(vertex shader의 Tesellation 버전)
* 6. Geometry shader : 폴리곤의 개수를 최적화. 화면상에 굳이 그릴 필요가 없는 경우에는 삭제, 
* 7. Steam output : 지금까지 연산했던 정점들을 버퍼에 저장.(선택적)
* 8. Clipping : 화면 밖에 있는 정점들을 제거
* 9. Screen mapping : 화면 안에 있는 정점들을 픽셀 좌표로 변환
* 10. Triangle traversal : 정점이 이루는 삼각형이 어떤 픽셀들을 커버하는지 계산. 이때, 보간이 이루어진다.
* 11. Pixel shader : 마지막으로 실행되는 셰이더. 텍스처나 색상을 적용하거나 빛을 적용, 그림자 등을 처리한다.
* 12. Output merger : 최종적으로 스텐실이 적용되는 부분이다.
*/

bool D3D::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result; // COM(Component Object Model)에서 함수 호출의 성공여부. 보통 HRESULT 사용
	int error;
	
	IDXGIFactory* factory; // DXGI를 생성해주는 인터페이스. DXGI를 통해 하드웨어 정보를 읽어올 수 있다.
	// DXGI은 어떤 실체가 아니라 COM기반의 인터페이스들을 아우르는 말이다.
	// IDXGIAdapter, IDXGIOutput 같은 인터페이스들은 COM 객체로 실제 연결되어 있는 라이브러리와 API와 통신한다.
	IDXGIAdapter* adapter; // 그래픽카드 인터페이스
	IDXGIOutput* adapterOutput; // 그래픽 어댑터에 연결된 출력 장치...에 대한 인터페이스(like IDXGIAdapter)
	unsigned int numerator, denominator; // 모니터 주사율을 나타내는 분수 값. 만일 60Hz라면 numerator는 60,denomitor는 1

	DXGI_MODE_DESC* displayModeList; // DXGI_MODE_DESC는 특정 디스플레이 모드를 기술
	unsigned int numModes; // 모니터에서 지원하는 디스플레이 모드(해상도, 주사율, 포멧 같은)의 개수

	DXGI_ADAPTER_DESC adapterDesc; // 선택된 그래픽 어댑터의 상세 정보(이름, 벤더ID, 디바이스ID, 전용 메모리 등등)을 저장
	unsigned long long stringLength; // DXGI_ADAPTER_DESC 어댑터 이름의 길이
	
	D3D_FEATURE_LEVEL featureLevel; // D3D_FEATURE_LEVEL 열거형

	// 스왑체인
	/*
	* 렌더링된 결과물을 출력하기 전에 저장하는 버퍼.
	* 스왑체인은 보통 Double buffering으로 되어 있다.
	* - back buffer : GPU가 실제 렌더링 작업을 수행하는 버퍼
	* - front buffer : 모니터 화면에 표시될 이미지가 저장되는 버퍼
	* 즉, 먼저 back buffer를 사용해 렌더링 이미지를 생성하고 그 결과를 front buffer에 저장하면 화면에 출력되는 방식
	*/
	DXGI_SWAP_CHAIN_DESC swapChainDesc; // 스왑체인
	ID3D11Texture2D* backBuffer; // 스왑체인 백버퍼

	// 스텐실
	/*
	* 스텐실은 마스킹이라고 보면된다.
	* 다른 셰이더가 처리해야할 부분을 미리 마스킹을 해 두어, 그 부분만 처리하도록 하는 것
	* 일반적으로 거울 효과나 빛 효과(그림자)를 표현하기 위해 미리 마스킹해서 이후에 따로 처리하도록 한다.
	*/
	D3D11_TEXTURE2D_DESC dapthBufferDesc; // 3D 오브젝트의 깊이 정보 저장 - 원근감, 뒤에 있는 오브젝트를 가리기 위함
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc; // 깊이/스텐실 테스트의 동작 방식을 정의. ㅓ떤 조건에서 픽셀이 깊이/스텐실 버퍼에 쓰여지고 테스트될지 등을 설정
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc; // 생성된 깊이/스텐실의 뷰를 정의

	D3D11_RASTERIZER_DESC rasterDesc; // 3D 지오메트리를 2D픽셀로 변환
	
	float fieldOfView, screenAspect;

	mVsyncEnabled = vsync;

	// DirectX 그래픽 인터페이스 팩토리 생성
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// 그래픽 인터페이스 어댑터 생성
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// 출력할 모니터 나열
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// 모니터에 대한 DXGI_FORMAT_R8G8B8A8_UNORM(화면 버퍼 형식) 디스플레이 형식에 맞는 모드 수를 가져온다
	// numModes가 들어가고, 마지막 매개변수(list 자리)가 NULL인 것에 집중
	// DXGI : DirectX Graphics Infrastructure
	// R8G8B8A8 : R 8bit, G 8bit, B 8bit, Alpha 8bit
	// UNORM : unsigned normalized 0-255를 0.0-1.0 값으로 사용
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// DXGI_FORMAT_R8G8B8A8_UNORM 버퍼를 지원하는 모니터의 모드들을 저장할 리스트 생성
	// 모드 = 그래픽 카드가 모니터에 뿌릴 수 있는 스팩. 화면의 해상도, 주사율, 픽셀 형식 등등을 저장
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// 지정한 모니터의 모드를 채운다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}
	
	// 모드 리스트를 순회
	for (unsigned int i = 0; i < numModes; i++)
	{
		// 지정한 윈도우 창의 해상도와 맞는 모드를 찾는다.
		// 하나의 모니터에는 하나의 모드만 있는 것이 아니라, 가능한 모든 모드가 있다.
		// 예를 들어, 4K 모니터에는 QHD모드도 있고, FHD 해상도의 모드도 있다.
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// 그래픽 카드 어댑터의 디스크립터를 가져온다.
	// adapter는 IDXGIAdapter - 인터페이스라서, 어떤 그래픽 카드든 실행 할 수 있는 메서드를 제공
	// adapter describe는 실제 그래픽 카드에 대한 정보(구조체)
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// 그래픽 카드 메모리 크기를 메가바이트 단위로 저장
	mVideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// 그래픽 카드 이름을 문자열로 저장
	error = wcstombs_s(&stringLength, mVideoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	delete[] displayModeList;
	displayModeList = 0;

	adapterOutput->Release();
	adapterOutput = 0;

	adapter->Release();
	adapter = 0;

	factory->Release();
	factory = 0;
}