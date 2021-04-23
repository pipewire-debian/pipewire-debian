# PipeWire PPA for Ubuntu
## Upstream Version of PipeWire for ubuntu to be maintained from now.
## Upstream is maintained a 15days release cycle.


- [PipeWire-Upstream](https://gitlab.freedesktop.org/pipewire/pipewire) **Forked From Here And**
- [PipeWire-debian-Upstream](https://salsa.debian.org/utopia-team/pipewire/-/tree/debian/0.3.25-1) **Forked From Here**
- [libfdk-aac2](https://packages.ubuntu.com/hirsute/libfdk-aac2) **Dependencies**
- [libopenaptx0](https://packages.ubuntu.com/hirsute/libopenaptx0) **Dependencies**
- [libldacbt-abr2](https://packages.ubuntu.com/hirsute/libldacbt-abr2) **Dependencies**
- [libldacbt-enc2](https://packages.ubuntu.com/hirsute/libldacbt-enc2) **Dependencies**

# Usage

```bash
curl -SsL https://pipewire-debian.github.io/pipewire-debian/KEY.gpg | sudo apt-key add -
sudo curl -SsL -o /etc/apt/sources.list.d/pipewire.list https://pipewire-debian.github.io/pipewire-debian/pipewire.list
sudo apt update

# Iinstall dependencies

sudo apt install libfdk-aac2 libldacbt-{abr,enc}2 libopenaptx0

# Install pipewire and addtional packages

sudo apt install gstreamer1.0-pipewire libpipewire-0.3-{0,dev,modules} libspa-0.2-{bluetooth,dev,jack,modules} pipewire{,-{audio-client-libraries,bin,locales,tests}}

# Additionally If you want to install `pipewire-doc`, do follow-

sudo apt install pipewire-doc

# You don't need to uninstall PulseAudio to enable PipeWire, disable and mask PulseAudio related services and stop them

systemctl --user stop pulseaudio
systemctl --user mask pulseaudio

# After Installation, Enable pipewire related services

systemctl --user --now enable pipewire{,-pulse}{.socket,.service} pipewire-media-session.service

# Still if your system does't have any sound, please reboot
```
# Wiki

- [Gentoo-wiki](https://wiki.gentoo.org/wiki/PipeWire)
- [Arch-wiki](https://wiki.archlinux.org/index.php/PipeWire)
- [Debian-wiki](https://wiki.debian.org/PipeWire)

# Note - 

If you have any issue regarding this PPA package, create a issue here.

### Otherwise If you have any issue regarding on pipewire or have any feature requests, create an issue to [upstream](https://gitlab.freedesktop.org/pipewire/pipewire) url.

# Sources

Original Maintainers (usually from Debian):
Utopia Maintenance Team
Jeremy Bicha
[Repo-Source](https://salsa.debian.org/utopia-team/pipewire/-/tree/debian/0.3.25-1)

