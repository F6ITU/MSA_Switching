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
const char out5 = 10;
const char out4 = 9;
const char out3 = 6;
const char out2 = 5;
const char out1 = 3;

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


//déclaration des variables globales du MSA
byte msa_vna = 0;
boolean trans_refl;
boolean fwd_reverse;
boolean RFG1;
boolean RFG2;
int val_pot; // pour atténuateur
unsigned int Resistance;
byte att;
unsigned int att_disp;
// déclaration des fonctions et de leurs paramètres
byte msa_read();
byte msa_write();
boolean STO_VNA;
boolean STO_VNA_OLD;

byte Trans_read();
byte Trans_write();
byte STO_TRANS;
byte STO_TRANS_OLD;

byte GHZ_read();
byte GHZ_write();
byte STO_GHZ;
byte STO_GHZ_OLD;

byte ATT_read();
byte ATT_write();
byte STO_ATT;
byte STO_ATT_OLD;

// dessin des crochets sur afficheur lcd

byte L1[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B00001,
  B00001,
  B00001,
  B00001
};

byte L2[8] = {
  B00001,
  B00001,
  B00001,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

byte L3[8] = {
  B10000,
  B10000,
  B10000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

byte L4[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B10000,
  B10000,
  B10000,
  B10000
};

//-----------------------------------------------------DEBUT DU SETUP-----------------------------------------------------------------------------------------------
void setup()
{
  //-----------------------------------FOR DEBUGGING ONLY------------------------------------------
  Serial.begin(9600);
  Serial.println("Lancement");
  //-----------------------------------FOR DEBUGGING ONLY------------------------------------------
  // initialisation de l'afficheur
  lcd.init();

  //mise en mémoire des caractères spéciaux pour le lcd
  lcd.createChar(0, L1);
  lcd.createChar(1, L2);
  lcd.createChar(2, L3);
  lcd.createChar(3, L4);
  lcd.clear();

  //MODULE P0WARDUINo gpio locales
  pinMode(out6, OUTPUT); // déclanchement 28V
  pinMode(out5, OUTPUT); // on/off
  pinMode(out4, OUTPUT);// LED S21
  pinMode(out3, OUTPUT);// LED S12
  pinMode(out2, OUTPUT);// LED S11
  pinMode(out1, OUTPUT);// LED S22


  //out6 va ouvrir un circuit 12V de mise en fonctionnement du step-up 28V, (via FET de puissance) et sera mis en service un peu avant
  //les déclanchements, puis désactivé immédiatement après pour éliminer tout risque de bruit de découpage durant la mesure
  //les autres sorties sont de simples indicateurs LED d'activité (params S, marche/arret)

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
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
  digitalWrite(out5, LOW);
  // vérifie que le courant est coupé coté relais 28V... on peut commencer
  digitalWrite(out5, HIGH);
  //Arduino On, Analyzer On, éventuellement après un délai de boot système qu'il faudra déterminer

  // - Initialisation atténuateur HP3321 à 0 dB

  //PCA9555 paramétrage
  // configurer les 17 pins en mode "sortie"
  // a noter que uint8_t est équivalent à byte (ou char)
  for
  (uint8_t i = 0; i < 18; i++)

  {
    ioport.pinMode(i, OUTPUT);
  }
  // initialisation des variables des fonctions
  STO_VNA = 0; //mode MSA
  STO_GHZ = 0; // mode 0-1 GHz
  STO_TRANS = 0; // mode msa par défaut, don't care pour sto_trans qui change dès que VNA
  STO_ATT = 0; // par défaut, ATT est à 0 dB
  STO_VNA_OLD = 0;
  STO_GHZ_OLD = 0;
  STO_TRANS_OLD = 0;
  STO_ATT_OLD = 0;
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

  // todo : faire un beep en cas de changement d'état

  // initialisation des variables
  STO_VNA_OLD = STO_VNA;
  STO_GHZ_OLD = STO_GHZ;
  STO_TRANS_OLD = STO_TRANS;
  STO_ATT_OLD = STO_ATT;


  msa_read();


  if (STO_VNA != STO_VNA_OLD)
  {
    Serial.println("changement");
    Serial.print("vna         = ");
    Serial.println(STO_VNA);
    Serial.print("vna old    =  ");
    Serial.println(STO_VNA_OLD);
    msa_write();
  }

  Trans_read();

  if (STO_TRANS != STO_TRANS_OLD)
  {
    Serial.println("changement");
    Serial.print("trans         = ");
    Serial.println(STO_TRANS);
    Serial.print("trans old    =  ");
    Serial.println(STO_TRANS_OLD);
    Trans_write();
  }
  GHZ_read();

  {
    if (STO_GHZ != STO_GHZ_OLD)
      GHZ_write();
  }

  ATT_read();
  {
    if (STO_ATT != STO_ATT_OLD)
      ATT_write();
  }

  delay(100);
}

// A pondre : mémoriser un "état" de mesure et ne déclancher la proc QUE s'il y a changement

//---------------------------------------------------------------FIN  DU PROGRAMME----------------------------------------------------------------------------------




//---------------------------------les fonctions de lecture---------------------------------------------------------------------------------------------------
//________________________________________MSA/VNA lecture__________________________
byte msa_read()

//---------------------------MSA/VNA---------------------------------
//input in5, output MSA=ED6, VNA=ED7,
{

  msa_vna = analogRead(in5); //Rappel : in5= A6 broche analogique

  if

  (msa_vna == LOW)  // msa
  {
    STO_VNA = 0;
    lcd.setCursor(0, 0);
    lcd.print(" SPECTRUM  ANALYZER ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    digitalWrite(out4, LOW);// extinction des led vna de param S

    return STO_VNA;
  }
  else

  {
    STO_VNA = 1;
    lcd.setCursor(0, 0);
    lcd.print("  VECTOR  ANALYZER  ");
    return STO_VNA;
  }
}

//________________________________________TRANS-REFL/FWD-REV lecture_______________
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

  TRANSMISSION 2<------<      REFLECTION |_______>1
  REVERSE      1<------<      FORWARD    |_______>1
   TRANSMISSION 1<------<      REFLECTION |_______>2
   FORWARD      2<------<      REVERSE     |_______>2

*/

byte Trans_read()
{
  trans_refl = digitalRead(in1);
  fwd_reverse = digitalRead(in2);
  msa_vna = analogRead(in5); //avant de jouer avec les extensions vna, je vérifie que l'on ne soit pas en mode msa

  if
  (msa_vna == LOW)  // msa
  {
    STO_VNA = 0;
    STO_TRANS = 0;
    STO_TRANS_OLD = 0;
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    return STO_TRANS;
  }
  else if
  (trans_refl == LOW && fwd_reverse == LOW)
  {
    STO_TRANS = 1; // vna transmission forward (S21)
    lcd.setCursor(0, 2);
    lcd.print("TRANSMISSION  >---->");
    lcd.setCursor(0, 3);
    lcd.print("FORWARD         S21 ");
    digitalWrite(out1, HIGH);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    digitalWrite(out4, LOW);//allume la led de façade S21
    return STO_TRANS;
  }

  else if
  (trans_refl == LOW && fwd_reverse == HIGH)
  {
    STO_TRANS = 2; //vna transmission reverse (s12)
    lcd.setCursor(0, 2);
    lcd.print("TRANSMISSION  <----<");
    lcd.setCursor(0, 3);
    lcd.print("REVERSE         S12 ");
    digitalWrite(out1, LOW);
    digitalWrite(out2, HIGH);
    digitalWrite(out3, LOW);
    digitalWrite(out4, LOW);//allume la led de façade S12
    return STO_TRANS;
  }

  else if
  (trans_refl == HIGH && fwd_reverse == LOW)
  {
    STO_TRANS = 3; // vna reflexion forward (s11)
    lcd.setCursor(0, 2);
    lcd.print("REFLECTION     <---");
    lcd.setCursor(19, 2);
    lcd.write(byte(0));
    lcd.setCursor(0, 3);
    lcd.print("FORWARD    S11 <---");
    lcd.setCursor(19, 3);
    lcd.write(byte(1));
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, HIGH);
    digitalWrite(out4, LOW); //allume la led de façade S11
    return STO_TRANS;
  }

  else if
  (trans_refl == HIGH && fwd_reverse == HIGH)
  {
    STO_TRANS = 4; // vna reflexion reverse (s22)
    lcd.setCursor(0, 2);
    lcd.print("REFLECTION     ---->");
    lcd.setCursor(15, 2);
    lcd.write(byte(3));
    lcd.setCursor(0, 3);
    lcd.print("REVERSE    S22 ---->");
    lcd.setCursor(15, 3);
    lcd.write(byte(2));
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
    digitalWrite(out3, LOW);
    digitalWrite(out4, HIGH);//allume la led de façade S22
    return STO_TRANS;
  }

}

//________________________________________GHZ lecture_______________

// Input G1 in3, Input G2 In4  (A1) Output G1=ED4, G2=ED5,

// bit 2 BS0 Band Selection, low order bit
// bit 3 BS1 Band Selection, high order bit
// 00=0/1 GHz
//01=1/2 GHz
//10=2/3 GHz

byte GHZ_read()

{
  RFG1 = digitalRead(in3); //Rappel : in3 = A2
  RFG2 = digitalRead(in4); //Rappel : in4 = A3

  if (RFG1 == LOW && RFG2 == LOW)
  {
    STO_GHZ = 0;
    lcd.setCursor(9, 1);
    lcd.print("Band:0-1GHz");
    return STO_GHZ;

  }

  else if
  (RFG1 == LOW && RFG2 == HIGH)
  {
    STO_GHZ = 1;
    lcd.setCursor(9, 1);
    lcd.print("Band:1-2GHz");
    return STO_GHZ;
  }
  //bande 2 GHz confirmée

  else if
  (RFG1 == HIGH && RFG2 == LOW)
  {
    STO_GHZ = 2;
    lcd.setCursor(9, 1);
    lcd.print("Band:2-3GHz");
    // a priori, 1 et 3 GHz utilisent une même route donc la même commutation, seul le soft change.
    return STO_GHZ;
  }
  else
  {
    STO_GHZ = 3;
    lcd.setCursor(9, 1);
    lcd.print("Band:ERROR!");
    return STO_GHZ;
  }
}

//________________________________________ATT lecture_______________

// Input In6 (analogique A7, potar)  Output ED8 à ED13
//
//10dB on=ED8, 40dB on=ED10, 20dB on=ED12,
//10dB off=ED9, 40dB off=ED11, 20dB off=ED13
byte ATT_read()

{
  Resistance = analogRead(in6);
  // sélection des valeurs ed8 à ed13 selon val de "att"

  att = map(Resistance, 0, 1023, 0, 79);
  //----------------atténuateur 0dB
  if
  (att >= 0 && att <= 9)
  {
    STO_ATT = 0;
    att_disp == 00;
    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(STO_ATT);
    lcd.setCursor(5, 1);
    lcd.print("0dB ");
    return STO_ATT;
  }
  //----------------atténuateur 10dB
  else if
  (att >= 10 && att <= 19)
    //
  {
    STO_ATT = 1;
    att_disp == 10;
    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(STO_ATT);
    lcd.setCursor(5, 1);
    lcd.print("0dB ");
    return STO_ATT;
  }
  //----------------atténuateur 20dB
  else if
  (att >= 10 && att <= 29)
  {
    STO_ATT = 2;
    att_disp == 20;
    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(STO_ATT);
    lcd.setCursor(5, 1);
    lcd.print("0dB ");
    return STO_ATT;
  }
  //----------------atténuateur 30dB
  else if
  (att >= 30 && att <= 39)
  {
    STO_ATT = 3;
    att_disp == 30;
    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(STO_ATT);
    lcd.setCursor(5, 1);
    lcd.print("0dB ");
    return STO_ATT;
  }
  //----------------atténuateur 40dB
  else if
  (att >= 40 && att <= 49)
  {
    STO_ATT = 4;
    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(STO_ATT);
    lcd.setCursor(5, 1);
    lcd.print("0dB ");
    return STO_ATT;
  }
  //----------------atténuateur 50dB
  else if
  (att >= 50 && att <= 59)
  {
    STO_ATT = 5;
    att_disp == 50;
    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(STO_ATT);
    lcd.setCursor(5, 1);
    lcd.print("0dB ");
    return STO_ATT;
  }
  //----------------atténuateur 60dB
  else if
  (att >= 60 && att <= 69)
  {
    STO_ATT = 6;
    att_disp == 60;
    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(STO_ATT);
    lcd.setCursor(5, 1);
    lcd.print("0dB ");
    return STO_ATT;
  }
  //----------------atténuateur 70dB
  else if
  (att >= 70 && att <= 79)
  {
    STO_ATT = 7;
    att_disp == 70;
    lcd.setCursor(0, 1);
    lcd.print("ATT:");
    lcd.setCursor(4, 1);
    lcd.print(STO_ATT);
    lcd.setCursor(5, 1);
    lcd.print("0dB ");
    return STO_ATT;
  }
}
//---------------------------------les fonctions d'écriture----------------------------------------------------------------------------------------------------------
//________________________________________MSA/VNA écriture__________________________

byte msa_write()
{
  if
  (msa_vna == LOW)  // donc mode msa

  {
    STO_VNA = 0;
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED6, HIGH); // = relais en position MSA collent
    delay(100); //100 ms de temps de collage des relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED6, LOW);// replace le niveau logique du relais à zéro
    return STO_VNA;
  }
  else

  {
    STO_VNA = 1; // le soft est en configuration VNA
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED7, HIGH); //ED7 = relais en position VNA collent
    delay(100); //100 ms de temps de collage des relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED7, LOW);// replace le niveau logique du relais à zéro
    return STO_VNA;
  }
}
//_____________________________TRANS-REFL/FWD-REV écriture__________________________


