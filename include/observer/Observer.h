#ifndef OBSERVER_H
#define OBSERVER_H

namespace Lib{

class Observer
{
public:
    virtual ~Observer(){ }
    virtual void notify(void*) = 0;
};

}

#endif // !OBSERVER_H
