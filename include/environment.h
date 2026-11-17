#pragma once

#include <string>
#include <set>
#include <map>
#include <vector>

#include "token.h"
#include "expr.h"

using Symbol = std::string;
using State = std::string;
using Set = std::set<std::string>;

struct Tape {
	Set alphabet;
	std::vector<Symbol> cells;
};

struct TmAction {
	State next;
	Symbol write;
	token_type action;
};

enum class CtmActionType { MOVE_RIGHT, MOVE_LEFT, CALL_TM, CALL_CTM, WRITE, HALT, GOTO };

struct CtmAction {
	CtmActionType type;
	std::string name;
};

struct CtmRow {
	std::string label;
	expr* condition; // lives in the AST
	std::vector<CtmAction> chain;
};

struct Ctm {
	bool any_alphabet; // "_"
	Set alphabet;
	std::vector<CtmRow> rows;
};

using Deltas = std::map<std::pair<State,Symbol>, TmAction>;

struct TMachine {
	Set states;
	Set alphabet;
	State start_state;
	const Deltas* deltas;
	bool deciding;
};

class Environment {
public:
	std::map<std::string, Set> alphabets, state_sets;
	std::map<std::string, Deltas> trans_funcs;
	std::map<std::string, TMachine> machines;
	std::map<std::string, Tape> tapes;
	std::map<std::string, Ctm> ctms;
};
