Manual tests for PipeWire
=========================

Introspection (pipewire)
------------------------

Install `pipewire`.

Run `pw-cli dump`.

Video streams
-------------

Install `pipewire` and `pipewire-tests`.

Run `/usr/libexec/installed-tests/pipewire-0.3/examples/video-src`
(or `video-src-alloc`). It will print a node ID, for example 42.

In another terminal, run
`/usr/libexec/installed-tests/pipewire-0.3/examples/video-play 42`
or `/usr/libexec/installed-tests/pipewire-0.3/examples/export-sink 42`,
replacing 42 by the node ID you saw. You should get a window with an
animation.

Similarly, if you uncomment the lines

```
add-spa-lib videotestsrc videotestsrc/libspa-videotestsrc
create-object spa-node-factory factory.name=videotestsrc node.name=videotestsrc Spa:Pod:Object:Param:Props:patternType=1
```

in `/etc/pipewire/pipewire.conf`, then you should see a node in the output
of `pw-cli ls Node` with `node.name = "videotestsrc"`. Pass its node ID
to `video-play` to see a different animation.

V4L2 cameras
------------

If you have a camera, run
`/usr/libexec/installed-tests/pipewire-0.3/examples/local-v4l2` or
`/usr/libexec/installed-tests/pipewire-0.3/examples/spa/local-v4l2`.
You should get a camera stream displayed in a window (but this might
fail if it cannot negotiate a suitable capture resolution).

Audio sink
----------

Use `pw-play` to play a WAV file.

Audio test source
-----------------

If you uncomment the lines

```
add-spa-lib audiotestsrc audiotestsrc/libspa-audiotestsrc
create-object adapter factory.name=audiotestsrc node.name=my-test
```

in `/etc/pipewire/pipewire.conf`, then you should see a node in the output
of `pw-cli ls Node` with `node.name = "my-test"`. You can record from it
with `pw-record --target ${node id here} test.wav` (press Ctrl+C to
stop recording).

ALSA client plugin (pipewire-audio-client-libraries)
----------------------------------------------------

Install `pipewire-audio-client-libraries` and `alsa-utils`.
Make sure PulseAudio is not currently playing audio and is configured
to release the audio device when not in use.

`aplay -L` should list `pipewire`.

`aplay -D pipewire /usr/share/sounds/alsa/Front_Center.wav` should
play a sound.

JACK client library replacement (pipewire-audio-client-libraries)
-----------------------------------------------------------------

Install `pipewire`, `pipewire-audio-client-libraries`, `alsa-utils` and
`sndfile-tools`. Do not have a real JACK server running.
Make sure PulseAudio is not currently playing audio and is configured
to release the audio device when not in use.

`aplay -D jack /usr/share/sounds/alsa/Front_Center.wav` and
`sndfile-jackplay /usr/share/sounds/alsa/Front_Center.wav` should fail
with:

```
jack server is not running or cannot be started
```

`pw-jack aplay -D jack /usr/share/sounds/alsa/Front_Center.wav`
should succeed.

* TODO: Currently it prints

        aplay: set_params:1343: Sample format non available
        Available formats:
        - FLOAT_LE

    and segfaults.

`pw-jack sndfile-jackplay /usr/share/sounds/alsa/Front_Center.wav`
should succeed.

* TODO: Currently it prints
    `Cannot connect output port 0 (alsa_pcm:playback_1)` and plays
    silence for the length of the test file.

PulseAudio client library replacement (pipewire-audio-client-libraries)
-----------------------------------------------------------------------

Install `pipewire`, `pipewire-audio-client-libraries`, `alsa-utils` and
`pulseaudio-utils`. Make sure PulseAudio is not currently playing audio
and is configured to release the audio device when not in use.

Setup: let your pulseaudio service become idle, then
`pkill -STOP pulseaudio`.

`paplay /usr/share/sounds/alsa/Front_Center.wav` should hang (because
PulseAudio has been stopped).

`pw-pulse paplay /usr/share/sounds/alsa/Front_Center.wav` should play
the audio.

Teardown: `pkill -CONT pulseaudio` to return it to normal.

GStreamer elements (gstreamer1.0-pipewire)
------------------------------------------

Install `gstreamer1.0-tools` and `gstreamer1.0-pipewire`.
Make sure PulseAudio is not currently playing audio and is configured
to release the audio device when not in use.

Run: `gst-inspect-1.0 pipewire`. It should list `pipewiresrc`,
`pipewiresink` and `pipewiredeviceprovider`.

Run: `gst-inspect-1.0 pipewiresrc`. It should list details.

Run: `gst-inspect-1.0 pipewiresink`. It should list details.

Run: `gst-launch-1.0 audiotestsrc '!' pipewiresink`. It should beep
until you press Ctrl+C.

Run: `gst-launch-1.0 pipewiresrc '!' videoconvert '!' autovideosink`.
You should get a webcam image (if you have a webcam).
