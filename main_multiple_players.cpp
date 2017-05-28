/*
POPL-II Project-I
Multiplayer Game - ProjecTron
Group - 12
*/

//Client Code
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#define max_players 4
#define Size_x 120
#define Size_y 70
int bound_x = Size_x-1;
int bound_y = Size_y-1;


int my_id;
sf::IpAddress server_ip;
long long int player_score;
unsigned short server_port = 55002;
sf::UdpSocket socket_client;
sf::Uint16 player_count, speed;
sf::Uint16 cur_position_x[max_players], cur_position_y[max_players];
sf::Uint16 old_position_x[max_players], old_position_y[max_players];
sf::Uint16 player_status[max_players], players_direction[max_players];
sf::Uint16 dir, next_x, next_y, alive_count, send_no = 1, rec_no, rec_no_old;



//||||||||||||||||||||||TRON CLASS|||||||||||||||||||||||||||||||||||||||||||||||||||



class Tron{
  private:
    std::array<std::array<int, Size_x>, Size_y> board;
    std::string player_username;

  public:
    void set_initials(std::string);
    std::string get_username();
    void set_speed(sf::RenderWindow&);
    void init_board();
    void move();
    void draw(sf::RenderWindow&);
    void update_board();
    void Remove_player(int);
    bool check_collision(int);
};





void Tron::set_initials(std::string username)
{
    player_score = 1;
    player_username = username;
}


std::string Tron::get_username()
{
    return player_username;
}

void Tron::set_speed(sf::RenderWindow& targetWindow)
{
    targetWindow.setFramerateLimit(speed);
}

void Tron::init_board()
{
    for (int y = 0; y < signed(board.size()); y++)
    {
        for (int x = 0; x < signed(board.at(0).size()); x++)
        {
            board.at(y).at(x) = 0;
        }
    }
    for (int x = 0; x < signed(board.at(0).size()); x++)
    {
        board.at(0).at(x) = -1;
    }

    for (int x = 0; x < signed(board.at(0).size()); x++)
    {
        board.at(bound_y).at(x) = -1;
    }
    for (int x = 0; x < signed(board.size()); x++)
    {
        board.at(x).at(0) = -1;
    }
    for (int x = 0; x < signed(board.size()); x++)
    {
        board.at(x).at(bound_x) = -1;
    }
}

//Predict Next X,Y
void Tron::move()
{
    if      (dir == 0) {next_y = cur_position_y[my_id] - 1; next_x = cur_position_x[my_id];}
    else if (dir == 1) {next_x = cur_position_x[my_id] + 1; next_y = cur_position_y[my_id];}
    else if (dir == 2) {next_y = cur_position_y[my_id] + 1; next_x = cur_position_x[my_id];}
    else if (dir == 3) {next_x = cur_position_x[my_id] - 1; next_y = cur_position_y[my_id];}
}




void Tron::draw(sf::RenderWindow& targetWindow)
{
    for (int y = 0; y < signed(board.size()); y++)
    {
        for (int x = 0; x < signed(board.at(0).size()); ++x)
        {
            //0 - empty , 1-4 players , -1 = boundary
            if (board.at(y).at(x) != 0)
            {
                sf::RectangleShape block;
                block.setSize(sf::Vector2f(10, 10));
                block.setPosition(x * 10, y * 10);
                if      (board.at(y).at(x) == 1)  { block.setFillColor(sf::Color::Green); }
                else if (board.at(y).at(x) == 2)  { block.setFillColor(sf::Color::Cyan);  }
                else if (board.at(y).at(x) == 3)  { block.setFillColor(sf::Color::Red);   }
                else if (board.at(y).at(x) == 4)  { block.setFillColor(sf::Color::Blue);  }
                else if (board.at(y).at(x) == -1) { block.setFillColor(sf::Color::White); }
                targetWindow.draw(block);
            }
        }
    }

}


