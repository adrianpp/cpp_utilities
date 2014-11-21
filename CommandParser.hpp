/**************************************

This file facilitates creating a list of commands that can then be executed by parsing a string.
The main class is CommandParser. It has several functions to add commands, and the ability to execute a string.
As a motivating example, consider the following:

class UserType;
. . .
void simpleFunction();
void boolFunction(bool arg);
void twoArgFunction(bool arg0, int arg1);
void userTypeFunction(UserType arg);
. . .
CommandParser cmdList;
cmdList.addCommand<>("simple", "A simple command that takes no arguments", std::bind(&simpleCommand));
cmdList.addCommand<bool>("moreComplicated", "A slightly more complicated command that takes a single argument", std::bind(&boolFunction, _1));
cmdList.addCommand<bool,int>("twoArgs", "A call that takes two arguments", std::bind(&twoArgFunction, _1, _2));
cmdList.addCommand<UserType>("userFunc", "A call that takes a non-builtin type", std::bind(&userTypeFunction, _1));
. . .
cmdList.execute("simple"); //calls simpleCommand(), returns true
cmdList.execute("moreComplicated false"); //calls boolFunction(false), returns true
cmdList.execute("moreComplicated true"); //calls boolFunction(true), returns true
cmdList.execute("twoArgs true 7"); //calls twoArgFunction(true, 7), returns true
cmdList.execute("userFunc myCustomParsedValue"); //calls userTypeFunction() with the UserType created by parsing 'myCustomParsedValue', returns true
cmdList.execute("novalid expression"); //calls nothing, returns false


CommandParser will work with any function value, including function pointers, functors, std::bind (including binding to class functions), and c++11 lambda expressions.
This leads to incredibly powerful and easy scripting capabilities.

For CommandParser to accept a command, the template class TokenParser<ArgumentType> must be defined for every argument. There are no built-in TokenParsers - this is
because different applications may need slightly different requirements, even for built-in types (parsing an int in an application may require it to be in hex, for example).
Creating a TokenParser<> template specialization is very simple:

template<> class TokenParser<bool> {
public:
	static ParsedToken<bool> parse(std::string token)
	{
		if( token == "1" or token == "on" or token == "true" or token == "TRUE" )
			return ParsedToken<bool>(true);
		if( token == "0" or token == "off" or token == "false" or token == "FALSE" )
			return ParsedToken<bool>(false);
		return ParsedToken<bool>();
	}
};

. . . for example, allows any function that takes a bool to accept a multitude of different strings as valid input to that function.
If a valid parsing cannot be completed (either because the token did not match, or any other reason), a ParsedToken<Type>() must be returned to signal that the parsing
did not complete successfully.

Additionally, a help string that lists the valid commands along with their arguments and descriptions is available as the return value of function CommandParser::getHelpString();
For the above example, this would return a string with the following contents:

Valid options:
  simple                        - A simple command that takes no arguments
  moreComplicated  [bool]       - A slightly more complicated command that takes a single argument
  twoArgs          [bool] [int] - A call that takes two arguments
  userFunc         [UserType]   - A call that takes a non-builtin type


The following is a full program listing that combines all of the above:

#include "CommandParser.hpp"

class UserType {
	//intentionally empty
};

template<>
class TokenParser<UserType> {
public:
  static ParsedToken<UserType> parse(std::string s)
	{
		return ParsedToken<UserType>(UserType());
	}
};
template<>
class TokenParser<bool> {
public:
  static ParsedToken<bool> parse(std::string s)
	{
		if( s == "1" )
			return ParsedToken<bool>(true);
		if( s == "0" )
			return ParsedToken<bool>(false);
		return ParsedToken<bool>();
	}
};
template<>
class TokenParser<int> {
public:
  static ParsedToken<int> parse(std::string s)
	{
		int val = -1;
    std::stringstream ss(s);
		ss >> val;
		std::stringstream check;
		check << val;
		if( check.str() == s ) //check passed!
			return ParsedToken<int>(val);
		return ParsedToken<int>();
	}
};

#include <iostream>

void simpleFunction()
{
	std::cout << "SimpleFunction() called.\n";
}
void boolFunction(bool arg)
{
	std::cout << "boolFunction(" << arg << ") called.\n";
}
void twoArgFunction(bool arg0, int arg1)
{
	std::cout << "twoArgFunction(" << arg0 << ", " << arg1 << ") called.\n";
}
void userTypeFunction(UserType arg)
{
	std::cout << "userTypeFunction() called.\n";
}

int main()
{
	using namespace std::placeholders;
	CommandParser cmdList;
	cmdList.addCommand<>("simple", "A simple command that takes no arguments", std::bind(&simpleFunction));
	cmdList.addCommand<bool>("moreComplicated", "A slightly more complicated command that takes a single argument", std::bind(&boolFunction, _1));
	cmdList.addCommand<bool,int>("twoArgs", "A call that takes two arguments", std::bind(&twoArgFunction, _1, _2));
	cmdList.addCommand<UserType>("userFunc", "A call that takes a non-builtin type", std::bind(&userTypeFunction, _1));

	std::cout << cmdList.getHelpString();

	cmdList.execute("simple"); //calls simpleFunction(), returns true
	cmdList.execute("moreComplicated 0"); //calls boolFunction(false), returns true
	cmdList.execute("moreComplicated 1"); //calls boolFunction(true), returns true
	cmdList.execute("twoArgs 1 7"); //calls twoArgFunction(true, 7), returns true
	cmdList.execute("userFunc myCustomParsedValue"); //calls userTypeFunction() with the UserType created by parsing 'myCustomParsedValue', returns true
	cmdList.execute("novalid expression"); //calls nothing, returns false

  return 0;
}

Running this program should print out:

Valid options:
  simple                        - A simple command that takes no arguments
  moreComplicated  [bool]       - A slightly more complicated command that takes a single argument
  twoArgs          [bool] [int] - A call that takes two arguments
  userFunc         [UserType]   - A call that takes a non-builtin type
SimpleFunction() called.
boolFunction(0) called.
boolFunction(1) called.
twoArgFunction(1, 7) called.
userTypeFunction() called.

***************************************/

