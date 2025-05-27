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

// ������ ����������
/*
* ��� �׷����� �ﰢ��(������)���� �̷���� ����
* ������ ������������ �� ������ �����͵��� ȭ�鿡 ����ϱ������ �� ����
* ù ������ ������ ������(���� ������)���� ����
* 1. Input assembler : GPU �޸𸮿� ���� ����, �ε��� ���� ���� �� �����͸� �Է�
* 2. Vertex shader : 3D ������ ���ǵ� �������� ��ġ�� ī�޶� ����, ���ٰ�, ���� ���� ����� 2D ȭ�� ������ ������ ��ġ�� ��ȯ
* 3. Hull shader : Tessellation ������ ������ ������ ���, �󸶳� �� ������ ����
* 4. Tessellation : ���� ���� ������ ���� GPU���� �ǽð����� �� ���� ���������� ����ȭ�� �������� ���δ�.
* 5. Domain shader : Tessellation���� ������ �������� ȭ����� ��ġ�� ��ȯ(vertex shader�� Tesellation ����)
* 6. Geometry shader : �������� ������ ����ȭ. ȭ��� ���� �׸� �ʿ䰡 ���� ��쿡�� ����, 
* 7. Steam output : ���ݱ��� �����ߴ� �������� ���ۿ� ����.(������)
* 8. Clipping : ȭ�� �ۿ� �ִ� �������� ����
* 9. Screen mapping : ȭ�� �ȿ� �ִ� �������� �ȼ� ��ǥ�� ��ȯ
* 10. Triangle traversal : ������ �̷�� �ﰢ���� � �ȼ����� Ŀ���ϴ��� ���. �̶�, ������ �̷������.
* 11. Pixel shader : ���������� ����Ǵ� ���̴�. �ؽ�ó�� ������ �����ϰų� ���� ����, �׸��� ���� ó���Ѵ�.
* 12. Output merger : ���������� ���ٽ��� ����Ǵ� �κ��̴�.
*/

bool D3D::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result; // COM(Component Object Model)���� �Լ� ȣ���� ��������. ���� HRESULT ���
	int error;
	
	IDXGIFactory* factory; // �ϵ���� ���ҽ�(�׷��� ī��)�� Ž���ϰ� �����ϴ� ��ü
	IDXGIAdapter* adapter; // ������ �׷���ī��
	IDXGIOutput* adapterOutput; // �׷��� ����Ϳ� ����� ��� ��ġ(�����)
	unsigned int numerator, denomiator; // ����� �ֻ����� ��Ÿ���� �м� ��. ���� 60Hz��� numerator�� 60,denomitor�� 1

	DXGI_MODE_DESC* displayModeList; // DXGI_MODE_DESC�� Ư�� ���÷��� ��带 ���
	unsigned int numModes; // ����Ϳ��� �����ϴ� ���÷��� ���(�ػ�, �ֻ���, ���� ����)�� ����

	DXGI_ADAPTER_DESC adapterDesc; // ���õ� �׷��� ������� �� ����(�̸�, ����ID, ����̽�ID, ���� �޸� ���)�� ����
	unsigned long long stringLegth; // DXGI_ADAPTER_DESC ����� �̸��� ����
	
	D3D_FEATURE_LEVEL featureLevel; // D3D_FEATURE_LEVEL ������

	// ����ü��
	/*
	* �������� ������� ����ϱ� ���� �����ϴ� ����.
	* ����ü���� ���� Double buffering���� �Ǿ� �ִ�.
	* - back buffer : GPU�� ���� ������ �۾��� �����ϴ� ����
	* - front buffer : ����� ȭ�鿡 ǥ�õ� �̹����� ����Ǵ� ����
	* ��, ���� back buffer�� ����� ������ �̹����� �����ϰ� �� ����� front buffer�� �����ϸ� ȭ�鿡 ��µǴ� ���
	*/
	DXGI_SWAP_CHAIN_DESC swapChainDesc; // ����ü��
	ID3D11Texture2D* backBuffer; // ����ü�� �����

	// ���ٽ�
	/*
	* ���ٽ��� ����ŷ�̶�� ����ȴ�.
	* �ٸ� ���̴��� ó���ؾ��� �κ��� �̸� ����ŷ�� �� �ξ�, �� �κи� ó���ϵ��� �ϴ� ��
	* �Ϲ������� �ſ� ȿ���� �� ȿ��(�׸���)�� ǥ���ϱ� ���� �̸� ����ŷ�ؼ� ���Ŀ� ���� ó���ϵ��� �Ѵ�.
	*/
	D3D11_TEXTURE2D_DESC dapthBufferDesc; // 3D ������Ʈ�� ���� ���� ���� - ���ٰ�, �ڿ� �ִ� ������Ʈ�� ������ ����
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc; // ����/���ٽ� �׽�Ʈ�� ���� ����� ����. �ö� ���ǿ��� �ȼ��� ����/���ٽ� ���ۿ� �������� �׽�Ʈ���� ���� ����
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc; // ������ ����/���ٽ��� �並 ����

	D3D11_RASTERIZER_DESC rasterDesc; // 3D ������Ʈ���� 2D�ȼ��� ��ȯ
	
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