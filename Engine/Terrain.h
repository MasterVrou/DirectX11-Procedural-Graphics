#pragma once
#include "PerlinNoise.h"

using namespace DirectX;

class Terrain
{
private:
	int m_offset = 2;
	float m_scale = 0.1f;
	int m_noiseMultiplier = 7;
	PerlinNoise perlinNoise;

	struct VertexType
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texture;
		DirectX::SimpleMath::Vector3 normal;
	};
	struct HeightMapType
	{
		float x, y, z;
		float nx, ny, nz;
		float u, v;
	};
	
	struct Triangle
	{
		DirectX::SimpleMath::Vector3 vertex[3];
	};
	struct Square
	{
		Triangle triangles[2];
	};
public:
	Terrain();
	~Terrain();

	bool Initialize(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Render(ID3D11DeviceContext*);
	bool GenerateWaterMap(ID3D11Device*);
	bool GenerateGroundMap(ID3D11Device*);
	bool Update();
	float* GetWavelength();
	float* GetAmplitude();
	float* GetScale();
	int GetOffset();
	void SetOffset(int offset);
	void UpdateChanges(ID3D11Device* device);

	std::vector<Square> squares;
	std::vector<DirectX::SimpleMath::Vector3> highPoints;
	DirectX::SimpleMath::Vector3 highPoint1 = DirectX::SimpleMath::Vector3(0 ,0, 0);
	DirectX::SimpleMath::Vector3 highPoint2 = DirectX::SimpleMath::Vector3(0, 0, 0);
	DirectX::SimpleMath::Vector3 highPoint3 = DirectX::SimpleMath::Vector3(0, 0, 0);


private:
	bool CalculateNormals();
	void Shutdown();
	void ShutdownBuffers();
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);
	bool pointDistance(DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B);

private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	ID3D11Buffer * m_vertexBuffer, *m_indexBuffer;
	 
	int m_vertexCount, m_indexCount;
	float m_frequency, m_amplitude, m_wavelength;
	HeightMapType* m_heightMap;

	//arrays for our generated objects Made by directX
	std::vector<VertexPositionNormalTexture> preFabVertices;
	std::vector<uint16_t> preFabIndices;
};

