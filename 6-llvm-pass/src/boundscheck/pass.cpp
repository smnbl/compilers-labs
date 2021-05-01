#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/DerivedTypes.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

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
        Type* T_int = llvm::Type::getInt64Ty(F.getContext());
        Type *T_int32 = Type::getInt32Ty(F.getContext());

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
            DILocation *location = dyn_cast<DILocation>(GEP->getMetadata("dbg"));
            assert(location && "could not cast dbg to DILocation");
            auto filename = ((DIFile*) location->getScope()->getSubprogram()->getFile())->getFilename();

       		auto filename_string = (const std::string) filename;
			auto line_number = location->getLine();

            LLVM_DEBUG(dbgs() << "filename: " << filename_string << "\n");
            LLVM_DEBUG(dbgs() << "line_number: " << line_number << "\n");

            auto array = dyn_cast<AllocaInst>(GEP->getOperand(0));
            assert(array && "array cast to AllocaInst failed");

            auto array_type = dyn_cast<ArrayType>(array->getAllocatedType());
            assert(array_type && "could not dyn_cast to array_type!");
            uint64_t size = array_type->getNumElements();

			LLVM_DEBUG(dbgs() << "size: " << size << "\n");

            if (GEP->hasAllConstantIndices()) {
            	// 2. static indexing
				LLVM_DEBUG(dbgs() << "static!" << "\n");

	            const ConstantInt* index_val = dyn_cast<ConstantInt>(GEP->getOperand(2));
	            assert(index_val && "index_val cast to ConstantInt failed");

	            uint64_t index = index_val->getZExtValue();
				LLVM_DEBUG(dbgs() << "index: " << index << "\n");

				// perform static bounds check
	            if (index >= size || index < 0) {
	            	llvm::report_fatal_error(fmt::format("out-of-bounds array access detected at {}:{}", filename_string, line_number));
                }

                changed = false;
            } else {
            	// 2. dynamic indexing
				LLVM_DEBUG(dbgs() << "dynamic!" << "\n");

	            // Error message template for dynamic case:
	            auto message = "out-of-bounds array access";

	            BasicBlock *assertion_fails_block = llvm::BasicBlock::Create(F.getContext(), "bounds_assertion_failed", &F);;

				// add cond before GEP
				Builder.SetInsertPoint(GEP);
	            Value* is_below = Builder.CreateICmpSLT(GEP->getOperand(2), llvm::ConstantInt::get(T_int, 0), "bounds check cond, index not below 0");
	            Value* is_above = Builder.CreateICmpSGE(GEP->getOperand(2), llvm::ConstantInt::get(T_int, size), "bounds check cond, index not too big");
	            Value* cond = Builder.CreateOr(is_above, is_below);

				// split block
				SplitBlockAndInsertIfThen(cond, GEP, true, nullptr, nullptr, nullptr, assertion_fails_block);

	            Builder.SetInsertPoint(assertion_fails_block);
				auto *filename_ptr = Builder.CreateGlobalStringPtr(filename_string);
				auto *assertion_info = Builder.CreateGlobalStringPtr(message);
	            Builder.CreateCall(Assert, llvm::ArrayRef<llvm::Value*>{assertion_info, filename_ptr, llvm::ConstantInt::get(T_int32, line_number)});
	            Builder.CreateUnreachable();

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
