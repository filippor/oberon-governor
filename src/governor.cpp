#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include "yaml-cpp/yaml.h"
#include "governor.hpp"
#include "oberon.hpp"

void Governor::run() {
	int load, move = 0, old_opp;
	bool overheat_warned = false;
	while (r) {
		const std::chrono::time_point start = std::chrono::system_clock::now();
		old_opp = opp;
		Oberon::Temperature temp = gpu.getTemperature();
		load = gpu.getLoad();

		// Proactive Thermal Management
		if (temp.gfx >= gfx_temp_soft_lim || temp.soc >= soc_temp_hard_lim) { // Use soft limit for proactive scaling
			if (temp.gfx >= gfx_temp_hard_lim || temp.soc >= soc_temp_hard_lim) { // Use hard limit for emergency throttle
				opp = 0;
				move = -(overheat_reset_ms / polling_delay_ms);
				if (!overheat_warned) {
					std::cerr << std::format("[{}] GPU overheated, throttling - Silencing future warnings of this type", std::chrono::system_clock::now()) << std::endl;
					overheat_warned = true;
				}
			} else {
				// Proactively scale down one OPP to cool down
				move = -(overheat_reset_ms / polling_delay_ms);
				opp = std::max(opp - 1, 0);
			}
		} else {
			if (load >= up_threshold_low) {
				 move= move = std::min(25,move + 1) ;
				 if (opp < opp_c && move > 5) { // Require 5 consecutive high-load polls to scale up
					// Intelligent, granular load-based frequency control
					if (load >= up_threshold_high) {
							// Aggressive scale up
							move = std::max(0,move);
							opp = opp_c;
					} else{
						// Gradual scale up
							opp++;
					}
				 }
			} else if (load <= down_threshold_high) {
				// Gradual scale down
				move= move = std::max(-15,move -1 ) ;
				if(opp > 0 && move < -10){// Require 10 consecutive low-load polls to scale down
					if (load <= down_threshold_low) {
						// Aggressive scale down
						opp = 0;
					} else {// Require 10 consecutive low-load polls to scale down
						opp--;
					}
				}
			}else {
				if ( move < 0) {
					move++;
				} else if (move > 0) {
					move--;
				}
			}
		}

		// Set OPP and wait for next poll
		if(opp != old_opp){
			gpu.setOpp(opp);
		}
		std::this_thread::sleep_until(start + std::chrono::milliseconds(move > 20 ? polling_delay_ms * 10: polling_delay_ms));
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
