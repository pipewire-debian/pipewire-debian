-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: pipewire
Binary: libpipewire-0.3-0, libpipewire-0.3-dev, libpipewire-0.3-modules, libspa-0.2-dev, libspa-0.2-modules, pipewire-doc, pipewire-locales, pipewire, pipewire-bin, pipewire-audio-client-libraries, pipewire-tests, gstreamer1.0-pipewire, libspa-0.2-bluetooth, libspa-0.2-jack
Architecture: linux-any all
Version: 0.3.26-1
Maintainer: Sourav Das forked from Utopia Maintenance Team <souravdas142@gmail.com>
Uploaders: Sourav Das <souravdas142@gmail.com>
Homepage: https://pipewire.org/
Standards-Version: 4.5.1
Vcs-Browser: https://gitlab.freedesktop.org/pipewire/pipewire
Vcs-Git: https://gitlab.freedesktop.org/pipewire/pipewire.git
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, gnome-desktop-testing, gstreamer1.0-tools, pkg-config
Build-Depends: debhelper-compat (= 12), doxygen <!nodoc>, graphviz <!nodoc>, libasound2-dev, libbluetooth-dev, libdbus-1-dev, libglib2.0-dev (>= 2.32.0), libgstreamer-plugins-base1.0-dev, libgstreamer1.0-dev, libjack-jackd2-dev (>= 1.9.10), libldacbt-abr-dev [!s390x !hppa !m68k !powerpc !ppc64 !sparc64], libldacbt-enc-dev [!s390x !hppa !m68k !powerpc !ppc64 !sparc64], libncurses-dev, libopenaptx-dev (>= 0.2.0-5~), libpulse-dev (>= 11.1), libsbc-dev, libsdl2-dev, libsndfile1-dev (>= 1.0.20), libsystemd-dev [linux-any], libudev-dev [linux-any], libv4l-dev, meson (>= 0.50.0), pkg-config (>= 0.22), systemd [linux-any], xmltoman
Package-List:
 gstreamer1.0-pipewire deb libs optional arch=linux-any
 libpipewire-0.3-0 deb libs optional arch=linux-any
 libpipewire-0.3-dev deb libdevel optional arch=linux-any
 libpipewire-0.3-modules deb libs optional arch=linux-any
 libspa-0.2-bluetooth deb libs optional arch=linux-any
 libspa-0.2-dev deb libdevel optional arch=linux-any
 libspa-0.2-jack deb libs optional arch=linux-any
 libspa-0.2-modules deb libs optional arch=linux-any
 pipewire deb video optional arch=linux-any
 pipewire-audio-client-libraries deb libs optional arch=linux-any
 pipewire-bin deb video optional arch=linux-any
 pipewire-doc deb doc optional arch=all profile=!nodoc
 pipewire-locales deb localization optional arch=all
 pipewire-tests deb misc optional arch=linux-any
Checksums-Sha1:
 5c2ba9dd3bfb2fb1a41a3aadeb1be86cc26a00d8 1447673 pipewire_0.3.26.orig.tar.gz
 74abc1d03c8b38f6ed91ff1d05854cb271b8ae65 15356 pipewire_0.3.26-1.debian.tar.xz
Checksums-Sha256:
 05cc9d25de45290c025da5da1b94fc705bddacd93cf3690d0b2988c1ac501ee1 1447673 pipewire_0.3.26.orig.tar.gz
 4b2727e055208c3a4066d5d3f4f62df2ea819a7c3abb402c4cbdeabc5168d92f 15356 pipewire_0.3.26-1.debian.tar.xz
Files:
 6c35534bc647085ccbe608daf190a39c 1447673 pipewire_0.3.26.orig.tar.gz
 690077df8057d8c9d9fdcc9a23121561 15356 pipewire_0.3.26-1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQIzBAEBCgAdFiEEaLc9vKmhQwt+0iydY8WARVl9EscFAmCCue0ACgkQY8WARVl9
EscLRxAAjymitrs32eXc46SF+J/f3mVO4sFsJ8jNI7tXXwmHxo6WuSuedgsFXdEe
c5PPTk7UmL4VT/ZmsviD792KBPdsWdxyuaUCUi1E1AhpR2WNW0VvLLgLJ+sjx1mv
Je0lUJ0xcOIWPfNd1AAYaAcXCkM15UE3k4aj3xgE/emfKkRGD+pORgHzKeoxYPJU
f6wlY3SSU7l/RFzs1RP07+tbujf7k0itCZRRbfKRdgzf06ojoaI+hqn+Q6b570ZB
kgFKB5cL+nlm3TUvzi6yC4i9SqaLNh0L5TjpaDRIcu34xTFVFB8wnKUfVWh8Dn1h
is58VblTvg3tYtE9yRIPkl1oNXUWfedXIE55PV8bAa8tGYf2p6ElkHtuNlHDA8++
nPcTi4d/HexrY3pLXZH7/EzfuAmLbGeR/qv/MckmJCH4E8VamW6Q4lrm9IzV4Sos
nmLcdBRhNGazf2RpjUnzkz0n1jc2H0YHdAB1vBxs+DNr/5onDdbAK7G7Ipzlxntd
d8Yq30/bCURtIbhUPOpFZhQBi9CDgFMi1jyaDk9jK3b9qIXs+hFchC4UrFmxnVSM
hle7ocIles0b0j0BrruxES8Bvc30CLGcGvJgA/z69JJ/w7BAkXhJn5eHAocO3mr0
Y6Fv4QExIPziZFBOLfidf2bo2SS2j08UkUocw9eoCYkQLV57m/U=
=V8Uc
-----END PGP SIGNATURE-----
