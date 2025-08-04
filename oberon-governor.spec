Name:           oberon-governor
Version:        v0.0.1
Release:        1.20250804092901933900.main.0.gd25555d%{?dist}
Summary:        Oberon Governor for bc-250

License:        MIT
URL:            https://gitlab.com/mothenjoyer69/oberon-governor
Source0:        oberon-governor-v0.0.1.tar.gz

BuildRequires:  libdrm-devel cmake make g++ git

%description
A simple daemon for AMD Oberon based systems that automatically governs GPU voltage and frequency based on load and temperature.
Testing is only performed on the ASRock BC-250.

%prep
%autosetup -n oberon-governor-v0.0.1


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
