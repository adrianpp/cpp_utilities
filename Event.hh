#ifndef _UTIL_BASE_EVENT_H__
#define _UTIL_BASE_EVENT_H__

#include <string>
#include <vector>
#include <map>
#include <memory>

class EventBase {
public:
  EventBase();
	virtual ~EventBase();
	typedef std::string Topic;
	virtual Topic getTopic()=0;
};

class ListenerBase {
public:
  ListenerBase();
	virtual void processEvent(std::shared_ptr<EventBase>)=0;
};

template<class EventClass, class FuncType> class FuncListener : public ListenerBase {
	FuncType func;
public:
  FuncListener(FuncType f) : func(f) {}
	virtual void processEvent(std::shared_ptr<EventBase> event)
	{
		if( std::shared_ptr<EventClass> castEvent = std::dynamic_pointer_cast<EventClass>(event) )
			func(castEvent);
	}
};

template<class EventClass, class FuncType>
std::shared_ptr<ListenerBase> createListenerForEvent(FuncType func)
{
	return std::shared_ptr<ListenerBase>(new FuncListener<EventClass,FuncType>(func));
}

class EventRouter {
private:
	std::map<EventBase::Topic, std::vector<std::shared_ptr<ListenerBase> > > listeners;
public:
  EventRouter();
	void registerListener(std::shared_ptr<ListenerBase> listener, EventBase::Topic topic="*"); //default to all topics
	void unregisterListener(std::shared_ptr<ListenerBase> listener, EventBase::Topic topic="*"); //default to all topics
	void publishEvent(std::shared_ptr<EventBase> event);
	void publishEvent(EventBase* event); //utility function that constructs shared_ptr and passes it to publish; will take ownership
};

#endif
