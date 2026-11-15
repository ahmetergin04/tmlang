#pragma once

#include <string>
#include <set>
#include <map>
#include <vector>

#include "token.h"

using Symbol = std::string;
using State = std::string;
using Set = std::set<std::string>;

struct Action {
	State next;
	Symbol write;
	token_type action;
};

using Deltas = std::map<std::pair<State,Symbol>, Action>;

struct Machine {
	Set states;
	Set alphabet;
	State start_state;
	const Deltas* deltas;
	bool deciding;
};

class Environment {
public:
	std::map<std::string, Set>                 alphabets, state_sets;
	std::map<std::string, Deltas>              trans_funcs;
	std::map<std::string, Machine>             machines;
	std::map<std::string, std::vector<Symbol>> tapes;
};
