Name:           oberon-governor
Version:        0.1.0
Release:        1.20250806203911260285.testing.5.g11cbfce%{?dist}
Summary:        Oberon Governor for bc-250

License:        MIT
URL:            https://gitlab.com/mothenjoyer69/oberon-governor
Source0:        oberon-governor-0.1.0.tar.gz

BuildRequires:  libdrm-devel cmake make g++ git

%description
A simple daemon for AMD Oberon based systems that automatically governs GPU voltage and frequency based on load and temperature.
Testing is only performed on the ASRock BC-250.

%prep
%autosetup -n oberon-governor


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

