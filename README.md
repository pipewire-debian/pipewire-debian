<!-- PipeWire-Debian-->

<p align="center">
<a href="https://gitlab.freedesktop.org/pipewire/pipewire">
  <img alt="PipeWire Original Repo" src="https://github.com/souravdas142/dotfiles/raw/master/.local/share/logo/pw2.png" width="656" height="300">
</a>
</p>

<p align="center">

<a href="https://github.com/pipewire-debian/pipewire-debian/">
<img src="https://img.shields.io/website?down_message=Yes&label=Maintained&logo=GITHUB&up_message=Yes&url=https%3A%2F%2Fgithub.com%2Fpipewire-debian%2Fpipewire-debian">
</a>

<a href="https://github.com/pipewire-debian/pipewire-debian/tree/development">
<img src="https://img.shields.io/github/last-commit/pipewire-debian/pipewire-debian/development?color=%23ffA000&label=Last%20commit%20on%20Development&logo=GITHUB&style=plastic">
</a>

<img src="https://img.shields.io/github/last-commit/pipewire-debian/pipewire-debian/master?color=%23ffA000&label=master&logo=GITHUB&style=plastic">


<a href="https://github.com/souravdas142/">
<img src="https://img.shields.io/website?down_message=Sourav%20Das&label=Packager&logo=GITHUB&up_message=Sourav%20Das&url=https%3A%2F%2Fgithub.com%2Fsouravdas142">
</a>

</p>

<p align="center">
<a href="https://pipewire-debian.github.io/pipewire-debian/">
<img src="https://img.shields.io/website?label=Git%20Website&logo=GITHUB&url=https%3A%2F%2Fpipewire-debian.github.io%2Fpipewire-debian%2F">
</a>

<a href="https://launchpad.net/~pipewire-debian/+archive/ubuntu/pipewire-upstream">
<img src="https://img.shields.io/website?down_message=PipeWire%200.3.26&label=Launchpad%20PPA&logo=UBUNTU&up_message=PipeWire%200.3.26&url=https%3A%2F%2Flaunchpad.net%2F~pipewire-debian%2F%2Barchive%2Fubuntu%2Fpipewire-upstream">
</a>

</p>

# PipeWire PPA for Ubuntu (>= 20.04)
## Upstream Version of PipeWire for ubuntu to be maintained from now.
## Upstream is maintained a 15days release cycle.

|Link|Title|
|:---:|:---:|
|[Original-PipeWire-Upstream](https://gitlab.freedesktop.org/pipewire/pipewire) | **Forked From Here And**
|[PipeWire-debian-Upstream](https://salsa.debian.org/utopia-team/pipewire/-/tree/debian/0.3.25-1) | **Forked From Here**
|[libfdk-aac2](https://packages.ubuntu.com/hirsute/libfdk-aac2) | **Dependencies**
|[libopenaptx0](https://packages.ubuntu.com/hirsute/libopenaptx0) | **Dependencies**
|[libldacbt-abr2](https://packages.ubuntu.com/hirsute/libldacbt-abr2) | **Dependencies**
|[libldacbt-enc2](https://packages.ubuntu.com/hirsute/libldacbt-enc2) | **Dependencies**


# HURRAY [Launchpad PPA](https://launchpad.net/~pipewire-debian/+archive/ubuntu/pipewire-upstream) Added  

:warning: **THAT MEAN THIS REPO IS NOT OBSELETE. I WILL SYNC THIS REPO WITH LAUNCHPAD PPA TO SUPPORT OTHER DEBIAN BASED DISTROS. THE [DEVLEOPMENT](https://github.com/pipewire-debian/pipewire-debian/tree/development) BRANCH IS IMPORTANT OF THIS REPO, I WILL BE USING THAT BRANCH TO PUSH NEW PATCH RELATED TO BUILDING PIPEWIRE AND ITS DEPENDENCIES.**

## Add Launchpad PPA
 :bulb: **RECOMMENDED & CONVIENT WAY, SEE NEXT [Section](#or-add-github-ppa) FOR OTHER DEBIAN BASED DISTRO**

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


## OR Add Github PPA
:bulb: **Use IFF you have any strong Reason - Mostly for other than Ubuntu (Kali,MX,parrot,mint,deepin,etc)**

```bash
curl -SsL https://pipewire-debian.github.io/pipewire-debian/KEY.gpg | sudo apt-key add -
sudo curl -SsL -o /etc/apt/sources.list.d/pipewire.list https://pipewire-debian.github.io/pipewire-debian/pipewire.list
sudo apt update

```

## **LETS FOLLOW THE REST OF INSTALLATION INSTRUCTION AFTER ADDING ONE OF THE ABOVE TWO PPA**


```bash
# Install dependencies

sudo apt install libfdk-aac2 libldacbt-{abr,enc}2 libopenaptx0

# If `libfdk-aac2` not found install `libfdk-aac`
# Install pipewire and addtional packages

sudo apt install gstreamer1.0-pipewire libpipewire-0.3-{0,dev,modules} libspa-0.2-{bluetooth,dev,jack,modules} pipewire{,-{audio-client-libraries,bin,locales,tests}}

# Additionally If you want to install `pipewire-doc`, do follow-

sudo apt install pipewire-doc

# You don't need to uninstall PulseAudio to enable PipeWire, disable and mask PulseAudio related services and stop them

systemctl --user --now disable  pulseaudio.{socket,service}
systemctl --user mask pulseaudio

# After Installation, Enable pipewire related services

systemctl --user --now enable pipewire{,-pulse}{.socket,.service} pipewire-media-session.service

# You can check which server is in use by, as your regular user, running:

pactl info | grep '^Server Name'

# Still if your system does't have any sound, please reboot
```

# Wiki

- [Gentoo-wiki](https://wiki.gentoo.org/wiki/PipeWire)
- [Arch-wiki](https://wiki.archlinux.org/index.php/PipeWire)
- [Debian-wiki](https://wiki.debian.org/PipeWire)

# :fire: Note - 

If you have any issue regarding this PPA package, create a issue here.

### For feature requests or Bugs, create an issue on [upstream](https://gitlab.freedesktop.org/pipewire/pipewire) url.

# Original Credits

Original Project Maintainer ([Wim Taymans](https://gitlab.freedesktop.org/wtaymans)) - [Repo-Source](https://gitlab.freedesktop.org/pipewire/pipewire)

Original Maintainers (usually from Debian):
Utopia Maintenance Team
Jeremy Bicha
[Repo-Source](https://salsa.debian.org/utopia-team/pipewire/-/tree/debian/0.3.25-1)

