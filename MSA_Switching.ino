//déclaration de la lib I2C
#include "Wire.h"
// test de syntaxe par rapport à #include <ma_lib.h>
//déclaration de l'affichage I2C
#include "LiquidCrystal_I2C.h"
//déclaration de la lib pca9555 de Nico Verduin
//https://github.com/nicoverduin/PCA9555
#include "clsPCA9555.h"
//----- Adressage matériel -----
//Afficheur
// variables API : Adresse, nbr chr, nbr lignes
LiquidCrystal_I2C lcd(0x3F, 20, 4);
// PCA9555, adresse I2C
PCA9555 ioport(0x20);

//P0warduino gpio, déclaration matérielle
//les sorties de puissance (une seul utilisée pour l'instant)
// attention : ce sont des sorties pwm. Comme deux des entrées ne sont pas utilisée (une sur PCA9555,
//l'autre sur P0warduino), elle pourraient être employées en mesure de température et ainsi piloter
// un (ou deux) ventilateur(s) commandé(s) en PWM via un simple boucle PID... et roule ma poule.

const char out6 = 11;

const char out5 = 10;

const char out4 = 9;

const char out3 = 6;

const char out2 = 5;

const char out1 = 3;

// les entrées
const char in1 = A0;

const char in2 = A1;

const char in3 = A2;

const char in4 = A3;

int in5 = A6;

int in6 = A7;

// variable de stockage de la valeur lue sur A6 et A7
int att_val = 0;
int msa_vna = 0;

