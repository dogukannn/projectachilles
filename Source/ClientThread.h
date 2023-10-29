#pragma once
#include <queue>
#include <thread>

#include "NetMessages.h"

class ClientThread : public MessageQueues<MoveEvent>
{
public:
	bool Start();

	void Run();

	~ClientThread();

	ENetHost* client = nullptr;
    ENetPeer* peer = nullptr;

	bool Running = false;
	std::thread Thread;
};
