#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include "yaml-cpp/yaml.h"
#include "governor.hpp"
#include "oberon.hpp"

void Governor::run() {
	int load, power, up = 0, down = 0, overheat = 0;
	bool overheat_warned = false;
	while (r) {
		const std::chrono::time_point start = std::chrono::system_clock::now();

		Oberon::Temperature temp = gpu.getTemperature();
		load = gpu.getLoad();
		power = gpu.getPower(); // Assuming getPower() is implemented as discussed

		// Proactive Thermal Management
		if (temp.gfx >= gfx_temp_soft_lim || temp.soc >= soc_temp_hard_lim) { // Use soft limit for proactive scaling
			if (temp.gfx >= gfx_temp_hard_lim || temp.soc >= soc_temp_hard_lim) { // Use hard limit for emergency throttle
				opp = 0;
				overheat = (overheat_reset_ms / polling_delay_ms);
				if (!overheat_warned) {
					std::cerr << std::format("[{}] GPU overheated, throttling - Silencing future warnings of this type", std::chrono::system_clock::now()) << std::endl;
					overheat_warned = true;
				}
			} else {
				// Proactively scale down one OPP to cool down
				up = -5;
				opp = std::max(opp - 1, 0);
			}
		} else if (overheat) {
			overheat--;
		} else {
			// Intelligent, granular load-based frequency control
			if (load >= up_threshold_high) {
				// Aggressive scale up
				up++; down = 0;
				if (up > 5) { // Require 3 consecutive high-load polls to scale up
					opp = opp_c;
				}
			} else if (load >= up_threshold_low) {
				// Gradual scale up
				up++; down = 0;
				if (opp < opp_c && up > 3) {
					opp++;
				}
			} else if (load <= down_threshold_low) {
				// Aggressive scale down
				down++; up = 0;
				if (down > 10) {
					opp = 0;
				}
			} else if (load <= down_threshold_high) {
				// Gradual scale down
				down++; up = 0;
				if (opp > 0 && down > 5) {
					opp--;
				}
			}
		}

		// Set OPP and wait for next poll
		gpu.setOpp(opp);
		std::this_thread::sleep_until(start + std::chrono::milliseconds(polling_delay_ms));
	}
}

void Governor::stop() {
	r = false;
}

Governor::Governor(Oberon& gpu) : gpu(gpu) {
	opp_c = gpu.getOpps() - 1;
	// Load governor configuration from YAML
	YAML::Node config = YAML::LoadFile("/etc/oberon-config.yaml");
	YAML::Node governor_config = config["governor"];

	polling_delay_ms = governor_config["polling_delay_ms"].as<int>();
	up_threshold_high = governor_config["up_threshold_high"].as<int>();
	up_threshold_low = governor_config["up_threshold_low"].as<int>();
	down_threshold_high = governor_config["down_threshold_high"].as<int>();
	down_threshold_low = governor_config["down_threshold_low"].as<int>();
	gfx_temp_soft_lim = governor_config["gfx_temp_soft_lim"].as<int>();
	gfx_temp_hard_lim = governor_config["gfx_temp_hard_lim"].as<int>();
	soc_temp_hard_lim = governor_config["soc_temp_hard_lim"].as<int>();
	overheat_reset_ms = governor_config["overheat_reset_ms"].as<int>();
}
