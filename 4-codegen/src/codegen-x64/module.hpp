#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include "llvm/Support/FormattedStream.h"

#include <ostream>
#include <string>
#include <vector>

namespace codegen_x64 {
struct Instruction {
    Instruction(const std::string &opcode,
                const std::vector<std::string> &operands,
                const std::string &comment = "");

    std::string opcode;
    std::vector<std::string> operands;
    std::string comment;
};

struct BasicBlock {
    explicit BasicBlock(const std::string &name,
                        const std::string &comment = "",
                        bool is_global = false);

    std::string name;
    std::string comment;
    bool is_global;

    std::vector<Instruction> instructions;
};

struct Module {
    std::vector<BasicBlock> blocks;
};

llvm::formatted_raw_ostream &operator<<(llvm::formatted_raw_ostream &os,
                                        const Instruction &ins);
llvm::formatted_raw_ostream &operator<<(llvm::formatted_raw_ostream &os,
                                        const BasicBlock &bbl);
llvm::formatted_raw_ostream &operator<<(llvm::formatted_raw_ostream &os,
                                        const Module &mod);

Module &operator<<(Module &mod, const BasicBlock &bbl);
Module &operator<<(Module &mod, const Instruction &ins);

}; // namespace codegen_x64

#endif /* end of include guard: PROGRAM_HPP */
