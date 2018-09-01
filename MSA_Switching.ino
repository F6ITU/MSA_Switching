//déclaration de la lib I2C
#include "Wire.h"
//déclaration de l'affichage I2C
#include "LiquidCrystal_I2C.h"
//déclaration de la lib pca9555 de Nico Verduin
//https://github.com/nicoverduin/PCA9555
#include "clsPCA9555.h"
//-------------------------------------------------------------Adressage matériel ---------------------------------------------
//Afficheur
// variables API : Adresse, nbr chr, nbr lignes
LiquidCrystal_I2C lcd(0x3F, 20, 4);
// PCA9555, adresse I2C
PCA9555 ioport(0x20);

//P0warduino gpio, déclaration matérielle
//les sorties de puissance (une seul utilisée pour l'instant)

const char out6 = 11;

// les entrées
const char in1 = A0;
// Transmission/Réflexion
const char in2 = A1;
// Forward/Reverse
const char in3 = A2;
// 1/2 GHz
const char in4 = A3;
// 2/3 GHz
int in5 = A6;
//msa/vna
int in6 = A7;
//attenuateur

// variable de stockage de la valeur lue sur A6 et A7
int att_val;
int msa_vna;

//déclaration des variables globales du MSA
boolean trans_refl;
boolean fwd_reverse;
boolean RFG1;
boolean RFG2;
int val_pot; // pour atténuateur
byte Resistance;
byte att;
byte att_disp;

//-----------------------------------------------------DEBUT DU SETUP-----------------------------------------------------------------------------------------------
void setup()
{
  //-----------------------------------FOR DEBUGGING ONLY------------------------------------------
  Serial.begin(9600);
  Serial.println("Lancement");
  //-----------------------------------FOR DEBUGGING ONLY------------------------------------------
  lcd.init(); // initialisation de l'afficheur
  //MODULE P0WARDUINo gpio locales
  pinMode(out6, OUTPUT);
  //out6 va ouvrir un circuit 12V de mise en fonctionnement du step-up 28V, (via FET de puissance) et sera mis en service un peu avant
  //les déclanchements, puis désactivé immédiatement après pour éliminer tout risque de bruit de découpage durant la mesure

  // -----------------------définition des fonctions broches d'entrées

  pinMode(in1, INPUT);
  //A0 trans_refl

  pinMode(in2, INPUT);
  //A1 fwd_reverse aka S21/S12

  pinMode(in3, INPUT);
  //A2 G1
  pinMode(in4, INPUT);
  //A3 G2

  pinMode(in5, INPUT);
  // A6 msa_vna

  pinMode(in6, INPUT);
  //A7 att

  digitalWrite(out6, LOW);
  // vérifie que le courant est coupé coté relais 28V... on peut commencer

  // - Initialisation atténuateur HP3321 à 0 dB

  //PCA9555 paramétrage
  // configurer les 17 pins en mode "sortie"
  // a noter que uint8_t est équivalent à byte (ou char)
  for
  (uint8_t i = 0; i < 18; i++)

  {
    ioport.pinMode(i, OUTPUT);
  }
}
// pour une déclaration broche à broche :   ioport.pinMode(ED14, INPUT); (ou OUTPUT)
// dans le main, l'activation de la broche se fera par
//ioport.digitalWrite(ED14, HIGH);

// déclaration des sorties PCA9555 dans le même ordre que celui utilisé pour les entrées :
// TRANS=ED0,
//REFL=ED1,
//FWD=ED2, REV=ED3,
//G1=ED4,
//G2=ED5,
//MSA=ED6,
//VNA=ED7,

//---------------------MSA connecteur P4 documentation---------------------------
// bit 0 VS0 Video Filter Address, low order bit ----don't care
// bit 1 VS1 Video Filter Address, high order bit----don't care
// bit 2 BS0 Band Selection, low order bit
// bit 3 BS1 Band Selection, high order bit
// bit 4 FR DUT Direction, Forward (0) or Reverse
// bit 5 TR VNA Selection, Transmission (0) or Reflection
// bit 6 Spare --------------------------------------don't care
// bit 7 PS Pulse Start (a/k/a Latch Pulse), common to all latching relays.----don't care
// Normally high; goes low briefly to initiate relay latching, if any.


//---------------------------------------------------------------DEBUT DU PROGRAMME------------------------------------------------------------------------------

