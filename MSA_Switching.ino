//déclaration de la lib I2C
#include "Wire.h"
//déclaration de l'affichage I2C
#include "LiquidCrystal_I2C.h"
//déclaration de la lib pca9555 de Nico Verduin
//https://github.com/nicoverduin/PCA9555
#include "clsPCA9555.h"
//----- --------------------------------------------------------Adressage matériel ---------------------------------------------
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
  { lcd.setCursor(12, 0);
    lcd.print("Mode MSA");

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
    lcd.setCursor(12, 0);
    lcd.print("Mode VNA");
    Serial.print("mode msa"); //debug
    Serial.println(); //debug
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED7, HIGH); //ED7 = relais en position VNA collent
    delay(100); //100 ms de temps de collage des relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED7, LOW);// replace le niveau logique du relais à zéro
  }

  //---------------------------TRANSMISSION/REFLEXION ---------------------------------
  //input in1, output TRANS=ED0, REFL=ED1,

  { trans_refl = digitalRead(in1); //Rappel : in1 = A0
    if (trans_refl == HIGH)
    { lcd.setCursor(0, 2);
      lcd.print("Reflexion S11/S22   ");

      digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
      ioport.digitalWrite(ED1, HIGH); // = relais en position Reflexion colle
      delay(100); //100 ms de temps de collage du relais
      digitalWrite(out6, LOW); // je coupe le jus et
      ioport.digitalWrite(ED1, LOW);// replace le niveau logique du relais à zéro
    }
    else
    { lcd.setCursor(0, 2);
      lcd.print("Transmission S21/S12");

      digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
      ioport.digitalWrite(ED0, HIGH); // = relais en position Reflexion colle
      delay(100); //100 ms de temps de collage du relais
      digitalWrite(out6, LOW); // je coupe le jus et
      ioport.digitalWrite(ED0, LOW);// replace le niveau logique du relais à zéro
    }
  }
  //---------------------------------FWD-REVERSE S11/S22 ou S12/S21 -----------------
  // Input in2 (A1) Output FWD=ED2, REV=ED3

  {
    fwd_reverse = digitalRead(in2);
    if (fwd_reverse == HIGH)
    { lcd.setCursor(0, 3);
      lcd.print("Forward :-----> S21");

      digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
      ioport.digitalWrite(ED2, HIGH); // = relais en position Reflexion colle
      delay(100); //100 ms de temps de collage du relais
      digitalWrite(out6, LOW); // je coupe le jus et
      ioport.digitalWrite(ED2, LOW);// replace le niveau logique du relais à zéro
    }
    else
    { lcd.setCursor(0, 3);
      lcd.print("Reverse :<----- S12");

      digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
      ioport.digitalWrite(ED3, HIGH); // = relais en position Reflexion colle
      delay(100); //100 ms de temps de collage du relais
      digitalWrite(out6, LOW); // je coupe le jus et
      ioport.digitalWrite(ED3, LOW);// replace le niveau logique du relais à zéro
    }

    //---------------------------------------MODE 0-1, 1-2 et 2-3 GHz ------------------------
    // Input G1 in3, Input G2 In4  (A1) Output G1=ED4, G2=ED5,

  }
  RFG1 = digitalRead(in3); //Rappel : in3 = A2
  RFG2 = digitalRead(in4); //Rappel : in4 = A3

  if (RFG1 == LOW && RFG2 == LOW)
  { lcd.setCursor(0, 1);
    lcd.print("Bande : 0-1 GHz     ");
  }
  //bande 1 GHz confirmée
  else if
  (RFG1 == LOW && RFG2 == HIGH)
  { lcd.setCursor(0, 1);
    lcd.print("Bande : 1-2 GHz     ");
  }
  //bande 2 GHz confirmée

  else if
  (RFG1 == HIGH && RFG2 == LOW)
  { lcd.setCursor(0, 1);
    lcd.print("Bande : 2-3 GHz     ");

  }
  else
  { lcd.setCursor(0, 1);
    lcd.print("Bande : ERREUR      ");
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

    lcd.setCursor(0, 0);
    lcd.print("Att.:");
    lcd.setCursor(6, 0);
    lcd.print(att_disp);
    lcd.setCursor(8, 0);
    lcd.print("dB");
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

    lcd.setCursor(0, 0);
    lcd.print("Att.:");
    lcd.setCursor(6, 0);
    lcd.print(att_disp);
    lcd.setCursor(8, 0);
    lcd.print("dB");
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

    lcd.setCursor(0, 0);
    lcd.print("Att.:");
    lcd.setCursor(6, 0);
    lcd.print(att_disp);
    lcd.setCursor(8, 0);
    lcd.print("dB");
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

    lcd.setCursor(0, 0);
    lcd.print("Att.:");
    lcd.setCursor(6, 0);
    lcd.print(att_disp);
    lcd.setCursor(8, 0);
    lcd.print("dB");
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

    lcd.setCursor(0, 0);
    lcd.print("Att.:");
    lcd.setCursor(6, 0);
    lcd.print(att_disp);
    lcd.setCursor(8, 0);
    lcd.print("dB");
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

    lcd.setCursor(0, 0);
    lcd.print("Att.:");
    lcd.setCursor(6, 0);
    lcd.print(att_disp);
    lcd.setCursor(8, 0);
    lcd.print("dB");
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

    lcd.setCursor(0, 0);
    lcd.print("Att.:");
    lcd.setCursor(6, 0);
    lcd.print(att_disp);
    lcd.setCursor(8, 0);
    lcd.print("dB");
    //Insertion de la valeur d'atténuation
  }


  //-------------------------------Mode 0-1 GHz--------------
  // bit 2 BS0 Band Selection, low order bit
  // bit 3 BS1 Band Selection, high order bit
  // 00=0/1 GHz
  //01=1/2 GHz
  //10=2/3 GHz
  RFG1 = digitalRead(in3); //Rappel : in3 = A2
  RFG2 = digitalRead(in4); //Rappel : in4 = A3
  {

  }
  if (RFG1 == LOW && RFG2 == LOW)
  { lcd.setCursor(0, 1);
    lcd.print("Bande : 0-1 GHz     ");
  }
  //bande 1 GHz confirmée
  else if
  (RFG1 == LOW && RFG2 == HIGH)
  { lcd.setCursor(0, 1);
    lcd.print("Bande : 1-2 GHz     ");
  }
  //bande 2 GHz confirmée

  else if
  (RFG1 == HIGH && RFG2 == LOW)
  { lcd.setCursor(0, 1);
    lcd.print("Bande : 2-3 GHz     ");

  }
  else
  { lcd.setCursor(0, 1);
    lcd.print("Bande : ERREUR      ");
  }

  //------------------------------------Mode FWD  -----------------
  //P4D4
  // bit 4 FR DUT Direction, Forward (0) or Reverse

  fwd_reverse = digitalRead(in2); //Rappel : in2 = A1
  if (fwd_reverse == LOW)
  { lcd.setCursor(0, 3);
    lcd.print("Forward :-----> S21");
  }

  //P4D4 est bas
  else
  { lcd.setCursor(0, 3);
    lcd.print("Reverse :<---- S12");
  }
  //P4D4 est haut

  //--------------Mode Transmission-------------------------------
  //P4D5
  // bit 5 TR VNA Selection, Transmission (0) or Reflection

  {
    trans_refl = digitalRead(in1); //Rappel : in1 = A0
    if (trans_refl == LOW)
      // mode transmission S21 (ou S12)
    { lcd.setCursor(0, 2);
      lcd.print("Transmission S21/S12");
      lcd.print("Reflexion S11/S22   ");
    }
    else
      // mode réflexion S11 (ou S22)
    { lcd.setCursor(0, 2);
      lcd.print("Reflexion S11/S22   ");
    }
  }
}
