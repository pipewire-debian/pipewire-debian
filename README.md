<!-- PipeWire-Debian-->

<p align="center">
  <a href="https://gitlab.freedesktop.org/pipewire/pipewire">
    <img alt="PipeWire Original Repo" src="https://github.com/souravdas142/dotfiles/raw/master/.local/share/logo/pw2.png" width="656" height="300">
  </a>
</p>

<p align="center">
  <!-- Maintained -->
  <a href="https://github.com/pipewire-debian/pipewire-debian/">
    <img src="https://img.shields.io/website?down_message=Yes&label=Maintained&logo=GITHUB&up_message=Yes&url=https%3A%2F%2Fgithub.com%2Fpipewire-debian%2Fpipewire-debian">
  </a>
  <!-- Last commit on dev -->
  <a href="https://github.com/pipewire-debian/pipewire-debian/tree/development">
    <img src="https://img.shields.io/github/last-commit/pipewire-debian/pipewire-debian/development?color=%23ffA000&label=Last%20commit%20on%20Development&logo=GITHUB&style=plastic">
  </a>
  <!-- Last commit on master -->
  <img src="https://img.shields.io/github/last-commit/pipewire-debian/pipewire-debian/master?color=%23ffA000&label=master&logo=GITHUB&style=plastic">
  <!-- Packager -->
  <a href="https://github.com/souravdas142/">
    <img src="https://img.shields.io/website?down_message=Sourav%20Das&label=Packager&logo=GITHUB&up_message=Sourav%20Das&url=https%3A%2F%2Fgithub.com%2Fsouravdas142">
  </a>
</p>

<p align="center">
  <!-- Website status -->
  <a href="https://pipewire-debian.github.io/pipewire-debian/">
    <img src="https://img.shields.io/website?label=Git%20Website&logo=GITHUB&url=https%3A%2F%2Fpipewire-debian.github.io%2Fpipewire-debian%2F">
  </a>
  <!-- PPA -->
  <a href="https://launchpad.net/~pipewire-debian/+archive/ubuntu/pipewire-upstream">
    <img src="https://img.shields.io/website?down_message=PipeWire%200.3.26&label=Launchpad%20PPA&logo=UBUNTU&up_message=PipeWire%200.3.26&url=https%3A%2F%2Flaunchpad.net%2F~pipewire-debian%2F%2Barchive%2Fubuntu%2Fpipewire-upstream">
  </a>
</p>

# PipeWire PPA for Ubuntu (>= 20.04)
#### _An upstream version of PipeWire for Ubuntu maintained with a 15 day release cycle_

