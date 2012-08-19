#ifndef __LW_POD_MODELEXT_H__
#define __LW_POD_MODELEXT_H__

namespace lw{

	class AnimChannel{
	public:
		static AnimChannel* create(FILE* pf, int frameCount);
		virtual ~AnimChannel(){}
		virtual float value(float t) = 0;
	};

	class LinerChannel : public AnimChannel{
	public:
		LinerChannel(FILE* pf, int frameCount);
		virtual float value(float t);

	private:
		std::vector<float> _frames;
	};

	class StepChannel : public AnimChannel{
	public:
		StepChannel(FILE* pf);
		virtual float value(float t);

	private:
		std::map<int, float> _frames;
	};
} //namespace lw


#endif //__LW_POD_MODELEXT_H__