// Server Code
/**
  * POPL-II Project-I
  * Multiplayer Game - ProjecTron
  * Group - 12
*/
#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <ctime>
#include <condition_variable>
#define max_players 4
#define Size_x 120
#define Size_y 70


//Common Variables.
sf::UdpSocket socket;
int alive_count;
sf::IpAddress Players_IP[max_players];
unsigned short Players_PORT[max_players];
sf::String usernames[max_players];
std::string s_usernames[max_players];


//Shared Variable Between Threads
sf::Uint16 new_position_x[max_players], new_position_y[max_players], new_direction[max_players];
sf::Uint16 old_position_x[max_players], old_position_y[max_players], old_direction[max_players];
sf::Uint16 packet_send_no, packet_recv_no[max_players], packet_recv_no_old[max_players]; 
sf::Uint16 speed = 30, player_count, player_status[max_players];
std::mutex m_location;
std::mutex m_print;


//Function Which Accepts Connections till player 4 or count given
void accept_connections()
{
	sf::Packet packet;
	sf::IpAddress Client_IP;
	int count;
	unsigned short Client_Port;
	std::cout << "Players Connected:" << std::endl;
	std::cout << "Enter Player Count ? ->  ";
	std::cin >> count;
	while(count > max_players)
	{
		std::cout << "Maximum Players Allowed is \"" << max_players << "\" \nEnter Player count Again -> ";
		std::cin >> count;
	}
	while(player_count < count)
	{
		socket.receive(packet, Client_IP, Client_Port);
		packet >> usernames[player_count];
		s_usernames[player_count] = usernames[player_count]; 
		Players_IP[player_count] = Client_IP;
		Players_PORT[player_count] = Client_Port;
		// std::cout <<  player_count << " " << s_usernames[player_count] << "   -   " << Client_IP << ":" << Client_Port << std::endl;
		player_count++;
	}
}

void copy_location(sf::Uint16 old_location[], sf::Uint16 new_location[])
{
	for(int i=0; i<player_count; i++)
	{
		old_location[i] = new_location[i]; 
	}
}

// void print_location()
// {
// 	m_print.lock();
// 	for(int i=0;i<player_count;i++)
// 	{
// 		std::cout << i << " " << player_status[i] << " " << new_direction[i] << " " << new_position_x[i] << " " << new_position_y[i] << std::endl;
// 	}
// 	m_print.unlock();
// }



//Send Each Client their ID, Player_connected
//And Initialize their initial Location

void initialize_and_send()
{
	sf::Packet packet;
	srand(time(NULL));
	for(int i=0; i<player_count; i++)
	{
		packet.clear();
		new_position_x[i] = (sf::Uint16)(rand()%(Size_x/2) + Size_x/4);
		new_position_y[i] = (sf::Uint16)(rand()%(Size_y/2) + Size_y/4);
		new_direction[i] =  (sf::Uint16)(rand()%4);
		player_status[i] = (sf::Uint16)1;
		packet << i << player_count << speed;
		socket.send(packet, Players_IP[i], Players_PORT[i]);
	}
}



//Send Current Positions of all Players
void broadcast_players(sf::Uint16 packet_no)
{
	sf::Packet packet;
	// m_print.lock();
	packet << packet_no;	
	// std::cout<<"Send : "<< packet_no;
	for(int i=0;i<player_count;i++)
	{
		packet << i << player_status[i] << new_direction[i] << new_position_x[i] << new_position_y[i];
		// std::cout << i << " "<< player_status[i] << " "<< new_direction[i]<< " " << new_position_x[i] << " "<< new_position_y[i]<<"    ";
	}
	// std::cout<<std::endl;
	// m_print.unlock();
	for(int i=0; i<player_count; i++)
	{
		socket.send(packet, Players_IP[i], Players_PORT[i]);
	}
}






