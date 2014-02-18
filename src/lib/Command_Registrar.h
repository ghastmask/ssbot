#ifndef COMMAND_REGISTRAR_H
#define COMMAND_REGISTRAR_H

#include "command.h"
#include <memory>

struct Command_Create_Base
{
	virtual std::unique_ptr<Cmd> create(Command_Config const & cfg) = 0;
};

template < class Command_T >
struct Command_Create : public Command_Create_Base
{
	virtual std::unique_ptr<Cmd> create(Command_Config const & cfg)
	{
		return std::make_unique<Command_T>(cfg);
	}
};

struct Command_Factory
{ 
	static std::unique_ptr<Cmd> create(Command_Config const & cfg);
	
	template < class T >
	static bool register_command()
	{
		factory[T::name()] = std::make_unique<Command_Create<T>>();
		return true;
	}
	using Factory = std::map< std::string, std::unique_ptr<Command_Create_Base> >;
	static Factory factory;
};

#endif