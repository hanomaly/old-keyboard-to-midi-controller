# old_keyboard_to_midi_controller

*NOTE:* This code and circuit diagram works for my specific keyboard. Your keyboard will likely have a different keyboard matrix (e.g. different numbers of wires coming from the ribbons). You will have to figure out how to change things around for your keyboard. That said, for the piezos, joystick and midi port this will all be fine for you and this will give you the general idea of how to set things up for your keyboard. Also, there is of course no obligation to use the Arduino Nano like I am, you can use whichever type you have.

**[Video](https://youtu.be/TeOvE9mf0BA)**

Software requirements (if using USB to plug into computer):
- [hairless midi to serial bridge](https://projectgus.github.io/hairless-midiserial/). See Step 5 from [this instructable](https://www.instructables.com/Send-and-Receive-MIDI-with-Arduino/#step5) for more explanation of this.
- If you're using Windows, also [loopMIDI](http://www.tobias-erichsen.de/software/loopmidi.html). You'll need to run both this and hairless to send MIDI data from the arduino to whichever software you want.
- Any software that can take midi input such as a DAW (Reaper, Ableton, etc).

**Circuit diagram**

The diodes I used were 1N4148.
P.S. sorry it's a bit rough.
![Midi controller circuit diagram](https://github.com/hanomaly/old_keyboard_to_midi_controller/blob/master/circuit_diagram.png)