byte Trans_write()
{


  trans_refl = digitalRead(in1);
  fwd_reverse = digitalRead(in2);

  if
  (trans_refl == LOW && fwd_reverse == LOW)
  {
    STO_TRANS = 1; // transmission forward (S21)
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED0, HIGH);
    ioport.digitalWrite(ED2, HIGH);
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED0, LOW);
    ioport.digitalWrite(ED2, LOW);
    return STO_TRANS;
  }

  else if
  (trans_refl == LOW && fwd_reverse == HIGH)
  {
    STO_TRANS = 2; //transmission reverse (s12)
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED0, HIGH);
    ioport.digitalWrite(ED3, HIGH);
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED0, LOW);
    ioport.digitalWrite(ED3, LOW);
    return STO_TRANS;
  }

  else if
  (trans_refl == HIGH && fwd_reverse == LOW)
  {
    STO_TRANS = 3; // reflexion forward (s11)
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED1, HIGH);
    ioport.digitalWrite(ED2, HIGH);
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED1, LOW);
    ioport.digitalWrite(ED2, LOW);
    return STO_TRANS;
  }

  else if
  (trans_refl == HIGH && fwd_reverse == HIGH)
  {
    STO_TRANS = 4; // reflexion reverse (s22)
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED1, HIGH);
    ioport.digitalWrite(ED3, HIGH);
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED1, LOW);
    ioport.digitalWrite(ED3, LOW);
    return STO_TRANS;
  }
}

