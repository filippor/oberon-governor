#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include "governor.hpp"
#include "oberon.hpp"

void Governor::run() {
	int load, up = 0, down = 0, overheat = 0;
	bool overheat_warned = false;
	while (r) {
		const std::chrono::time_point start = std::chrono::system_clock::now();

		// Thermal throttling
		Oberon::Temperature temp = gpu.getTemperature();
		if (temp.gfx >= GOV_GFX_TEMP_LIM || temp.soc >= GOV_SOC_TEMP_LIM) {
			opp = 0;
			overheat = (GOV_OVERHEAT_RESET_MS / GOV_POLLING_DELAY_MS);
			if (!overheat_warned) {
				std::cerr << std::format("[{}] GPU overheated, throttling - Silencing future warnings of this type", std::chrono::system_clock::now()) << std::endl;
				overheat_warned = true;
			}
			goto end;
		}

		if (overheat) {
			overheat--;
			goto end;
		}

		// Load based frequency control
		load = gpu.getLoad();

		if (load >= GOV_UP_THRESHOLD)
			up++;
		else
			up = 0;

		if (load < GOV_DOWN_THRESHOLD)
			down++;
		else
			down = 0;

		if (up >= (GOV_UP_DELAY_MS / GOV_POLLING_DELAY_MS)) {
			up = 0;
			opp = opp_c; // Jump straight to maximum frequency under load
		} else if (down >= (GOV_DOWN_DELAY_MS / GOV_POLLING_DELAY_MS)) {
			down = 0;
			opp = std::max(opp - 1, 0);
		}

	end:
		// Set OPP and wait for next poll
		gpu.setOpp(opp);
		std::this_thread::sleep_until(start + std::chrono::milliseconds(GOV_POLLING_DELAY_MS));
	}
}

void Governor::stop() {
	r = false;
}

Governor::Governor(Oberon& gpu) : gpu(gpu) {
	opp_c = gpu.getOpps() - 1;
}
