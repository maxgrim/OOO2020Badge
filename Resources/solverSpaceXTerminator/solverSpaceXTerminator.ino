#define BUFFER_LENGTH 100
static char inputBuffer[BUFFER_LENGTH + 1];
static uint16_t commandCharsRead = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  pinMode(D8, OUTPUT);
  digitalWrite(D8, LOW);
}

void parseAndExecute()
{
    if (!strcmp(inputBuffer, "Shoot now!\r")) {
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(D8, HIGH);
      delay(18);
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(D8, LOW);
    }
}

bool readSerial() {
  while (Serial.available())
    {
        char c = Serial.read();

        switch (c)
        {
        case '\n':
            inputBuffer[commandCharsRead] = '\0';
            if (commandCharsRead > 0)
            {
                commandCharsRead = 0;

                // Signal that the reading is done
                return true;
            }
        default:
            Serial.write(c);

            if (commandCharsRead < BUFFER_LENGTH)
            {
                inputBuffer[commandCharsRead++] = c;
            }

            inputBuffer[commandCharsRead] = '\0';

            break;
        }
    }

    return false;
}

void loop() {
  if(readSerial()) {
    parseAndExecute();
  }
}
