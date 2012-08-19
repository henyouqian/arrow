#include "stdafx.h"
#include "lwTask.h"

namespace lw {

	typedef std::list<Task*> TaskList;
	typedef std::map<int, TaskList> TaskMap;
	typedef std::list<Task*> PendingTasks;

	struct TaskMgrData{
		TaskMap _tasks;
		TaskList _allTasks;
		PendingTasks _pendingTasks;
	};

	namespace{
		TaskMgrData* _spData = NULL;
	}
	

	void Task::addDelayCall(DelayCall dc, float t){
		lwassert(dc);
		DelayCallData dcd;
		dcd.delayCall = dc;
		dcd.t = t;
		_delayCallList.push_back(dcd);
	}

	//void TaskMgr::init(){
	//	lwassert(_spData == NULL);
	//	_spData = new TaskMgrData;
	//	lwassert(_spData);
	//}

	void TaskMgr::quit(){
		lwassert(_spData);
		removeAll();
		delete _spData;
		_spData = NULL;
	}

	bool TaskMgr::add(Task& task, int priority){
		if ( task._status != Task::STATUS_IDLE ){
			lwerror(L"task._status != Task::STATUS_IDLE, status=" << task._status);
			return false;
		}
		task._status = Task::STATUS_ADDING;
		task._priority = priority;
		_spData->_pendingTasks.push_back(&task);
		return true;
	}

	bool TaskMgr::remove(Task& task){
		if ( task._status != Task::STATUS_RUNNING ){
			lwerror(L"task._status != Task::STATUS_RUNNING, status=" << task._status);
			return false;
		}
		task._status = Task::STATUS_REMOVEING;
		_spData->_pendingTasks.push_back(&task);
		return true;
	}

	void TaskMgr::removeAll(){
		TaskMap::iterator it = _spData->_tasks.begin();
		TaskMap::iterator end = _spData->_tasks.end();
		TaskList::iterator itl;
		TaskList::iterator endl;
		Task* pTask = NULL;
		for ( ; it != end; ++it ){
			itl = it->second.begin();
			endl = it->second.end();
			for ( ; itl != endl; ++itl ){
				pTask = *itl;
				lwassert(pTask);
				pTask->vEnd();
			}
		}
		_spData->_tasks.clear();
		_spData->_pendingTasks.clear();

		{
			TaskList& tasks = _spData->_allTasks;
			TaskList::iterator it = tasks.begin();
			TaskList::iterator itEnd = tasks.end();
			for ( ; it != itEnd; ++it ){
				(*it)->vDestroy();
			}
			tasks.clear();
		}
	}

	void TaskMgr::main(float elepsed){
		TaskMap& tasks = _spData->_tasks;

		TaskMap::iterator it = tasks.begin();
		TaskMap::iterator end = tasks.end();
		TaskList::iterator itl;
		TaskList::iterator endl;
		Task* pTask = NULL;
		for ( ; it != end; ++it ){
			itl = it->second.begin();
			endl = it->second.end();
			for ( ; itl != endl; ++itl ){
				pTask = *itl;
				lwassert(pTask);
				pTask->vMain(elepsed);
				{
					std::list<Task::DelayCallData>::iterator it = pTask->_delayCallList.begin();
					std::list<Task::DelayCallData>::iterator itEnd = pTask->_delayCallList.end();
					while ( it != itEnd ){
						it->t -= elepsed;
						if ( it->t <= 0 ){
							(pTask->*(it->delayCall))();
							it = pTask->_delayCallList.erase(it);
							itEnd = pTask->_delayCallList.end();
						}else{
							++it;
						}
					}
				}
				
				
				//pTask->_status = Task::STATUS_IDLE;
			}
		}
		
	}

	void TaskMgr::draw(float elapsed){
		TaskMap& tasks = _spData->_tasks;
		TaskMap::iterator it = tasks.begin();
		TaskMap::iterator end = tasks.end();
		TaskList::iterator itl;
		TaskList::iterator endl;
		Task* pTask = NULL;
		for ( ; it != end; ++it ){
			itl = it->second.begin();
			endl = it->second.end();
			for ( ; itl != endl; ++itl ){
				pTask = *itl;
				lwassert(pTask);
				pTask->vDraw(elapsed);
				//pTask->_status = Task::STATUS_IDLE;
			}
		}
	}

	void TaskMgr::processPending(){
		TaskMap& tasks = _spData->_tasks;
		PendingTasks& pendingTasks = _spData->_pendingTasks;
		TaskMap::iterator it = tasks.begin();
		TaskMap::iterator end = tasks.end();
		TaskList::iterator itl;
		TaskList::iterator endl;
		TaskList::iterator itPending;
		TaskList::iterator endPending;
		Task* pTask = NULL;
		if ( !pendingTasks.empty() ){
			itPending = pendingTasks.begin();
			endPending = pendingTasks.end();
			for ( ; itPending != endPending; ++itPending ){
				pTask = *itPending;
				lwassert(pTask);
				Task::Status status = pTask->_status;
				int priority = pTask->_priority;
				it = tasks.find(priority);
				end = tasks.end();
				if ( status == Task::STATUS_ADDING ){
					if ( it == end ){
						TaskList tl;
						tasks[priority] = tl;
						it = tasks.find(priority);
					}
					it->second.push_back(pTask);
					pTask->_status = Task::STATUS_RUNNING;
					pTask->vBegin();
				}else if ( status == Task::STATUS_REMOVEING ){
					if ( it == end ){
						lwerror(L"remove task failed: task priority doesn't exist: " << priority);
						continue;
					}else{
						itl = it->second.begin();
						endl = it->second.end();
						for ( ; itl != endl; ++itl ){
							if ( *itl == pTask ){
								it->second.erase(itl);
								pTask->_status = Task::STATUS_IDLE;
								pTask->vEnd();
								break;
							}
						}
					}
				}else{
					lwerror(L"status must be Task::STATUS_ADDING or Task::STATUS_REMOVEING");
				}
			}
			pendingTasks.clear();
		}
	}

	void TaskMgr::onTouchEvent(std::vector<TouchEvent>& events){
		TaskMap& tasks = _spData->_tasks;
		TaskMap::iterator it = tasks.begin();
		TaskMap::iterator end = tasks.end();
		TaskList::iterator itl;
		TaskList::iterator endl;
		Task* pTask = NULL;
		for ( ; it != end; ++it ){
			itl = it->second.begin();
			endl = it->second.end();
			for ( ; itl != endl; ++itl ){
				pTask = *itl;
				lwassert(pTask);
				if ( pTask->_status == Task::STATUS_RUNNING ){
					if ( pTask->vOnTouchEvent(events) ){
						return;
					}
				}
			}
		}
	}

	Task::Task()
	:_status(STATUS_IDLE),_priority(0){
		if ( _spData == NULL ){
			_spData = new TaskMgrData;
			lwassert(_spData);
		}
			
		TaskList& tasks = _spData->_allTasks;
		tasks.push_back(this);
	}

	Task::~Task(){
		if ( _spData ){
			TaskList& tasks = _spData->_allTasks;
			TaskList::iterator it = tasks.begin();
			TaskList::iterator itEnd = tasks.end();
			for ( ; it != itEnd; ++it ){
				if ( *it == this ){
					tasks.erase(it);
					return;
				}
			}
		}
	}

	bool Task::start(int priority){
		return TaskMgr::add(*this, priority);
	}

	bool Task::stop(){
		return TaskMgr::remove(*this);
	}

} //namespace lw {