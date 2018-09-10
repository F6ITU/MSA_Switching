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
unsigned int Resistance;
 byte att;
unsigned int att_disp;

int STO_MSA = 1;
int STO_ATT = 1;
int STO_TRANS = 1;
int STO_FWD = 1;
int STO_GHZ = 1;
int STO_MSA_OLD = 1;
int STO_ATT_OLD = 1;
int STO_TRANS_OLD = 1;
int STO_FWD_OLD = 1;
int STO_GHZ_OLD = 1;

// déclaration des fonctions de lecture des entrées
int MSA_R();
int ATT_R();
int GHZ_R();
int TRANS_R();

// déclaration des fonctions d'écriture (commande relais) des sorties
int MSA_WR();
int ATT_WR();
int GHZ_WR();
int TRANS_WR();

// dessin des crochets

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

  // pour une déclaration broche à broche :   ioport.pinMode(ED14, INPUT); (ou OUTPUT)
  // dans le main, l'activation de la broche se fera par
  //ioport.digitalWrite(ED14, HIGH);

  // déclaration des sorties PCA9555 dans le même ordre que celui utilisé pour les entrées :
  //TRANS=ED0,
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
  // initialisation des variables globales d'etat


}
//---------------------------------------------------------------DEBUT DU PROGRAMME------------------------------------------------------------------------------

void loop()
{
  lcd.backlight();

  /*
     En production, ne pas oublier le temps de boot du msa, ou conditionner le démarrage
     de l'arduino à un signal exterieur unique (carte de contrôle ?)
     dans un premier temps, je lance chaque fonction une à une. Chaque fonction retourne une valeur
    boolean STO_MSA;*
    int     STO_ATT;
    int     STO_TRANS;
    int     STO_FWD;
    int     STO_GHZ;*
     Ces fonctions doivent donc être lancées depuis void Setup() puisqu'exécutées une seule fois
     Ensuite, dans le main, je me contente de lire les entrées et vérifier si elle sont égales
     a la valeur stockée. Si oui, on ignore et on boucle, si non, on appelle la fonction.
     Les fonctions ne font que lire et retourner la valeur initiale générée par la carte de contrôle
  */

  // effacement de la zone refl/fwd lorsque sur la position MSA
  // faire un beep en cas de changement d'état

  // rename des états en état_old pour comparaison ultérieure

  STO_MSA = STO_MSA_OLD;
  STO_ATT = STO_ATT_OLD;
  STO_TRANS = STO_TRANS_OLD;
  STO_FWD = STO_FWD_OLD;
  STO_GHZ = STO_GHZ_OLD;


  //---------------------------MSA/VNA---------------------------------
  //input in5, output MSA=ED6, VNA=ED7,
  MSA_R();
  if
  (STO_MSA_OLD != STO_MSA)
  {
    MSA_WR();
  }
  else
  {}

  GHZ_R();
  if
  (STO_GHZ_OLD != STO_GHZ)
  {
    GHZ_WR();
  }
  else
  {}
  ATT_R();
  if
  (STO_ATT_OLD != STO_ATT)
  {
    ATT_WR();
  }
  else
  {}
  TRANS_R();
  if
  (STO_TRANS_OLD != STO_TRANS)
  {
    TRANS_WR();
  }
  else
  {}
}
 // teste l'état MSA/VNA au lancement, stocke la valeur dans la variable boolenne STO_MSA
 msa_vna = analogRead(in5);
  if
  (msa_vna == 0)  // Inter est à Gnd
    STO_MSA = 1;
  { lcd.setCursor(0, 0);
    lcd.print("   SCALAR ANALYZER  ");
  }
  else
    STO_MSA = 2
  {
    lcd.setCursor(0, 0);
    lcd.print("   VECTOR ANALYZER  ");
  }
  Serial.print("variable Sto_ MSA= ");
  Serial.print(STO_MSA);

  // teste l'état TRANS/REFL et FWD/REV, stocke la valeur dans la variable int STO_TRANS
  trans_refl = digitalRead(in1);
  fwd_reverse = digitalRead(in2);

  if
  (trans_refl == LOW && fwd_reverse == LOW)
    STO_TRANS = 1
  { lcd.setCursor(0, 2);
    lcd.print("TRANSMISSION>--S21->");
    lcd.setCursor(0, 3);
    lcd.print("FORWARD     >------>");
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
    STO_TRANS = 2
  { lcd.setCursor(0, 2);
    lcd.print("TRANSMISSION<--S12-<");
    lcd.setCursor(0, 3);
    lcd.print("REVERSE     <------<");

  }

  else if
  (trans_refl == HIGH && fwd_reverse == LOW)
    STO_TRANS = 3
  { lcd.setCursor(0, 2);
    lcd.print("REFLECTION  <--S11-");
    lcd.setCursor(19, 2);
    lcd.write(byte(0));
    lcd.setCursor(0, 3);
    lcd.print("FORWARD     <------");
    lcd.setCursor(19, 3);
    lcd.write(byte(1));
  }

  else if
  (trans_refl == HIGH && fwd_reverse == HIGH)
    STO_TRANS = 4
  { lcd.setCursor(0, 2);
    lcd.print("REFLECTION  --S22-->");
    lcd.setCursor(12, 2);
    lcd.write(byte(3));
    lcd.setCursor(0, 3);
    lcd.print("REVERSE     ------->");
    lcd.setCursor(12, 3);
    lcd.write(byte(2));
  }

  //teste l'état BANDE 0-1, 1-2 et 2-3 GHz
  RFG1 = digitalRead(in3); //Rappel : in3 = A2
  RFG2 = digitalRead(in4); //Rappel : in4 = A3

  if (RFG1 == LOW && RFG2 == LOW)
    STO_GHZ = 1;
  { lcd.setCursor(9, 1);
    lcd.print("Band:0-1GHz");
  }

  else if
  (RFG1 == LOW && RFG2 == HIGH)
    STO_GHZ = 2;
  { lcd.setCursor(9, 1);
    lcd.print("Band:1-2GHz");
  }

  else if
  (RFG1 == HIGH && RFG2 == LOW)
    STO_GHZ = 3;
  { lcd.setCursor(9, 1);
    lcd.print("Band:2-3GHz");

  }
  else
    STO_GHZ = 4;
  { lcd.setCursor(9, 1);
    lcd.print("Band:ERROR!");
  }
  //------------------------------------------------------------------------FIN DU PROGRAMME -----------------------------------------------------------------------

 
