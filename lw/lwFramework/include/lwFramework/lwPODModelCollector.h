#ifndef __LW_POD_MODEL_COLLECTOR_H__
#define __LW_POD_MODEL_COLLECTOR_H__

#include "lwPODModelRes.h"

namespace lw{

	class PODModelCollector
	{
	public:
		static void collect(class PODModel* pModel, int nodeIndex, AlphaMode alphaMode);
		static void draw();
	};


} //namespace lw


#endif //__LW_POD_MODEL_COLLECTOR_H__