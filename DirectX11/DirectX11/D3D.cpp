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
	
	IDXGIFactory* factory; // 하드웨어 리소스(그래픽 카드)를 탐색하고 관리하는 객체
	IDXGIAdapter* adapter; // 개별의 그래픽카드
	IDXGIOutput* adapterOutput; // 그래픽 어댑터에 연결된 출력 장치(모니터)
	unsigned int numerator, denomiator; // 모니터 주사율을 나타내는 분수 값. 만일 60Hz라면 numerator는 60,denomitor는 1

	DXGI_MODE_DESC* displayModeList; // DXGI_MODE_DESC는 특정 디스플레이 모드를 기술
	unsigned int numModes; // 모니터에서 지원하는 디스플레이 모드(해상도, 주사율, 포멧 같은)의 개수

	DXGI_ADAPTER_DESC adapterDesc; // 선택된 그래픽 어댑터의 상세 정보(이름, 벤더ID, 디바이스ID, 전용 메모리 등등)을 저장
	unsigned long long stringLegth; // DXGI_ADAPTER_DESC 어댑터 이름의 길이
	
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

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}
}