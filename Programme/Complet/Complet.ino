/*

"Projet poulailler"
Autonome

© 2015 - SCEP - Gauthier M.

*/

#include <SoftwareSerial.h>

//Définition des pins Arduino
#define captLum 0
#define pinSens 12
#define pinVitesse 10
#define finCourseBas 4
#define finCourseHaut 5
#define pinLed 2

//Initiation des variables
int lum;
bool estOuvert;
bool estFermee;
unsigned long tps;

//Reglage des paramètre
#define seuilNuit 200
#define seuilJour 200
#define interval 5000
#define vitesseMax 255
#define vitesseMin 0

SoftwareSerial esp8266(8,9);

//Initialisation
void setup()
{
	Serial.begin(9600);
	esp8266.begin(9600);


	pinMode(captLum, INPUT); //Declaration broche LDR
	pinMode(pinSens, OUTPUT); //Declaration broche sens moteur
	pinMode(pinVitesse, OUTPUT); //Declaration broche vitesse moteur

	pinMode(finCourseBas, INPUT); //Declaration broche fin de course bas
	digitalWrite(finCourseBas, HIGH); //Resistance pull up interne

	pinMode(finCourseHaut, INPUT); //Declaration broche fin de course haut
	digitalWrite(finCourseHaut, HIGH); //Resistance pull up interne

	pinMode(pinLed, OUTPUT); //Declaration broche led
	digitalWrite(pinLed, LOW); //Choix couleur led : LOW  --> Vert != HIGH --> Rouge
	digitalWrite(3, HIGH); //Activation de la led


	estOuvert = true;
	estFermee = false;

}

//Démarage du programme
void loop()
{
	tps = millis();

	while(estOuvert)
	{
		debug();
		manuel();

	    lum = analogRead(captLum);

	    Serial.print(" ");
	    Serial.println(lum);

	    //Des qu'il fait sombre, tps prend une valeur fixe
	    if(lum > seuilNuit){
	        tps = millis();
	        digitalWrite(pinLed, LOW); //Led vert/lumineux
	    } else{
	    	digitalWrite(pinLed, HIGH); //Led rouge/sombre
	    }

	    int difference = millis() - tps;

	    if(difference > interval){
	        descente();
	        Serial.print(" ");
	        Serial.println(estOuvert);
	        estOuvert = false;
	    }
	}

	while(estFermee)
	{
		debug();
		manuel();

	    lum = analogRead(captLum);

	    Serial.print(" ");
	    Serial.println(lum);



	    if(lum > seuilJour){
	        digitalWrite(pinLed, LOW);
	        montee();
	        Serial.print(" ");
	        Serial.println(estFermee);
	        estFermee = false;
	    }
	}
}


//Fonction pour montée
void montee()
{
	debug();

	//Met le moteur en marche (sens ouverture) 
	//tant qu'il n'est pas entierement ouvert
	while(!estOuvert)
	{
	    digitalWrite(pinSens, LOW);
		digitalWrite(pinVitesse, vitesseMax);


		estOuvert = digitalRead(finCourseHaut);
	}
	//Arrete le moteur une fois ouvert
	digitalWrite(pinVitesse, vitesseMin);
}

//Fonction pour déscendre
void descente()
{
	debug();
	
	//Met le moteur en descente (sens fermeture) 
	//tant qu'il n'est pas entierement fermé
	while(!estFermee)
	{
	    digitalWrite(pinSens, HIGH);
	    digitalWrite(pinVitesse, vitesseMax);



	    //Si il n'arrive pas en bas, et qu'il remonte (enroulement dans l'autre sens)
	    //Interval+3000 permet d'attendre 3sec, sinon au debut l'interupteur est enfoncé
	    int difference = millis() - tps;
	    while( (difference > interval+3000) && digitalRead(finCourseHaut) == HIGH ) {
	    	arretUrgence();
	    }

	    estFermee = digitalRead(finCourseBas);
	}
	//Arrete le moteur une fois fermé
	digitalWrite(pinVitesse, vitesseMin);
}

//Control manuel
void manuel()
{
	
	while (Serial.available() || esp8266.available()) 
	{
    char action;
		if (esp8266.available())
		{
			action = esp8266.read();
			etatESP();
		}
		else
		{
			action = Serial.read();
		}



		if(action == 'y'){
		    //Serial.println("Montee jusqu en haut");
		    montee();
		}

		if(action == 'z'){
		    //Serial.println("Descente jusqu en bas");
		    descente();
		}

		if(action == 'm'){
			//Serial.println("Montée");
		    monteeManuel();
		}

		if(action == 'd'){
			//Serial.println("Déscente");
		    descenteManuel();
		}

		if(action == 'a'){
			//Serial.println("Arret");
		    arretManuel();
		}
	}
}

void monteeManuel()
{
    digitalWrite(pinSens, LOW);
	digitalWrite(pinVitesse, vitesseMax);
}


void descenteManuel()
{
	digitalWrite(pinSens, HIGH);
	digitalWrite(pinVitesse, vitesseMax);
}

void arretManuel()
{
	digitalWrite(pinVitesse, vitesseMin);
}

void arretUrgence()
{
	for(;;)
	{
	    arretManuel();
		//Led shield clignote
		digitalWrite(pinLed, LOW);
		delay(250);
		digitalWrite(pinLed, HIGH);
		delay(250);
		Serial.println("Arret d'urgence !!");
	}
}


//Fonction débugage
void debug()
{
	Serial.print(tps);
        Serial.print(" ");
	Serial.print(estOuvert);
        Serial.print(" ");
    Serial.print(digitalRead(finCourseHaut));
		Serial.print(" ");
	Serial.print(estFermee);
		Serial.print(" ");
	Serial.print(digitalRead(finCourseBas));
		Serial.print(" ");
	Serial.print(lum);
		Serial.print(" ");
}


void etatESP()
{
	esp8266.print(String(estOuvert) + ";" + String(estFermee) + ";" + String(finCourseHaut) + ";" + String(finCourseBas) + ";" + String(lum) + ";" + String(tps));

/*
Decodage chaine de caractère

String etat = Serial.read();

// given: String command
int data[MAX_ARGS];
int numArgs = 0;

int beginIdx = 0;
int idx = command.indexOf(",");

String arg;
char charBuffer[16];

while (idx != -1)
{
    arg = command.substring(beginIdx, idx);
    arg.toCharArray(charBuffer, 16);

    // add error handling for atoi:
    data[numArgs++] = atoi(charBuffer);
    beginIdx = idx + 1;
    idx = command.indexOf(",", beginIdx);
}

data[numArgs++] = command.substring(beginIdx);

*/

}