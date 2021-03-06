// CardGameServerRemake.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "iostream"
#include "string"
#include "WS2tcpip.h"
#include "list"
#include "fstream"
#include "Card.h"
#include "sstream"
#include "vector"
#include "algorithm"
#include "conio.h"
#include "thread"
#include "mutex"
#include "deque"
#include "regex"
#include "iterator"
#include "Commands.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "MousePointers.h"
#pragma comment (lib,"ws2_32.lib")

int cardwidth = 105;
int cardheight = 89;
int lastupdate = 0;
int screenwidth = 1920;
int screenheight = 1080;

std::list<PlayerMouse*> Mouses = std::list<PlayerMouse*>();

std::list<Card*> Cards = std::list<Card*>();
std::list<Card*> lastselected = std::list<Card*>();
int players = 5;

std::list<std::string> playerid = std::list<std::string>();

std::list<std::thread> threadstoclose = std::list<std::thread>();
std::mutex mtx;
std::mutex ShutDown;
bool timetoclose = false;

int returnrandom(int before, int now) {



	if (before == now) {


		now = returnrandom(before, rand() & 2 + 0);

	}

	return now;
}


void shakelist(std::list<Card*>* lastselected) {

	int thebefore = 0;

	std::list<Card*> korten2 = std::list<Card*>();

	std::list<Card*> korten3 = std::list<Card*>();



	for (std::list<Card*>::iterator it = lastselected->begin(); it != lastselected->end(); it++) {

		Card* korta = *it;

		thebefore = returnrandom(thebefore, thebefore);

		if (thebefore == 0) {
			korten2.push_back(korta);
		}
		else {
			korten3.push_back(korta);
		}
		korta->flipped = true;
		OutputDebugStringW(std::to_wstring(thebefore).c_str());
		OutputDebugString(L"\n");
		korta->xcord = 20;
		korta->ycord = 20;



	}

	for (std::list<Card*>::iterator it = korten2.begin(); it != korten2.end();it++) {
		lastselected->remove(*it);
		thebefore = returnrandom(thebefore, thebefore);

		if (thebefore == 0) {
			lastselected->push_front(*it);
		}
		else {
			lastselected->push_back(*it);
		}

	}

	for (std::list<Card*>::iterator it = korten3.begin(); it != korten3.end();it++) {
		lastselected->remove(*it);
		thebefore = returnrandom(thebefore, thebefore);

		if (thebefore == 0) {
			lastselected->push_front(*it);
		}
		else {
			lastselected->push_back(*it);
		}

	}


}



Card* getbyid(int cardid) {
	try {
		for (std::list<Card*>::iterator it = lastselected.begin(); it != lastselected.end(); it++) {
			Card* korta = *it;
			if (korta->id == cardid) {
				return korta;
			}
		}
	}
	catch (std::exception e) {
		return NULL;
	}
	return NULL;
}


int server = 0;


std::list<Commands> ReturnCommands(std::string message) {
	std::list<Commands> thecommands = std::list<Commands>();

	if (std::regex_search(message, std::regex("COMMANDSLIST;")) > 0) {
		std::string commands = message;

		commands = commands.replace(0,13,"");


		std::list<std::string> ends = std::list<std::string>();

		std::list<std::string> respond = std::list<std::string>();


		std::istringstream iss(commands);

		std::istreambuf_iterator<char> begin(iss), end;
		std::string line = "";
		while (begin != end) {
			std::string current = std::string(1, char(*begin++));
			
			if (current == ";") {
				line = line + current;
				
				ends.push_back(line);
				line = "";
			}
			else {
				line = line + current;
			}
		}


		for (std::string x : ends) {
			Commands TheCommand;
			TheCommand.variables = std::list<std::string>();
			int at = 0;

			std::istringstream es(x);
			std::istreambuf_iterator<char> begine(es), ende;
			std::string currentline = "";
			while (begine != ende) {
				std::string current = std::string(1, char(*begine++));

				if (current == ";") {
					if (at == 0) {
						TheCommand.command = currentline;
						thecommands.push_back(TheCommand);
						currentline = "";
					}
					else {
						TheCommand.variables.push_back(currentline);
						currentline = "";
						thecommands.push_back(TheCommand);
					}

				}
				if (current == ":") {

					if (at == 0) {
						TheCommand.command = currentline;
						currentline = "";
					}
					else {
						TheCommand.variables.push_back(currentline);
						currentline = "";
					}
					at = at + 1;
				}
				else {
					if (current != ";") {
						currentline = currentline + current;
					}
				}

			}



		}

		
	}
	
	return thecommands;
}



