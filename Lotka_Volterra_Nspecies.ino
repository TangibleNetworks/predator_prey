// Lotka_Volterra_2types_with_lib.ino 
// Tangible Networks
//
// Generalised Lotka-Volterra dynamics with two node types (DIP settable):
// 	0. Primary Producer (dip 1 off)
//	1. Predator         (dip 1 on)
//
// Push button acts as an injection of target species into the system  
//
// Equations are numerically integrated using first order Euler method

// NOTES:
// dipi sets the port usage. off-> Output i is prey. on-> Output i is predator.
//
// if no inputs connected:  default to positive growth, basal species, green.
// else
//   if no predators  -> top-predator. colour:red. growth rate.                               3
//   if no prey       -> basal                                                                2
//   else             -> intermediate predator. colour: yellow. growth rate: normal predator. 1

// Should the node indicate an error if two ports of the same type are connected together??

#include <math.h>
#include<TN.h>                      // Requires TN.h, TN.cpp, Keywords.txt in folder <Arduino>/Libraries/TN/

// Model parameters
float dt = 0.02;                    // size of time step for Euler method
float population = 0;               // initial populations are exactly zero
float old_population = 0;
float growth_rates[] = {2, -1, -2};     // these are the possible values for the species intrinsic growth rates (intrinsically prey grow, predators die).
float couplings[] = {0.5,0.5};          // this defines strenght of species interactions (impact one species has on another)

int numberOfConnections = 0;            
int trophicLevel = 0;                  // 0:basal; 1:intermediate; 2:top-predator. This determines the colour and intrinsic growth parameter.
int dips[] = {0, 0, 0};
float inputs[] = {0,0,0};

const float population_max = 20;


TN Tn = TN(-population_max,population_max);    // Create TN object.

void setup () {}                   // setup of the Unit is handled by the constructor of the TN class. Nice.

void loop() {
  
  readDips();  
  readInputs();                    // store the inputs locally as they are used bu the node to determine its trophic level
  
  trophicLevel = whichLevel();
  old_population = population;  

  updatePopulation();
  sendPopulation();
  ledWrite();

  delay(10);                     // length of delay to be tuned..
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////Predator-Prey functions.
////For use in N species simulation. (3 types of species, inferred by unit. Dip switches set Output port function.)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readDips(){

    dips[0] = Tn.dip1();
    dips[1] = Tn.dip2();
    dips[2] = Tn.dip3();
   
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readInputs(){

    inputs[0] = Tn.analogRead(1);
    inputs[1] = Tn.analogRead(2);
    inputs[2] = Tn.analogRead(3);
   
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int whichLevel(){
// returns the trophic level of the species
  
  numberOfConnections = Tn.isConnected(1)+Tn.isConnected(2)+Tn.isConnected(3);
  
  if (dips[0]+dips[1]+dips[2]==3){
    return 2;
  }
  else if (dips[0]+dips[1]+dips[2]==0){
    return 0;
  }
  
  if(numberOfConnections==0){
    return 0;
  }

  //check inputs to determine if node is connected to only prey of predators
  int numberOfPrey = 0;
  int numberOfPredators = 0;
  
  for (int i=0;i<3;i++){
    
    if (inputs[i]>0 && Tn.isConnected(i+1)){
      numberOfPrey++;
    }
    else if (inputs[i]<0 &&Tn.isConnected(i+1)){
      numberOfPredators++;  
    }
  }
  
  if (numberOfPrey==numberOfConnections){
    return 2;  
  }
  else if (numberOfPredators==numberOfConnections){
    return 0;
  }
  else {
    return 1;
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void updatePopulation(){
  
// if switch is not pressed, update population based on Lotka-Volterra dynamics:
  if (!Tn.sw()){

    intrinsicDynamics();  
    interactionDynamics();
  
  }
  // if switch is pressed, increase the population and ignore the LV dynamics:
  else{
    population += 0.02;	
  }

  // bound the population at population_max
  population = min(population,population_max);
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void intrinsicDynamics() {
 
//  population += growth_rates[trophicLevel]*old_population*dt;
  population += growth_rates[trophicLevel]*old_population*dt*(1+10*Tn.pot());
 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void interactionDynamics(){

  for (int i=1; i<=3; i++) {
      if (Tn.isConnected(i)){
        population += dt*couplings[dips[i-1]]*Tn.analogRead(i)*old_population;
      }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendPopulation(){
 
  for (int i=1; i<=3; i++) {
    
    if (!dips[i-1]){
      Tn.analogWrite(i,population);
    }
    else if (dips[i-1]){
      Tn.analogWrite(i,-population);
    }
  }  
  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ledWrite(){
  
  if (trophicLevel==0){    
    Tn.colour(0, round(population*255/population_max), 0);    // basal        -> green
  }
  else if (trophicLevel==2){
    Tn.colour(round(population*255/population_max), 0, 0);    // top predator -> red
  }
  else if (trophicLevel==1){
    Tn.colour(round(population*255/population_max), round(population*255/population_max), 0);    // intermediate predator -> yellow
  }  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
