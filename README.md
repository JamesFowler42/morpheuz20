Description

Morpheuz Sleep Monitor with Smart Alarm
=======================================

Do you ever wonder how well you sleep? The Morpheuz Sleep Monitor uses the Pebble's built in accelerometer to monitor your night's sleep and provide a graph of how much you moved overnight.

Morpheuz is a watch app that monitors your movement and periodically transmits this information to your iPhone or Android phone, where the Javascript component within the Pebble app stores it. The Morpheuz configuration page in the Pebble app shows a graph, allows the setting of a smart alarm and resetting for the next night.

The "smart alarm" defines your earliest and latest wake up times. Morpheuz will do a 30 second watch vibration if you stir during that time period or, should you remain motionless, at the end of that time period.

No data is stored anywhere other than on the phone. Data is sent to the configuration page for graphing but this stores no information.

It's simple to install. There is no iOS or Android app to buy, nothing to sign up for and it's free.

Graph page: http://raw.github.com/JamesFowler42/morpheuz20/master/config-screen.png

Please enjoy and let me know how you get on.

Notes
-------
1. Power consumption on the watch was low during testing, as data sets are collected every 2.5 seconds and then transmitted to the phone every two minutes. 

2. Whilst the app makes every effort to trigger the alarm please consider using another alarm at the end of your "smart alarm" period. 

Version 1.5.0
-------------
* SDK2 BETA4
* Use Pebble accelerometer vibration indication - much simpler than my
original technique
* Add inverse display
* Improved algorithm for determining sleep quality

Version 1.4.0
-------------
Ensured that parseInt is always called with a radix of 10. Conversion in some browsers (iOS) makes 09 = 9 decimal. In others (Android) makes 09 = 0. 

Version 1.2.0
-------------
* Make config buttons work for Android
* Ensures that if the app hasn’t got focus then we don’t consider the
accelerometer data (probably the alarm is going off)
* Ensure it’s own alarm doesn’t produce a movement spike
* Plot the start and end of the smart alarm period, plus now the time
the smart alarm actually went off.
* Visual clues to tie the start and end times against the graph
* Reduced the emphasis of the trend line - which is interesting but not
*that* interesting.

Version 1.1.0
-------------
* Compiled for 2.0 beta3
* Improved graphing adding trend line
* CSV mailto: now vertical
* Minutes on CSV now minutes not months.
* Serious code tidyup on web hosted side

Version 1.0.0
---------------
* Better graphics
* Smart alarm times on watch screen
* Nicer fonts
* Now includes MIT Licence.

Version 0.8.0
-------------
* Copy option - copies as CSV data in a mailto (appears as an email document)

Version 0.7.0
-------------
* Battery meter on screen at all times
* Version shown on watch face and in config window
* Date (ISO format) shown on graph
* Resolution of graph now 1 point per 10 mins not 1 point per 15 mins.

Version 0.6.0
-------------
* Watchapp not watchface.
* Automatic self monitoring to reboot comms and accelerometer if problems occur
* Better graphing - gaps where no data found.
* Removed bug with resetting timer service repeatedly - could have resulted in poor battery life.

Version 0.5.0
-------------
Added Smart Alarm. Set a time period from the earliest time to the latest time you wish to be woken and Morpheuz will wake you during that period with 30 seconds of vibration. It will attempt to alarm when you are restless rather than in a deep sleep. Unless it has already done so it will alarm at the end of the period.

Added a time scale to the sleep activity graph.

Extended the monitoring timeframe to 9 hours.

PLEASE NOTE: This is beta software on a beta OS. Whilst I am confident Morpheuz will not attempt to alarm outside the period specified, there is a possibility that it may not alarm at all. Please ensure that you set a backup alarm. During testing I've had the watch face swap during the night and this disables monitoring and the alarm function.

Also note, the new timeframe is only saved when the "Reset and Go" button is pressed, not when "Done" is pressed.

The vibration can be cancelled by switching to another watch face.

Version 0.1.0
-------------
Original version.


