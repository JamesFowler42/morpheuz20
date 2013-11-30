Description

Morpheuz Sleep Monitor
====================

Do you ever wonder how well you sleep? The Morpheuz Sleep Monitor uses the Pebble's built in accelerometer to monitor your night's sleep and provide a graph of how much you moved overnight.

Morpheuz is a watch face that provides simple date and time. However in the background it monitors movement and periodically transmits this information to your iPhone or Android phone, where the Javascript application stores it. Using the configuration page from the Pebble iOS or Android app brings up a graph of that information and allows it to be reset for the next night.

No data is stored anywhere other than on the phone.

Tests so far indicate it is not profligate with Pebble battery power, using the hardware to capture samples over 2.5 second intervals, processing these without display updates and then summarising and pushing to the phone every minute.

It's simple to install. There is no iOS or Android app to buy, nothing to sign up for and it's free.

Please enjoy and let me know how you get on.

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


