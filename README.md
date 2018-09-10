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


Ce code sert à piloter des relais 28V bi-stables (16 bobines) à partir de 8 signaux 0-5V monostables. 

Pour exciter ces relais, un régulateur step-up est mis en fonctions uniquement si l’état des signaux TTL a changé (pour éviter tout bruit d’alimentation à découpage durant une mesure) 
Un écran LCD doit refléter l’état des commutations et ne fournir que les informations nécessaires (ex : en mode MSA, analyseur de spectre, les indications « direct/réfléchi » et « sens de la mesure » n’ont pas de signification et ne doivent pas être affichées
Schématiquement : 
-------------------------------------------------- Dans le Setup() -----------------------------------------------------------------
Des variables sont initialisées par défaut : 
-	Mode MSA
-	Atténuateur 0 dB
-	Bande 0-1 GHz
-	Mode transmission (non affiché puisque non VNA)
-	Sens direct (forward) 
-----------------------------------------------------Dans le « Main » (loop() )-----------------------------------------------------
-	Ces variables sont rebaptisées « variable_old » pour chaque paramètre
-	5 fonctions lecture différentes sont chaînées, chacune chargée de la lecture de l’état effectif des relais/signaux et de l’affichage de l’état sur un LCD.
-	Ces fonctions retournent l’état effectif sous forme de variable
-	Variable est comparée à Variable_old
o	Si Variable !=Variable_old, alors lancer une ou plusieurs des cinq  fonctions écriture chargée de commuter les relais, modifier l’affichage et retourner une variable actualisée.
o	Si Variable = Variable_old, alors passer au test suivant
-	Fin du programme, retour en début de boucle
-------------------------------------------------------Liste des fonctions de lecture() ---------------------------------------------
Lecture Mode MSA()
Lecture Atténuateur()
Lecture Bande()
Lecture Mode trans/refl() 
Lecture Sens direct/rev ()
----------------------- -------------------------------Liste des fonctions Ecriture relais et afficheur() ---------------------------
Ecriture Mode MSA()
Ecriture Atténuateur()
Ecriture Bande()
Ecriture Mode trans/refl() 
Ecriture Sens direct/rev ()



