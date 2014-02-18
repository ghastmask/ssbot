// Move to Command
#include "Test_Host.h"
#include "../lib/commtypes.h"
#include "../lib/commands/Password.h"
#include "../lib/Command_Registrar.h"
#include <gtest/gtest.h>

namespace {
	bool executed = false;
	struct My_Command : public Cmd
	{
		My_Command(Command_Config const & cfg) : Cmd(cfg) { }
		void execute(Host_Base & h, Command & c, Player * p)
		{
			executed = true;
		}
		static std::string const & name() {
			static std::string n("My_Command");
			return n;
		}
	};
}


TEST(command_registrar_ut, my_command)
{
	Player p(25, "test", "testsquad", 1, 1, 1, 1, 1, 1, false, 0);
	BOT_INFO info;
	Test_Host h(info);
	Command_Config c{ "My_Command", "help", OP_Owner, true };
	EXPECT_THROW(Command_Factory::create(c), std::runtime_error);
	Command_Factory::register_command<My_Command>();
	auto it = Command_Factory::create(c);
	Command cmd("foo");
	EXPECT_FALSE(executed);
	it->execute(h, cmd, &p);
	EXPECT_TRUE(executed);
}

TEST(command_registrar_ut, password_auto_registration)
{

	Command_Config c{ "password", "help", OP_Owner, true };
	auto it = Command_Factory::create(c);
	EXPECT_EQ("help", it->config().help);
	EXPECT_EQ("password", it->config().name);
	EXPECT_EQ(OP_Owner, it->config().min_level);
	EXPECT_TRUE(it->config().show_in_help);

	Command_Config c2{ "unknown", "help", OP_Owner, true };
	EXPECT_THROW(Command_Factory::create(c2), std::runtime_error);
}