void sendtoport() {

	try {
		players = players + 1;
		WSADATA wsData;
		WORD ver = MAKEWORD(2, 2);



		int wsOk = WSAStartup(ver, &wsData);


		if (wsOk != 0) {

			std::cout << "Error Ok!" << std::endl;
			sendtoport();


		}

		SOCKET listening = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


		if (listening == INVALID_SOCKET) {
			std::cout << "Invalid socket!" << std::endl;
		}

		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(54000);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;



		bind(listening, (sockaddr*)&hint, sizeof(hint));




		listen(listening, SOMAXCONN);

		sockaddr_in client;
		int clientSize = sizeof(client);




		SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
		/*
		const char optval = 1;
		int resulter = setsockopt(clientSocket, NULL,  SO_KEEPALIVE, &optval, sizeof(int));
		*/


		char host[NI_MAXHOST]; // remote name
		char service[NI_MAXHOST]; // service i.e port that the client connection is through


		ZeroMemory(host, NI_MAXHOST); //ZeroMemory
		ZeroMemory(service, NI_MAXHOST); //ZeroMemory

		if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
			std::cout << host << " connected on port " << service << std::endl;
		}
		else {
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
		}

		closesocket(listening);

			char buf[16384];
			std::string playername = "";
			std::thread* nytttrad = new std::thread(sendtoport);
			PlayerMouse* spelaremus = new PlayerMouse;
			spelaremus->mousename = "loading";
			spelaremus->mousex = 25;
			spelaremus->mousey = 25;
			while (true) {
			//mtx.lock();
			ShutDown.lock();
			
			bool wasandis = timetoclose;
			ShutDown.unlock();
			if (wasandis == true) {
				closesocket(clientSocket);
			}
			ZeroMemory(buf,16384);

			// Wait for client to send data :)

			// Echo message back to client


			int bytesReceived = recv(clientSocket, buf, 16384, 0);
			std::string message = std::string(buf, 0, bytesReceived);
			if (bytesReceived == SOCKET_ERROR) {
				std::cout << "Error socket error - probably caused of FORCED disconnect!" << std::endl;
				mtx.lock();

				Mouses.remove(spelaremus);

				mtx.unlock();
				break;

			}
			if (bytesReceived == 0) {
				std::cout << "Client disconnected " << std::endl;
				mtx.lock();
				Mouses.remove(spelaremus);
				mtx.unlock();
				break;
			}


			ZeroMemory(buf,16384);
			if (message == "CREATE ID") {
				mtx.lock();
				spelaremus->mousex = 25;
				spelaremus->mousey = 25;
				spelaremus->mousename = "loading";
				Mouses.push_back(spelaremus);
				std::string userInput = std::to_string(players);
				std::cout << userInput;
				int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);	
				if (sendResult == SOCKET_ERROR) {
					std::cout << "Oh no! \n";
				}

				mtx.unlock();

			}
			
			if (message == "SET PLAYERNAME") {

				std::string okaycommand = "okay";
				send(clientSocket, okaycommand.c_str(), okaycommand.size() + 1, 0);
				std::cout << "SET PLAYERNAME \n";
				ZeroMemory(buf,16384);
				bytesReceived = recv(clientSocket, buf, 16384, 0);
				message = std::string(buf, 0, bytesReceived);
				spelaremus->mousename = message;
				send(clientSocket, message.c_str(), message.size() + 1, 0);


			}


			if (std::regex_search(message, std::regex("CARDUPDATETOSERVER ")) > 0) {
				std::stringstream ss(message);
				std::string thecardid;
				std::string theflipped;
				std::string thexcord;
				std::string theycord;
				ss >> thecardid;
				ss >> thecardid;
				ss >> theflipped;
				ss >> thexcord;
				ss >> theycord;

				float thexwhitelist = stof(thexcord);
				float theywhitelist = stof(theycord);


				mtx.lock();
				if (thexwhitelist + cardwidth > screenwidth) {

					thexcord = std::to_string(screenwidth - cardwidth);

				}

				if (theywhitelist + cardheight > screenheight) {

					theycord = std::to_string(screenheight - cardheight);
		

				}


				if (thexwhitelist < 0) {

					thexcord = "0";

				}

				if (theywhitelist < 0) {
					theycord = "0";
				}

				Card* whichisit = getbyid(stoi(thecardid));
				whichisit->xcord = stof(thexcord);
				whichisit->ycord = stof(theycord);
				whichisit->flipped = stoi(theflipped);
				lastselected.remove(whichisit);
				lastselected.push_back(whichisit);
				mtx.unlock();
			}


			if (message == "CARD WIDTH") {
				mtx.lock();
				std::string userInput = std::to_string(cardwidth);
				mtx.unlock();
				int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);
				if (sendResult == SOCKET_ERROR) {
					std::cout << "Oh no! \n";
				}
				

			}

			if (message == "CARD HEIGHT") {
				mtx.lock();
				std::string userInput = std::to_string(cardheight);
				mtx.unlock();
				int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);
				if (sendResult == SOCKET_ERROR) {
					std::cout << "Oh no! \n";
				}


			}

			if (message == "CARD GET") {
				mtx.lock();
				for (std::list<Card*>::iterator it = lastselected.begin(); it != lastselected.end();it++) {
					Card* korta = *it;
					std::string userInputer = korta->texture + " | " + std::to_string(korta->id) + " | " + std::to_string(korta->flipped) + " | " + std::to_string(korta->xcord) + " | " + std::to_string(korta->ycord);
					send(clientSocket, userInputer.c_str(), userInputer.size() + 1, 0);
					ZeroMemory(buf,16384);
					recv(clientSocket, buf, 16384, 0);
				}
				std::string closeCommand = "CLOSE";

				send(clientSocket, closeCommand.c_str(), closeCommand.size() + 1, 0);

				mtx.unlock();

			}
			
			if (std::regex_search(message, std::regex("COMMANDSLIST;")) > 0) {
				mtx.lock();
				std::string commands = message;
				std::list<Commands> allcommands = ReturnCommands(message);
				std::string response = "COMMANDSLIST;";
				for (Commands x : allcommands) {
					if (x.command == "CARDUPDATECLIENT") {
						for (Card* x : lastselected) {
							std::string toadd = "UPDATECARD:" + std::to_string(x->id) + ":" + std::to_string(x->xcord) + ":" + std::to_string(x->ycord) + ":"+std::to_string(x->flipped)+";";
							response = response + toadd;
						}

					}
					if (x.command == "MOUSEUPDATECLIENT") {
						for (PlayerMouse* xi : Mouses) {
							if (xi != spelaremus) {
								std::string toadd = "UPDATEMOUSE:";
								// Player name has been set to mouse!
								try {
									toadd = toadd + xi->mousename + ":" + std::to_string(xi->mousex) + ":" + std::to_string(xi->mousey) + ";";
									response = response + toadd;

								}
								catch (int e) {

								}
							}
								

							
						}

					}
					if (x.command == "MOUSEUPDATESERVER") {

						

							float xcord;
							float ycord;

							int at = 0;
							for (std::string i : x.variables) {
								
								if (at == 0) {
									xcord = std::stof(i);
								}
								else {
									ycord = std::stof(i);
								}
								at = at + 1;
							}

							spelaremus->mousex = xcord;
							spelaremus->mousey = ycord;
							

						

					}

					if (x.command == "SENDMESSAGE") {

						for (std::string sentmessage : x.variables) {

							
							if (sentmessage == "SHUFFLE") {

							

							}

						}



					}

					if (x.command == "UPDATECARD") {
						int at = 0;
						int cardid;
						float cardx;
						float cardy;
						float cardflipped;
						for (std::string xx : x.variables) {

							if (at == 0) {
								cardid = std::stoi(xx);
							}
							if (at == 1) {
								cardx = std::stof(xx);
							}
							if (at == 2) {
								cardy = std::stof(xx);
							}
							if (at == 3) {
								cardflipped = std::stoi(xx);
							}
				
							at = at + 1;
						}
						Card* thecard = getbyid(cardid);
						if (thecard != nullptr) {
							thecard->xcord = cardx;
							thecard->ycord = cardy;
							thecard->flipped = cardflipped;
							lastselected.remove(thecard);
							lastselected.push_back(thecard);
							if (thecard->xcord < 0) {
								thecard->xcord = 0;
							}
							if (thecard->xcord + cardwidth > screenwidth) {

								thecard->xcord = screenwidth - cardwidth;

							}

							if (thecard->ycord < 0) {

								thecard->ycord = 0;

							}

							if (thecard->ycord + cardheight > screenheight) {


								thecard->ycord = screenheight - cardheight;

							}

						}

					}

				}
				mtx.unlock();
				send(clientSocket, response.c_str(), response.size() + 1, 0);

			}
	

			if (std::regex_search(message, std::regex("CARD UPDATE SIMPEL")) > 0) {
				std::string letsbuildit = "";
				mtx.lock();
				for (std::list<Card*>::iterator it = lastselected.begin(); it != lastselected.end(); it++) {
					Card* thecardtoupdate = *it;
					letsbuildit = letsbuildit + std::to_string(thecardtoupdate->id) + " " + std::to_string(thecardtoupdate->flipped) + " " + std::to_string(thecardtoupdate->xcord) + " " + std::to_string(thecardtoupdate->ycord) + " \n";
				}
				mtx.unlock();
				send(clientSocket, letsbuildit.c_str(), letsbuildit.size() + 1, 0);
				
			}

			if (message == "CARD PLAYER UPDATE LIST") {
				std::string letsbuildt = "";
			}

			if (message == "CARD FLIP") {
				mtx.lock();
				shakelist(&lastselected);
				lastupdate = lastupdate + 1;
				std::cout << "CARD FLIP COMMAND!";
				mtx.unlock();
			}


			ZeroMemory(buf,16384);

		}

		closesocket(clientSocket);

	}
	catch (std::exception e) {

		std::cout << "Error!";
	}

	//WSACleanup();

}




BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{

	case CTRL_CLOSE_EVENT:
		printf("Ctrl-Close event\n\n");


		
			ShutDown.lock();

			timetoclose = true;

			ShutDown.unlock();


		

		return TRUE;

	default:
		return FALSE;
	}
}



int main()
{

	if (SetConsoleCtrlHandler(CtrlHandler, TRUE))
	{
		std::cout << "Control handler!";
	}

	std::ifstream input("card/cards.txt");

	std::string line;

	int at = 0;	

	for (std::string line; getline(input, line); )
	{

		LPCWSTR sd;
		std::wstring input = std::wstring(line.begin(), line.end());


		Card* kortet = new Card(at, 20, 20, line, true);
		kortet->flipped = -1;
	
		lastselected.push_back(kortet);
		Cards.push_back(kortet);



		//std::cout << "Added card! " << std::to_string(at) << " " << line << std::endl;



		at = at + 1;


	}




	std::thread* nytttrad = new std::thread(sendtoport);

	srand(time(NULL));
	while (true) {
		std::string command;
		std::getline(std::cin,command);
		std::cout << command;

		if (command == "SHUFFEL") {

			std::list<Card*> theshuffel = std::list<Card*>();
			mtx.lock();

			for (Card* x : lastselected) {
				int current = rand() & 0 + 1;

				if (current == 0) {
					theshuffel.push_back(x);
				}
				else {
					theshuffel.push_front(x);
				}
				x->xcord = 20;
				x->ycord = 20;
				x->flipped = -1;

			}
			lastselected = theshuffel;
			std::cout << "DONE!";


			mtx.unlock();

		}

	}
	return 0;
}


