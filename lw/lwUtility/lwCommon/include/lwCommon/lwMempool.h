#ifndef __LW_MEMPOOL_H__
#define	__LW_MEMPOOL_H__

/****************************************************************
注意此类不会调用构造析构，不会检测重复free，不会自动free
****************************************************************/

#include <list>
#include <assert.h>

namespace lw
{

template <typename T, UINT NR_ELEM>
class Mempool
{
public:
	typedef T Elem_t;
	Mempool()
	{
		for ( unsigned i = 0; i < NR_ELEM; ++i )
		{
			_freeList.push_back(_elems+i);
		}
	}

	Elem_t* malloc()
	{
		if ( _freeList.empty() )
			return NULL;
		else
		{
			Elem_t* p = (*_freeList.begin());
			_freeList.pop_front();
			return p;
		}
	}
	void free(Elem_t*& p)
	{
		assert(p >= _elems && p <= _elems + NR_ELEM - 1);
		_freeList.push_front(p);
		p = 0;
	}


private:
	Elem_t	_elems[NR_ELEM];
	std::list<Elem_t*> _freeList;
};

}	//namespace lw

#endif // __LW_MEMPOOL_H__