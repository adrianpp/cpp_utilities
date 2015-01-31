#include "Event.hh"
#include <algorithm>

EventBase::EventBase()
{
}

EventBase::~EventBase()
{
}

ListenerBase::ListenerBase()
{
}

std::vector<EventBase::Topic> getAllPossibleTopics(EventBase::Topic topic) //given a topic like MachineEvent::Memory::Read, returns the topics *, MachineEvent::*, MachineEvent::Memory::*, and MachineEvent::Memory::Read
{
	std::vector<EventBase::Topic> ret;
	size_t pos = 0;
	while( true )
	{
		size_t new_pos = topic.find("::", pos);
		if( new_pos == std::string::npos )
		{
			ret.push_back(topic);
			break;
		}
		new_pos += 2; //length of "::"
		ret.push_back(topic.substr(0, new_pos)+"*");
		pos = new_pos;
	}
	return ret;
}

EventRouter::EventRouter()
{
}

void EventRouter::registerListener(std::shared_ptr<ListenerBase> listener, EventBase::Topic topic)
{
	listeners[topic].push_back(listener);
}

void EventRouter::unregisterListener(std::shared_ptr<ListenerBase> listener, EventBase::Topic topic)
{
	while( true )
	{
		std::vector<std::shared_ptr<ListenerBase> >::iterator LI = std::find(listeners[topic].begin(), listeners[topic].end(), listener);
		if(LI == listeners[topic].end() )
			break;
		listeners[topic].erase(LI);
	}
}

void EventRouter::publishEvent(std::shared_ptr<EventBase> event)
{
	if( !event )
		return;
	std::vector<EventBase::Topic> topic_matches = getAllPossibleTopics(event->getTopic());
	for(std::vector<EventBase::Topic>::iterator TI = topic_matches.begin(); TI != topic_matches.end(); ++TI)
	{
		for(std::vector<std::shared_ptr<ListenerBase> >::iterator LI = listeners[*TI].begin(); LI != listeners[*TI].end(); ++LI)
		{
			if( *LI == NULL )
				continue;
			(*LI)->processEvent(event);
		}
	}
}

void EventRouter::publishEvent(EventBase* event)
{
	publishEvent(std::shared_ptr<EventBase>(event));
}
