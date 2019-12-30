#include <unordered_set>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "Pattern.h"

using namespace std;
using namespace llvm;

using dg = unordered_map<Instruction*, pair<unordered_set<Instruction*>, unordered_set<Instruction*>>>;

namespace {
	struct GraphPass : public ModulePass {
		static char ID;
		GraphPass() : ModulePass(ID) { }

		bool isRelevant(Instruction &I, unordered_set<string> allowed) {
			auto B = I.getParent();
			if (pred_begin(B) == pred_end(B) && I.isIdenticalTo(&B->front()))
				return true;
			if (succ_begin(B) == succ_end(B) && I.isIdenticalTo(&B->back()))
				return true;
			if (auto call = dyn_cast<CallInst>(&I)) {
				if (auto func = call->getCalledFunction()) {
					if (allowed.find(func->getName()) != allowed.end())
						return true;
					if (!func->isDeclaration())
						return true;
				}
			}
			return false;
		}

		dg getGraph(Function &F, unordered_set<string> allowed) {
			dg graph;
			for (auto& B : F) {
				auto node = &B.front();
				graph[node];
				for (auto& I : B) {
					if (isRelevant(I, allowed) && !I.isIdenticalTo(node)) {
						graph[node].second.emplace(&I);
						graph[&I].first.emplace(node);
						node = &I;
					}
				}
				for (auto it = succ_begin(&B); it != succ_end(&B); ++it) {
					auto edge = &(*it)->front();
					graph[edge].first.emplace(node);
					graph[node].second.emplace(edge);
				}
			}
			for (auto& B : F) {
				if (!isRelevant(B.front(), allowed)) {
					auto node = &B.front();
					auto& edges = graph.at(node);
					for (auto& edge : edges.first) {
						auto& succs = graph.at(edge).second;
						succs.erase(node);
						for (auto succ : edges.second)
							succs.emplace(succ);
					}
					for (auto& edge : edges.second) {
						auto& preds = graph.at(edge).first;
						preds.erase(node);
						for (auto pred : edges.first)
							preds.emplace(pred);
					}
					graph.erase(node);
				}
			}
			return graph;
		}

		virtual bool runOnModule(Module &M) override {
			unordered_map<Function*, dg> graphs;
			auto allowed = getAllowed();
			for (auto& F : M)
				if (!F.isDeclaration())
					graphs[&F] = getGraph(F, allowed);
			runAnalysis(graphs);
			return false;
		}
	};
}

char GraphPass::ID = 0;
static RegisterPass<GraphPass> X("graph", "GraphPass", true, true);
