#ifndef SUBJECT_H
#define SUBJECT_H

#include "Observer.h"

#include <vector>

namespace Lib{

class Subject
{
public:
    virtual void addObserver(Observer* ob){
        m_observers.push_back(ob);
	}
	
	virtual void removeObserver(Observer* ob)
	{
        for(std::vector<Observer*>::iterator it = m_observers.begin();it != m_observers.end();++it){
            if(*it == ob){
                m_observers.erase(it);
                break;
            }
        }
	}
	
	virtual void notifyAll(void* msg)
	{
		for(std::vector<Observer*>::size_type i=0;i<m_observers.size();++i)
			m_observers[i]->notify(msg);
	}
	
protected:
	std::vector<Observer*> m_observers;
};

}

#endif // !SUBJECT_H
