#ifndef __LW_PICKING_H__
#define __LW_PICKING_H__

namespace lw{

	class Camera;
	void getPickingRay(cml::Vector3& origin, cml::Vector3& dir, short screenX, short screenY, const Camera& cam);

} //namespace lw

#endif //__LW_PICKING_H__