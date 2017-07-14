LCD Blinkenlights
=================

Wait, what?
-----------

Be, inc. shipped the BeBox computer with Blinkenlights, LED bars which would
show the CPU load directly on the machine front panel. How cool is that?

Now, the BeBox isn't a very modern and exciting machine as it used to be. Still,
there should be more blinkenlights.

Here is your opportunity to have your own CPU load meter, using a PicoLCD module.
Each CPU gets a bar in the bargraph, with up to 3 CPUs per LCD line.

What do I need?
---------------

- A machine running Haiku with at least 1 free USB port.
- A PicoLCD module to plug on said port. Both 20x2 and 20x4 modules should work,
  for up to 6 and up to 12 CPUs, respectively.

How do I use it?
----------------

Compile it, run it.

    gcc-x86 example.c devpicolcd.c -lusb-1.0
    ./a.out

Relax and watch the blinkenlights!
