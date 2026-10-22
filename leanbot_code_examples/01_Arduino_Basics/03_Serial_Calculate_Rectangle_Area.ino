#include <Leanbot.h>
 
int calculateArea(int length, int width) {
  return length * width;  // return the product of length and width as the area
}
 
void setup() {
  Serial.begin(9600);  // Initialize serial communication with a baud rate of 9600
  
  int length, width, area;  // Variables to store length, width, and area of rectangle
  
  Serial.println("Enter the length of the rectangle: ");  // Ask for length
  while (!Serial.available());  // Wait for user input
  length = Serial.parseInt();  // Read the length entered by the user
  
  Serial.println("Enter the width of the rectangle: ");  // Ask for width
  while (!Serial.available());  // Wait for user input
  width = Serial.parseInt();  // Read the width entered by the user
  
  area = calculateArea(length, width);  // Call the calculateArea function to find the area
  
  Serial.print("The area of the rectangle is ");  // Print the message
  Serial.println(area);  // Print the area
}
 
void loop() {
  // Empty loop
}
