#ifndef HOLYC_TRANSLATOR_HPP
#define HOLYC_TRANSLATOR_HPP

#include <stdexcept>
#include <string>

namespace holyc {

class TranslationError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

std::string translate(const std::string &source,
                      const std::string &source_name = "<input>");

} // namespace holyc

#endif
