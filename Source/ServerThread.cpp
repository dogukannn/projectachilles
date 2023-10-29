#include "ServerThread.h"

bool ServerThread::Start()
{
	Running = true;
	if (enet_initialize () != 0) {
        printf("An error occurred while initializing ENet.\n");
        return false;
    }
    ENetAddress address = {0};
    address.host = ENET_HOST_ANY; /* Bind the server to the default localhost.     */
    address.port = 7777; /* Bind the server to port 7777. */
    #define MAX_CLIENTS 32
    /* create a server */
    server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);

    if (server == NULL) {
        printf("An error occurred while trying to create an ENet server host.\n");
        return false;
    }

    printf("Started a server...\n");

	//PushOutgoing("First message from Server!!");

	Thread = std::thread([this]()
		{
			Run();
		});
    return true;
}

void ServerThread::Run()
{
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. (WARNING: blocking) */
	while (Running) {
		if(enet_host_service(server, &event, 10) > 0)
		{
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				printf("A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
				/* Store any relevant client information here. */
				event.peer->data = "Client information";
				peer = event.peer;
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				printf("message is arrived!!\n");
				//printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
				//	event.packet->dataLength,
				//	event.packet->data,
				//	event.peer->data,
				//	event.channelID);
				PushReceieve(*(MoveEvent*)event.packet->data);
				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s disconnected.\n", event.peer->data);
				/* Reset the peer's client information. */
				event.peer->data = NULL;
				break;

			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				printf("%s disconnected due to timeout.\n", event.peer->data);
				/* Reset the peer's client information. */
				event.peer->data = NULL;
				break;

			case ENET_EVENT_TYPE_NONE:
				break;
			}
		}
		if (!peer)
			continue;
		MoveEvent msg;
		while(PopOutgoing(msg))
		{
			ENetPacket * packet = enet_packet_create ((void*)&msg, 
										  sizeof(MoveEvent),
										  ENET_PACKET_FLAG_RELIABLE);
			/* Extend the packet so and append the string "foo", so it now */
			/* contains "packetfoo\0"                                      */
			//enet_packet_resize(packet, strlen("packetfoo") + 1);
			//strcpy(&packet->data[strlen("packet")], "foo");
			/* Send the packet to the peer over channel id 0. */
			/* One could also broadcast the packet by         */
			/* enet_host_broadcast (host, 0, packet);         */
			//enet_host_broadcast(server, 0, packet);
			enet_peer_send (peer, 0, packet);
			enet_host_flush (server);
			printf("Message sent to the client\n");
		}
	}
}

ServerThread::~ServerThread()
{
	Running = false;
	Thread.join();
    enet_host_destroy(server);
    enet_deinitialize();
}
