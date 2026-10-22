#include <Leanbot.h> // include the Leanbot library
 
int addNumbers(int a, int b) { 
  int result = a + b; // store the sum of a and b in result
  return result; // return the result
}
 
void setup() {
  int x = 5, y = 3, z; // declare and initialize variables x, y, and z
  z = addNumbers(x, y); // call the addNumbers function and store the returned result in z
  
  Serial.begin(9600); // initialize the serial communication with a baud rate of 9600
  Serial.print("The sum of "); // print the string "The sum of "
  Serial.print(x); // print the value of x
  Serial.print(" and "); // print the string " and "
  Serial.print(y); // print the value of y
  Serial.print(" is "); // print the string " is "
  Serial.println(z); // print the value of z and add a new line
}
 
void loop() {
  // empty loop, as there is no code to be executed continuously
}