void loop()
{
  lcd.backlight();

  //---------------------------MSA/VNA---------------------------------
  //input in5, output MSA=ED6, VNA=ED7,

  msa_vna = analogRead(in5); //Rappel : in5= A6 broche analogique
  Serial.println(msa_vna);// debug
  if
  (msa_vna == 0)  // Inter est à Gnd
  { lcd.setCursor(0, 0);
    lcd.print("   SCALAR ANALYZER  ");

    Serial.print("mode msa"); //debug
    Serial.println(); //debug
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED6, HIGH); // = relais en position MSA collent
    delay(100); //100 ms de temps de collage des relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED6, LOW);// replace le niveau logique du relais à zéro
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("   VECTOR ANALYZER  ");
    Serial.print("mode msa"); //debug
    Serial.println(); //debug
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED7, HIGH); //ED7 = relais en position VNA collent
    delay(100); //100 ms de temps de collage des relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED7, LOW);// replace le niveau logique du relais à zéro
  }

  //---------------------------TRANSMISSION/REFLEXION ---------------------------------
  //input in1, output 
  //                    TRANS=ED0, 
  //                    REFL=ED1,
  //P4 bit 5 TR VNA Selection, Transmission (0) or Reflection (1)

  // Input in2 (A1) Output 
  //                    FWD=ED2, 
  //                    REV=ED3
  //P4 bit 4 DUT Direction, Forward (0) or Reverse (1)
  /*
    Si TRANS et FWD alors S21
    Si TRANS et REV alors S12
    Si REFL  et FWD alors S11
    Si REFL  et REV alors S22
                              Affichage ligne 2 et 3

    TRANSMISSION<--12--<      REFLECTION |--S11-->       LES DEUX LIGNES BLANCHES SI EN MODE MSA
    REVERSE     <------<      FORWARD    |_______>

  */



  trans_refl = digitalRead(in1);
  fwd_reverse = digitalRead(in2);

  if
  (trans_refl == LOW && fwd_reverse == LOW)
  { lcd.setCursor(0, 2);
    lcd.print("TRANSMISSION>--S21->");
    lcd.setCursor(0, 3);
    lcd.print("FORWARD     >----->");
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED0, HIGH); 
    ioport.digitalWrite(ED2, HIGH); 
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED0, LOW); 
    ioport.digitalWrite(ED2, LOW); 

  }

  else if
  (trans_refl == LOW && fwd_reverse == HIGH)
  { lcd.setCursor(0, 2);
    lcd.print("TRANSMISSION<--S12--<");
    lcd.setCursor(0, 3);
    lcd.print("REVERSE     <------<");
    
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED0, HIGH); 
    ioport.digitalWrite(ED3, HIGH); 
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED0, LOW); 
    ioport.digitalWrite(ED3, LOW);
  }

  else if
  (trans_refl == HIGH && fwd_reverse == LOW)
  { lcd.setCursor(0, 2);
    lcd.print("REFLECTION  <--S11-!");
    lcd.setCursor(0, 3);
    lcd.print("FORWARD     <------!");

    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED1, HIGH); 
    ioport.digitalWrite(ED2, HIGH); 
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED1, LOW); 
    ioport.digitalWrite(ED2, LOW);
  }

  else
    (trans_refl == HIGH && fwd_reverse == HIGH);
  { lcd.setCursor(0, 2);
    lcd.print("REFLECTION  !--S22->");
    lcd.setCursor(0, 3);
    lcd.print("REVERSE     !------>");

    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED1, HIGH); 
    ioport.digitalWrite(ED3, HIGH); 
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED1, LOW); 
    ioport.digitalWrite(ED3, LOW);
  }

  //---------------------------------------BANDE 0-1, 1-2 et 2-3 GHz ------------------------
  // Input G1 in3, Input G2 In4  (A1) Output G1=ED4, G2=ED5,

  // bit 2 BS0 Band Selection, low order bit
  // bit 3 BS1 Band Selection, high order bit
  // 00=0/1 GHz
  //01=1/2 GHz
  //10=2/3 GHz


  RFG1 = digitalRead(in3); //Rappel : in3 = A2
  RFG2 = digitalRead(in4); //Rappel : in4 = A3

  if (RFG1 == LOW && RFG2 == LOW)
  { lcd.setCursor(9, 1);
    lcd.print("Band:0-1GHz");
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED4, HIGH); // = relais en position Inverse (S12/S22) colle
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED4, LOW);// replace le niveau logique du relais à zéro

  }

  else if
  (RFG1 == LOW && RFG2 == HIGH)
  { lcd.setCursor(9, 1);
    lcd.print("Band:1-2GHz");
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED5, HIGH); // = relais en position Inverse (S12/S22) colle
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED5, LOW);// replace le niveau logique du relais à zéro
  }
  //bande 2 GHz confirmée

  else if
  (RFG1 == HIGH && RFG2 == LOW)
  { lcd.setCursor(9, 1);
    lcd.print("Band:2-3GHz");
    // a priori, 1 et 3 GHz utilisent une même route donc la même commutation, seul le soft change.
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED4, HIGH); // = relais en position Inverse (S12/S22) colle
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED4, LOW);// replace le niveau logique du relais à zéro

  }
  else
  { lcd.setCursor(9, 1);
    lcd.print("Band:ERROR!");
  }

  //------------------------------------------------------ATTENUATEUR--------------

  // Input In6 (analogique A7, potar)  Output ED8 à ED13
  //
  //10dB on=ED8, 40dB on=ED10, 20dB on=ED12,
  //10dB off=ED9, 40dB off=ED11, 20dB off=ED13

  Resistance = analogRead(val_pot);
  // sélection des valeurs ed8 à ed13 selon val de "att"

  att = map(Resistance, 0, 1023, 0, 79);
  //----------------atténuateur 0dB
  if
  (att <= 9)
  {
    att_disp = 00;
    Serial.print("atténuateur 0dB");
    Serial.println();
    Serial.print("atténuateur");
    Serial.println(att_disp);
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED9, HIGH);
    ioport.digitalWrite(ED11, HIGH);
    ioport.digitalWrite(ED13, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED9, LOW);
    ioport.digitalWrite(ED11, LOW);
    ioport.digitalWrite(ED13, LOW);

  }
  //----------------atténuateur 10dB
  else if
  (att >= 10 && att <= 19)
    //
  {
    att_disp = 10;
    Serial.print("atténuateur 10dB");
    Serial.println();
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED8, HIGH);
    ioport.digitalWrite(ED11, HIGH);
    ioport.digitalWrite(ED13, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED8, LOW);
    ioport.digitalWrite(ED11, LOW);
    ioport.digitalWrite(ED13, LOW);

    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(att_disp);
    lcd.setCursor(6, 1);
    lcd.print("dB ");
    //Insertion de la valeur d'atténuation

  }
  //----------------atténuateur 20dB
  else if
  (att >= 10 && att <= 99)
  {
    att_disp = 20;
    Serial.print("atténuateur 20dB");
    Serial.println();
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED9, HIGH);
    ioport.digitalWrite(ED11, HIGH);
    ioport.digitalWrite(ED12, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED9, LOW);
    ioport.digitalWrite(ED11, LOW);
    ioport.digitalWrite(ED12, LOW);

    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(att_disp);
    lcd.setCursor(6, 1);
    lcd.print("dB ");
    //Insertion de la valeur d'atténuation
  }
  //----------------atténuateur 30dB
  else if
  (att >= 30 && att <= 39)
  {
    att_disp = 30;
    Serial.print("atténuateur 30dB");
    Serial.println();
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED8, HIGH);
    ioport.digitalWrite(ED11, HIGH);
    ioport.digitalWrite(ED12, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED8, LOW);
    ioport.digitalWrite(ED11, LOW);
    ioport.digitalWrite(ED12, LOW);

    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(att_disp);
    lcd.setCursor(6, 1);
    lcd.print("dB ");
    //Insertion de la valeur d'atténuation
  }
  //----------------atténuateur 40dB
  else if
  (att >= 40 && att <= 49)
  {
    att_disp = 40;
    Serial.print("atténuateur 40dB");
    Serial.println();
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED9, HIGH);
    ioport.digitalWrite(ED10, HIGH);
    ioport.digitalWrite(ED13, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED9, LOW);
    ioport.digitalWrite(ED10, LOW);
    ioport.digitalWrite(ED13, LOW);

    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(att_disp);
    lcd.setCursor(6, 1);
    lcd.print("dB ");
    //Insertion de la valeur d'atténuation
  }
  //----------------atténuateur 50dB
  else if
  (att >= 50 && att <= 59)
  {
    att_disp = 50;
    Serial.print("atténuateur 50dB");
    Serial.println();
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED8, HIGH);
    ioport.digitalWrite(ED10, HIGH);
    ioport.digitalWrite(ED13, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED8, LOW);
    ioport.digitalWrite(ED10, LOW);
    ioport.digitalWrite(ED13, LOW);

    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(att_disp);
    lcd.setCursor(6, 1);
    lcd.print("dB ");
    //Insertion de la valeur d'atténuation
  }
  //----------------atténuateur 60dB
  else if
  (att >= 60 && att <= 69)
  {
    att_disp = 60;
    Serial.print("atténuateur 60dB");
    Serial.println();
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED9, HIGH);
    ioport.digitalWrite(ED10, HIGH);
    ioport.digitalWrite(ED12, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED9, LOW);
    ioport.digitalWrite(ED10, LOW);
    ioport.digitalWrite(ED12, LOW);

    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(att_disp);
    lcd.setCursor(6, 1);
    lcd.print("dB ");
    //Insertion de la valeur d'atténuation
  }
  //----------------atténuateur 70dB
  else
    (att >= 70 && att <= 79);
  {
    att_disp = 70;
    Serial.print("atténuateur 70dB");
    Serial.println();
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED8, HIGH);
    ioport.digitalWrite(ED10, HIGH);
    ioport.digitalWrite(ED12, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED8, LOW);
    ioport.digitalWrite(ED10, LOW);
    ioport.digitalWrite(ED12, LOW);

    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(att_disp);
    lcd.setCursor(6, 1);
    lcd.print("dB ");
    //Insertion de la valeur d'atténuation
  }
}

/* description de l'affichage
    VECTOR ANALYZER   /// SCALAR ANALYZER
    ATT:XXdB Band:0-1GHz
    TRANSMISSION >---21-->      REFLECTION <--S11--|        LES DEUX LIGNES BLANCHES SI EN MODE MSA
    FWD                         FWD        <-------|

    TRANSMISSION <---12--<      REFLECTION |--S22-->       LES DEUX LIGNES BLANCHES SI EN MODE MSA
    REVERSE                     REVERSE    |_______>
*/
