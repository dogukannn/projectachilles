#pragma once
#include <queue>
#include <thread>

#include "NetMessages.h"

class ServerThread : public MessageQueues<MoveEvent>
{
public:
	bool Start();

	void Run();

	~ServerThread();

	ENetHost* server = nullptr;
    ENetPeer* peer = nullptr;

	bool Running = false;
	
	std::thread Thread;
};
