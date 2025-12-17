module visibility;

import modules.common;

namespace visibility
{
    public func frustumVisible(common::BoundingSphere sphere, common::Frustum frustum)->bool
    {
        for (int i = 0; i < 6; i++)
        {
            let plane = frustum.planes[i];
            let distance = dot(plane.xyz, sphere.center) + plane.w;
            if (distance < -sphere.radius)
                return false;
        }
        return true;
    }
}
