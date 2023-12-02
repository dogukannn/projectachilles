#include "ClientThread.h"
#define START_TIMEOUT 10

bool ClientThread::Start()
{
	Running = true;
	if (enet_initialize() != 0) {
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return false;
	}

	client = { 0 };
	client = enet_host_create(NULL /* create a client host */,
	                          1 /* only allow 1 outgoing connection */,
	                          2 /* allow up 2 channels to be used, 0 and 1 */,
	                          0 /* assume any amount of incoming bandwidth */,
	                          0 /* assume any amount of outgoing bandwidth */);
	if (client == NULL) {
		fprintf(stderr,
		        "An error occurred while trying to create an ENet client host.\n");
		return false;
	}

	ENetAddress address = { 0 };
	ENetEvent event;
	peer = { 0 };
	/* Connect to some.server.net:1234. */
	enet_address_set_host(&address, "127.0.0.1");
	address.port = 7777;
	/* Initiate the connection, allocating the two channels 0 and 1. */
	peer = enet_host_connect(client, &address, 2, 0);
	if (peer == NULL) {
		fprintf(stderr,
		        "No available peers for initiating an ENet connection.\n");
		return false;
	}
	/* Wait up to 5 seconds for the connection attempt to succeed. */
	if (enet_host_service(client, &event, START_TIMEOUT) > 0 &&
		event.type == ENET_EVENT_TYPE_CONNECT) {
		puts("Connection to some.server.net:1234 succeeded.");
	} else {
		/* Either the 5 seconds are up or a disconnect event was */
		/* received. Reset the peer in the event the 5 seconds   */
		/* had run out without any significant event.            */
		enet_peer_reset(peer);
		puts("Connection to some.server.net:1234 failed.");
	}

    printf("Started the client...\n");

	///* Create a reliable packet of size 7 containing "packet\0" */
	//ENetPacket * packet = enet_packet_create ("im in the game", 
	//										  strlen ("im in the game") + 1, 
	//										  ENET_PACKET_FLAG_RELIABLE);
	///* Extend the packet so and append the string "foo", so it now */
	///* contains "packetfoo\0"                                      */
	////enet_packet_resize(packet, strlen("packetfoo") + 1);

	////strcpy(&packet->data[strlen("packet")], "foo");
	///* Send the packet to the peer over channel id 0. */
	///* One could also broadcast the packet by         */
	///* enet_host_broadcast (host, 0, packet);         */
	//enet_peer_send (peer, 0, packet);
	//enet_host_flush (client);

	//printf("Message sent to the server\n");

	//PushOutgoing("First message from client!!");
	Thread = std::thread([this]()
		{
			Run();
		});
    return true;
}

void ClientThread::Run()
{
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. (WARNING: blocking) */
	while (Running) {
		if(enet_host_service(client, &event, 10) > 0)
		{
			switch (event.type) {
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
			//enet_host_broadcast(client, 0, packet);
			enet_peer_send (peer, 0, packet);
			enet_host_flush (client);
			printf("Message sent to the client\n");
		}
	}
}

ClientThread::~ClientThread()
{
	if (!Running)
		return;
	Running = false;
	Thread.join();
    enet_host_destroy(client);
    enet_deinitialize();
}
