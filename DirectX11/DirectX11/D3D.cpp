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
	
	IDXGIFactory* factory; // DXGI�� �������ִ� �������̽�. DXGI�� ���� �ϵ���� ������ �о�� �� �ִ�.
	// DXGI�� � ��ü�� �ƴ϶� COM����� �������̽����� �ƿ츣�� ���̴�.
	// IDXGIAdapter, IDXGIOutput ���� �������̽����� COM ��ü�� ���� ����Ǿ� �ִ� ���̺귯���� API�� ����Ѵ�.
	IDXGIAdapter* adapter; // �׷���ī�� �������̽�
	IDXGIOutput* adapterOutput; // �׷��� ����Ϳ� ����� ��� ��ġ...�� ���� �������̽�(like IDXGIAdapter)
	unsigned int numerator, denominator; // ����� �ֻ����� ��Ÿ���� �м� ��. ���� 60Hz��� numerator�� 60,denomitor�� 1

	DXGI_MODE_DESC* displayModeList; // DXGI_MODE_DESC�� Ư�� ���÷��� ��带 ���
	unsigned int numModes; // ����Ϳ��� �����ϴ� ���÷��� ���(�ػ�, �ֻ���, ���� ����)�� ����

	DXGI_ADAPTER_DESC adapterDesc; // ���õ� �׷��� ������� �� ����(�̸�, ����ID, ����̽�ID, ���� �޸� ���)�� ����
	unsigned long long stringLength; // DXGI_ADAPTER_DESC ����� �̸��� ����
	
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

	// DirectX �׷��� �������̽� ���丮 ����
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// �׷��� �������̽� ����� ����
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// ����� ����� ����
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// ����Ϳ� ���� DXGI_FORMAT_R8G8B8A8_UNORM(ȭ�� ���� ����) ���÷��� ���Ŀ� �´� ��� ���� �����´�
	// numModes�� ����, ������ �Ű�����(list �ڸ�)�� NULL�� �Ϳ� ����
	// DXGI : DirectX Graphics Infrastructure
	// R8G8B8A8 : R 8bit, G 8bit, B 8bit, Alpha 8bit
	// UNORM : unsigned normalized 0-255�� 0.0-1.0 ������ ���
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// DXGI_FORMAT_R8G8B8A8_UNORM ���۸� �����ϴ� ������� ������ ������ ����Ʈ ����
	// ��� = �׷��� ī�尡 ����Ϳ� �Ѹ� �� �ִ� ����. ȭ���� �ػ�, �ֻ���, �ȼ� ���� ����� ����
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// ������ ������� ��带 ä���.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}
	
	// ��� ����Ʈ�� ��ȸ
	for (unsigned int i = 0; i < numModes; i++)
	{
		// ������ ������ â�� �ػ󵵿� �´� ��带 ã�´�.
		// �ϳ��� ����Ϳ��� �ϳ��� ��常 �ִ� ���� �ƴ϶�, ������ ��� ��尡 �ִ�.
		// ���� ���, 4K ����Ϳ��� QHD��嵵 �ְ�, FHD �ػ��� ��嵵 �ִ�.
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// �׷��� ī�� ������� ��ũ���͸� �����´�.
	// adapter�� IDXGIAdapter - �������̽���, � �׷��� ī��� ���� �� �� �ִ� �޼��带 ����
	// adapter describe�� ���� �׷��� ī�忡 ���� ����(����ü)
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// �׷��� ī�� �޸� ũ�⸦ �ް�����Ʈ ������ ����
	mVideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// �׷��� ī�� �̸��� ���ڿ��� ����
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