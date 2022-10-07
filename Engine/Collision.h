#pragma once
#include "DeviceResources.h"
#include <SimpleMath.h>


class Collision 
{
public:
	bool RayTriIntersect(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 dest, DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 p3);
	bool RaySphereIntersection(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 dest, DirectX::SimpleMath::Vector3 center, float r);
	bool SpherePlaneIntersection(DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B, DirectX::SimpleMath::Vector3 D, DirectX::SimpleMath::Vector3 center, float r);
};
