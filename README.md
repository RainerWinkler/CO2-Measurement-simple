# CO2-Measurement-simple

This repository contains simple CO2 measurement devices for about than 40 Euros or 50$. 
There are multiple designs, the most simple needs no soldering and can be build with minor technical understanding.

The intention of this repository is to provide informations how to build a CO2 monitor which:

* Uses easily available, cheap components - So that mass production is feasible
* Restricts to features which are really needed for a CO2 monitor during the COVID-19 epidemic - Reduce complexity and reduce problems with wrong usage
* Has MIT license - Can easily be copied and distributed by everyone, even commercial


Please read this document carefully to see what restrictions follow from this design.

# With Display and Warning LED

This design has warning lights, a display and logic to enable it to be powered by a power bank which auto switch off logic.

<img src="../../raw/master/CO2Monitor3/Example529ppm.jpg" width="250">

<img src="../../raw/master/CO2Monitor3/Example861ppm.jpg" width="250">

<img src="../../raw/master/CO2Monitor3/Example1741ppm.jpg" width="250">

See [CO2 Monitor Design 3](CO2Monitor3/CO2Monitor3.md) for details.

# Very simple design

This is the very simple design. It needs a conversion table to get the ppm value from the reading. No soldering needed, the connection is made with screw terminals:

<img src="../../raw/master/CO2Monitor2/VerySimpleCO2MeasurementFinishedTopWithScala.JPG" width="250">

Parts needed:

<img src="../../raw/master/CO2Monitor2/PartsVerySimpleCO2Measurement.JPG" width="250">

Tools needed:

<img src="../../raw/master/CO2Monitor2/ToolsVerySimpleCO2Measurement.JPG" width="250">

See [CO2 Monitor Design 1](CO2Monitor2/CO2Monitor2.md) for details.

# Simple design

This is the simple design:

<img src="../../raw/master/CO2Monitor1/SimpleCO2MeasuremenFrontView.jpg" width="250">

See [CO2 Monitor Design 1](CO2Monitor1/CO2Monitor1.md) for details.

# Further informations

Datas heet of the sensor https://www.winsen-sensor.com/sensors/co2-sensor/mh-z19b.html. The most actual data sheet is linked there. To get older versions you may try a google search with MH-Z19B site:winsen-sensor.com

To calibrate follow the data sheet. Bring the sensor to clean ambient air for at least 20 minutes. Push the button which connects to HD for at least 7 seconds. Be careful not to breath against the device when you do this. 

The auto calibrate option is the default, so the sensor will calibrate itself every 24 hours. This cannot be deactivated with this device as there is no data connection to the sensor. The data sheet writes correctly: "This method is suitable for office and home environment, not suitable for agriculture greenhouse, farm,
refrigerator, etc."

The default range of the MH-Z19B is not specified in the manual. I deduced that it is 5000 ppm from the voltage measured with clean air. I did now a simple calibration of the device to check this assumption. I put the sensor in a bucket with a volume of 5.7 liter and added a small fan. The bucket is closed with a thin foil.
I used a CO2 bottle from a soda club machine to fill a small bag with CO2. The CO2 was drawn into a syringe.
I added 15 ml CO2 to the bucket. After 10 minutes the reading changed from 530 ppm to 2690 ppm. The ambient air pressure was 957 mbar. I expect an increase of 2462 ppm, the measured increase was 2160 ppm. 

Formula: ( 15 ml / 5700 ml ) * ( 957 mbar / 1023 mbar ) = 0.002462 = 2462 ppm

This is 88% of the expected value. This is plausible when I assume that the syringe was not filled with pure CO2. I assume therefore that my assumption is correct that a new MH-Z19B starts with a default range of 5000 ppm.

I correct for the air pressure because the sensor measures the absorption of infrared light by CO2. With lower pressure, less CO2 will be in the chamber. So the sensor will measure less CO2 unless a correction for the air pressure is made. I guess this is not by this device.
