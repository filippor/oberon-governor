#ifndef GOVERNOR_OBERON_HPP
#define GOVERNOR_OBERON_HPP

#include <amdgpu.h>
#include <atomic>
#include <cstdint>
#include <deque>
#include <fstream>
#include <mutex>
#include <thread>

#define BIT(x)	(1U << (x))

#define OB_GRBM_REG			0x8010
#define OB_GRBM_GUI_ACTIVE		BIT(31)

#define OB_ACTIVE_SAMPLE_BUF_MS		100
#define OB_ACTIVE_SAMPLE_DELAY_MS	5

class Oberon {
private:
	// Linux "drivers/gpu/drm/amd/include/kgd_pp_interface.h" gpu_metrics_v2_2 struct
	struct GPUMetrics {
		struct {
			uint16_t	structure_size;
			uint8_t		format_revision;
			uint8_t		content_revision;
		} header;

		// Temperature
		uint16_t		temperature_gfx;
		uint16_t		temperature_soc;
		uint16_t		temperature_core[8];
		uint16_t		temperature_l3[2];
		// Utilization
		uint16_t		average_gfx_activity;
		uint16_t		average_mm_activity;
		// Timestamp
		uint64_t		system_clock_counter;
		// Power/Energy
		uint16_t		average_socket_power;
		uint16_t		average_cpu_power;
		uint16_t		average_soc_power;
		uint16_t		average_gfx_power;
		uint16_t		average_core_power[8];
		// Average Clocks
		uint16_t		average_gfxclk_frequency;
		uint16_t		average_socclk_frequency;
		uint16_t		average_uclk_frequency;
		uint16_t		average_fclk_frequency;
		uint16_t		average_vclk_frequency;
		uint16_t		average_dclk_frequency;
		// Current Clocks
		uint16_t		current_gfxclk;
		uint16_t		current_socclk;
		uint16_t		current_uclk;
		uint16_t		current_fclk;
		uint16_t		current_vclk;
		uint16_t		current_dclk;
		uint16_t		current_coreclk[8];
		uint16_t		current_l3clk[2];
		// Throttle Status
		uint32_t		throttle_status;
		// Fans
		uint16_t		fan_pwm;

		uint16_t		padding[3];
		// Throttle Status
		uint64_t		indep_throttle_status;
	};

	struct PCIID {
		uint16_t vendor;
		uint16_t device;
	} static const ids[];

	struct OPP {
		int frequency;
		int voltage;
	} static const opps[];
	int opp;

	amdgpu_device_handle amdgpu_handle;

	std::atomic<bool> run = true;

	std::deque<bool> busy_sample_buffer = std::deque<bool>(OB_ACTIVE_SAMPLE_BUF_MS / OB_ACTIVE_SAMPLE_DELAY_MS, false);
	std::mutex busy_sample_mutex;
	std::thread busy_sample_thread;

	std::ifstream metrics;
	std::ofstream ctl;

	void sampleThread();

public:
	struct Temperature {
		int gfx;
		int soc;
	};

	int getLoad();
	int getOpps();
	Temperature getTemperature();
	void setOpp(int opp);

	Oberon();
	~Oberon();
};

#endif // GOVERNOR_OBERON_HPP
