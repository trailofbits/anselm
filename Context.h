#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"

#include <string>
#include <regex>
#include <unordered_set>
#include <utility>
#include <vector>

using Node = llvm::Instruction*;
using Edges = std::unordered_set<Node>;
using DirectedGraph = std::unordered_map<Node, std::pair<Edges, Edges>>;
using Path = std::vector<Node>;

class Context {
public:
	Context(std::string filename);
	void pass(llvm::Function &F);
private:
	std::unordered_set<std::string> names;
	std::basic_regex<char> regex;
	void traverse(llvm::Function &F);
	void inspect(Path path);
	bool isRelevant(llvm::Instruction &I);
};

#endif // CONTEXT_H_