//Recieve Updated Positions form Clients
//This Function is called by Thread
void recieve_locations()
{
	while(alive_count != 1)
	{
		sf::Packet packet;
		sf::IpAddress ip;
		unsigned short port;
		int id;
		sf::Uint16 x, y, dir, status, t_count;
		
		packet.clear();
		socket.receive(packet, ip, port);
		packet  >> id >> t_count >> status >> dir >> x >> y;


		m_location.lock();
		if(t_count > packet_recv_no_old[id])
		{
			packet_recv_no[id] = t_count;
			packet_recv_no_old[id] = packet_recv_no[id];
			new_position_x[id] = x;
			new_position_y[id] = y;
			new_direction[id]  = dir;
			player_status[id]  = status;
		}
			// m_print.lock();
			// std::cout<<"Recv : "<< t_count << " " << id <<  " " << status << " " << dir << " " << x << " "<< y << std::endl;
			// m_print.unlock();
		// 	// lastmodified = false;
		// }
		// else
		// {
		// 	m_print.lock();
		// 	std::cout<<"Rejected Recv : "<< t_count << " " << id <<  " " << status << " " << dir << " " << x << " "<< y << std::endl;
		// 	m_print.unlock();
		// }
		m_location.unlock();
	}
}



// This Function is called by Thread
// Send Updated Loction of players after every 50 ms
void send_locations()
{
	//while > 1 player remains
	while(alive_count != 1)
	{

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		m_location.lock();
		packet_send_no++;
		alive_count = 0;

		// Search If Some Player didnt send his location
		// If so assume same firection going
		for(int i=0;i<player_count; i++)
		{
			if(player_status[i] == (sf::Uint16)1)
			{
				alive_count++;
				if(new_position_x[i] == old_position_x[i] && new_position_y[i] == old_position_y[i])
				{	
					switch(new_direction[i])
					{
						case 0:
							new_position_y[i] = new_position_y[i] - 1;
							new_position_x[i] = new_position_x[i];
							break;

						case 1:
							new_position_y[i] = new_position_y[i];
							new_position_x[i] = new_position_x[i] + 1;
							break;

						case 2:
							new_position_y[i] = new_position_y[i] + 1;
							new_position_x[i] = new_position_x[i];
							break;

						case 3:
							new_position_y[i] = new_position_y[i];
							new_position_x[i] = new_position_x[i] - 1;
							break;
					}
					// std::cout<<"modified - "<<new_direction[i]<<" "<<new_position_x[i]<<" "<<new_position_y[i]<<" == "<<old_position_x[i] <<" "<<old_position_y[i]<<" STATUS = "<<player_status[i]<<std::endl;
					packet_recv_no_old[i] = packet_recv_no_old[i]+1;
				}
			}
		}

		//Sent many times so that loss minimized
		broadcast_players(packet_send_no);
		broadcast_players(packet_send_no);
		copy_location(old_position_y, new_position_y);
		copy_location(old_position_x, new_position_x);
		copy_location(old_direction, new_direction);
		broadcast_players(packet_send_no);
		broadcast_players(packet_send_no);
		broadcast_players(packet_send_no);
		m_location.unlock();
	}
}



int main()
{

	int i;
	sf::IpAddress server_IP = sf::IpAddress::getLocalAddress();
	if(socket.bind(55002) != sf::Socket::Done)
	{
		std::cout << "Error in Port Binding port tried = 55002 " << std::endl;
		return 0;
	}
	std::cout << "Connect to "<< server_IP.toString() << ":55002" << " For Incomming Connections" << std::endl;


	//Accept Connections from Clients
	accept_connections();

	//Initialize Them
	initialize_and_send();
	copy_location(old_position_x, new_position_x);
	copy_location(old_position_y, new_position_y);
	copy_location(old_direction, new_direction);
    
	// std::cout<<"Init : "<<std::endl;
	// print_location();


	//BroadCast Initial Location
	broadcast_players(0);

	//Give players time to react
	sleep(1);

	std::thread t_send, t_recieve;
	t_send = std::thread(send_locations);
	t_recieve = std::thread(recieve_locations);

	// Wait For Thread to finish
	t_send.join();
	t_recieve.join();

	
	// for(int i=0;i<player_count ; i++)
	// {
	// 	if(player_status[i]==(sf::Uint16)1)
	// 		std::cout<<"User = "<<s_usernames[i]<<" Won"<<std::endl;
	// }
	// print_location();
	return 0;
}
