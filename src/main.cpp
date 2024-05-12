#include "interpreter.hpp"
#include <fstream>
#include <sstream>

int main(int argc, char **argv) {
  argc--;
  argv++;

  Interpreter interpreter{};
  std::string line{};

  if (argc == 0) {
    while (true) {
      std::cout << "> ";
      std::getline(std::cin, line);
      if (!line.empty() && line != "exit") {
        interpreter.source(line);
        try {
          interpreter.run();
        } catch (const std::runtime_error &e) {
          std::cerr << e.what() << std::endl;
          return EXIT_FAILURE;
        }
      } else {
        return EXIT_SUCCESS;
      }
    }
  } else if (argc == 1) {
    std::ifstream input_file{argv[0]};
    for (; std::getline(input_file, line);) {
      if (!line.empty()) {
        interpreter.source(line);
        try {
          interpreter.run();
        } catch (const std::exception &e) {
          std::cerr << e.what() << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}
