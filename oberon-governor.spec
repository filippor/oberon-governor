Name:           oberon-governor
Version:        v0.1.1
Release:        1.20250813220914508553.main.1.gfb93b25%{?dist}
Summary:        Oberon Governor for bc-250

License:        MIT
URL:            https://gitlab.com/mothenjoyer69/oberon-governor
Source0:        oberon-governor-v0.1.1.tar.gz

BuildRequires:  libdrm-devel cmake make g++ git yaml-cpp-devel

%description
A simple daemon for AMD Oberon based systems that automatically governs GPU voltage and frequency based on load and temperature.
Testing is only performed on the ASRock BC-250.

%prep
%autosetup -n oberon-governor-v0.1.1


%build
%cmake
%cmake_build


%install
%cmake_install


%files
%license LICENSE
%doc README.md
    %{_sysconfdir}/oberon-config.yaml
    %{_bindir}/oberon-governor
    /usr/lib/systemd/system/oberon-governor.service

%changelog

* Wed Aug 13 2025 Filippo Rossoni <filippo.rossoni@gmail.com> - v0.1.1-1.20250813220914508553.main.1.gfb93b25
- try to fix build (Filippo Rossoni)
