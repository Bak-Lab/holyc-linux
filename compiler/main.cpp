#include "translator.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct Options {
  fs::path source;
  fs::path output;
  fs::path emit_c;
};

void usage(std::ostream &stream)
{
  stream << "Usage: holyc SOURCE [-o OUTPUT] [--emit-c PATH]\n"
            "Compile a supported HolyC source file for Linux.\n";
}

Options parse_options(int argc, char **argv)
{
  Options options;
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "-h" || argument == "--help") {
      usage(std::cout);
      std::exit(0);
    }
    if (argument == "-o" || argument == "--output" ||
        argument == "--emit-c") {
      if (++index >= argc)
        throw std::runtime_error(argument + " requires a path");
      if (argument == "--emit-c")
        options.emit_c = argv[index];
      else
        options.output = argv[index];
      continue;
    }
    if (!argument.empty() && argument.front() == '-')
      throw std::runtime_error("unknown option: " + argument);
    if (!options.source.empty())
      throw std::runtime_error("only one source file may be compiled at a time");
    options.source = argument;
  }

  if (options.source.empty())
    throw std::runtime_error("a HolyC source file is required");
  return options;
}

std::string read_file(const fs::path &path)
{
  std::ifstream input(path, std::ios::binary);
  if (!input)
    throw std::runtime_error("cannot read source file: " + path.string());
  return {std::istreambuf_iterator<char>(input),
          std::istreambuf_iterator<char>()};
}

void write_file(const fs::path &path, const std::string &content)
{
  if (!path.parent_path().empty())
    fs::create_directories(path.parent_path());
  std::ofstream output(path, std::ios::binary);
  if (!output)
    throw std::runtime_error("cannot write file: " + path.string());
  output << content;
  if (!output)
    throw std::runtime_error("failed while writing file: " + path.string());
}

bool is_project_root(const fs::path &path)
{
  return fs::is_regular_file(path / "runtime/include/holyc.h") &&
         fs::is_regular_file(path / "runtime/src/holyc_runtime.c");
}

fs::path search_parents(fs::path path)
{
  path = fs::absolute(path);
  while (true) {
    if (is_project_root(path))
      return path;
    const fs::path parent = path.parent_path();
    if (parent == path)
      return {};
    path = parent;
  }
}

fs::path find_project_root()
{
  if (fs::path root = search_parents(fs::current_path()); !root.empty())
    return root;

  std::error_code error;
  const fs::path executable = fs::read_symlink("/proc/self/exe", error);
  if (!error) {
    if (fs::path root = search_parents(executable.parent_path()); !root.empty())
      return root;
  }
  throw std::runtime_error(
      "cannot locate runtime/include/holyc.h; run holyc from its project tree");
}

std::vector<std::string> pkg_config_flags()
{
  std::array<char, 4096> buffer{};
  std::string output;
  FILE *pipe = popen("pkg-config --cflags --libs sdl2", "r");
  if (!pipe)
    throw std::runtime_error("could not start pkg-config");
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe))
    output += buffer.data();
  const int status = pclose(pipe);
  if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    throw std::runtime_error(
        "SDL2 development files were not found (pkg-config package: sdl2)");
  }

  std::istringstream words(output);
  std::vector<std::string> flags;
  for (std::string flag; words >> flag;)
    flags.push_back(std::move(flag));
  return flags;
}

int run_process(const std::vector<std::string> &arguments)
{
  std::vector<char *> argv;
  argv.reserve(arguments.size() + 1);
  for (const std::string &argument : arguments)
    argv.push_back(const_cast<char *>(argument.c_str()));
  argv.push_back(nullptr);

  const pid_t child = fork();
  if (child == -1)
    throw std::runtime_error("could not create compiler process");
  if (child == 0) {
    execvp(argv.front(), argv.data());
    std::perror("holyc: could not start clang");
    _exit(127);
  }

  int status;
  if (waitpid(child, &status, 0) == -1)
    throw std::runtime_error("could not wait for compiler process");
  return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

} // namespace

int main(int argc, char **argv)
{
  try {
    Options options = parse_options(argc, argv);
    if (!fs::is_regular_file(options.source))
      throw std::runtime_error("source file not found: " +
                               options.source.string());

    const fs::path root = find_project_root();
    const fs::path source = fs::absolute(options.source);
    const fs::path build = root / ".holyc-build";
    const std::string generated =
        holyc::translate(read_file(source), source.string());
    const fs::path generated_path =
        build / "generated" / (source.stem().string() + ".c");
    write_file(generated_path, generated);

    if (!options.emit_c.empty())
      write_file(fs::absolute(options.emit_c), generated);

    fs::path output = options.output.empty()
                          ? build / source.stem()
                          : fs::absolute(options.output);
    fs::create_directories(output.parent_path());

    std::vector<std::string> command = {
        "clang",
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Wno-strict-prototypes",
        "-I",
        (root / "runtime/include").string(),
        generated_path.string(),
        (root / "runtime/src/holyc_runtime.c").string(),
        "-o",
        output.string(),
    };
    const std::vector<std::string> sdl_flags = pkg_config_flags();
    command.insert(command.end(), sdl_flags.begin(), sdl_flags.end());

    const int result = run_process(command);
    if (result != 0)
      throw std::runtime_error("clang failed with exit code " +
                               std::to_string(result));
    std::cout << fs::absolute(output).string() << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "holyc: " << error.what() << '\n';
    return 1;
  }
}
