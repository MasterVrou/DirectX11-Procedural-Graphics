#include "Collision.h"
#include "pch.h"


bool RaySphereIntersection(DirectX::SimpleMath::Vector3 og, DirectX::SimpleMath::Vector3 dest, DirectX::SimpleMath::Vector3 center, float r)
{
    //D = B^2 - 4 * A * G 
    float A = pow(dest.x - og.x, 2) + pow(dest.y - og.y, 2) + pow(dest.z - og.z, 2);
    float B = 2 * ((dest.x - og.x) * (og.x - center.x) + (dest.y - og.y) * (og.y - center.y) + (dest.z - og.z) * (og.z - center.z));
    float G = pow(og.x - center.x, 2) + pow(og.y - center.y, 2) + pow(og.z - center.z, 2) - pow(r, 2);

    float D = pow(B, 2) - 4 * A * G;

    if (D >= 0)
    {
        return true;
    }

    return false;
}

bool SpherePlaneIntersection(DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B, DirectX::SimpleMath::Vector3 D, DirectX::SimpleMath::Vector3 center, float r)
{
    //sphere bottom point
    //DirectX::SimpleMath::Vector3 center = DirectX::SimpleMath::Vector3(center.x, center.y, center.z);

    float maxCx = center.x + 3;
    float minCx = center.x - 3;
    float maxCz = center.z + 3;
    float minCz = center.z - 3;


    if (center.y >= A.y - 0.5 && center.y <= A.y + 0.5)
    {
        if (maxCz >= A.z && minCz <= B.z)
        {
            if (maxCx >= A.x && minCx <= D.x)
            {
                return true;
            }
        }
    }

    return false;
}
