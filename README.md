dwm_status
==========

Status bar for dwm.

Currently implemented features.

* Laptop battery charge (percent)
* CPU percent usage
* Memory percent usage
* Currently playing spotify song
* date

N.B.
====
Spotify is implemented by looking for a window whose name is "Spotify".
It then holds on to the reference to that window and checks for changes
in the name, which spotify switched to "Artist - Song" when a song plays.
Due to this there is a bug where if spotify is already running *and* playing
a song when dwm_status is run, it will not find the window to pull the name
from. To correct this, pause the song for a second and then play it again.
This will give dwm_status time to find the spotify window and then poll it
for changes.

The memory percent usage seems to be off when compared to utilities such as
htop. I suspect this is due to not taking into account buffers and cache
which are reported in /proc/meminfo. At any rate I'm overestimating memory
usage which is clearly better than underestimating it.
