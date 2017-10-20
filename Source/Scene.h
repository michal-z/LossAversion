#pragma once


class DirectX12;
class Scene
{
public:
	explicit Scene(const DirectX12& dx12);
	void Initialize();
	void Update(double frameTime, float frameTimeDelta);
	void Draw() const;

private:
	const DirectX12& m_Dx12;

	ID3D12PipelineState* m_Pso;
	ID3D12RootSignature* m_Rs;
	ID3D12Resource* m_LinesVb[2];
	uint8_t* m_LinesVbCpuAddr[2];
	D3D12_VERTEX_BUFFER_VIEW m_LinesVbView[2];

	void CreateBuffers();
};

inline Scene::Scene(const DirectX12& dx12) : m_Dx12(dx12)
{
}