//_____________________________GHz écriture__________________________

byte GHZ_write()
{

  RFG1 = digitalRead(in3); //Rappel : in3 = A2
  RFG2 = digitalRead(in4); //Rappel : in4 = A3

  if (RFG1 == LOW && RFG2 == LOW)
  {
    STO_GHZ = 0;
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED4, HIGH); // = relais
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED4, LOW);// replace le niveau logique du relais à zéro
    return STO_GHZ;
  }

  else if
  (RFG1 == LOW && RFG2 == HIGH)
  {
    STO_GHZ = 1;
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED5, HIGH); // = relais
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED5, LOW);// replace le niveau logique du relais à zéro
    return STO_GHZ;
  }
  //bande 2 GHz confirmée

  else if
  (RFG1 == HIGH && RFG2 == LOW)
  {
    STO_GHZ = 2;
    // a priori, 1 et 3 GHz utilisent une même route donc la même commutation, seul le soft change.
    digitalWrite(out6, HIGH); // j'envoie du 28 V de partout
    ioport.digitalWrite(ED4, HIGH); // = relais
    delay(100); //100 ms de temps de collage du relais
    digitalWrite(out6, LOW); // je coupe le jus et
    ioport.digitalWrite(ED4, LOW);// replace le niveau logique du relais à zéro
    return STO_GHZ;
  }
  else
  {
    STO_GHZ = 3;
    lcd.setCursor(9, 1);
    lcd.print("Band:ERROR!");
    return STO_GHZ;
  }
}