void Tron::Remove_player(int id)
{
    for (int y = 0; y < signed(board.size()); y++)
    {
        for (int x = 0; x < signed(board.at(0).size() - 1); x++)
        {
            if(board.at(y).at(x) = id + 1)
            {
                board.at(y).at(x) = 0;
            }
        }
    }

    for (int x = 0; x < signed(board.at(0).size()); x++)
    {
        board.at(0).at(x) = -1;
    }

    for (int x = 0; x < signed(board.at(0).size()); x++)
    {
        board.at(bound_y).at(x) = -1;
    }
    for (int x = 0; x < signed(board.size()); x++)
    {
        board.at(x).at(0) = -1;
    }
    for (int x = 0; x < signed(board.size()); x++)
    {
        board.at(x).at(bound_x) = -1;
    }


}

//Check for Collisions - Wall and Player
bool Tron::check_collision(int i)
{
    if(player_status[i] == (sf::Uint16)1 && cur_position_y[i] < Size_y && cur_position_x[i] < Size_x && board.at(cur_position_y[i]).at(cur_position_x[i]) != 0)
    {
        player_status[i] = (sf::Uint16)0;
        this->Remove_player(i);
        return true;
    }
    return false;
}

void Tron::update_board()
{
    for(int i=0; i<player_count; i++)
    {
        check_collision(i);
    }

    alive_count = 0;
    for(int i=0;i<player_count; i++)
    {
        if(player_status[i] == (sf::Uint16)1)
        {
            alive_count++;
            board.at(cur_position_y[i]).at(cur_position_x[i]) = i + 1;
        }
    }

}




//|||||||||||||||||||||||||MISCLENIOUS FUNTIONS||||||||||||||||||||||||||||||||||


void copy_location(sf::Uint16 old_location[], sf::Uint16 new_location[])
{
    for(int i=0; i<player_count; i++)
    {
        old_location[i] = new_location[i];
    }
}


// void print_location()
// {
//     for(int i=0;i<player_count;i++)
//     {
//         std::cout << " " << i << " " << player_status[i] << " " << dir << " " << cur_position_x[i] << " " << cur_position_y[i] << "     ";
//     }
// }


//|||||||||||||||||||||||||||||CLIENT CODE||||||||||||||||||||||||||||||||||||||||||||||||||||||||||


namespace client
{

    void connect_server(sf::IpAddress ip, unsigned short port, sf::String username)
    {
        sf::Packet packet;
        packet << username;
        socket_client.send(packet, ip, port);
    }

    void get_inititals()
    {
        sf::IpAddress ip;
        sf::Packet packet;
        unsigned short port;
        socket_client.receive(packet, ip, port);
        packet >> my_id >> player_count >> speed;

        for(int i=0;i<player_count;i++)
        {
            player_status[i] = (sf::Uint16)1;
        }
        // std::cout<<my_id<<" "<<player_count<<" "<<speed<<std::endl;
    }

    void get_locations()
    {
        sf::IpAddress ip;
        sf::Packet packet;
        unsigned short port;
        int id;
        socket_client.receive(packet, ip, port);
        packet >> rec_no;
        // std::cout<<rec_no<<" ";
        if(rec_no > rec_no_old)
        {
            rec_no_old = rec_no;
            for(int i=0;i<player_count;i++)
            {
                packet >> id;
                packet >> player_status[id] >> players_direction[id] >> cur_position_x[id] >> cur_position_y[id];
            }
        }
        else
        {
            get_locations();
        }
    }


    void get_locations1()
    {
        sf::IpAddress ip;
        sf::Packet packet;
        unsigned short port;
        int id;
        socket_client.receive(packet, ip, port);
        packet >> rec_no;
        rec_no_old = rec_no;
        for(int i=0;i<player_count;i++)
        {
            packet >> id;
            packet >> player_status[id] >> players_direction[id] >> cur_position_x[id] >> cur_position_y[id];
        }
    }






    void send_my_location(sf::Uint16 x, sf::Uint16 y)
    {
        sf::Packet packet;
        packet << my_id << send_no << player_status[my_id] << dir << x << y;
        send_no++;
        // std::cout << "Send "<<send_no<<" : "<< my_id <<" " << player_status[my_id] << " " << dir << " " << x << " " << y << std::endl;
        socket_client.send(packet, server_ip, server_port);
    }

}

