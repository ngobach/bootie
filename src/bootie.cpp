#include <stdint.h>
#include <unistd.h>

#include <argparse/argparse.hpp>
#include <battery/embed.hpp>
#include <cstdio>
#include <string>

#include "nativeio.hpp"

using namespace std;

void cmd_install(string target) {
  nativeio::install_to(target);
}

void cmd_list() {
  auto disks = nativeio::get_disks();
  fprintf(stderr, "Available disks:\n");

  for (auto& disk : disks) {
    fprintf(stderr, "%s (%s - %s)\n", disk.identifier.c_str(),
            disk.label.c_str(),
            nativeio::friendly_size_name(disk.size).c_str());
  }
}

int main(int argc, char* argv[]) {
  argparse::ArgumentParser program("bootie");

  argparse::ArgumentParser list_command("list");
  list_command.add_description("List available targets");
  program.add_subparser(list_command);

  argparse::ArgumentParser install_command("install");
  install_command.add_description(
      "Install Bootie to the hard drive at the given path");
      install_command.add_argument("dest", "Target disk");
  program.add_subparser(install_command);

  try {
    program.parse_args(argc, argv);
  } catch (exception const& err) {
    program.print_help();

    return 0;
  }

  if (program.is_subcommand_used(list_command)) {
    cmd_list();
  } else if (program.is_subcommand_used(install_command)) {
    cmd_install(install_command.get<std::string>("dest"));
  } else {
    program.print_help();
  }

  return 0;
}
