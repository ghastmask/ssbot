#include "Command_Registrar.h"


std::unique_ptr<Cmd>
Command_Factory::
create(Command_Config const & cfg)
{
	auto && it = factory.find(cfg.name);
	if (it == factory.end())
	{
		throw std::runtime_error(cfg.name + " is not a valid command.");
	}
	return it->second->create(cfg);
}
Command_Factory::Factory Command_Factory::factory;
