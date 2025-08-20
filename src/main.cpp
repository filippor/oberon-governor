#include <cstdlib>
#include <format>
#include <iostream>
#include <signal.h>
#include "governor.hpp"
#include "oberon.hpp"

const std::string help_text =
	"oberon-governor v" + std::string(GOVERNOR_VERSION) + "\n"
	"This program takes no arguments and must be run as root on an AMD Oberon system, on a systemd distro it can be run through oberon-governor.service\n"
	"Currently the only tested system is the ASRock BC-250.\n"
	"Option:\n"
	"-h,--help\t:print this help message\n"
	"-v,--verbose\t:print all state change\n"
	"\n";

Governor* g;

void stop(int signal) {
    switch (signal) {
        case SIGHUP:
            std::cerr << "Received SIGHUP (Hangup). Shutting down gracefully." << std::endl;
            break;
        case SIGINT:
            std::cerr << "Received SIGINT (Interrupt). Shutting down gracefully." << std::endl;
            break;
        case SIGQUIT:
            std::cerr << "Received SIGQUIT (Quit). Shutting down gracefully." << std::endl;
            break;
        case SIGTERM:
            std::cerr << "Received SIGTERM (Termination). Shutting down gracefully." << std::endl;
            break;
        default:
            std::cerr << "Received unknown signal (" << signal << "). Shutting down." << std::endl;
            break;
    }

	if (g)
		g->stop();
}

int main(int argc, char *argv[]) {
	bool verbose = false;
	for (int i = 1; i < argc; i++) {
		bool found = false;

		// Help
		for (const std::string a : {"-h", "--help"}) {
			if (a == argv[i]) {
				found = true;
				break;
			}
		}
		if (found) { // Print help text and exit
			std::cout << help_text;
			std::exit(EXIT_SUCCESS);
		}
		// Verbose
		for (const std::string a : {"-v", "--verbose"}) {
			if (a == argv[i]) {
				verbose = true;
				break;
			}
		}
		// Unknown argument
		if(!verbose){
			std::cout << std::format("Unknown argument '{}'", argv[i]) << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

	Oberon oberon(verbose);
	Governor governor(oberon,verbose);

	g = &governor;
	for (const int s : { SIGHUP, SIGINT, SIGQUIT, SIGTERM })
		signal(s, stop);

	std::cout << "Starting governor" << std::endl;
	governor.run();
	std::cout << "Stopping governor" << std::endl;
}
