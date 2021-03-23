#include "codegen-x64/module.hpp"
#include "codegen-x64/optimise-x64.hpp"

#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <iterator>
#include <map>

#define DEBUG_TYPE "optimiser-x64"

// Command-line options to select which optimisation to apply.
llvm::cl::bits<codegen_x64::Optimisations> OptimisationsBits(
    llvm::cl::desc("Available peephole optimisations:"),
    llvm::cl::values(clEnumValN(codegen_x64::Optimisations::NopElimination,
                                "optimisation-nop-elimination",
                                "Eliminate NOP instructions"),
                     clEnumValN(codegen_x64::Optimisations::Assignment1,
                                "optimisation-1", "Your optimisation 1"),
                     clEnumValN(codegen_x64::Optimisations::Assignment2,
                                "optimisation-2", "Your optimisation 2"),
                     clEnumValN(codegen_x64::Optimisations::Assignment3,
                                "optimisation-3", "Your optimisation 3")));

// Statistic to see how many NOPs were removed.
STATISTIC(NumNopsEliminated,
          "The number of NOP instructions that were eliminated");

void codegen_x64::OptimiserX64::optimise(Module &module) {
    bool changed = true;

    while (changed) {
        changed = false;

        if (OptimisationsBits.isSet(Optimisations::NopElimination))
            changed |= optimiseNopElimination(module);

        if (OptimisationsBits.isSet(Optimisations::Assignment1))
            changed |= optimise1(module);

        if (OptimisationsBits.isSet(Optimisations::Assignment2))
            changed |= optimise2(module);

        if (OptimisationsBits.isSet(Optimisations::Assignment3))
            changed |= optimise3(module);
    }
}

bool codegen_x64::OptimiserX64::optimiseNopElimination(Module &mod) {
    bool changed = false;

    // Remove all "nop" instructions from all basic blocks.
    for (auto &bbl : mod.blocks) {
        auto &insns = bbl.instructions;
        auto it = std::remove_if(
            std::begin(insns), std::end(insns),
            [](const Instruction &ins) { return ins.opcode == "nop"; });
            

        // Update statistic
        NumNopsEliminated += std::distance(it, std::end(insns));

        // Check if anything was removed
        if (it != std::end(insns))
            changed = true;

        insns.erase(it, std::end(insns));
    }

    return changed;
}

bool codegen_x64::OptimiserX64::optimise1(Module &mod) {
    // ASSIGNMENT: Implement your first peephole optimisation here.
    // Return true only if you have changed the assembly.
    // First optimization: push and pop changes to move
    bool changed = false;
    for (auto &bbl : mod.blocks) {
       for (auto it = std::begin(bbl.instructions);
            it != std::end(bbl.instructions) &&
            std::next(it) != std::end(bbl.instructions) &&
            std::next(std::next(it)) != std::end(bbl.instructions);
            ++it) {
            auto &cur_ins = *it;
            auto &next_ins = *std::next(it);
            
            if (cur_ins.opcode == "pushq" && next_ins.opcode == "popq"){
                
                auto next_next_ins = std::next(std::next(it));
                changed = true;
                next_ins.opcode = "movq" ;
                next_ins.operands = {cur_ins.operands[0], next_ins.operands[0]};
                next_ins.comment = "opt1";
                makeNop(cur_ins);
                if (it != std::begin(bbl.instructions)){
                    auto prev_ins = std::prev(it);
                    while (prev_ins->opcode == "pushq" && next_next_ins->opcode == "popq" && prev_ins != std::begin(bbl.instructions) &&  next_next_ins != std::end(bbl.instructions)){
                        next_next_ins->opcode = "movq" ;
                        next_next_ins->operands = {prev_ins->operands[0], next_next_ins->operands[0]};
                        next_next_ins->comment = "opt1";
                        makeNop(*prev_ins);
                        prev_ins = std::prev(prev_ins);
                        next_next_ins = std::next(next_next_ins);
                    }
                }

            }
            
        }
    }
    return changed;
}

bool codegen_x64::OptimiserX64::optimise2(Module &mod) {
    // ASSIGNMENT: Implement your second peephole optimisation here.
    // Return true only if you have changed the assembly.
    bool changed = false;

    
    for (auto &bbl : mod.blocks) {
        std::map<std::string, int> dict; //registers saved with their values
        int i = 0;
        for (auto it = std::begin(bbl.instructions);
            it != std::end(bbl.instructions) ;
            ++it) {
            auto &cur_ins = *it;
            if (cur_ins.opcode == "movq"){
                std::string firstop = cur_ins.operands[0];
                std::string secondop = cur_ins.operands[1];
                if (firstop.at(0) == '$' && secondop.at(0) == '%'){
                    dict[secondop] = i;
                }
            }
            else if(cur_ins.opcode == "addq" || cur_ins.opcode == "subq"){ //check if in the map
                if (dict.find(cur_ins.operands[0])!=dict.end()){
                    int nr = dict.find(cur_ins.operands[0])->second;
                    auto oldinstr = bbl.instructions[nr];
                    cur_ins.operands[0]= oldinstr.operands[0];
                    makeNop(oldinstr);
                    changed = true;
                }//invalidate second operand
                dict.erase(cur_ins.operands[1]);
            }
            else{
                int oplen = cur_ins.operands.size();
                if (oplen>0){//invalidate last operand
                    dict.erase(cur_ins.operands[oplen-1]);
                }
            }
            i++;

            
            
        }
    }
    return changed;
}

bool codegen_x64::OptimiserX64::optimise3(Module &mod) {
    // ASSIGNMENT: Implement your third peephole optimisation here.
    // Return true only if you have changed the assembly.
    bool changed = false;
    for (auto &bbl : mod.blocks) {
       for (auto it = std::begin(bbl.instructions);
            it != std::end(bbl.instructions) &&
            std::next(it) != std::end(bbl.instructions);
            ++it) {
            auto &cur_ins = *it;
            auto &next_ins = *std::next(it);
            if (cur_ins.operands.size()>=2){
                if (cur_ins.opcode == "movq" && cur_ins.operands[0]==cur_ins.operands[1]){
                    makeNop(cur_ins);
                    changed = true;
                }
            }
        }
    }
    return changed;
}

void codegen_x64::OptimiserX64::makeNop(Instruction &ins) {
    ins.opcode = "nop";
    ins.operands.clear();
}

