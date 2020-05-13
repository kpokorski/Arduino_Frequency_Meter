#include <LiquidCrystal.h>

int Kontrast=145;
LiquidCrystal lcd(11, 10, 5, 4, 3, 2);  



unsigned int time = 0;
unsigned int TimeFromTab;
unsigned int period;//okres
byte index = 0;
float frequency;//częstotliwość
int MaxNachylenie = 0;//nachylenie maksymalne
int CurrNachylenie;//aktyalne nachylenie

byte skip = 0;//zlicza ile razy się nie udało
byte nachylenieTol = 3;//nachylenie - tolerancja
int time_tabTol = 10;//tolerancja czasowa

byte current_data = 0;
byte previous_data = 0;
int time_tab[10];//tablica czasów
int nachylenie[10];//tablica nachyleń

void setup(){
  analogWrite(6,Kontrast);
  lcd.begin(16, 2);
  lcd.setCursor(4,0); 
  lcd.print("Miernik");
  lcd.setCursor(1,1);
  lcd.print("Częstotliwości");

  cli();//dsiable interrupts
  
  //set up continuous sampling of analog pin 0 at 38.5kHz
 
  //Ustawianie taktowanie 32kHz na pinie A0 i mutowanie innych
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); 
  ADMUX |= (1 << ADLAR); 
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); 
  ADCSRA |= (1 << ADATE); 
  ADCSRA |= (1 << ADIE); 
  ADCSRA |= (1 << ADEN); 
  ADCSRA |= (1 << ADSC); 
  
  sei();
}

ISR(ADC_vect) {

  previous_data = current_data;
  current_data = ADCH;//dostanie wartości z A0
  
  if (previous_data < 127 && current_data >=127){//Przejście przez 2,5V
    CurrNachylenie = current_data - previous_data;//obliczanie nachylenia
    if (abs(CurrNachylenie-MaxNachylenie)<nachylenieTol){//porównanie nachylenia
      nachylenie[index] = CurrNachylenie;
      time_tab[index] = time;
      time = 0;
      if (index == 0){
 
        skip = 0;
        index++;
      }
      else if (abs(time_tab[0]-time_tab[index])<time_tabTol  //sprawdzenie czasów i nachylenia 
      && abs(nachylenie[0]-CurrNachylenie)<nachylenieTol){
        
        TimeFromTab = 0;
        for (byte i=0;i<index;i++){
          TimeFromTab+=time_tab[i];  
        }
        period = TimeFromTab;//okres
        time_tab[0] = time_tab[index];
        nachylenie[0] = nachylenie[index];
        index = 1;

        skip = 0;
      }
      else{//przechodzi przez 2,5V ale nie spełnia warunku
        index++;
        if (index > 9){
          reset();
        }
      }
    }
    else if (CurrNachylenie>MaxNachylenie){//nowe max nachylenie
      MaxNachylenie = CurrNachylenie;
      time = 0;
      skip = 0;
      index = 0;
    }
    else{
      skip++;
      if (skip>9){
        reset();
      }
    }
  }
  
  time++;
}

void reset(){
  index = 0;
  skip = 0;
  MaxNachylenie = 0;
}




void loop(){
  
  
  frequency = 38462/float(period);//próbkowanie/okres
  
   lcd.clear();
   lcd.setCursor(1, 0);
   lcd.print("Czestotliwosc:");
   lcd.setCursor(4, 1);
   lcd.print(frequency);
   lcd.print("Hz");
  
  delay(1000);
  
}
