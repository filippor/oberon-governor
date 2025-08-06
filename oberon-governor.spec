Name:           oberon-governor
Version:        working
Release:        1.20250806203701502209.testing.0.ge24b075%{?dist}
Summary:        Oberon Governor for bc-250

License:        MIT
URL:            https://gitlab.com/mothenjoyer69/oberon-governor
Source0:        oberon-governor-working.tar.gz

BuildRequires:  libdrm-devel cmake make g++ git

%description
A simple daemon for AMD Oberon based systems that automatically governs GPU voltage and frequency based on load and temperature.
Testing is only performed on the ASRock BC-250.

%prep
%autosetup -n oberon-governor-working


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
* Wed Aug 06 2025 filippor <filippo.rossoni@gmail.com> - working-1.20250806203701502209.testing.0.ge24b075
- Development snapshot (e24b075b)
