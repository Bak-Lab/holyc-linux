#include "translator.hpp"

#include <cctype>
#include <regex>
#include <string>

namespace holyc {
namespace {

const std::regex unsupported_pattern(
    R"(\b(InU(8|16|32)|OutU(8|16|32)|CLI|STI|Reboot|SysHlt)\b)");
const std::regex entry_pattern(
    R"(^[ \t]*([A-Za-z_][A-Za-z0-9_]*)[ \t]*;[ \t]*(//[^\r\n]*)?[ \t]*$)");

std::string trim_right(std::string value)
{
  while (!value.empty() &&
         std::isspace(static_cast<unsigned char>(value.back())) != 0)
    value.pop_back();
  return value;
}

std::string escape_line_filename(const std::string &value)
{
  std::string escaped;
  escaped.reserve(value.size());
  for (char character : value) {
    if (character == '\\' || character == '"')
      escaped.push_back('\\');
    escaped.push_back(character);
  }
  return escaped;
}

} // namespace

std::string translate(const std::string &source, const std::string &source_name)
{
  std::smatch unsupported_match;
  if (std::regex_search(source, unsupported_match, unsupported_pattern)) {
    throw TranslationError(source_name + ": unsupported hardware operation: " +
                           unsupported_match.str());
  }

  const std::string trimmed_source = trim_right(source);
  const std::size_t final_newline = trimmed_source.find_last_of('\n');
  const std::size_t entry_begin =
      final_newline == std::string::npos ? 0 : final_newline + 1;
  const std::string entry_line = trimmed_source.substr(entry_begin);
  std::smatch entry_match;
  if (!std::regex_match(entry_line, entry_match, entry_pattern)) {
    bool found_earlier_entry = false;
    std::size_t line_begin = 0;
    while (line_begin < entry_begin) {
      const std::size_t newline = source.find('\n', line_begin);
      const std::size_t line_end =
          newline == std::string::npos ? source.size() : newline;
      const std::string line = source.substr(line_begin, line_end - line_begin);
      std::smatch earlier_match;
      if (std::regex_match(line, earlier_match, entry_pattern))
        found_earlier_entry = true;
      if (newline == std::string::npos)
        break;
      line_begin = newline + 1;
    }
    if (found_earlier_entry)
      throw TranslationError(source_name +
                             ": entry point must be the final statement");
    throw TranslationError(source_name +
                           ": expected a final bare entry point such as 'Main;'");
  }

  const std::string entry = entry_match[1].str();
  const std::string body = trim_right(source.substr(0, entry_begin));
  const std::string escaped_name = escape_line_filename(source_name);
  return "#include \"holyc.h\"\n#line 1 \"" + escaped_name + "\"\n" + body +
         "\n\n#line 1 \"<holyc-entry>\"\n"
         "int main(int argc, char **argv)\n"
         "{\n"
         "  if (!HCInit(argc, argv))\n"
         "    return 1;\n"
         "  " +
         entry +
         "();\n"
         "  HCShutdown();\n"
         "  return 0;\n"
         "}\n";
}

} // namespace holyc
