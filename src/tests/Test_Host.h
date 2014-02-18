#ifndef TEST_HOST_H
#define TEST_HOST_H

#include "../lib/host.h"
#include <string>
#include <vector>

struct Test_Host : public Host_Base
{
	Test_Host(BOT_INFO & info) : Host_Base(info) { }
	void sendPrivate(Player *player, char *msg) { private_msgs_.push_back(msg); }
	void sendTeam(char *msg) { team_msgs_.push_back(msg); }
	void sendTeamPrivate(Uint16 team, char *msg){ }
	void sendPublic(char *msg) { public_msgs_.push_back(msg); }
	void sendPublicMacro(char *msg) { }
	void sendChannel(char *msg) { }			// #;Message
	void sendRemotePrivate(char *msg) { }		// :Name:Messsage
	void sendRemotePrivate(char *name, char *msg) { }

	std::vector < std::string > private_msgs_;
	std::vector < std::string > public_msgs_;
	std::vector < std::string > team_msgs_;
	std::vector < std::string > team_private_msgs_;
	std::vector < std::string > public_macro_msgs_;
	std::vector < std::string > channel_msgs_;

	void postRR(clientMessage *cm)  { }
	void revokeAccess(BYTE access) { }
	void revokeAccess(char *name) { }
	void changeArena(char *name) { };
	void disconnect(bool notify) { };

};

#endif