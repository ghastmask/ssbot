#include "../botdb.h"
#include <gtest/gtest.h>

//opEntry *findOperator(char *name);
//opEntry *addOperator(char *name, Operator_Level level);
//opEntry *addOperator(char *name, char *pass, Operator_Level level);
//bool removeOperator(char *name);

//void aliasCommand(char *&command);
//void addAlias(char *command, char *alias);
//bool killAlias(char *alias);
//cmdAlias *findAlias(char *alias);
//String getAliasList(char *command);

TEST(botdb_ut, find_operator)
{
	BOT_DATABASE bd;
	EXPECT_EQ(nullptr, bd.findOperator("foo"));
	bd.addOperator("foo", Operator_Level::OP_SysOp);
	auto * op = bd.findOperator("foo");
	EXPECT_EQ("foo", op->getName());
	EXPECT_EQ(Operator_Level::OP_SysOp, op->getAccess());
}

TEST(botdb_ut, add_operator)
{
	BOT_DATABASE bd;

	bd.addOperator("foo", Operator_Level::OP_Player);
	bd.addOperator("foo2", Operator_Level::OP_SysOp);
	bd.addOperator("foo3", Operator_Level::OP_Player);
	bd.addOperator("foo4", Operator_Level::OP_Player);
	bd.addOperator("foo5", Operator_Level::OP_God);
	bd.addOperator("foo6", Operator_Level::OP_God);
	
	std::vector<std::string> order{ "foo5", "foo6", "foo2", "foo", "foo3", "foo4" };
	std::vector<opEntry> ops(bd.opList.begin(), bd.opList.end());
	ASSERT_EQ(order.size(), ops.size());
	for (size_t i = 0; i < order.size(); ++i)
	{
		EXPECT_EQ(order[i], ops[i].getName());
	}
}

TEST(botdb_ut, remove_operator)
{
	BOT_DATABASE bd;

	EXPECT_FALSE(bd.removeOperator("foo"));
	EXPECT_FALSE(bd.removeOperator("foo2"));

	bd.addOperator("foo", Operator_Level::OP_Player);
	bd.addOperator("foo2", Operator_Level::OP_SysOp);

	EXPECT_TRUE(bd.removeOperator("foo2"));
	ASSERT_EQ(1U, bd.opList.size());
	EXPECT_STREQ("foo", bd.opList.front().getName());

	EXPECT_FALSE(bd.removeOperator("foo2"));

	EXPECT_TRUE(bd.removeOperator("foo"));
	ASSERT_EQ(0U, bd.opList.size());
}