#ifndef _COMMAND_PARSER_HPP__
#define _COMMAND_PARSER_HPP__

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#error CommandParser.hpp requires --std=c++0x!
#else

#include <memory>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <stdexcept>

//A class that contains a parsed token. If parsing failed, isValid() returns false, and getValue() cannot be called. If parsing
//succeeded, then isValid() returns true and getValue() can be called.
//It is not necessary to specialize this template.
template<class T>
class ParsedToken {
	std::shared_ptr<T> value;
public:
	ParsedToken() : value(NULL) {}
	ParsedToken(T v) : value(new T(v)) {}
	bool isValid() {return value != NULL;}
	T getValue() {if(!isValid()) throw std::runtime_error("Cannot convert incorrectly parsed token!"); return *value;}
};


//For every type used as an argument to a CommandParser command, TokenParser<> must be specialized with that argument type.
template<class T>	class TokenParser {
	template<class U> class always_false {
	public:
		const static bool value = false;
	};
public:
  //In your specialization, you must have a static function parse() that takes a std::string and returns a ParsedToken<SameTypeAsSpecialization>
  static ParsedToken<T> parse(std::string)
	{
		static_assert(always_false<T>::value, "You must specialize TokenParser<> with the type listed above as T in order to use CommandParser - see comments at top of CommandParser.hpp for example.");
	}
};

//This class facilitates getting a human-readable name from a type. If a specific string is needed,
//a specialization can be created that returns the specific string needed - see specializations for bool,
//int, and std::string below for an example.
template<class T>
class HumanReadableTypename {
public:
	static std::string get()
	{
		std::stringstream ss;
		ss << typeid(T).name();
		int n;
		ss >> n;
		std::string ret;
		getline(ss, ret);
		return ret;
	}
};

template<>
class HumanReadableTypename<bool> {
public:
  static std::string get() {return "bool";}
};

template<>
class HumanReadableTypename<int> {
public:
  static std::string get() {return "int";}
};

template<>
class HumanReadableTypename<std::string> {
public:
  static std::string get() {return "string";}
};

//Base class of a command
class CommandOptionBase {
	std::string commandName;
  std::string description;
public:
  CommandOptionBase(std::string c, std::string d) : commandName(c), description(d) {}
  std::string getCommandName() {return commandName;}
	std::string getDescription() {return description;}
	virtual std::string getArgumentString()=0;
	//parse returns true if parsing succeeded, otherwise returns false
	//parse will always be called before exec, allowing parsed values to be saved
	virtual bool parse(std::string)=0;
	//executes the command
	virtual void exec()=0;
};


//A helper class of a command with no arguments
template<class Function>
class CommandOption0 : public CommandOptionBase {
	Function func;
public:
  CommandOption0(std::string c, std::string d, Function f) : CommandOptionBase(c,d), func(f) {}
	virtual std::string getArgumentString() {return "";}
	virtual bool parse(std::string optionStr)
	{
		return true;
	}
	virtual void exec()
	{
		func();
	}
};

//A helper class of a command with 1 argument
template<class Argument0, class Function>
class CommandOption1 : public CommandOptionBase {
	Function func;
	ParsedToken<Argument0> argument0;
public:
  CommandOption1(std::string c, std::string d, Function f) : CommandOptionBase(c,d), func(f) {}
	virtual std::string getArgumentString()
	{
		std::stringstream ss;
		ss << " [" << HumanReadableTypename<Argument0>::get() << "]";
		return ss.str();
	}
	virtual bool parse(std::string optionStr)
	{
		std::stringstream str(optionStr);
#define parseArg(n) std::string arg##n; str >> arg##n; argument##n = TokenParser<Argument##n>::parse(arg##n); if(!argument##n.isValid())return false;
		parseArg(0);
#undef parseArg
		return true;
	}
	virtual void exec()
	{
		func(argument0.getValue());
	}
};

