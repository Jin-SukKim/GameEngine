#include <vector>

#include "MeshModel.h"
#include "D3D11Utils.h"
#include "GeometryGenerator.h"


void MeshModel::Initialize(ComPtr<ID3D11Device>& device, const std::vector<MeshData>& meshes)
{
	// Texture Sampler 설정
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	// Filtering은 Linear Interpolation으로 설정
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	// wrap/crap/mirror 등 옵션 설정 가능
	// Texture도 2D Texture, 3D Textrue가 있기 때문에 W 좌표까지 사용 가능
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // x
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // y
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // z
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// 하나의 모델이 여러 개의 메쉬로 이루어져 있지만 Sampler와 Constant buffer는
	// 하나를 사용해야 여러 메쉬들이 하나의 모델처럼 보인다.
	// Sampler State 생성
	device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

	// Constant Buffer 생성
	m_constantVSBufferData.world = Matrix(); // World 행렬
	m_constantVSBufferData.view = Matrix(); // View 행렬
	m_constantVSBufferData.proj = Matrix(); // Projection 행렬
	D3D11Utils::CreateConstantBuffer(device, m_constantVSBufferData, m_meshVSConstantBuffer);
	D3D11Utils::CreateConstantBuffer(device, m_constantPSBufferData, m_meshPSConstantBuffer);

	for (const MeshData& mData : meshes)
	{
		std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>();
		// VertexBuffer 생성 후 CPU -> GPU 데이터 복사
		D3D11Utils::CreateVertexBuffer(device, mData.vertices, newMesh->vertexBuffer);
		// Index Buffer 생성 후 CPU -> GPU 데이터 복사
		D3D11Utils::CreateIndexBuffer(device, mData.indices, newMesh->indexBuffer);
		// Index Count 복사
		newMesh->indexCount = (UINT)mData.indices.size();

		
		// Texture 생성
		if (!mData.texturePath.empty()) {
			D3D11Utils::CreateTexture(device, mData.texturePath,
				newMesh->m_meshTexture, newMesh->m_textureResourceView);
		}

		// 하나의 모델이므로 같은 constant data 사용
		newMesh->constantBufferVS = m_meshVSConstantBuffer;
		newMesh->constantBufferPS = m_meshPSConstantBuffer;

		m_meshes.push_back(newMesh);
	}

	// Shader 생성

	// InputLayer의 데이터 형식으로 Vertex Shader에 들어가는 데이터 구조체와 같게 설정
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		// COLOR가 시작되는 위치로 POSITION이 RGB 각각 4 byte씩 할당한 다음부터 시작하므로 4(byte) * 3(개수)
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 9, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// InputLayout & Vertex Shader 생성
	D3D11Utils::CreateVSAndInputLayout(device, L"Shader/BasicVS.hlsl", inputElements,
		m_meshVertexShader, m_meshInputLayout);

	// Pixel Shader 생성
	D3D11Utils::CreatePS(device, L"Shader/BasicPS.hlsl", m_meshPixelShader);

	// Normal Vector 그리기 - 위에서 생성한 InputLayout을 같이 사용
	m_normal = std::make_shared<Mesh>();

	std::vector<Vertex> normalVertices;
	std::vector<uint32_t> normalIndices;

	// 여러 메쉬의 normal들을 하나로 합치기
	size_t offset = 0;
	for (const MeshData& mData : meshes) {
		// 생성한 Mesh의 data 사용
		for (size_t i = 0; i < mData.vertices.size(); i++)
		{
			Vertex v = mData.vertices[i];

			// 시작점과 끝점의 좌표가 같지만 texcoord의 값이 0.f, 1.f로 다르기에 vertex shader에서 변환
			// Normal Vector는 선분이기에 texcoord는 x좌표만 사용
			// 시작점
			v.texcoord.x = 0.f;
			normalVertices.push_back(v);

			// 끝점
			v.texcoord.x = 1.f;
			normalVertices.push_back(v);

			// 0-1 선분, 2-3 선분 etc
			normalIndices.push_back(uint32_t(2 * (i + offset)));
			normalIndices.push_back(uint32_t(2 * (i + offset) + 1));
		}
		// 1d array를 2d array로 사용하는 것처럼 여러 메쉬들의 normal을
		// offset을 활용해 하나로 합쳐준다.
		offset += mData.vertices.size();
	}

	// Normal Line의 vertex와 index buffer 생성
	D3D11Utils::CreateVertexBuffer(device, normalVertices, m_normal->vertexBuffer);
	m_normal->indexCount = (UINT)normalIndices.size();
	D3D11Utils::CreateIndexBuffer(device, normalIndices, m_normal->indexBuffer);

	D3D11Utils::CreateConstantBuffer(device, m_constantNormalBufferData, m_meshNormalConstantBuffer);

	// Normal Vector Line의 Shader 생성
	D3D11Utils::CreateVSAndInputLayout(
		device, L"Shader/NormalVS.hlsl", inputElements, m_meshNormalVertexShader, m_meshInputLayout);
	D3D11Utils::CreatePS(device, L"Shader/NormalPS.hlsl", m_meshNormalPixelShader);
}

