#include <amdgpu.h>
#include <chrono>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#include <xf86drm.h>
#include "oberon.hpp"
#include "yaml-cpp/yaml.h"

const Oberon::PCIID Oberon::ids[] = {
	{ 0x1002, 0x13fe }, // CYAN_SKILLFISH2
};

const Oberon::OPP Oberon::opps[] = { 
    { 
      	YAML::LoadFile("/etc/oberon-config.yaml")["opps"][0]["frequency"][0]["min"].as<int>(), 
        YAML::LoadFile("/etc/oberon-config.yaml")["opps"][1]["voltage"][0]["min"].as<int>()
    },  
    { 
      	YAML::LoadFile("/etc/oberon-config.yaml")["opps"][0]["frequency"][1]["max"].as<int>(), 
        YAML::LoadFile("/etc/oberon-config.yaml")["opps"][1]["voltage"][1]["max"].as<int>()
    } 
};

void Oberon::sampleThread() {
	while (run.load()) {
		const std::chrono::time_point start = std::chrono::system_clock::now();

		uint32_t reg;
		amdgpu_read_mm_registers(amdgpu_handle, OB_GRBM_REG / 4, 1, 0xffffffff, 0, &reg);
		bool busy = reg & OB_GRBM_GUI_ACTIVE;

		busy_sample_mutex.lock();
		busy_sample_buffer.pop_front();
		busy_sample_buffer.push_back(busy);
		busy_sample_mutex.unlock();

		std::this_thread::sleep_until(start + std::chrono::milliseconds(OB_ACTIVE_SAMPLE_DELAY_MS));
	}
}

int Oberon::getLoad() {
	int load = 0;

	busy_sample_mutex.lock();
	for (const bool busy : busy_sample_buffer)
		load += busy;
	busy_sample_mutex.unlock();

	return load * (100 / (OB_ACTIVE_SAMPLE_BUF_MS / OB_ACTIVE_SAMPLE_DELAY_MS));
}

int Oberon::getOpps() {
	return sizeof(opps) / sizeof(OPP);
}

Oberon::Temperature Oberon::getTemperature() {
	GPUMetrics buf;

	metrics.read(reinterpret_cast<char*>(&buf), sizeof(GPUMetrics));
	metrics.seekg(0);

	return { buf.temperature_gfx / 100, buf.temperature_soc / 100 };
}

void Oberon::setOpp(int opp) {
	if (this->opp == opp)
		return;

	OPP o = opps[opp];
	ctl << "vc 0 " + std::to_string(o.frequency) + " " + std::to_string(o.voltage) << std::endl;
	ctl << "c" << std::endl;
	this->opp = opp;
}

Oberon::Oberon() {
	// Get DRM device
	int count = drmGetDevices(nullptr, 0);
	if (count <= 0)
		throw std::runtime_error("Failed to detect DRM devices");
	drmDevicePtr devices[count];
	if (drmGetDevices(devices, count) < 0)
		throw std::runtime_error("Failed to get DRM device list");

	bool found = false;
	int fd;
	std::filesystem::path sysfs;
	for (int i = 0; i < count; i++) {
		if (devices[i]->bustype != DRM_BUS_PCI)
			continue;

		bool ok = false;
		for (int j = 0; j < sizeof(ids) / sizeof(PCIID); j++)
			if (devices[i]->deviceinfo.pci->vendor_id == ids[j].vendor &&
			    devices[i]->deviceinfo.pci->device_id == ids[j].device) {
				ok = true;
				break;
			}

		if (!ok)
			continue;

		for (int n : { DRM_NODE_RENDER, DRM_NODE_PRIMARY }) {
			if (!(1 << n & devices[i]->available_nodes))
				continue;
			int f = open(devices[i]->nodes[n], O_RDWR);
			if (f >= 0) {
				found = true;
				fd = f;
				sysfs = std::filesystem::path("/sys/class/drm").append(std::filesystem::path(devices[i]->nodes[n]).filename().string()).append("device"); // TODO: Replace this with something saner?
				break;
			}
		}
		if (found)
			break;
	}
	drmFreeDevices(devices, count);
	if (!found)
		throw std::runtime_error("Failed to open DRM device");

	// Initialize AMDGPU device
	uint32_t libdrm_major, libdrm_minor;
	if (amdgpu_device_initialize(fd, &libdrm_major, &libdrm_minor, &amdgpu_handle)) {
		close(fd);
		throw std::runtime_error("Failed to initialize AMDGPU device");
	}
	close(fd);

	// Open metrics sysfs node
	metrics.exceptions(~std::ios::goodbit);
	metrics.open(std::filesystem::path(sysfs).append("gpu_metrics"), std::ios::binary);

	// Open clock/voltage control sysfs node
	ctl.exceptions(~std::ios::goodbit);
	ctl.open(std::filesystem::path(sysfs).append("pp_od_clk_voltage"));

	// Put GPU in known state
	setOpp(0);

	// Start sampling thread
	busy_sample_thread = std::thread(&Oberon::sampleThread, this);
}

Oberon::~Oberon() {
	run = false;
	busy_sample_thread.join();
	setOpp(0);
	metrics.close();
	amdgpu_device_deinitialize(amdgpu_handle);
}
