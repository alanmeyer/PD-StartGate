//--------------------------------------------------------------------------------------------------
//
// Name:    PD-StartGate
// Author:  Alan Meyer
// Link:    https://github.com/alanmeyer/PD-StartGate
// Version: 1.01
// Date:    2016-01-18
// Updated: 2016-01-19
//
// Based on the Arduino Uno
// Designed to open a Pinewood Derby Start Gate Solenoid
// Switches:
//       Open:    Momentary Switch that instructs the gate to open when pressed
//       Disable: Allows an administrator (typically running the computer) to prevent the gate
//                from being opened when the system isn't ready
//
// LEDs:
//       Disable: On when the software is preventing the gate from opening
//
// Solenoid: Controlled by the software and powered when not disabled
//           and when the Open button is pressed
//
//
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//                                             Types
//--------------------------------------------------------------------------------------------------
#define    BOOL            unsigned char
#define    UINT8           unsigned char
#define    UINT16          unsigned int
#define    UINT32          unsigned long
#define    UINT64          unsigned double
#define    SINT8           signed char
#define    SINT16          signed int
#define    SINT32          signed long
#define    SINT64          signed double


//--------------------------------------------------------------------------------------------------
//                                        Port Pin Defines
//--------------------------------------------------------------------------------------------------
//  Port B                   Port C                Port D
//   B0  (8)                  C0                    D0 RXD (0)
//   B1  (9)                  C1                    D1 TXD (1)
//   B2 (10)                  C2                    D2     (2)
//   B3 (11)                  C3                    D3     (3)
//   B4 (12)                  C4 SDA                D4     (4)
//   B5 (13)                  C5 SCL                D5     (5)
//                                                  D6     (6)
//                                                  D7     (7)


#define   SCOPE_PIN        8    // For Debugging

#define   SW_OPEN         10
#define   SW_DISABLE      11
#define   SOLENOID_EN     12
#define   LED_DISABLE     13


//--------------------------------------------------------------------------------------------------
//                                            Switch
//--------------------------------------------------------------------------------------------------
#define   DEBOUNCE_MS     20


//--------------------------------------------------------------------------------------------------
//                                            States
//--------------------------------------------------------------------------------------------------
#define   S_WAIT_FOR_ENABLE       1
#define   S_WAIT_FOR_OPEN         2
#define   S_OPEN                  3
#define   S_CLOSE                 4


//--------------------------------------------------------------------------------------------------
//                                           Solenoid
//--------------------------------------------------------------------------------------------------
// The settings will control the on vs. off time
// Target 10% duty cycle or less
#define   SOLENOID_TIME_ON_MS     500
#define   SOLENOID_TIME_OFF_MS    5000


//--------------------------------------------------------------------------------------------------
// Routine:      setup()
//
// Description:  Routine automatically called by the system at startup
//               Use this routine to initalize our system
//--------------------------------------------------------------------------------------------------
// the setup function runs once when you press reset or power the board
void setup()
{
  UINT8 index;
  
  // Init our inputs and outputs
  pinMode(SW_DISABLE, INPUT_PULLUP);
  pinMode(SW_OPEN, INPUT_PULLUP);
  pinMode(LED_DISABLE, OUTPUT);
  pinMode(SOLENOID_EN, OUTPUT);
  pinMode(SCOPE_PIN, OUTPUT);

  // Flash the disable LED to alert the user
  // that the system is (re)starting
  for (index = 0; index < 10; index++)
  {
      digitalWrite(LED_DISABLE, HIGH);
      delay(100);
      digitalWrite(LED_DISABLE, LOW);
      delay(100);
  }
}


//--------------------------------------------------------------------------------------------------
// Routine:      loop()
//
// Description:  This routine is called after startup
//               Run the main routines here
//--------------------------------------------------------------------------------------------------
void loop()
{
    static  UINT8     state = S_WAIT_FOR_ENABLE;
            UINT16    delay_ms;
            UINT16    time_cur, time_last;

    // State Machine to manage the switches and solenoid control
    // In each state, take extra care to disable the solenoid (for protection from overheating)
    switch (state)
    {
        // Sit in this state as long as the Disable switch is set
        // or if the Open switch is being held down
        case S_WAIT_FOR_ENABLE:
            if ((digitalRead(SW_DISABLE)) && (digitalRead(SW_OPEN))) state = S_WAIT_FOR_OPEN;
            digitalWrite(SOLENOID_EN, LOW);
            digitalWrite(LED_DISABLE, HIGH);
            delay_ms = DEBOUNCE_MS;
            break;

        // Sit in this state until Disable is pressed (go back)
        // or if Open is pressed (move forward)
        case S_WAIT_FOR_OPEN:
            if      (!digitalRead(SW_DISABLE)) state = S_WAIT_FOR_ENABLE;
            else if (!digitalRead(SW_OPEN))    state = S_OPEN;
            digitalWrite(SOLENOID_EN, LOW);
            digitalWrite(LED_DISABLE, LOW);
            delay_ms = DEBOUNCE_MS;         
            break;

        // Open the gate by temporarily enabling the solenoid
        case S_OPEN:
            state = S_CLOSE;
            digitalWrite(LED_DISABLE, HIGH);
            digitalWrite(SOLENOID_EN, HIGH);
            delay_ms = SOLENOID_TIME_ON_MS;
            break;

        // Use a long delay after the solenoid is turned off to ensure that our duty cycle is low ( < 10%)
        // Note: This is a transient state, which falls thru back to Wait for Enable
       case S_CLOSE:
            state = S_WAIT_FOR_ENABLE;
            digitalWrite(SOLENOID_EN, LOW);
            delay_ms = SOLENOID_TIME_OFF_MS;
            break;

        // Should never get here but reset to Wait for Enable if we do
        default:
            state = S_WAIT_FOR_ENABLE;
            digitalWrite(SOLENOID_EN, LOW);
            delay_ms = DEBOUNCE_MS;
            break; 
    }

    // If we need to code other activities, put it here and use millis() to monitor time
    // !!! Do not allow this to fall thru until the delay is expired
    time_last = millis();
    while (delay_ms)
    {

        // Do other activities here
        // Note: Be Cooperative (i.e. Try not to spend too much time here to prevent the system from hanging)

        // Toggle our scope pin just to see activity
        digitalWrite(SCOPE_PIN,!digitalRead(SCOPE_PIN));

        // Check for a timer tick here
        // If there's a new time, decrement our timer value
        // Note: May be multiple passes thru this loop if we do a lot of slow operations above
        time_cur = millis();
        while (time_last != time_cur)
        {
            if (delay_ms) delay_ms--;     // Only decrement if > 0
            time_last++;                  // Keep incrementing until last catches up with current
        }
    }

    // Ensure that the Solenoid is turned off by this point
    digitalWrite(SOLENOID_EN, LOW);
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
