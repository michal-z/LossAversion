#include "Pch.h"
#include "DirectX12.h"
#include "Library.h"
#include "Scene.h"


void Scene::Initialize()
{
	m_Dx12.GetCmdList()->Close();

	std::vector<uint8_t> csoVS = LoadFile("Assets/Shaders/TriangleVS.cso");
	std::vector<uint8_t> csoPS = LoadFile("Assets/Shaders/SolidPS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.VS = { csoVS.data(), csoVS.size() };
	psoDesc.PS = { csoPS.data(), csoPS.size() };
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	VHR(m_Dx12.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_Pso)));
	VHR(m_Dx12.GetDevice()->CreateRootSignature(0, csoVS.data(), csoVS.size(), IID_PPV_ARGS(&m_Rs)));
}

void Scene::Update(double frameTime, float frameTimeDelta)
{
}

void Scene::Draw()
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
	cmdList->OMSetRenderTargets(1, &backBufferHandle, 0, nullptr);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	cmdList->ClearRenderTargetView(backBufferHandle, clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(depthBufferHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

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