void create_text(std::string text,int x, int y, int size, int color, sf::RenderWindow& targetWindow)
{
  sf::Text text1;
  sf::Font font;
  font.loadFromFile("arial.ttf");
  text1.setFont(font);
  text1.setString(text);
  switch(color)
  {
    case 0:
        text1.setColor(sf::Color::Green);
    case 1:
        text1.setColor(sf::Color::Blue);
    case 2:
        text1.setColor(sf::Color::White);
    case 3:
        text1.setColor(sf::Color::Yellow);
    case 4:
        text1.setColor(sf::Color::Cyan);
    case 5:
        text1.setColor(sf::Color::Magenta);
    case 6:
        text1.setColor(sf::Color::Red);
        break;
    default:
        break;
    }
  text1.setPosition(x,y);
  text1.setCharacterSize(size);
  targetWindow.draw(text1);

}//create_text("text", );




//||||||||||||||||||||||||||||||||||MAIN CODE||||||||||||||||||||||||||||||


int main()
{
    int count = 0;
    std::string f_str, server_ip_std ,my_std_username;
    sf::Texture SplashScreen;
    sf::Texture Controls;
    sf::Texture texture;

    if(!SplashScreen.loadFromFile("Tron1.jpg")){return EXIT_FAILURE;}
    if(!Controls.loadFromFile("Controls.jpg")){return EXIT_FAILURE;}
    sf::Sprite sprite(texture), Splash_sprite(SplashScreen), Controls_img(Controls);
    int Splash_scr=1,new_game_start = 0, Welcome_scr=0,Welcome_scr_count=0, Join_Game_scr=0,Join_Game_scr_count=0, Controls_scr=0,Username_scr=0,Username_scr_count=0,windowChange=0;
    std::string s="";
    sf::Font font;
    font.loadFromFile("arial.ttf");
    Tron player1;

    // Score Print
    sf::Text score_text;
    score_text.setFont(font);
    score_text.setColor(sf::Color::Red);
    score_text.setPosition(350,650);
    score_text.setCharacterSize(100);

    //Winner Screen
    sf::Text win_text;
    win_text.setFont(font);
    win_text.setColor(sf::Color::Cyan);
    win_text.setPosition(350,450);
    win_text.setCharacterSize(100);

    sf::String my_username = my_std_username;
    std::cout<<"Server Ip = ";
    std::cin>>server_ip_std;
    std::cout<<"Username = ";
    std::cin>>my_std_username;

    
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Window");
    window.setVerticalSyncEnabled(true);

    while (window.isOpen() && new_game_start == 0)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
            {
                window.close();
            }

            if(Splash_scr==1 && event.type== sf::Event::KeyPressed)
            {
                if(event.key.code==sf::Keyboard::Escape)
                {
                    window.close();
                }
                else
                {
                    Splash_scr=0;
                    Welcome_scr=1;
                    windowChange=1;
                }

                if(windowChange==1)
                {
                    windowChange=0;
                    break;
                }
            }

            if(Welcome_scr==1 && event.type==sf::Event::KeyPressed)
            {
                //std::cout << "Entered Welcome Screen" << std::endl;
                switch (event.key.code)
                  {
                       case sf::Keyboard::Up:
                       {
                             if(Welcome_scr_count != 0)--Welcome_scr_count;
                             break;
                       }
                       case sf::Keyboard::Down:
                       {
                             if(Welcome_scr_count != 2)++Welcome_scr_count;
                             break;
                       }
                     case sf::Keyboard::Escape:
                     {
                          window.close();
                     }
                     case sf::Keyboard::Return:
                     {
                           Welcome_scr=0;
                           switch((Welcome_scr_count%4))
                           {
                                   case 0:
                                   {
                                        Join_Game_scr=1;
                                      windowChange = 1;
                                        break;
                                   }
                                   case 1:
                                   {
                                        Controls_scr=1;
                                      windowChange = 1;
                                        break;
                                   }
                                   case 2:
                                   {
                                        window.close();
                                      break;
                                   }
                                   default:
                                   {
                                          break;
                                   }

                            }

                                 break;
                    }
                    default:
                    {
                        break;
                    }

                  }
                if(windowChange == 1)
                {
                    windowChange = 0;
                    Join_Game_scr_count=0;
                    s="";
                    break;
                }

            }

            if(Username_scr==1 && event.type == sf::Event::TextEntered)
            {

                if(event.text.unicode==8){
                    if(s.size())
                    {
                        s.erase(s.size()-1,s.size());
                    }
                }
                else if(Username_scr_count== 0)
                {
                    if (event.text.unicode < 128) {
                        s.push_back((char)event.text.unicode);;
                    }
                }
            
            }

            else if(Username_scr==1 && event.type==sf::Event::KeyPressed)
            {
                switch(event.key.code)
                {
                    case sf::Keyboard::Up:
                    {
                        if(Username_scr_count != 0)
                            --Username_scr_count;
                        break;
                    }
                    case sf::Keyboard::Down:
                    {
                        if(Username_scr_count!= 2)
                            ++Username_scr_count;
                        break;
                    }

                    case sf::Keyboard::Return:
                    {
                        Username_scr=0;
                        
                        switch(Username_scr_count)
                        {
                            case 0:
                            {
                                Username_scr=1;
                                Username_scr_count=1;

                                break;
                            }
                            case 1:
                            {
                                new_game_start=1;
                                windowChange=1;
                                break;
                            }
                            case 2:
                            {
                                Join_Game_scr=1;
                                windowChange=1;
                                break;
                            }
                            default:
                                break;
                        }
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if(windowChange==1)
                {
                    windowChange=0;
                    s="";
                    break;
                }
            }


            if(Join_Game_scr==1 && event.type == sf::Event::TextEntered){

                  if(event.text.unicode==8){
                      if(s.size())
                      {
                          s.erase(s.size()-1,s.size());
                      }
                  }
                  else if(Join_Game_scr_count== 0)
                  {
                      if (event.text.unicode < 128) {
                          s.push_back((char)event.text.unicode);
                      }
                  }
              }
            else if(Join_Game_scr==1 && event.type == sf::Event::KeyPressed)
            {
                switch(event.key.code)
                {
                    case sf::Keyboard::Up:
                    {
                        if(Join_Game_scr_count != 0)--Join_Game_scr_count;
                        break;
                    }
                    case sf::Keyboard::Down:
                    {
                        if(Join_Game_scr_count!= 2)++Join_Game_scr_count;
                        break;
                    }

                    case sf::Keyboard::Return:
                    {
                        Join_Game_scr=0;
                        switch(Join_Game_scr_count)
                        {
                            case 0:
                            {
                                Join_Game_scr=1;
                                Join_Game_scr_count=1;
                                break;
                            }
                            case 1:
                            {
                                Username_scr=1;
                                // previous=1;
                                windowChange=1;
                                break;
                            }
                            case 2:
                            {
                                Welcome_scr=1;
                                windowChange=1;
                                break;
                            }
                            default:
                                break;
                        }
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if(windowChange==1)
                {
                    windowChange=0;
                    Username_scr_count=0;
                    s="";
                    break;
                }
            }

            if(Controls_scr==1&&event.type==sf::Event::KeyPressed)
            {
                switch(event.key.code)
                {
                    case sf::Keyboard::Return:
                    {
                        Controls_scr=0;
                        Welcome_scr=1;
                        windowChange=1;
                        break;
                    }
                    default:
                        break;
                }
                if(windowChange==1)
                {
                    windowChange=0;
                    Welcome_scr_count=0;
                    break;
                }
            }

        }




        if(Splash_scr==1){
            window.clear();
            window.draw(Splash_sprite);
            create_text("Press Any Key To Continue",750,950,15,3,window);
            window.display();
        }

          if(Welcome_scr == 1){
            window.clear();
            window.draw(sprite);
            create_text("Projectron",750, 50, 80,0, window);
            create_text("~>",800, (200+30*(Welcome_scr_count)),27,0,window);
            // create_text("New Game",850,200,27,0, window);
            create_text("Join Game",850,200,27,0, window);
            create_text("Controls",850,230,27,0, window);
            create_text("Quit",850,260,27,0, window);
            window.display();
        }
        if(Username_scr==1){
            window.clear();
            window.draw(sprite);
            create_text("Projectron",750, 50, 80,0, window);
            create_text("~>",780,(300+(350*Username_scr_count)),30,0,window);
            create_text("Enter Username:", 825,250,30,0,window);
            create_text(s,835,300,30,0,window);
            create_text("Proceed to Game",825,650,30,0,window);
            create_text("Previous Menu", 825,1000,20,0,window);
            window.display();
        }
        if(Join_Game_scr==1){
            window.clear();
            window.draw(sprite);
            create_text("Projectron",750, 50, 80,0, window);
            create_text("Join Server",820, 220, 40, 0, window);
            create_text("~>",780,(350+(325*Join_Game_scr_count)),20,0,window);
            create_text("Enter the IP of the Server:",800,300,20,0, window);
            create_text("Next",865,675,20,0,window);
            create_text("Back to Main Menu",815,1000,20,0,window);
            create_text(s, 825,350,25,0,window);
            window.display();
        }
        if(Controls_scr==1){
            window.clear();
            window.draw(sprite);
            create_text("Projectron",750, 50, 80,0, window);
            window.draw(Controls_img);
            Controls_img.setPosition(sf::Vector2f(800,450));
            create_text("Use Arrow keys to Move.",710, 620, 40, 0, window);
            create_text("~>",750,1000,20,0,window);
            create_text("Back to Main Menu",820,1000,20,0,window);
            window.display();
        }

      }
    


    player1.set_initials(my_username);
    server_ip = sf::IpAddress(server_ip_std);

    socket_client.bind(sf::Socket::AnyPort);
    client::connect_server(server_ip, server_port, my_username);
    client::get_inititals();
    client::get_locations1();

    // std::cout<<"Init : "<<std::endl;
    // print_location();




    player1.set_speed(window);
    player1.init_board();


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (player_status[my_id] == (sf::Uint16)1 && event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                    case sf::Keyboard::Up:
                        if(dir!=2)
                            {dir = 0;}
                        break;
                    case sf::Keyboard::Right:
                        if(dir!=3)
                            {dir = 1;}
                        break;
                    case sf::Keyboard::Down:
                        if(dir!=0)
                            {dir = 2;}
                        break;
                    case sf::Keyboard::Left:
                        if(dir!=1)
                            {dir = 3;}
                        break;
                    default:
                        break;
                }
            }
        }
        window.clear();
        window.draw(sprite);

        if(alive_count == 1)
        {
            for(int i=0;i<player_count;i++)
            {
                if(player_status[i] == (sf::Uint16)1)
                {
                    win_text.setString("Player = "+ std::to_string(i)+" Won");
                }
            }
            if(count == 1 && player_status[my_id] == (sf::Uint16)1)
            {
                client::send_my_location(next_x, next_y);
                count++;
            }
            window.draw(win_text);
            f_str = std::to_string(player_score);
            score_text.setString("Your Score is "+f_str);
            window.draw(score_text);
        }
        else
        {
            if(player_status[my_id] == (sf::Uint16)1)
            {
                player1.move();
                client::send_my_location(next_x, next_y);
                copy_location(old_position_y, cur_position_y);
                copy_location(old_position_x, cur_position_x);
                // std::cout<<"\n"<<rec_no_old<<" () ";
                client::get_locations();
                // std::cout<<"Recv : ";
                // print_location();

                if(player1.check_collision(my_id))
                {
                    player_status[my_id] == (sf::Uint16)0;
                    // std::cout<<"\nColi :";
                    // print_location();
                    alive_count--;
                    client::send_my_location(next_x, next_y);
                    f_str = std::to_string(player_score);
                    score_text.setString("Your Score is "+f_str);
                }
                else
                {
                    player1.update_board();
                    player_score++;
                    player1.draw(window);
                }
            }
            else
            {
                // std::cout<<"\n"<<rec_no_old<<" () ";
                client::get_locations();
                // std::cout<<"Recv : ";
                // print_location();
                player1.update_board();
                player1.draw(window);
            }
        }
        window.display();
    }
    return EXIT_SUCCESS;
}















