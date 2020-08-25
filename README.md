# CO2-Measurement-simple

A very simple CO2 measurement device for about than 40 Euros or 50$. 
There are two designs, one very simple. For this you need a calculator to get the CO2 concentration.
A less simple design uses an analogue multiplier to calculate a voltage that has a simple relation to the ppm value.

Please read this document carefully to see what restrictions follow from this design.

This is the simple design:

<img src="../../raw/master/SimpleCO2MeasuremenFrontView.jpg" width="250">
<img src="../../raw/master/SimpleCO2MeasuremenTopView.jpg"  width="250">
<img src="../../raw/master/SimpleCO2MeasuremenBottomView.jpg"  width="250">

The power cable is a cut USB cable.

It uses a MH-Z19B sensor, two integrated operation amplifiers and a small voltmeter.
There is no need for a further microcontroller. 

For potential casings see [Casings](Casing.md).

The very simple design is 

<img src="../../raw/master/VerySimpleCO2Measurement.png">

The simple design is

<img src="../../raw/master/SimpleCO2Measurement.png">

Datas heet of the sensor https://www.winsen-sensor.com/sensors/co2-sensor/mh-z19b.html. The most actual data sheet is linked there. To get older versions you may try a google search with MH-Z19B site:winsen-sensor.com

To calibrate follow the data sheet. Bring the sensor to clean ambient air for at least 20 minutes. Push the button which connects to HD for at least 7 seconds. Be careful not to breath against the device when you do this. 

The auto calibrate option is the default, so the sensor will calibrate itself every 24 hours. This cannot be deactivated with this device as there is no data connection to the sensor. The data sheet writes correctly: "This method is suitable for office and home environment, not suitable for agriculture greenhouse, farm,
refrigerator, etc."

The default range of the MH-Z19B is not specified in the manual. I deduced that it is 5000 ppm from the voltage measured with clean air. I did now a simple calibration of the device to check this assumption. I put the sensor in a bucket with a volume of 5.7 liter and added a small fan. The bucket is closed with a thin foil.
I used a CO2 bottle from a soda club machine to fill a small bag with CO2. The CO2 was drawn into a syringe.
I added 15 ml CO2 to the bucket. After 10 minutes the reading changed from 530 ppm to 2690 ppm. The ambient air pressure was 957 mbar. I expect an increase of 2462 ppm, the measured increase was 2160 ppm. 

Formula: ( 15 ml / 5700 ml ) * ( 957 mbar / 1023 mbar ) = 0.002462 = 2462 ppm

This is 88% of the expected value. This is plausible when I assume that the syringe was not filled with pure CO2. I assume therefore that my assumption is correct that a new MH-Z19B starts with a default range of 5000 ppm.

I correct for the air pressure because the sensor measures the absorption of infrared light by CO2. With lower pressure, less CO2 will be in the chamber. So the sensor will measure less CO2 unless a correction for the air pressure is made. Which is not the case for this device.
