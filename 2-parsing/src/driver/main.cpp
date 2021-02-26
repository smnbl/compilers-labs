#include "ast/ast.hpp"
#include "ast/prettyprinter.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"

#include "llvm/Support/CommandLine.h"

#include <cstdlib>
#include <fmt/core.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                         llvm::cl::desc("<input file>"),
                                         llvm::cl::init("-"));

llvm::cl::opt<bool>
    DumpTokens("dump-tokens",
               llvm::cl::desc("Dump tokens after lexical analysis"),
               llvm::cl::init(false));

llvm::cl::opt<bool>
    AsciiMode("ascii-mode",
              llvm::cl::desc("Dump AST in ASCII mode instead of Unicode"),
              llvm::cl::init(false));

int main(int argc, char *argv[]) {
  // Parse command-line arguments
  llvm::cl::ParseCommandLineOptions(argc, argv);

  // Convert input file/stdin to string
  std::string inputContents;
  if (InputFilename == "-") {
    inputContents = std::string{std::istreambuf_iterator<char>(std::cin),
                                std::istreambuf_iterator<char>()};
  } else {
    std::ifstream inputFile{InputFilename};
    inputContents = std::string{std::istreambuf_iterator<char>(inputFile),
                                std::istreambuf_iterator<char>()};
  }

  // Phase 1: lexical analysis
  Lexer lexer{inputContents};
  std::vector<Token> tokens = lexer.getTokens();

  if (DumpTokens) {
    for (const Token &token : tokens) {
      std::string location =
          fmt::format("{}:{} -> {}:{}", token.begin.line, token.begin.col,
                      token.end.line, token.end.col);

      fmt::print("{:20}{:20}{:20}\n", location, token.lexeme,
                 token_type_to_string(token.type));
    }
  }

  if (lexer.hadError())
    return EXIT_FAILURE;

  // Phase 2: parsing
  Parser parser{tokens};

  auto root = parser.parse();

  if (parser.hadError())
    return EXIT_FAILURE;

  ast::PrettyPrinter printer(std::cout, AsciiMode);
  printer.visit(*root, "", true);

  return EXIT_SUCCESS;
}
