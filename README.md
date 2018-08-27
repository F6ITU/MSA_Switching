# MSA_Switching

This sofware is under development.

Working versions will be stored in specific branches. 

Master is an unstable and ever evolving work.

Frontend switching firmware for the Scotty's Modular Analyzer (Arduino sketch)

This is a very simple sequencial switching program for every MSA builder

MSA is a homebrewed 0-3 GHz spectrum analyzer

http://www.scottyspectrumanalyzer.us/msaslim.html

The MSA application software controls several switching signals to pass 
* From the Spectrum Analyzer mode to the Vector Analyzer one
* From the Transmission (S21) to the reflexion (S11) mode
* From the Forward to the Reverse mode (DUT inversion, for S12 & S22 measurement)
* From the 0-1 GHz to the 1-2 GHz or 2-3 GHz spectrum analysis 

Switching signals are delivered by the Control Board with "always on" or "always off" TTL levels. 

As most coaxial relays are using 28V and need temporary switching pulses (latching relays cannot 
widstand permanent switching voltage), a hardware interface must be used to translate these "level" TTL signals 
into "momentary" 28V/100ms pulses.

This interface is based on an arduino board -more precisely a P0warduino shield, 
and a 16 I/O I2C driven open collector driver board, AlexI2C

Both boards are designed under Kicad and located on this github repository at 
  https://github.com/F6ITU/P0wArduino
  and 
  https://github.com/F6ITU/Alexi2C
  
General description could respectively be found at 
  https://wiki.electrolab.fr/Projets:Lab:2016:Ardui_P0wa
and 
https://wiki.electrolab.fr/Projets:Lab:2017:Peripheriques_Angelia#Alexi2C

An I2C lcd display (4 lines 20 chr) displays the different modes in real time. 

One input and 3 output, which are not controled by the MSA client software, are dedicated to a HP33321, 33320 or 33322 programmable step attenuator
(or any 3 section attenuator using latching relays) 




