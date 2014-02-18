#include "Password.h"

static bool reg = Command_Factory::register_command<Password_Cmd>();

Password_Cmd::
Password_Cmd(Command_Config const & cfg)
: Cmd(cfg)
{
}

std::string const & 
Password_Cmd::
name()
{
	static std::string name("password");
	return name;
}

void
Password_Cmd::
execute(Host_Base & h, Command & c, Player * p)
{
	String s;
	if (*c.final_)
	{
		s += "?password=";
		s += c.final_;
		h.sendPublic(s.msg);

		h.botInfo.setLogin(h.botInfo.name, c.final_, h.botInfo.staffpw);

		h.sendPrivate(p, "Updated local bot parameters and requested network password change");
	}
	else
	{
		s += "Current bot password: ";
		s += h.botInfo.password;
		h.sendPrivate(p, s.msg);
	}
}