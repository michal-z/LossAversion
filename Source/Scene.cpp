#include "Pch.h"
#include "DirectX12.h"
#include "Library.h"
#include "Scene.h"


void Scene::Initialize()
{
	m_Dx12.GetCmdList()->Close();

	std::vector<uint8_t> csoVs = LoadFile("Assets/Shaders/TriangleVS.cso");
	std::vector<uint8_t> csoPs = LoadFile("Assets/Shaders/SolidPS.cso");

	D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElements, (uint32_t)std::size(inputElements) };
	psoDesc.VS = { csoVs.data(), csoVs.size() };
	psoDesc.PS = { csoPs.data(), csoPs.size() };
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	VHR(m_Dx12.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_Pso)));
	VHR(m_Dx12.GetDevice()->CreateRootSignature(0, csoVs.data(), csoVs.size(), IID_PPV_ARGS(&m_Rs)));

	CreateBuffers();
}

void Scene::Update(double frameTime, float frameTimeDelta)
{
	const uint32_t frameIndex = m_Dx12.GetFrameIndex();

	XMFLOAT2* ptr = (XMFLOAT2*)m_LinesVbCpuAddr[frameIndex];

	*ptr++ = XMFLOAT2(-0.5f, -0.5f);
	*ptr++ = XMFLOAT2(0.8f, -0.5f);
	*ptr++ = XMFLOAT2(0.0f, 0.8f);
}

void Scene::Draw() const
{
	ID3D12CommandAllocator* cmdAlloc = m_Dx12.GetCmdAllocator();
	cmdAlloc->Reset();

	ID3D12GraphicsCommandList* cmdList = m_Dx12.GetCmdList();

	cmdList->Reset(cmdAlloc, nullptr);
	cmdList->RSSetViewports(1, &m_Dx12.GetViewport());
	cmdList->RSSetScissorRects(1, &m_Dx12.GetScissorRect());

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_Dx12.GetBackBuffer();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmdList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = m_Dx12.GetBackBufferHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE depthBufferHandle = m_Dx12.GetDepthBufferHandle();

	cmdList->OMSetRenderTargets(1, &backBufferHandle, 0, &depthBufferHandle);

	cmdList->ClearRenderTargetView(backBufferHandle, XMVECTORF32{ 0.0f, 0.2f, 0.4f, 1.0f }, 0, nullptr);
	cmdList->ClearDepthStencilView(depthBufferHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmdList->IASetVertexBuffers(0, 1, &m_LinesVbView[0]);
	cmdList->SetPipelineState(m_Pso);
	cmdList->SetGraphicsRootSignature(m_Rs);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(3, 1, 0, 0);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cmdList->ResourceBarrier(1, &barrier);

	cmdList->Close();

	m_Dx12.GetCmdQueue()->ExecuteCommandLists(1, (ID3D12CommandList**)&cmdList);
}

void Scene::CreateBuffers()
{
	ID3D12Device* device = m_Dx12.GetDevice();

	{ 	// vertex buffer
		auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(1000);
		for (uint32_t i = 0; i < 2; ++i)
		{
			VHR(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &vbDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_LinesVb[i])));

			VHR(m_LinesVb[i]->Map(0, &CD3DX12_RANGE(0, 0), (void**)&m_LinesVbCpuAddr[i]));

			m_LinesVbView[i].BufferLocation = m_LinesVb[i]->GetGPUVirtualAddress();
			m_LinesVbView[i].StrideInBytes = sizeof(XMFLOAT2);
			m_LinesVbView[i].SizeInBytes = 3 * m_LinesVbView[i].StrideInBytes;

			XMFLOAT2* ptr = (XMFLOAT2*)m_LinesVbCpuAddr[i];

			*ptr++ = XMFLOAT2(-0.5f, -0.5f);
			*ptr++ = XMFLOAT2(0.5f, -0.5f);
			*ptr++ = XMFLOAT2(0.0f, 0.5f);
		}
	}

	/*
	{ // constant buffer
		auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(64 * 1024);
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &cbDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(
	}
	*/
}
