#pragma once


class DirectX12;
class Scene
{
public:
	explicit Scene(const DirectX12& dx12);
	void Initialize();
	void Update(double frameTime, float frameTimeDelta);
	void Draw();

private:
	const DirectX12& m_Dx12;

	ID3D12PipelineState* m_Pso;
	ID3D12RootSignature* m_Rs;
};

inline Scene::Scene(const DirectX12& dx12) : m_Dx12(dx12)
{
}