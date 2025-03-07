#include "pitches.h"


// Pin definitions for the components
#define BUZZER_PIN 26 // Buzzer pin for playing the melody
#define LIGHT_SENSOR_PIN 34 // Light sensor analog pin for adjusting tone based on light level
#define LED_PIN 13 // LED pin for indicator light (shows when music is playing)
#define TRIGGER_PIN 16 // Ultrasonic trigger pin for measuring distance
#define ECHO_PIN 17 // Ultrasonic echo pin for receiving reflected sound
#define MAX_DISTANCE 100 // Maximum distance for the ultrasonic sensor (in cm)


// Variables for motion detection
#define SAMPLE_SIZE 2 // Number of samples for calculating the average distance
float distances[SAMPLE_SIZE]; // Array to store the most recent distance measurements
int currentIndex = 0; // Index for keeping track of where to store the next distance
int motionCount = 0; // Counter for motion detection events
#define MOTION_THRESHOLD 4 // Threshold value for detecting significant movement in cm
#define MAX_MOTION_COUNT 1 // Number of motion events required to stop the music


// Variables for light sensor
#define LIGHT_THRESHOLD 2000 // Threshold for light level to adjust tone (higher = higher tone)

// Harry Potter melody data (note frequencies and durations)
int melody[] = {
  NOTE_D4, NOTE_G4, NOTE_AS4, NOTE_A4,
  NOTE_G4, NOTE_D5, NOTE_C5, NOTE_A4,
  NOTE_G4, NOTE_AS4, NOTE_A4, NOTE_F4,
  NOTE_GS4, NOTE_D4, NOTE_D4, NOTE_G4,
  NOTE_AS4, NOTE_A4, NOTE_G4, NOTE_D5
};

int durations[] = {
  4, 4, 8, 4,
  2, 4, 2, 2,
  4, 8, 4, 2,
  4, 1, 4, 4,
  8, 4, 2, 4
};
void setup() {
  
  // Set pin modes for inputs and outputs
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output to play music
  pinMode(LIGHT_SENSOR_PIN, INPUT); // Set light sensor pin as input to read light levels
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output to show music status
  pinMode(TRIGGER_PIN, OUTPUT); // Set ultrasonic sensor trigger pin as output
  pinMode(ECHO_PIN, INPUT); // Set ultrasonic sensor echo pin as input
  
  // Start serial communication for debugging
  Serial.begin(115200);
  
  // Initialize the distances array to zero
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    distances[i] = 0.0;
  }
}

void loop() {
  static unsigned long startTime = millis(); // Store the start time to begin music after 10 seconds
  
  // Start music and turn on LED after 10 seconds
  if (millis() - startTime >= 10000) {
    playMusic(); // Play the melody
    digitalWrite(LED_PIN, HIGH); // Turn on the LED to indicate music is playing
    
    // Read the light level and adjust tone (gain)
    int lightLevel = analogRead(LIGHT_SENSOR_PIN);
    Serial.print("Light Level: ");
    Serial.println(lightLevel); // Print the light level for debugging
    
    // Adjust tone based on light level
    float gain = (lightLevel > LIGHT_THRESHOLD) ? 1.5 : 1.0;
    
    // Measure the distance using the ultrasonic sensor
    int tempDistance = ultrasonic_measure(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
    distances[currentIndex] = tempDistance; // Store the new distance measurement
    currentIndex = (currentIndex + 1) % SAMPLE_SIZE; // Update the index in a circular manner
    
    // Calculate the average distance of the last few readings
    float averageDistance = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) {
      averageDistance += distances[i];
    }
    averageDistance /= SAMPLE_SIZE; // Compute the average distance
    Serial.print("Average Distance: ");
    Serial.println(averageDistance); // Print the average distance for debugging
    
    // Detect motion if the change in distance exceeds the threshold
    if (abs(tempDistance - averageDistance) > MOTION_THRESHOLD) {
      motionCount++; // Increment the motion event counter
      Serial.print("Motion Event Count: ");
      Serial.println(motionCount); // Print the current motion event count for debugging
    }
    
    // Stop music and turn off LED if motion threshold is exceeded
    if (motionCount >= MAX_MOTION_COUNT) {
      noTone(BUZZER_PIN); // Stop the music
      digitalWrite(LED_PIN, LOW); // Turn off the LED
      Serial.println("Motion threshold exceeded. Music stopped."); // Notify the user
      while (true); // Halt the program indefinitely
    }
  }
}

// Function to play the Harry Potter theme music
void playMusic() {
  int size = sizeof(durations) / sizeof(int); // Get the number of notes in the melody
  for (int note = 0; note < size; note++) {
    int duration = 1000 / durations[note]; // Calculate the duration of the note
    
    // Read light level and adjust volume (gain) based on light intensity
    int lightLevel = analogRead(LIGHT_SENSOR_PIN);
    float gain = (lightLevel > LIGHT_THRESHOLD) ? 1.5 : 1.0; // Increase gain if light level is high
    int adjustedFrequency = melody[note] * gain; // Adjust the frequency of the note
    
    // Play the note if it's not a rest (0 represents a rest)
    if (melody[note] != 0) {
      tone(BUZZER_PIN, adjustedFrequency, duration); // Play the adjusted frequency
    }
    int pauseBetweenNotes = duration * 1.30; // Pause between notes to separate them
    delay(pauseBetweenNotes); // Wait for the next note to play
    noTone(BUZZER_PIN); // Stop the sound to ensure the note ends properly
    
    // Break the loop if motion threshold is exceeded
    if (motionCount >= MAX_MOTION_COUNT) {
      break; // Exit the loop if motion is detected
    }
  }
}

// Ultrasonic measurement function
int ultrasonic_measure(int trigPin, int echoPin, int max_distance) {
  
  // Trigger the ultrasonic sensor to send out a pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2); // Wait for a short time
  digitalWrite(trigPin, HIGH); // Send the pulse
  delayMicroseconds(10); // Wait for the pulse to be sent
  digitalWrite(trigPin, LOW); // End the pulse
  
  // Measure the duration of the pulse and convert it to distance
  int duration = pulseIn(echoPin, HIGH, max_distance * 59); // Pulse duration in microseconds
  return duration / 59; // Convert duration to distance (cm)
}
