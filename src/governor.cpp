#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include "yaml-cpp/yaml.h"
#include "governor.hpp"
#include "oberon.hpp"

void Governor::run() {
	int load, move = 0,wait =0 , old_opp;
	bool overheat_warned = false;
	int max_opp = _opp_count;
	while (r) {
		const std::chrono::time_point start = std::chrono::system_clock::now();
		old_opp = _opp;
		
		Oberon::Temperature temp = _gpu.getTemperature();
		load = _gpu.getLoad();

		// Proactive Thermal Management
		if (temp.gfx >= gfx_temp_soft_lim || temp.soc >= soc_temp_hard_lim) { // Use soft limit for proactive scaling
			if (!overheat_warned) {
				std::cerr << std::format("[{}] GPU overheated, throttling - Silencing future warnings of this type", std::chrono::system_clock::now()) << std::endl;
				overheat_warned = true;
			}
			if (temp.gfx >= gfx_temp_hard_lim || temp.soc >= soc_temp_hard_lim) { 
				// Use hard limit for emergency throttle to 0
				max_opp = 0;
				if(_verbose) std::cout << std::format("[{}] GPU Hard overheated, throttling max {}", std::chrono::system_clock::now(),max_opp) << std::endl;		
			} else {
				// Proactively scale down one OPP to cool down	
				max_opp = std::max(max_opp - 1, 0);
				if(_verbose) std::cout << std::format("[{}] GPU Soft overheated, throttling max {}", std::chrono::system_clock::now(),max_opp) << std::endl;	
			}
			//set wait time
			wait = overheat_reset_ms / polling_delay_ms; 
			_opp = max_opp;
		} else {	
			if (load >= up_threshold_low) {
				 move = std::min(50,move + 1);// increase consecutive low load to scale down
				 if (_opp < max_opp ) {
					if (load >= up_threshold_high) {
						// Aggressive scale up
						 _opp = max_opp;
						 if(_verbose) std::cout << std::format("[{}] GPU Aggressive scale up {} load {}", std::chrono::system_clock::now(),_opp,load) << std::endl;
					} else{
						// Gradual scale up
						_opp++;
						if(_verbose) std::cout << std::format("[{}] GPU Gradual scale up {} load {}", std::chrono::system_clock::now(),_opp,load) << std::endl;
					}
				 }
			} else if (load <= down_threshold_high && _opp > 0) {
				move = std::max(-100, move -1);
				if(_opp > 0 && move < -50){// Require 50 consecutive low-load polls to scale down
					if (load <= down_threshold_low) {
						// Aggressive scale down
						_opp = 0;
						if(_verbose) std::cout << std::format("[{}] GPU Aggressive scale down {} load {}", std::chrono::system_clock::now(),_opp,load) << std::endl;
					} else {
						// Gradual scale down
						_opp--;
						if(_verbose) std::cout << std::format("[{}] GPU Gradual scale down {} load {}", std::chrono::system_clock::now(),_opp,load) << std::endl;
					}
				}
			}else {
				// If is stable reduce move to easy change
				if ( move < 0) {
					move++;
				} else if (move > 0) {
					move--;
				}
			}
		}
		if(wait >= 0){
			wait --;
			if(wait == (overheat_reset_ms / polling_delay_ms /2)){
				//tentative slow recover at half wait					
				max_opp = std::min(_opp_count,max_opp +1);
				if(_verbose) std::cout << std::format("[{}] GPU Slow recovery max {}", std::chrono::system_clock::now(),max_opp) << std::endl;
			}else if (wait == 0){
				max_opp = _opp_count;
				if(_verbose) std::cout << std::format("[{}] GPU Full recovery max {}", std::chrono::system_clock::now(),max_opp) << std::endl;	
			}
		}

		// Set OPP and wait for next poll
		if(_opp != old_opp){
			move =0;
			_gpu.setOpp(_opp);
		}
		std::this_thread::sleep_until(start + std::chrono::milliseconds(polling_delay_ms));
	}
}

void Governor::stop() {
	r = false;
}

Governor::Governor(Oberon& gpu, bool verbose) : _gpu(gpu), _verbose(verbose) {
	_opp_count = gpu.getOpps() - 1;
	// Load governor configuration from YAML
	YAML::Node config = YAML::LoadFile("/etc/oberon-config.yaml");
	YAML::Node governor_config = config["governor"];
    if ( governor_config.IsDefined() ) {
	   polling_delay_ms = governor_config["polling_delay_ms"].as<int>();
	   up_threshold_high = governor_config["up_threshold_high"].as<int>();
	   up_threshold_low = governor_config["up_threshold_low"].as<int>();
	   down_threshold_high = governor_config["down_threshold_high"].as<int>();
	   down_threshold_low = governor_config["down_threshold_low"].as<int>();
	   gfx_temp_soft_lim = governor_config["gfx_temp_soft_lim"].as<int>();
	   gfx_temp_hard_lim = governor_config["gfx_temp_hard_lim"].as<int>();
	   soc_temp_hard_lim = governor_config["soc_temp_hard_lim"].as<int>();
	   overheat_reset_ms = governor_config["overheat_reset_ms"].as<int>();
	 }else{
	   polling_delay_ms = 100;
	   up_threshold_high = 85;
	   up_threshold_low = 70;
	   down_threshold_high = 45;
	   down_threshold_low = 5;
	   gfx_temp_soft_lim = 85;
	   gfx_temp_hard_lim = 90;
	   soc_temp_hard_lim = 90;
	   overheat_reset_ms = 5000;    
	 }
}
