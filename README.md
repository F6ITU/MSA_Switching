# MSA_Switching

Caveat emptor !

This sofware is under development.

Working versions will be stored in specific branches. 

Master is an unstable and ever evolving work.

This Arduino sketch is a frontend switching firmware for the Scotty's Modular Analyzer 0-3 GHz spectrum analyzer
(see http://www.scottyspectrumanalyzer.us/msaslim.html ) 

It's a is a very simple sequencial switching program for every MSA builder


The MSA application software controls several switching signals
* Spectrum Analyzer mode / Vector Analyzer one
* Transmission (S21) reflection (S11) mode
* Forward /Reverse mode (DUT inversion, for S12 & S22 measurement)
* 0-1 GHz, 1-2 GHz or 2-3 GHz spectrum's portion

An optional external control drives a 0/70 dB coaxial step attenuator

(HP33321, 33320 or 33322 programmable step attenuator or any 3 section attenuator using latching relays) 

Most of the switching signals are delivered by the Control Board with "always on" or "always off" TTL levels. 

As most coaxial relays used in the UHF realm are 28V devices and need temporary switching pulses,
 a hardware interface must be used to translate these "level" TTL signals into "momentary" 28V/100ms pulses. 
 
 This role is assumed by a "I2C to 16 Open Collector" interface board originally designed for the Alexiares Retrofit project codename AlexI2C, 
 * pcb files at   https://github.com/F6ITU/Alexi2C) 
 * General description at https://wiki.electrolab.fr/Projets:Lab:2017:Peripheriques_Angelia#Alexi2C
 
 The main board supporting the microcontroler is another project called P0warduino 
 
 *pcb files at https://github.com/F6ITU/P0wArduino, 
 * General description at https://wiki.electrolab.fr/Projets:Lab:2016:Ardui_P0wa

An I2C lcd display (4 lines 20 chr) displays the different modes in real time. 






