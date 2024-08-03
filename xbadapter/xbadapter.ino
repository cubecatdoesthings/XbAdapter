#define data_pin 13         // Data line (GPIO 13)
#define clock_pin 12        // Clock line (GPIO 12)
#define boot_button_pin 0   // Boot button (GPIO 0)

// Commands
const int led_cmd[10] = {0,0,1,0,0,0,0,1,0,0}; // LED Initialization Command (0x084)
const int boot_anim_cmd[10] = {0,0,1,0,0,0,0,1,0,1}; // Boot Animation Command (0x089)
const int sync_cmd[10] = {0,0,0,0,0,0,0,1,0,0}; // Sync Command (0x004)
const int turn_off_cmd[10] = {0,0,0,0,0,0,1,0,0,1}; // Turn Off Controllers Command (0x009)

bool button_pressed = false;  // Track if the button is pressed
bool syncing = false;         // Track if the system is in the sync process
unsigned long button_press_start = 0; // Timer for button press duration

void sendData(const int command[]) {
  pinMode(data_pin, OUTPUT);
  digitalWrite(data_pin, LOW); // Start sending data

  int prev_clock = 1;
  for (int i = 0; i < 10; i++) {
    while (prev_clock == digitalRead(clock_pin)) {} // Wait for clock change
    prev_clock = digitalRead(clock_pin);
    // Send data bit
    digitalWrite(data_pin, command[i]);
    while (prev_clock == digitalRead(clock_pin)) {} // Wait for clock change
    prev_clock = digitalRead(clock_pin);
  }
  digitalWrite(data_pin, HIGH);
  pinMode(data_pin, INPUT);
}

void setup() {
  pinMode(data_pin, INPUT);
  pinMode(clock_pin, INPUT);
  pinMode(boot_button_pin, INPUT_PULLUP); // Configure boot button pin with internal pull-up resistor

  delay(2000); // Short delay for stabilization

  // Initialize LEDs and display boot animation
  sendData(led_cmd);
  delay(100); // Short delay between commands
  sendData(boot_anim_cmd);
}

void loop() {
  bool current_button_state = (digitalRead(boot_button_pin) == LOW); // Check if button is pressed

  if (current_button_state) {
    if (!button_pressed) {
      // Button was just pressed
      button_pressed = true;
      button_press_start = millis(); // Record the start time
    } else if (millis() - button_press_start >= 1000 && !syncing) {
      // Button has been held for at least 1 second
      sendData(sync_cmd);
      syncing = true; // Mark as syncing
    }
  } else {
    // Button is not pressed
    if (button_pressed) {
      // Button was just released
      if (millis() - button_press_start < 1000) {
        sendData(turn_off_cmd);
        delay(100); // Short delay
        sendData(led_cmd); // Re-initialize LEDs after turning off controllers
      }
      button_pressed = false; // Reset button state
      syncing = false; // Reset syncing state
    }
  }

  delay(200); // Small delay to avoid busy-waiting
}
