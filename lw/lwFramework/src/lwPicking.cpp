#include "stdafx.h"
#include "lwPicking.h"
#include "lwCamera.h"
#include "lwApp.h"

namespace lw{

	void getPickingRay(cml::Vector3& origin, cml::Vector3& dir, short screenX, short screenY, const Camera& cam){
		float px = 0.0f;
		float py = 0.0f;
		
#ifdef __APPLE__
		const cml::Matrix44& proj = cam.getRawProj();
#else
		const cml::Matrix44& proj = cam.getProj();
#endif
		const App::Config config = App::getConfig();
		px = ((( 2.0f*screenX) / config.width) - 1.0f) / proj.data()[0];
		py = (((-2.0f*screenY) / config.height) + 1.0f) / proj.data()[5];

		origin[0] = 0; origin[1] = 0; origin[2] = 0;
		dir[0] = px; dir[1] = py; dir[2] = -1.f;
		
		cml::Matrix44 viewInverse = cam.getView();
		viewInverse.inverse();
		origin = cml::transform_point(viewInverse, origin);
		dir = cml::transform_vector(viewInverse, dir);
		dir.normalize();
	}

} //namespace lw
