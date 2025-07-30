// --- Pin Definitions for 3-Phase Bridge ---
// Phase A
const int A_HIGH_PIN = 10; // Controls P-FET level shifter for Phase A
const int A_LOW_PIN  = 11; // Controls N-FET for Phase A

// Phase B
const int B_HIGH_PIN = 8;  // Controls P-FET level shifter for Phase B
const int B_LOW_PIN  = 9;  // Controls N-FET for Phase B

// Phase C
const int C_HIGH_PIN = 6;  // Controls P-FET level shifter for Phase C
const int C_LOW_PIN  = 5;  // Controls N-FET for Phase C (Moved from Pin 7)

// --- Motor Control Parameters ---
int current_step_delay;
int run_delay = 10;           // Target delay in ms (faster speed). Can be changed live.
const int START_DELAY = 60;   // Starting delay in ms (slow speed)
const int ACCELERATION = 2;   // How much to decrease the delay each full revolution
const int SPEED_STEP = 2;     // How much to change speed per command (ms)
const int MAX_SPEED_DELAY = 4;// Safest fastest delay

boolean runMotor = false; // Motor will run by default on startup

// --- Safety Parameters ---
const int DEAD_TIME_US = 5; // Safety delay to prevent shoot-through

// Enum for clarity
enum State { S_OFF, S_HIGH, S_LOW };

void setup() {
  Serial.begin(9600);
  Serial.println("--- Custom Open-Loop BLDC Motor Driver ---");
  Serial.println("Commands: 's' (start/stop), '1'-'6' (pos), '0' (release), 'm' (faster), 'l' (slower)");

  // Set all 6 control pins to OUTPUT
  pinMode(A_HIGH_PIN, OUTPUT); pinMode(A_LOW_PIN, OUTPUT);
  pinMode(B_HIGH_PIN, OUTPUT); pinMode(B_LOW_PIN, OUTPUT);
  pinMode(C_HIGH_PIN, OUTPUT); pinMode(C_LOW_PIN, OUTPUT);

  // Start with all phases floating for safety
  setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_OFF);
  setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_OFF);
  setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_OFF);
  
  delay(1000); // Wait a moment before starting
  current_step_delay = START_DELAY; // Initialize speed
}

void loop() {
  // Check for serial commands first
  if (Serial.available() > 0) {
    char command = Serial.read();
    handleCommand(command);
  }

  // Only run the motor if the flag is true
  if (runMotor) {
    commutate(); // Run one full electrical revolution

    // Accelerate or decelerate towards the target run_delay
    if (current_step_delay > run_delay) {
      current_step_delay -= ACCELERATION;
      if (current_step_delay < run_delay) {
        current_step_delay = run_delay;
      }
    } else if (current_step_delay < run_delay) {
      current_step_delay += ACCELERATION;
       if (current_step_delay > run_delay) {
        current_step_delay = run_delay;
      }
    }
  }
}

void handleCommand(char cmd) {
  if (cmd == 's') {
    runMotor = !runMotor; // Toggle the run state
    if (!runMotor) {
      // Release the motor when stopped
      setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_OFF);
      setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_OFF);
      setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_OFF);
      Serial.println("Motor STOPPED.");
    } else {
      current_step_delay = START_DELAY; // Reset speed for ramp-up
      Serial.println("Motor STARTED.");
    }
  } else if (cmd >= '1' && cmd <= '6') {
    runMotor = false; // Stop the automatic rotation
    int position = cmd - '0'; // Convert char '1' to int 1, etc.
    Serial.print("Setting manual position: ");
    Serial.println(position);
    setManualPosition(position);
  } else if (cmd == '0') {
    runMotor = false;
    Serial.println("Releasing motor (all phases float).");
    setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_OFF);
    setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_OFF);
    setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_OFF);
  } else if (cmd == 'm') { // Faster
    run_delay -= SPEED_STEP;
    if (run_delay < MAX_SPEED_DELAY) {
      run_delay = MAX_SPEED_DELAY; // Don't go too fast
    }
    Serial.print("Target delay set to: ");
    Serial.println(run_delay);
  } else if (cmd == 'l') { // Slower
    run_delay += SPEED_STEP;
    if (run_delay > START_DELAY) {
      run_delay = START_DELAY; // Don't go slower than start speed
    }
    Serial.print("Target delay set to: ");
    Serial.println(run_delay);
  }
}

// Sets a single, static motor position based on the 6-step sequence
void setManualPosition(int position) {
  switch (position) {
    case 1: // A -> B
      setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_HIGH);
      setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_LOW);
      setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_OFF);
      break;
    case 2: // C -> B
      setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_OFF);
      setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_LOW);
      setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_HIGH);
      break;
    case 3: // C -> A
      setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_LOW);
      setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_OFF);
      setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_HIGH);
      break;
    case 4: // B -> A
      setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_LOW);
      setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_HIGH);
      setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_OFF);
      break;
    case 5: // B -> C
      setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_OFF);
      setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_HIGH);
      setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_LOW);
      break;
    case 6: // A -> C
      setPhaseState(A_HIGH_PIN, A_LOW_PIN, S_HIGH);
      setPhaseState(B_HIGH_PIN, B_LOW_PIN, S_OFF);
      setPhaseState(C_HIGH_PIN, C_LOW_PIN, S_LOW);
      break;
  }
}

// Runs one full 6-step electrical revolution
void commutate() {
  setManualPosition(1); delay(current_step_delay); if (!runMotor) return;
  setManualPosition(2); delay(current_step_delay); if (!runMotor) return;
  setManualPosition(3); delay(current_step_delay); if (!runMotor) return;
  setManualPosition(4); delay(current_step_delay); if (!runMotor) return;
  setManualPosition(5); delay(current_step_delay); if (!runMotor) return;
  setManualPosition(6); delay(current_step_delay);
}

// Universal Phase Control Function with built-in dead time for safety
void setPhaseState(int high_pin, int low_pin, State state) {
  if (state == S_HIGH) {
    digitalWrite(low_pin, LOW);
    delayMicroseconds(DEAD_TIME_US);
    digitalWrite(high_pin, HIGH);
  }
  else if (state == S_LOW) {
    digitalWrite(high_pin, LOW);
    delayMicroseconds(DEAD_TIME_US);
    digitalWrite(low_pin, HIGH);
  }
  else { // S_OFF (Float)
    digitalWrite(high_pin, LOW);
    digitalWrite(low_pin, LOW);
  }
}
