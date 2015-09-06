
// ================================================================================================
// -*- C++ -*-
// File: simple_cmdline_parser.hpp
// Author: Guilherme R. Lampert
// Created on: 29/01/15
// Brief: Simple helper to parse command line arguments.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#ifndef UTILS_SIMPLE_CMDLINE_PARSER_HPP
#define UTILS_SIMPLE_CMDLINE_PARSER_HPP

#include <string>
#include <vector>

namespace utils
{

// ========================================================
// CmdLineFlag:
// ========================================================

struct CmdLineFlag
{
	std::string name;  // Flag name, such as "xyz" for a flag "--xyz".
	std::string value; // Flag value, if this is a long flag, empty if it is a short flag such as "-x".
};

// ========================================================
// SimpleCmdLineParser:
// ========================================================

//
// Simple command line parser for command line tool applications.
// Having this class avoids the dependency with some Unix libraries
// such as `getopt`. The goal is to keep this code as portable as
// possible, plus, our needs for command-line parsing aren't very
// demanding, so this simple helper class does the trick.
//
// Flag formats accepted are:
// - Single flag string, e.g.: -x
// - Multi-flag string, e.g.: -xyz
// - Long flag, e.g.: --long_flag=123
// - Long flag with quoted value: --long_flag="hello world"
//
class SimpleCmdLineParser final
{
public:

	SimpleCmdLineParser() = default;
	SimpleCmdLineParser(unsigned int argc, const char * argv[]);

	// Parses a command line. If a command line was already parsed,
	// it is discarded, but its parsed flags remain. Errors are also cleared.
	// `argc`: Traditional arg count, as in 'main()'. First arg is the program name.
	// `argv`: Argument array, as in 'main()'. argv[0] is assumed to be the program name and ignored.
	// Returns true if arguments parsed without errors. False if errors encountered or if `argc` <= 1.
	bool parseCmdLine(unsigned int argc, const char * argv[]);

	// Current argument count, NOT including the program name.
	int getArgCount() const noexcept;

	// Get an argument from the current argument list.
	const std::string & getArg(unsigned int index) const;

	// Number of flags successfully parsed.
	int getFlagCount() const noexcept;

	// Check if a flag is present. Useful for testing boolean flags.
	bool hasFlag(const std::string & flagName) const noexcept;

	// Get a command line flag at a given position.
	const CmdLineFlag & getFlag(unsigned int index) const;

	// Get a flag + value pair. Useful for querying long flags.
	bool getFlag(const std::string & flagName, CmdLineFlag & flagOut) const;

	// Return a comma separated string with all command line flags. Useful for debug printing.
	std::string getFlagsString() const;

	// Waste all parsed flags.
	void clearFlags();

	// Check if the last command line parsed presented errors.
	bool hadErrors() const noexcept;

	// Get a read-only reference to the error list.
	const std::vector<std::string> & getErrorList() const;

	// Clears the error list.
	void clearErrors();

private:

	// Disallow copy and assignment.
	SimpleCmdLineParser(const SimpleCmdLineParser &) = delete;
	SimpleCmdLineParser & operator = (const SimpleCmdLineParser &) = delete;

	// Split command line into tokens, then separate individual flags.
	bool processFlags();

	// Add an error to the error list.
	void addError(std::string error);

	// Processed arg list, ready for query.
	std::vector<CmdLineFlag> commandFlags;

	// Arg list, as provided by the constructor.
	std::vector<std::string> arguments;

	// Errors encountered during parsing. Empty if none.
	std::vector<std::string> errorList;
};

} // namespace utils {}

#endif // UTILS_SIMPLE_CMDLINE_PARSER_HPP
