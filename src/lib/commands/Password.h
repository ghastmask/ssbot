#include "../command.h"
#include "../Command_Registrar.h"

struct Password_Cmd : public Cmd
{
	Password_Cmd(Command_Config const & cfg);
	static std::string const & name();

	virtual void execute(Host_Base & h, Command & c, Player * p) override;
};