void MeshModel::Initialize(ComPtr<ID3D11Device>& device, const std::string& basePath, const std::string& fileName)
{
	// 파일로부터 데이터 가져오기
	std::vector<MeshData> meshes = GeometryGenerator::ReadFromFile(basePath, fileName);
	// 가져온 데이터로 buffer 초기화
	Initialize(device, meshes);
}

void MeshModel::Render(ComPtr<ID3D11DeviceContext>& context, bool drawNormal)
{
	// Graphics Pipeline의 단계
	// 1. 그리고자 하는 모델의 Vertex와 Index 버퍼
	// 2. IA: Input-Assembler stage (입력(Vertex와 Index등)을 모아서 렌더링을 할 기본 단위(삼각형 등)을 만든다)
	//          (ex: vertex 3개를 모아 삼각형 만들기)
	// 3. VS: Vertex Shader - 모델 변환, 시점 변환, Projection
	// 4. Tessellation
	// 5. Geometry shader : 4, 5번은 생성한 모델을 더 자세하게 만드는 단계이다.
	// 6. RS: Rasterizer stage - 모델의 기하 정보를 pixel의 집합으로 만든다.
	// 7. PS: Pixel Shader - pixel들을 색을 결정
	// 8. OM: Output-Merger stage - depth buffering 적용 등 실제 FrameBuffer의 색깔 값을 결정한다.

	// Vertex Shader 설정
	context->VSSetShader(m_meshVertexShader.Get(), 0, 0);
	// Pixel Shader 설정
	context->PSSetShader(m_meshPixelShader.Get(), 0, 0);
	// Texture에 쓸 Sampler 넘기기
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf()); 


	// Vertex Buffer의 단위와 offset 설정
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	for (const std::shared_ptr<Mesh>& mesh : m_meshes)
	{
		/*
			// 여러 constant buffer 사용시
			ID3D11Buffer *pptr[1] = {
				m_constantBuffer.Get(),
			};
			m_context->VSSetConstantBuffers(0, 1, pptr);
		*/
		// Constant Buffer 설정
		// (0번 index부터 시작, 1개, constant buffer)
		context->VSSetConstantBuffers(0, 1, mesh->constantBufferVS.GetAddressOf());

		// Pixel Shader에 Texture와 Sampler를 넘겨준다.
		// Texture Data는 TextureResourceView로 Shader에서 사용하는 Resource의
		// 실질적 데이터가 들어가 있다.
		// 퀄리티가 좋은 Texture의 경우 여러 Texture를 함께 사용하는 경우가 많아 배열로 만들어 넘긴다.
		ComPtr<ID3D11ShaderResourceView> pixelResources[2] =
		{
			mesh->m_textureResourceView.Get()
		};
		// 현재 ResourceView와 사용할 Texture가 1개라 1로 설정
		context->PSSetShaderResources(0, 1, pixelResources->GetAddressOf()); // TextureResourceView 넘기기
		context->PSSetConstantBuffers(0, 1, mesh->constantBufferPS.GetAddressOf());

		// Input Layout 설정
		context->IASetInputLayout(m_meshInputLayout.Get());
		// Vertex/Index Buffer 설정
		context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Index Buffer가 가진 Vertex들의 연결관계 설정 (_TRIANGLESTRIP 등)
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // index 3개씩 묶어서 삼각형 만들기
		// GPU가 준비되면 Render
		// (몇 개를 그릴지 지정, Buffer에서 몇 번쨰 index로부터 그리기 시작할 지 지정)
		context->DrawIndexed(mesh->indexCount, 0, 0);
	}

	// Normal Vector 그리기
	if (drawNormal) {
		context->VSSetShader(m_meshNormalVertexShader.Get(), 0, 0);
		context->PSSetShader(m_meshNormalPixelShader.Get(), 0, 0);

		// Normal Vector의 Shader에서도 Mesh의 vertex constant buffer의 data를 사용
		ComPtr<ID3D11Buffer> pptr[2] = {
			m_meshVSConstantBuffer.Get(),
			m_meshNormalConstantBuffer.Get()
		};

		// 0번 index로 시작하고 2개의 buffer를 사용
		context->VSSetConstantBuffers(0, 2, pptr->GetAddressOf());
		
		context->IASetVertexBuffers(0, 1, m_normal->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_normal->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
		// 선분 그리기
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->DrawIndexed(m_normal->indexCount, 0, 0);
	}

}

void MeshModel::UpdateConstantBuffers(ComPtr<ID3D11DeviceContext>& context)
{
	// Constant Buffer Data를 CPU -> GPU 복사
	D3D11Utils::UpdateBuffer(context, m_constantVSBufferData, m_meshVSConstantBuffer);
	D3D11Utils::UpdateBuffer(context, m_constantPSBufferData, m_meshPSConstantBuffer);
	D3D11Utils::UpdateBuffer(context, m_constantNormalBufferData, m_meshNormalConstantBuffer);
}
