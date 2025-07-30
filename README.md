# Custom 3-Phase BLDC Motor Controller

This repository contains the Arduino code for a custom-built, open-loop Electronic Speed Controller (ESC) designed to drive a homemade 3-phase brushless DC (BLDC) motor. The entire project, from winding the motor to debugging this controller, is documented in the video below.

### Watch The Full Build and Debugging Journey on YouTube!

[**Click here to watch the video on YouTube**](https://www.youtube.com/watch?v=QumYZ-IiKww)

## Hardware Components

This ESC was built on a breadboard using common, through-hole components.

* **Controller:** 1x Arduino Uno (or compatible board)
* **High-Side MOSFETs:** 3x **IRF4905** (P-Channel)
* **Low-Side MOSFETs:** 3x **IRF520N** (N-Channel)
* **Level Shifters:** 3x **2N3904** or **2N2222** (General Purpose NPN BJT)
* **Resistors:**
    * 3x 1kΩ (for NPN base current limiting)
    * 6x 10kΩ (3 for P-FET pull-ups, 3 for N-FET pull-downs)
* **Bulk Capacitor:** 1x 1000µF, 25V+ (for power rail stabilization)
* **Power Supply:** A variable DC power supply capable of providing at least 9V and 2A.

## Code Explained

This sketch provides a simple but fully functional open-loop controller for a 3-phase BLDC motor. It includes an acceleration ramp for smooth startup and a full suite of serial commands for real-time control and testing.

### Key Sections

#### Pin Definitions
The code begins by defining which Arduino pins control the high and low side of each of the three motor phases (A, B, C). Note that `C_LOW_PIN` was moved to Pin 5 to avoid a hardware fault found on Pin 7 during debugging.

#### Motor Control Parameters
These constants define the motor's behavior:
* `START_DELAY`: The initial slow speed (in milliseconds per step) to ensure the motor starts reliably.
* `run_delay`: The target speed the motor will accelerate to. This can be changed live via serial commands.
* `ACCELERATION`: How quickly the motor speeds up.
* `DEAD_TIME_US`: A critical safety feature. This is a 5-microsecond pause inserted when switching a phase to ensure the high-side MOSFET is fully off before the low-side one turns on, preventing a short circuit (shoot-through).

#### `setup()`
Initializes serial communication, prints the list of available commands, and configures all six control pins as outputs. It starts with all phases floating (off) for safety.

#### `loop()`
The main loop continuously checks for new serial commands. If the `runMotor` flag is true, it calls the `commutate()` function to perform one electrical revolution and then adjusts the speed based on the acceleration logic.

#### `handleCommand(char cmd)`
This function processes user input from the Serial Monitor.
* **'s'**: Toggles the motor run state.
* **'1'-'6'**: Stops the motor and holds it at one of the six static commutation positions for testing.
* **'0'**: Releases the motor (all phases float).
* **'m' / 'l'**: Makes the target speed faster or slower.

#### `commutate()` & `setManualPosition(int position)`
These functions contain the core logic. `commutate()` calls `setManualPosition()` for each of the 6 steps in the correct sequence to produce smooth clockwise rotation. The sequence (`A->B`, `C->B`, `C->A`, etc.) was specifically ordered to match the physical windings of the custom motor shown in the video.

#### `setPhaseState(...)`
This is the low-level function that controls the MOSFETs. It takes a phase and a desired state (HIGH, LOW, or OFF) and sets the appropriate Arduino pins, making sure to include the `DEAD_TIME_US` delay for safety.

## How to Use

1.  **Build the Circuit:** Assemble the 3-phase driver on a breadboard according to the schematic shown in the video.
2.  **Upload the Code:** Upload this sketch to your Arduino Uno.
3.  **Connect:** Connect the Arduino to your computer via USB and connect the motor and a suitable power supply (e.g., 12V 2A) to the breadboard.
4.  **Open Serial Monitor:** Open the Arduino IDE's Serial Monitor and set the baud rate to **9600**.
5.  **Control the Motor:** Use the commands listed in the serial monitor to start, stop, and control the speed of your motor. The motor will start automatically by default; send 's' to stop it.

## Disclaimer

This project is for educational and experimental purposes. Running a motor on a breadboard can be unreliable and risks damaging components due to loose connections and electrical noise. The code is provided as-is. Proceed with caution.
