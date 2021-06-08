/*
 * Author:      Richard Cornwell (rich@sky-visions.com)
 *
 * Copyright (C) 2021 Richard Cornwell.
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE.QPL included in the packaging of this file.
 *
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#pragma once
#include <functional>

namespace core
{

class IEventCallback
{
    public:
    virtual ~IEventCallback() = default;
    virtual void operator() (void *ev) = 0;
    virtual bool operator == (IEventCallback* other) = 0;
};

template <class T>
class EventCallback : public IEventCallback
{
public:
    EventCallback(T* instance, void (T::*function)(void *ev))
        : instance(instance), function(function) {}
    virtual ~EventCallback() = default;
    void operator () (void *ev)
    {
        (instance->*function)(ev);
    }

    bool operator == (IEventCallback *other) override
    {
        EventCallback<T> *otherEventCallback = dynamic_cast<EventCallback<T> *>(other);
        if (otherEventCallback == nullptr)
            return false;
        return (this->function == otherEventCallback->function) &&
               (this->instance == otherEventCallback->instance);
    }
private:
    T* instance;
    void (T::*function)(void *ev);
};

class Event
{
public:
    Event()
    {
    }
    virtual ~Event()
    {
    }

    void addListener(IEventCallback* action)
    {
        Callbacks::iterator pos = find(callbackactions_.begin(),
                                       callbackactions_.end(), action);
        if (pos != callbackactions_.end()) {
            return;
        }
        callbackactions_.push_back(action);
    }
    void removeListener(IEventCallback* action)
    {
        Callbacks::iterator pos = find(callbackactions_.begin(),
                                       callbackactions_.end(), action);
        if (pos != callbackactions_.end()) {
            return;
        }
        callbackactions_.erase(pos);
    }
    void notify(void *ev)
    {
        for(IEventCallback* action : callbackactions_) {
            (*action)(ev);
        }
    }

private:
    typedef std::vector<IEventCallback*> Callbacks;
    Callbacks callbackactions_;
};

}