//déclaration des variables globales du MSA (ne doivent pas être situées dans les fonction setup() ou loop()
boolean trans_refl;
boolean fwd_reverse;
boolean RFG1;
boolean RFG2;
//-----------------------------------------------------DEBUT DU SETUP-----------------------------------------------------------------------------------------------
void setup()
{
//-----------------------------------FOR DEBUGGING ONLY------------------------------------------
Serial.begin(9600); 
Serial.println("My Sketch has started");
//-----------------------------------FOR DEBUGGING ONLY------------------------------------------

  lcd.init(); // initialisation de l'afficheur

  //MODULE P0WARDUINo gpio locales
  pinMode(out6, OUTPUT);
  //out6 va ouvrir un circuit 12V de mise en fonctionnement du step-up 28V, (via FET de puissance) et sera mis en service un peu avant
  //les déclanchements, puis désactivé immédiatement après pour éliminer tout risque de bruit de découpage durant la mesure

  pinMode(out5, OUTPUT);
  // A priori ne sert pas

  pinMode(out4, OUTPUT);
  // A priori ne sert pas

  pinMode(out3, OUTPUT);
  // A priori ne sert pas

  pinMode(out2, OUTPUT);
  // A priori ne sert pas

  pinMode(out1, OUTPUT);
  // A priori ne sert pas

  // -----------------------définition des fonctions broches d'entrées

  pinMode(in1, INPUT);//A0
  //trans_refl

  pinMode(in2, INPUT);//A1
  //fwd_reverse aka S21/S12

  pinMode(in3, INPUT);//A2
  //G1
  pinMode(in4, INPUT);//A3
  //G2

  pinMode(in5, INPUT);//A6
  //att

  pinMode(in6, INPUT);//A7
  //msa_vna

  digitalWrite(out6, LOW);
  // vérifie que le courant est coupé coté relais 28V... on peut commencer

  // - Initialisation atténuateur HP3321 à 0 dB

  //PCA9555 paramétrage
  // configurer les 17 pins en mode "sortie"
  // a noter que uint8_t est équivalent à byte (ou char)
  for (uint8_t i = 0; i < 18; i++) {
    ioport.pinMode(i, OUTPUT);
  }

  // pour une déclaration broche à broche :   ioport.pinMode(ED14, INPUT); (ou OUTPUT)
  // dans le main, l'activation de la broche se fera par
  //ioport.digitalWrite(ED14, HIGH);
  // déclaration des sorties PCA9555 dans le même ordre que celui utilisé pour les entrées :
  // TRANS=ED0, REFL=ED1, FWD=ED2, REV=ED3, G1=ED4,G2=ED5,MSA=ED6,VNA=ED7,10dB on=ED8,10dB off=ED9,40dB on=ED10,40dB off=ED11,20dB on=ED12,20dB off=ED13

  //-----------atténuateur à 0 dB au démarrage-----------------------
  // envoie 28 volts
  digitalWrite(out6, HIGH);

  ioport.digitalWrite(ED9, HIGH);
  delay(100);
  ioport.digitalWrite(ED9, LOW);
  ioport.digitalWrite(ED10, HIGH);
  delay(100);
  ioport.digitalWrite(ED10, LOW);
  ioport.digitalWrite(ED13, HIGH);
  delay(100);
  ioport.digitalWrite(ED13, LOW);
  //coupure 28V
  digitalWrite(out6, LOW);
  // envoi de l'indication d'atténuation à l'afficheur
  lcd.backlight();
  // Envoi du message
  //lcd.cursor_on();
  //lcd.blink_on();
  lcd.setCursor(0, 0);
  lcd.print("Att.:");
  lcd.setCursor(8, 0);
  lcd.print("dB");
  //Insertion de la valeur d'atténuation
  lcd.setCursor(0, 6);
  lcd.print("00");

  //--------------Vérification des autres états ---------------------

  //---------------------P4 documentation---------------------------
  // bit 0 VS0 Video Filter Address, low order bit ----don't care
  // bit 1 VS1 Video Filter Address, high order bit----don't care
  // bit 2 BS0 Band Selection, low order bit
  // bit 3 BS1 Band Selection, high order bit
  // bit 4 FR DUT Direction, Forward (0) or Reverse
  // bit 5 TR VNA Selection, Transmission (0) or Reflection
  // bit 6 Spare --------------------------------------don't care
  // bit 7 PS Pulse Start (a/k/a Latch Pulse), common to all latching relays.----don't care
  // Normally high; goes low briefly to initiate relay latching, if any.

  //--------------Mode MSA par défaut, vérification -----------------
  {
    msa_vna = digitalRead(in6); //Rappel : in6 = A7, inter en face avant
    if (msa_vna == LOW)
    { lcd.setCursor(12, 0);
      lcd.print("Mode MSA");
    }
    else
    { lcd.setCursor(12, 0);
      lcd.print("Mode VNA");
    }
  }
  //--------------Mode 0-1 GHz par défaut, vérification -------------
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

  //--------------Mode FWD par défaut, vérification -----------------
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

  //--------------Mode Transmission S21 par défaut, vérification ----
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
//---------------------------------------------------------------DEBUT DU PROGRAMME------------------------------------------------------------------------------

// Le programme teste l'état des entrées
void loop()
{
  //--------------Mode MSA/VNA -----------------  
  {
    msa_vna = analogRead(in6); //Rappel : in6 = A7 broche analogique
//-----------------------------------FOR DEBUGGING ONLY------------------------------------------    
    Serial.println(msa_vna);
//-----------------------------------FOR DEBUGGING ONLY------------------------------------------    
    if (msa_vna == 0)
    { lcd.setCursor(12, 0);
      //  lcd.print("Mode MSA");
      lcd.print("Mode MSA");
    }
    //MSA(); //Inter est à Gnd
    else
    {
      lcd.setCursor(12, 0);
      lcd.print("Mode VNA");
    }
    //VNA(); //Inter est à Vcc
  }
    //--------------Mode TRANSMISSION-REFLEXION S21-S11 ----
  {
    trans_refl = digitalRead(in1); //Rappel : in1 = A0
    if (trans_refl == HIGH)
    { lcd.setCursor(0, 2);
      lcd.print("Reflexion S11/S22   ");
    }
    //reflexion(); //P4D5 est haut
    else
    { lcd.setCursor(0, 2);
      lcd.print("Transmission S21/S12");
    }
    //transmission(); //P4D5 est bas
  }
  //--------------Mode FWD-REVERSES11/S22 ou S12/S21 -----------------
  //---- lire la valeur de trans_refl. 
  //Si FWD et trans, alors==>S21
  //Si FWD et refl alors ==>S11
  //Si REV et trans alors ==>S12
  //Si REV et refl alors ==>S22
  // tout simplement
  {
    fwd_reverse = digitalRead(in2); //Rappel : in2 = A1
    if (fwd_reverse == HIGH)
    { lcd.setCursor(0, 3);
      lcd.print("Forward :-----> S21");
    }
    //REV(); //P4D4 est haut
    else
    { lcd.setCursor(0, 3);
      lcd.print("Reverse :<----- S12");
    }
    // FWD(); //P4D4 est bas
 
   //--------------Mode 0-1, 1é et 2-3 GHz -------------
  }
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
  //--------------Mode Att 0-70dB -------------
  

  //- ajout de la commande de l'atténuateur (bouton ou potard ? )
  // appel de la fonction 0 db à 70 dB ?


  // reste à créer les fonctions ci-dessus qui
  // - vérifieront l'état de la variable attachée à la fonction.
  // - Si identique, sortie, sinon :
  // - enclancheront le step-up 28V pour exciter les relais
  // - utiliseront la fonction milli() pour spécifier les temps de commutation des relais
  // - modifieront l'affichage LCD en fonction de la valeur de l'état des entrées
  // - commuteront la sortie du solénoïde du relais concerné durant 100ms
  // - couperont l'arrivée du courant sur le step-up
  // -


}