/*
POPL-II Project-I
Multiplayer Game - ProjecTron
Group - 12


With Threads and Semaphores


//Client Code
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <condition_variable>
#define Size_x 120
#define Size_y 70
#define max_players 4

int bound_x = Size_x-1;
int bound_y = Size_y-1;
int my_id, my_score;
sf::IpAddress server_ip;
long long int player_score;
unsigned short server_port = 55002;
sf::UdpSocket socket_client;
sf::Uint16 player_count, speed;
sf::Uint16 cur_position_x[max_players], cur_position_y[max_players];
sf::Uint16 old_position_x[max_players], old_position_y[max_players];
sf::Uint16 player_status[max_players],player_status_old[max_players], players_direction[max_players];
sf::Uint16 temp_x[max_players], temp_y[max_players], temp_dir[max_players], temp_status[max_players];
sf::Uint16 dir = 1, next_x, next_y, alive_count, send_no = 0, rec_no, rec_no_old;



std::condition_variable cv;
std::mutex m;
bool ready = false;
bool processed = true;
bool recieved = false;


//||||||||||||||||||||||TRON CLASS|||||||||||||||||||||||||||||||||||||||||||||||||||



class Tron{
  private:
    std::array<std::array<int, Size_x>, Size_y> board;
    std::string player_username;

  public:
    void set_initials(std::string);
    std::string get_username();
    void set_speed(sf::RenderWindow&);
    void init_board();
    void move();
    void draw(sf::RenderWindow&);
    void update_board();
    void Remove_player(int);
    bool check_collision(int);
};





void Tron::set_initials(std::string username)
{
    player_score = 1;
    player_username = username;
}


std::string Tron::get_username()
{
    return player_username;
}

void Tron::set_speed(sf::RenderWindow& targetWindow)
{
    targetWindow.setFramerateLimit(speed);
}

void Tron::init_board()
{
    for (int y = 0; y < signed(board.size()); y++)
    {
        for (int x = 0; x < signed(board.at(0).size()); x++)
        {
            board.at(y).at(x) = 0;
        }
    }
    for (int x = 0; x < signed(board.at(0).size()); x++)
    {
        board.at(0).at(x) = -1;
    }

    for (int x = 0; x < signed(board.at(0).size()); x++)
    {
        board.at(bound_y).at(x) = -1;
    }
    for (int x = 0; x < signed(board.size()); x++)
    {
        board.at(x).at(0) = -1;
    }
    for (int x = 0; x < signed(board.size()); x++)
    {
        board.at(x).at(bound_x) = -1;
    }
}

//Predict Next X,Y
void Tron::move()
{
    if      (dir == 0) {next_y = cur_position_y[my_id] - 1; next_x = cur_position_x[my_id];}
    else if (dir == 1) {next_x = cur_position_x[my_id] + 1; next_y = cur_position_y[my_id];}
    else if (dir == 2) {next_y = cur_position_y[my_id] + 1; next_x = cur_position_x[my_id];}
    else if (dir == 3) {next_x = cur_position_x[my_id] - 1; next_y = cur_position_y[my_id];}
}




void Tron::draw(sf::RenderWindow& targetWindow)
{
    for (int y = 0; y < signed(board.size()); y++)
    {
        for (int x = 0; x < signed(board.at(0).size()); ++x)
        {
            //0 - empty , 1-4 players , -1 = boundary
            if (board.at(y).at(x) != 0)
            {
                sf::RectangleShape block;
                block.setSize(sf::Vector2f(10, 10));
                block.setPosition(x * 10, y * 10);
                if      (board.at(y).at(x) == 1)  { block.setFillColor(sf::Color::Green); }
                else if (board.at(y).at(x) == 2)  { block.setFillColor(sf::Color::Cyan);  }
                else if (board.at(y).at(x) == 3)  { block.setFillColor(sf::Color::Red);   }
                else if (board.at(y).at(x) == 4)  { block.setFillColor(sf::Color::Blue);  }
                else if (board.at(y).at(x) == -1) { block.setFillColor(sf::Color::White); }
                targetWindow.draw(block);
            }
        }
    }

}


void Tron::Remove_player(int id)
{
    for (int y = 1; y < signed(board.size() - 1); y++)
    {
        for (int x = 1; x < signed(board.at(0).size() - 1); x++)
        {
            if(board.at(y).at(x) = id + 1)
            {
                board.at(y).at(x) = 0;
            }
        }
    }

    //Added This
    for (int x = 0; x < signed(board.at(0).size()); x++)
    {
        board.at(0).at(x) = -1;
    }

    for (int x = 0; x < signed(board.at(0).size()); x++)
    {
        board.at(bound_y).at(x) = -1;
    }
    for (int x = 0; x < signed(board.size()); x++)
    {
        board.at(x).at(0) = -1;
    }
    for (int x = 0; x < signed(board.size()); x++)
    {
        board.at(x).at(bound_x) = -1;
    }


}

//Check for Collisions - Wall and Player
bool Tron::check_collision(int i)
{
    if(player_status[i] == (sf::Uint16)1 && cur_position_y[i] <= Size_y && cur_position_x[i] <= Size_x && board.at(cur_position_y[i]).at(cur_position_x[i]) != 0)
    {
        player_status[i] = (sf::Uint16)0;
        alive_count--;
        this->Remove_player(i);
        return true;
    }
    return false;
}

void Tron::update_board()
{
    for(int i=0; i<player_count; i++)
    {
        check_collision(i);
    }
    alive_count = 0;

    for(int i=0;i<player_count; i++)
    {
        if(player_status[i] == (sf::Uint16)1)
            alive_count++;
            board.at(cur_position_y[i]).at(cur_position_x[i]) = i + 1;
    }

}




//|||||||||||||||||||||||||MISCLENIOUS FUNTIONS||||||||||||||||||||||||||||||||||


void copy_location(sf::Uint16 old_location[], sf::Uint16 new_location[])
{
    for(int i=0; i<player_count; i++)
    {
        old_location[i] = new_location[i];
    }
}


void print_location()
{
    for(int i=0;i<player_count;i++)
    {
        std::cout << " " << i << " " << player_status[i] << " " << dir << " " << cur_position_x[i] << " " << cur_position_y[i] << "     ";
    }
}


//|||||||||||||||||||||||||||||CLIENT CODE||||||||||||||||||||||||||||||||||||||||||||||||||||||||||


namespace client
{

    void connect_server(sf::IpAddress ip, unsigned short port, sf::String username)
    {
        sf::Packet packet;
        packet << username;
        socket_client.send(packet, ip, port);
    }

    void get_inititals()
    {
        sf::IpAddress ip;
        sf::Packet packet;
        unsigned short port;
        socket_client.receive(packet, ip, port);
        packet >> my_id >> player_count >> speed;

        for(int i=0;i<player_count;i++)
        {
            player_status[i] = (sf::Uint16)1;
        }
        std::cout<<my_id<<" "<<player_count<<" "<<speed<<std::endl;
    }

    void get_locations()
    {
        sf::IpAddress ip;
        sf::Packet packet;
        unsigned short port;
        int id;


        while(alive_count != 1)
        {


            socket_client.receive(packet, ip, port);
            packet >> rec_no;
            alive_count =0;
            if(rec_no > rec_no_old)
            {
                rec_no_old = rec_no;
                for(int i=0;i<player_count;i++)
                {
                    packet >> id;
                    packet >> player_status[id] >> players_direction[id] >> cur_position_x[id] >> cur_position_y[id];
                    std::cout<<id<<" "<<player_status[id]<<" "<<players_direction[id]<<" "<< cur_position_x[id]<<" "<<cur_position_y[id];
                }
                send_no++;

                {
                    std::unique_lock<std::mutex> lk(m);
                    recieved = true;
                }
                cv.notify_one();
            }
            packet.clear();
        }
    }


    void get_locations1()
    {

            sf::IpAddress ip;
            sf::Packet packet;
            unsigned short port;
            int id;
            socket_client.receive(packet, ip, port);
            packet >> rec_no;
            rec_no_old = rec_no;
            for(int i=0;i<player_count;i++)
            {
                packet >> id;
                packet >> player_status[id] >> players_direction[id] >> cur_position_x[id] >> cur_position_y[id];
            }
    }






    void send_my_location()
    {
        sf::Packet packet;

        while(alive_count != 1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            packet.clear();
            packet << my_id << send_no << player_status[my_id] << dir << next_x << next_y;

            std::cout << "Send "<<send_no<<" : "<< my_id <<" " << player_status[my_id] << " " << dir << " " << next_x << " " << next_y << std::endl;
            socket_client.send(packet, server_ip, server_port);
        }

    }

}

//||||||||||||||||||||||||||||||||||MAIN CODE||||||||||||||||||||||||||||||

int main()
{
    int count = 0;
    std::string f_str, server_ip_std ,my_std_username;
    int game_start_screen = 1;
    sf::Texture texture;
    sf::Sprite sprite(texture);
    std::thread t0, t1;

    sf::Font font;
    font.loadFromFile("arial.ttf");
    Tron player1;

    // Score Print
    sf::Text score_text;
    score_text.setFont(font);
    score_text.setColor(sf::Color::Red);
    score_text.setPosition(350,650);
    score_text.setCharacterSize(100);

    //Winner Screen
    sf::Text win_text;
    win_text.setFont(font);
    win_text.setColor(sf::Color::Cyan);
    win_text.setPosition(350,450);
    win_text.setCharacterSize(100);

    std::cout << "Username = ";
    std::cin >> my_std_username;
    sf::String my_username = my_std_username;

    std::cout << "Server Ip = ";
    std::cin >> server_ip_std;
    server_ip = sf::IpAddress(server_ip_std);

    socket_client.bind(sf::Socket::AnyPort);
    client::connect_server(server_ip, server_port, my_username);
    client::get_inititals();
    client::get_locations1();

    std::cout<<"Init : "<<std::endl;
    print_location();

    player1.set_initials(my_username);

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Window");
    window.setVerticalSyncEnabled(true);
    player1.set_speed(window);
    player1.init_board();
    player1.move();

                send_no++;
                                send_no++;
                                                send_no++;
                                                                send_no++;

    t0 = std::thread(client::get_locations);
    t1 = std::thread(client::send_my_location);


    while (window.isOpen() && game_start_screen == 1)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (player_status[my_id] == (sf::Uint16)1 && event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                    case sf::Keyboard::Up:
                        if(dir!=2)
                            {dir = 0;}
                        break;
                    case sf::Keyboard::Right:
                        if(dir!=3)
                            {dir = 1;}
                        break;
                    case sf::Keyboard::Down:
                        if(dir!=0)
                            {dir = 2;}
                        break;
                    case sf::Keyboard::Left:
                        if(dir!=1)
                            {dir = 3;}
                        break;
                    default:
                        break;
                }
            }
        }
        window.clear();
        window.draw(sprite);

        if(alive_count == 1)
        {
            for(int i=0;i<player_count;i++)
            {
                if(player_status[i] == (sf::Uint16)1)
                {
                    win_text.setString("Player = "+ std::to_string(i)+" Won");
                }
            }
            window.draw(win_text);
            f_str = std::to_string(player_score);
            score_text.setString("Your Score is "+f_str);
            window.draw(score_text);
        }
        else
        {
            player1.move();
            {
                std::unique_lock<std::mutex> lk(m);
                cv.wait(lk, []{return recieved;});
                processed = false;

                if(player_status[my_id] == (sf::Uint16)1)
                {
                    if(player1.check_collision(my_id))
                    {
                        std::cout<<"\nColi :";
                        print_location();
                        alive_count--;
                        f_str = std::to_string(player_score);
                        score_text.setString("Your Score is "+f_str);
                    }
                    else
                    {
                        player1.update_board();
                        player_score++;
                        player1.draw(window);
                    }
                }
                else
                {
                    player1.update_board();
                    player1.draw(window);
                }
                recieved = false;
                processed = true;
                lk.unlock();
            }
            cv.notify_one();
        }
        window.display();
    }
    return EXIT_SUCCESS;
}

*/
