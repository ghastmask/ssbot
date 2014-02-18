#include "Test_Host.h"
// Move to Command
#include "../lib/commtypes.h"
#include "../lib/commands/Password.h"
#include "../lib/host.h"
#include <gtest/gtest.h>

//bool check(char *msg);			// Check against cmd
//bool checkParam(char *msg);		// Check against final

//void addParam(char *msg);		// Add a switch
//_switch *getParam(char type);	// Check against switches

TEST(command_ut, checks)
{
	Command c("test command again_test");
	ASSERT_TRUE(c.check("test"));
	ASSERT_FALSE(c.check("tests"));
	ASSERT_FALSE(c.check("!test"));

	ASSERT_TRUE(c.checkParam("command again_test"));
	ASSERT_FALSE(c.checkParam("again_test"));
}

TEST(command_ut, password)
{
	Player p(25, "test", "testsquad", 1, 1, 1, 1, 1, 1, false, 0);
	Command_Config c{ "password","help", OP_Owner, true };
	Password_Cmd cmd(c);
	BOT_INFO info;
	{
		Test_Host h(info);
		Command c("test command again_test");
		cmd.execute(h, c, &p);
		EXPECT_EQ("?password=command again_test", h.public_msgs_[0]);
		EXPECT_EQ("Updated local bot parameters and requested network password change", h.private_msgs_[0]);
	}
	{
		Test_Host h(info);
		Command c("test");
		strcpy(h.botInfo.password, "foo");
		cmd.execute(h, c, &p);
		EXPECT_EQ("Current bot password: foo", h.private_msgs_[0]);
	}
}
