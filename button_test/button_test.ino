
int buttonPin = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // make the pushbutton's pin an input
  pinMode(buttonPin, INPUT);
}

void loop() {
  // print button value in lines to be used in plotter
  Serial.println(digitalRead(buttonPin));
  
  delay(100);
}
