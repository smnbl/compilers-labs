#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/DerivedTypes.h"

#include <fmt/core.h>
#include <list>

#define DEBUG_TYPE "microcc::boundscheck"

using namespace llvm;

static cl::opt<bool>
    OptimiseBoundsCheck("optimise-microcc-bc",
                        cl::desc("Enable boundscheck optimisation"),
                        cl::init(false));

namespace {
struct BoundsCheck : public FunctionPass {
  private:
    Function *Assert = nullptr;

  public:
    static char ID;
    BoundsCheck() : FunctionPass(ID) {}

    /// Entry point of the pass; this function performs the actual analysis or
    /// transformation, and is called for each function in the module.
    ///
    /// The returned boolean should be `true` if the function was modified,
    /// `false` if it wasn't.
    bool runOnFunction(Function &F) override {
        IRBuilder<> Builder(F.getContext());

        LLVM_DEBUG({
            dbgs() << "BoundsCheck: processing function '";
            dbgs().write_escaped(F.getName()) << "'\n";
        });

        // Instantiate the assert function once per module
        if (Assert == nullptr || Assert->getParent() != F.getParent())
            Assert = getAssertFunction(F.getParent());

        // Find all GEP instructions
        // NOTE: we need to do this first, because the iterators get invalidated
        //       when modifying underlying structures
        std::list<GetElementPtrInst *> WorkList;
        for (auto &FI : F) {      // Iterate function -> basic blocks
            for (auto &BI : FI) { // Iterate basic block -> instructions
                if (auto *GEP = dyn_cast<GetElementPtrInst>(&BI))
                    WorkList.push_back(GEP);
            }
        }

        // Process any GEP instructions
        bool changed;

        for (auto *GEP : WorkList) {
            LLVM_DEBUG(dbgs() << "BoundsCheck: found a GEP, " << *GEP << "\n");

            // ASSIGNMENT: Implement your pass here.
            // get instruction location:
            DILocation *location = (DILocation*) GEP->getMetadata("dbg");
            auto filename = (const std::string) ((DIFile*) location->getScope()->getSubprogram()->getFile())->getFilename();
			auto line_number = location->getLine();

            LLVM_DEBUG(dbgs() << "filename: " << filename<< "\n");
            LLVM_DEBUG(dbgs() << "line_number: " << line_number << "\n");

            if (GEP->hasAllConstantIndices()) {
            	// 1. static indexing
				LLVM_DEBUG(dbgs() << "static!" << "\n");

	            const ConstantInt* index_val = dyn_cast<ConstantInt>(GEP->getOperand(2));
	            assert(index_val && "index_val cast to ConstantInt failed");
	            uint64_t index = index_val->getZExtValue();
				LLVM_DEBUG(dbgs() << "index: " << index << "\n");

	            auto array = dyn_cast<AllocaInst>(GEP->getOperand(0));
	            assert(array && "array cast to AllocaInst failed");

	            auto array_type = dyn_cast<ArrayType>(array->getAllocatedType());
	            assert(array_type && "could not dyn_cast to array_type!");
	            uint64_t size = array_type->getNumElements();

				LLVM_DEBUG(dbgs() << "size: " << size << "\n");

				// perform static bounds check
	            if (index >= size) {
	            	llvm::report_fatal_error(fmt::format("out-of-bounds array access detected at {}:{}", filename, line_number));
                }

                changed = false;
            } else {
            	// 2. dynamic indexing
				LLVM_DEBUG(dbgs() << "dynamic!" << "\n");

	            // Error message template for dynamic case:

	            auto message = "out-of-bounds array access";

	            // perform code rewriting
	            changed = true;
			}
        }

        return changed;
    }

  private:
    /// Get a function object pointing to the Sys V '__assert' function.
    ///
    /// This function displays a failed assertion, together with the source
    /// location (file name and line number). Afterwards, it abort()s the
    /// program.
    Function *getAssertFunction(Module *Mod) {
        Type *CharPtrTy = Type::getInt8PtrTy(Mod->getContext());
        Type *IntTy = Type::getInt32Ty(Mod->getContext());
        Type *VoidTy = Type::getVoidTy(Mod->getContext());

        std::vector<Type *> assert_arg_types;
        assert_arg_types.push_back(CharPtrTy); // const char *__assertion
        assert_arg_types.push_back(CharPtrTy); // const char *__file
        assert_arg_types.push_back(IntTy);     // int __line

        FunctionType *assert_type =
            FunctionType::get(VoidTy, assert_arg_types, true);

        Function *F = Function::Create(assert_type, Function::ExternalLinkage,
                                       "__assert", Mod);
        F->addFnAttr(Attribute::NoReturn);
        F->setCallingConv(CallingConv::C);
        return F;
    }

    // ASSIGNMENT: Add any helper functions you need here.
};
} // namespace

char BoundsCheck::ID = 0;
static RegisterPass<BoundsCheck> X("microcc-bc", "microcc boundscheck pass",
                                   false, false);
