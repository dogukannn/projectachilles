#pragma once
#include <mutex>
#include <queue>
#include <glm/vec3.hpp>

struct MoveEvent
{
	std::string UnitName;
	glm::vec3 Target;
};

template<class T>
class MessageQueues
{
	std::mutex receive;
	std::mutex outgoing;

	std::queue<T> ReceivingQueue;
	std::queue<T> OutgoingQueue;

public:
	void PushReceieve(T in);
	bool PopReceive(T& out);

	void PushOutgoing(T in);
	bool PopOutgoing(T& out);
};

template <class T>
void MessageQueues<T>::PushReceieve(T in)
{
	std::unique_lock lock(receive);
	ReceivingQueue.push(in);
}

template <class T>
bool MessageQueues<T>::PopReceive(T& out)
{
	std::unique_lock lock(receive);
	if(!ReceivingQueue.empty())
	{
		out = ReceivingQueue.front();
		ReceivingQueue.pop();
		return true;
	}
	return false;
}

template <class T>
void MessageQueues<T>::PushOutgoing(T in)
{
	std::unique_lock lock(outgoing);
	OutgoingQueue.push(in);
}

template <class T>
bool MessageQueues<T>::PopOutgoing(T& out)
{
	std::unique_lock lock(outgoing);
	if(!OutgoingQueue.empty())
	{
		out = OutgoingQueue.front();
		OutgoingQueue.pop();
		return true;
	}
	return false;
}