|                                               Link                                               |          Description           |
| :----------------------------------------------------------------------------------------------: | :----------------------: |
|          [Original-PipeWire-Upstream](https://gitlab.freedesktop.org/pipewire/pipewire)          | **Forked** |
| [PipeWire-debian-Upstream](https://salsa.debian.org/utopia-team/pipewire/-/tree/debian/0.3.25-1) |   **Forked**   |
|                  [libfdk-aac2](https://packages.ubuntu.com/hirsute/libfdk-aac2)                  |     **Dependencies**     |
|                 [libopenaptx0](https://packages.ubuntu.com/hirsute/libopenaptx0)                 |     **Dependencies**     |
|               [libldacbt-abr2](https://packages.ubuntu.com/hirsute/libldacbt-abr2)               |     **Dependencies**     |
|               [libldacbt-enc2](https://packages.ubuntu.com/hirsute/libldacbt-enc2)               |     **Dependencies**     |

## [Launchpad PPA](https://launchpad.net/~pipewire-debian/+archive/ubuntu/pipewire-upstream)

:warning: **This repo is not obsolete. I will sync this repo with the Launchpad PPA to support other Debian based distros. The [development](https://github.com/pipewire-debian/pipewire-debian/tree/development) branch is important as I will be using that branch to push new patches related to building PipeWire and its dependencies.**

## 1. PPA Configuration

### Add the Launchpad PPA...

:bulb: **The recommended & convenient way, see next [section](#or-the-github-ppa) for other Debian-based distros**

```bash
You can update your system with unsupported packages from this untrusted PPA by adding ppa:pipewire-debian/pipewire-upstream
to your system's Software Sources. (Read about installing)

sudo add-apt-repository ppa:pipewire-debian/pipewire-upstream
sudo apt-get update

Technical details about this PPA
This PPA can be added to your system manually by copying the lines below and adding them to your system's software sources.

deb http://ppa.launchpad.net/pipewire-debian/pipewire-upstream/ubuntu focal main
deb-src http://ppa.launchpad.net/pipewire-debian/pipewire-upstream/ubuntu focal main
```

### ...or the Github PPA

:bulb: **Use IFF you have a good reason - mostly for non-Ubuntu distros (eg. Kali, MX, Parrot, Mint, Deepin, etc)**

```bash
curl -SsL https://pipewire-debian.github.io/pipewire-debian/KEY.gpg | sudo apt-key add -
sudo curl -SsL -o /etc/apt/sources.list.d/pipewire.list https://pipewire-debian.github.io/pipewire-debian/pipewire.list
sudo apt update

```

## 2. Install PipeWire

### After adding one of the PPA's, follow the installation instructions below

```bash
# Install dependencies

sudo apt install libfdk-aac2 libldacbt-{abr,enc}2 libopenaptx0

# If `libfdk-aac2` not found install `libfdk-aac`
# Install pipewire and additional packages

sudo apt install gstreamer1.0-pipewire libpipewire-0.3-{0,dev,modules} libspa-0.2-{bluetooth,dev,jack,modules} pipewire{,-{audio-client-libraries,bin,locales,tests}}

# Additionally, if you want to install `pipewire-doc`

sudo apt install pipewire-doc
```   

    
## 3. Post Installation Steps    
You don't need to uninstall PulseAudio to enable PipeWire, disable and mask PulseAudio related services to stop them    
```bash
systemctl --user --now disable  pulseaudio.{socket,service}
systemctl --user mask pulseaudio
```
Enable and start PipeWire related services    
```bash
systemctl --user --now enable pipewire{,-pulse}.{socket,service} pipewire-media-session.service
```
You can check which server is in use by running (as a user):   
```bash
pactl info | grep '^Server Name'
```
If your system doesn't have any sound, please reboot    



# :book: Wiki

- [This Repo Wiki](https://github.com/pipewire-debian/pipewire-debian/wiki)
- [Upstream-README](https://gitlab.freedesktop.org/pipewire/pipewire/-/blob/master/README.md)
- [Upstream-wiki](https://gitlab.freedesktop.org/pipewire/pipewire/-/wikis/home)
- **Gentoo**
    - [Gentoo-wiki](https://wiki.gentoo.org/wiki/PipeWire)
    - [Gentoo-Bluetooth-Wiki](https://wiki.gentoo.org/wiki/Bluetooth)
- **Arch-Linux** 
    - [Arch-wiki](https://wiki.archlinux.org/index.php/PipeWire)
    - [Arch-Bluetooth-Wiki](https://wiki.archlinux.org/title/Bluetooth)
- [Debian-wiki](https://wiki.debian.org/PipeWire)

# :hammer_and_wrench: Troubleshooting  

**See in wiki page - [Troubleshooting](https://github.com/pipewire-debian/pipewire-debian/wiki/Troubleshooting)**

# :fire: Notice

If you have any issue regarding this PPA package, create a issue here.

**For features, requests or bugs, create an issue on [upstream](https://gitlab.freedesktop.org/pipewire/pipewire/-/issues).**

# :clap: Credits

Original project maintainer:
[Wim Taymans](https://gitlab.freedesktop.org/wtaymans) - [Source](https://gitlab.freedesktop.org/pipewire/pipewire)

Original maintainers (usually from Debian):
Utopia Maintenance Team - Jeremy Bicha - [Source](https://salsa.debian.org/utopia-team/pipewire/-/tree/debian/0.3.25-1)
