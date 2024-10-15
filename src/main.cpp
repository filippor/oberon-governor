#include <cstdlib>
#include <format>
#include <iostream>
#include <signal.h>
#include "governor.hpp"
#include "oberon.hpp"

const std::string help_text =
	"oberon-governor v" + std::string(GOVERNOR_VERSION) + "\n"
	"This program takes no arguments and must be run as root on an AMD Oberon system, on a systemd distro it can be run through oberon-governor.service\n"
	"Currently the only tested system is the ASRock BC-250.\n";

Governor* g;

void stop(int signal) {
	if (g)
		g->stop();
}

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		bool found = false;

		// Help
		for (const std::string& a : {"-h", "--help"}) {
			if (a == argv[i]) {
				found = true;
				break;
			}
		}
		if (found) { // Print help text and exit
			std::cout << help_text;
			std::exit(EXIT_SUCCESS);
		}

		// Unknown argument
		std::cout << std::format("Unknown argument '{}'", argv[i]) << std::endl;
		std::exit(EXIT_FAILURE);
	}

	Oberon oberon;
	Governor governor(oberon);

	g = &governor;
	for (const int s : { SIGHUP, SIGINT, SIGQUIT, SIGTERM })
		signal(s, stop);

	std::cout << "Starting governor" << std::endl;
	governor.run();
	std::cout << "Stopping governor" << std::endl;
}
