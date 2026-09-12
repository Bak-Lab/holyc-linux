#include "../compiler/translator.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string &message)
{
  if (!condition)
    throw std::runtime_error(message);
}

template <typename Function>
void require_error(Function function, const std::string &expected)
{
  try {
    function();
  } catch (const holyc::TranslationError &error) {
    require(std::string(error.what()).find(expected) != std::string::npos,
            "unexpected error: " + std::string(error.what()));
    return;
  }
  throw std::runtime_error("expected TranslationError");
}

} // namespace

int main()
{
  try {
    const std::string generated =
        holyc::translate("U0 Demo() {}\n\nDemo;\n", "Demo.HC");
    require(generated.find("#include \"holyc.h\"") != std::string::npos,
            "missing compatibility header");
    require(generated.find("Demo();") != std::string::npos,
            "missing entry-point call");
    require(generated.find("\nDemo;\n") == std::string::npos,
            "bare entry point was not removed");
    require(generated.find("int main(int argc, char **argv)") !=
                std::string::npos,
            "missing native entry point");

    require_error(
        [] {
          holyc::translate("U0 Main() {}\nMain;\nI64 later;\n", "Bad.HC");
        },
        "final statement");
    require_error(
        [] {
          holyc::translate("U0 Main() { OutU8(0x60, 1); }\nMain;",
                           "Unsafe.HC");
        },
        "hardware operation");
  } catch (const std::exception &error) {
    std::cerr << "translator_test: " << error.what() << '\n';
    return 1;
  }

  std::cout << "translator tests passed\n";
  return 0;
}