//A helper class of a command with 2 arguments
template<class Argument0, class Argument1, class Function>
class CommandOption2 : public CommandOptionBase {
	Function func;
	ParsedToken<Argument0> argument0;
	ParsedToken<Argument1> argument1;
public:
  CommandOption2(std::string c, std::string d, Function f) : CommandOptionBase(c,d), func(f) {}
	virtual std::string getArgumentString()
	{
		std::stringstream ss;
		ss << " [" << HumanReadableTypename<Argument0>::get() << "]"
		   << " [" << HumanReadableTypename<Argument1>::get() << "]";
		return ss.str();
	}
	virtual bool parse(std::string optionStr)
	{
		std::stringstream str(optionStr);
#define parseArg(n) std::string arg##n; str >> arg##n; argument##n = TokenParser<Argument##n>::parse(arg##n); if(!argument##n.isValid())return false;
		parseArg(0);
		parseArg(1);
#undef parseArg
		return true;
	}
	virtual void exec()
	{
		func(argument0.getValue(), argument1.getValue());
	}
};

//A helper class of a command with 3 arguments
template<class Argument0, class Argument1, class Argument2, class Function>
class CommandOption3 : public CommandOptionBase {
	Function func;
	ParsedToken<Argument0> argument0;
	ParsedToken<Argument1> argument1;
	ParsedToken<Argument2> argument2;
public:
  CommandOption3(std::string c, std::string d, Function f) : CommandOptionBase(c,d), func(f) {}
	virtual std::string getArgumentString()
	{
		std::stringstream ss;
		ss << " [" << HumanReadableTypename<Argument0>::get() << "]"
		   << " [" << HumanReadableTypename<Argument1>::get() << "]"
		   << " [" << HumanReadableTypename<Argument2>::get() << "]";
		return ss.str();
	}
	virtual bool parse(std::string optionStr)
	{
		std::stringstream str(optionStr);
#define parseArg(n) std::string arg##n; str >> arg##n; argument##n = TokenParser<Argument##n>::parse(arg##n); if(!argument##n.isValid())return false;
		parseArg(0);
		parseArg(1);
	  parseArg(2);
#undef parseArg
		return true;
	}
	virtual void exec()
	{
		func(argument0.getValue(), argument1.getValue(), argument2.getValue());
	}
};

//The main class - holds all of the commands
class CommandParser {
	typedef std::shared_ptr<CommandOptionBase> PointerType;
	typedef std::vector<PointerType> CommandListType;
	CommandListType commands;
public:
  CommandParser() {}
	template<class Function> void addCommand(std::string command, std::string desc, Function f)
  {
	  commands.push_back(PointerType(new CommandOption0<Function>(command, desc, f)));
  }
  template<class Argument0, class Function> void addCommand(std::string command, std::string desc, Function f)
  {
	  commands.push_back(PointerType(new CommandOption1<Argument0,Function>(command, desc, f)));
  }
	template<class Argument0, class Argument1, class Function> void addCommand(std::string command, std::string desc, Function f)
  {
	  commands.push_back(PointerType(new CommandOption2<Argument0,Argument1,Function>(command, desc, f)));
  }
	template<class Argument0, class Argument1, class Argument2, class Function> void addCommand(std::string command, std::string desc, Function f)
  {
	  commands.push_back(PointerType(new CommandOption3<Argument0,Argument1,Argument2,Function>(command, desc, f)));
  }
  //if more arguments are needed, see above for format of how to create the helper class and functions

  //get the help string of all the currently added commands
	std::string getHelpString()
	{
		std::stringstream ss;
		ss << "Valid options:\n";
		unsigned maxNameLen = 0;
		unsigned maxArgLen  = 0;
		unsigned maxDescLen = 0;
		for(CommandListType::iterator CI = commands.begin(); CI != commands.end(); ++CI)
		{
			if( maxNameLen < (*CI)->getCommandName().length() )
				maxNameLen = (*CI)->getCommandName().length();
			if( maxArgLen < (*CI)->getArgumentString().length() )
				maxArgLen = (*CI)->getArgumentString().length();
			if( maxDescLen < (*CI)->getDescription().length() )
				maxDescLen = (*CI)->getDescription().length();
		}
		for(CommandListType::iterator CI = commands.begin(); CI != commands.end(); ++CI)
		{
			ss << "  "
			   << std::left << std::setw(maxNameLen) << (*CI)->getCommandName()
				 << " "
			   << std::left << std::setw(maxArgLen) << (*CI)->getArgumentString()
				 << " - "
				 << std::left << (*CI)->getDescription() << "\n";
		}
		return ss.str();
	}
	//find the first command that matches in name, as well as has arguments that can be parsed from the remaining string
  bool execute(std::string line)
  {
		std::stringstream ss(line);
		std::string commandName;
		ss >> commandName;
		std::string remaining;
		getline(ss, remaining);
		for(CommandListType::iterator CI = commands.begin(); CI != commands.end(); ++CI)
		{
			if( (*CI)->getCommandName() == commandName )
			{
				if( (*CI)->parse(remaining) )
				{
					(*CI)->exec();
					return true;
				}
			}
		}
		return false;
	}
};

#endif
#endif
