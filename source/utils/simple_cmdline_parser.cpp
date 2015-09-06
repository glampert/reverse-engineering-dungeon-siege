
// ================================================================================================
// -*- C++ -*-
// File: simple_cmdline_parser.cpp
// Author: Guilherme R. Lampert
// Created on: 29/01/15
// Brief: Simple helper to parse command line arguments.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "utils/simple_cmdline_parser.hpp"
#include <utility>
#include <cassert>
#include <cctype>

namespace utils
{

// ========================================================
// SimpleCmdLineParser:
// ========================================================

SimpleCmdLineParser::SimpleCmdLineParser(const unsigned int argc, const char * argv[])
{
	parseCmdLine(argc, argv);
}

bool SimpleCmdLineParser::parseCmdLine(const unsigned int argc, const char * argv[])
{
	// Just the program name is not a parseable command line.
	if (argc <= 1)
	{
		return false;
	}

	// Errors and current command line are lost, if any:
	errorList.clear();
	arguments.clear();

	// Skip first, as it is the program name, store the rest:
	arguments.reserve(argc - 1);
	for (unsigned int i = 1; i < argc; ++i)
	{
		arguments.emplace_back(argv[i]);
	}

	return processFlags();
}

bool SimpleCmdLineParser::processFlags()
{
	//
	// Possible argument/flag formats:
	// 1) single flag string, e.g.: -x
	// 2) multi-flag string, e.g.: -xyz
	// 3) long flag, e.g.: --long_flag=123
	// 4) long flag with quoted value: --long_flag="hello world"
	//
	CmdLineFlag cmdLineFlag;
	const size_t numArgs = arguments.size();
	for (size_t i = 0; i < numArgs; ++i)
	{
		const std::string & arg = arguments[i];
		if (arg.length() > 2 && arg[0] == '-' && arg[1] == '-') // Long flag begin:
		{
			cmdLineFlag.name.clear();
			cmdLineFlag.value.clear();
			bool readingName = true;
			for (size_t c = 2; c < arg.length(); ++c) // Start at 2, skip "--"
			{
				if (arg[c] == '=' || arg[c] == '"')
				{
					readingName = false;
					continue;
				}
				if (readingName)
				{
					cmdLineFlag.name.push_back(arg[c]);
				}
				else
				{
					cmdLineFlag.value.push_back(arg[c]);
				}
			}
			commandFlags.push_back(cmdLineFlag);
		}
		else
		{
			if (arg[0] == '-')
			{
				for (size_t c = 1; c < arg.length(); ++c) // Start at 1, skip "-"
				{
					// Only lower/upper case letters are accepted!
					if (std::islower(arg[c]) || std::isupper(arg[c]))
					{
						const char flagChar[] = { arg[c], '\0' };
						cmdLineFlag.name = flagChar;
						cmdLineFlag.value.clear(); // Value is an empty string for simple flags.
						commandFlags.push_back(cmdLineFlag);
					}
					else
					{
						addError("Single character command flags must be only upper/lower-case letters! " + arg);
					}
				}
			}
			else
			{
				// Malformed flag, ignore.
				addError("Malformed command line flag " + arg);
			}
		}
	}

	return hadErrors();
}

int SimpleCmdLineParser::getArgCount() const noexcept
{
	return static_cast<int>(arguments.size());
}

const std::string & SimpleCmdLineParser::getArg(const unsigned int index) const
{
	assert(index < arguments.size());
	return arguments[index];
}

int SimpleCmdLineParser::getFlagCount() const noexcept
{
	return static_cast<int>(commandFlags.size());
}

bool SimpleCmdLineParser::hasFlag(const std::string & flagName) const noexcept
{
	for (const auto & cmdFlag : commandFlags)
	{
		if (cmdFlag.name == flagName)
		{
			return true;
		}
	}
	return false;
}

const CmdLineFlag & SimpleCmdLineParser::getFlag(const unsigned int index) const
{
	assert(index < commandFlags.size());
	return commandFlags[index];
}

bool SimpleCmdLineParser::getFlag(const std::string & flagName, CmdLineFlag & flagOut) const
{
	for (const auto & cmdFlag : commandFlags)
	{
		if (cmdFlag.name == flagName)
		{
			flagOut = cmdFlag;
			return true;
		}
	}

	flagOut.name.clear();
	flagOut.value.clear();
	return false;
}

std::string SimpleCmdLineParser::getFlagsString() const
{
	std::string flagStr;
	for (const auto & cmdFlag : commandFlags)
	{
		flagStr += cmdFlag.name;
		if (!cmdFlag.value.empty())
		{
			flagStr += "='" + cmdFlag.value + "'";
		}
		flagStr += ", ";
	}
	return flagStr;
}

void SimpleCmdLineParser::clearFlags()
{
	commandFlags.clear();
}

bool SimpleCmdLineParser::hadErrors() const noexcept
{
	return !errorList.empty();
}

const std::vector<std::string> & SimpleCmdLineParser::getErrorList() const
{
	return errorList;
}

void SimpleCmdLineParser::clearErrors()
{
	errorList.clear();
}

void SimpleCmdLineParser::addError(std::string error)
{
	errorList.push_back(std::move(error));
}

} // namespace utils {}
