#include "Context.h"

#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

#include <string>

static cl::opt<std::string> filename("anselm-pattern", cl::desc("Specify pattern file"), cl::value_desc("filename"));

namespace {
	struct Anselm : public FunctionPass {
		static char ID;
		Anselm() : FunctionPass(ID), context(filename.c_str()) {}

		virtual bool runOnFunction(Function &F) override {
			context.pass(F);
			return false;
		}

	private:
		Context context;
	};
}

char Anselm::ID = 0;
static RegisterPass<Anselm> X("anselm", "Detect function usage patterns", true, true);
