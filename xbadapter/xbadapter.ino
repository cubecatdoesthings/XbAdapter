#define data_pin 13         // Data line (GPIO 13)
#define clock_pin 12        // Clock line (GPIO 12)
#define boot_button_pin 0   // Boot button (GPIO 0)

const int led_cmd[10] = {0,0,1,0,0,0,0,1,0,0}; // LED Initialization
const int boot_anim_cmd[10] = {0,0,1,0,0,0,0,1,0,1}; // Boot Animation
const int sync_cmd[10] = {0,0,0,0,0,0,0,1,0,0}; // Sync
const int turn_off_cmd[10] = {0,0,0,0,0,0,1,0,0,1}; // Turn Off Controllers

bool button_pressed = false;  // Track if the button is pressed
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
  Serial.begin(115200);

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
    } else if (millis() - button_press_start >= 1000) {
      // Button has been held for at least 1 second
      Serial.println("Boot button held for 1 second or more. Syncing...");
      sendData(sync_cmd);
      delay(1000); // Wait for 1 second to avoid multiple sync commands
      button_pressed = false; // Reset button state
    }
  } else {
    // Button is not pressed
    if (button_pressed) {
      // Button was just released
      button_pressed = false; // Reset button state
      Serial.println("Boot button pressed. Turning off controllers...");
      sendData(turn_off_cmd);
      delay(1000); // Wait for 1 second to avoid multiple turn-off commands
    }
  }

  delay(200); // Small delay to avoid busy-waiting
}
