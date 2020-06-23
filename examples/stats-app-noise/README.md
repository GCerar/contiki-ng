# Description

This app configures radio in continuous transmission test mode.
Default setup is:

* Freq --> channel 16
* Power --> max
* PRBS mode
* payload --> 0xAA

## You can configure

### Duration

In stats-app.c configure *APP_DURATION*.

### Set frequency and power

Go to function rf2xx_CTTM_start() and configure:

* Frequency or channel in step **6** (refer to datasheet section x.x.x - RF Channel selection)
* Transmission power in step **4** (refer to datasheet section x.x.x - TX Output power)

### Select between PRBS or CW mode

Go to function rf2xx_CTTM_start() and configure steps **10**, **11** and **12** ... Refer to datasheet appendix A and picture below.

**PRBS**

![PRBS with data 0xAA](./img/PRBS.png)


**CW**

![CW - continuous wave](./img/CW.png)