//________________________________________ATT écriture_____________________

byte ATT_write()
{
  Resistance = analogRead(in6);
  // sélection des valeurs ed8 à ed13 selon val de "att"

  att = map(Resistance, 0, 1023, 0, 79);
  //----------------atténuateur 0dB
  if
  (att >= 0 && att <= 9)
  {
    STO_ATT = 0;
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED9, HIGH);
    ioport.digitalWrite(ED11, HIGH);
    ioport.digitalWrite(ED13, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED9, LOW);
    ioport.digitalWrite(ED11, LOW);
    ioport.digitalWrite(ED13, LOW);
    return STO_ATT;
  }
  //----------------atténuateur 10dB
  else if
  (att >= 10 && att <= 19)
    //
  {
    STO_ATT = 1;
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED8, HIGH);
    ioport.digitalWrite(ED11, HIGH);
    ioport.digitalWrite(ED13, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED8, LOW);
    ioport.digitalWrite(ED11, LOW);
    ioport.digitalWrite(ED13, LOW);
    return STO_ATT;
  }
  //----------------atténuateur 20dB
  else if
  (att >= 10 && att <= 29)
  {
    STO_ATT = 2;
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED9, HIGH);
    ioport.digitalWrite(ED11, HIGH);
    ioport.digitalWrite(ED12, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED9, LOW);
    ioport.digitalWrite(ED11, LOW);
    ioport.digitalWrite(ED12, LOW);
    return STO_ATT;
  }
  //----------------atténuateur 30dB
  else if
  (att >= 30 && att <= 39)
  {
    STO_ATT = 3;
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED8, HIGH);
    ioport.digitalWrite(ED11, HIGH);
    ioport.digitalWrite(ED12, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED8, LOW);
    ioport.digitalWrite(ED11, LOW);
    ioport.digitalWrite(ED12, LOW);
    return STO_ATT;
  }
  //----------------atténuateur 40dB
  else if
  (att >= 40 && att <= 49)
  {
    STO_ATT = 4;
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED9, HIGH);
    ioport.digitalWrite(ED10, HIGH);
    ioport.digitalWrite(ED13, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED9, LOW);
    ioport.digitalWrite(ED10, LOW);
    ioport.digitalWrite(ED13, LOW);
    return STO_ATT;
  }
  //----------------atténuateur 50dB
  else if
  (att >= 50 && att <= 59)
  {
    STO_ATT = 5;
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED8, HIGH);
    ioport.digitalWrite(ED10, HIGH);
    ioport.digitalWrite(ED13, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED8, LOW);
    ioport.digitalWrite(ED10, LOW);
    ioport.digitalWrite(ED13, LOW);
    return STO_ATT;
  }
  //----------------atténuateur 60dB
  else if
  (att >= 60 && att <= 69)
  {
    STO_ATT = 6;
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED9, HIGH);
    ioport.digitalWrite(ED10, HIGH);
    ioport.digitalWrite(ED12, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED9, LOW);
    ioport.digitalWrite(ED10, LOW);
    ioport.digitalWrite(ED12, LOW);
    return STO_ATT;
  }
  //----------------atténuateur 70dB
  else if
  (att >= 70 && att <= 79)
  {
    STO_ATT = 7;
    digitalWrite(out6, HIGH);
    ioport.digitalWrite(ED8, HIGH);
    ioport.digitalWrite(ED10, HIGH);
    ioport.digitalWrite(ED12, HIGH);
    delay(100);
    digitalWrite(out6, LOW);
    ioport.digitalWrite(ED8, LOW);
    ioport.digitalWrite(ED10, LOW);
    ioport.digitalWrite(ED12, LOW);
    return STO_ATT;
  }
}
//------------------------------------------------------zone de couper - coller ----------------------------------------------------------------------------------------------
