/****************************************************
 * OpenLRSng receiver code
 ****************************************************/




#define INTERNAL_I2C_PULLUPS           // Comment this line if external i2c pullup resistors are present in your hardware implementation

// End of configurable parameters
//*******************************************************************************************************************************************************************

// This version uses the Arduino supplied Wire library. There is another version that uses its own i2c primitives, and saves ~966 bytes.


void lcdClear();

// *********************
// i2c Eagle Tree Power Panel primitives
// *********************
  void i2c_ETPP_init () {
    Wire.beginTransmission(0x3B);        // ETPP i2c address: 0x3B in 7 bit form. Shift left one bit and concatenate i2c write command bit of zero = 0x3b in 8 bit form.
    Wire.write(0x00);                     // ETPP command register
    Wire.write(0x24);                     // Function Set 001D0MSL D : data length for parallel interface only; M: 0 = 1x32 , 1 = 2x16; S: 0 = 1:18 multiplex drive mode, 1�32 or 2�16 character display, 1 = 1:9 multiplex drive mode, 1�16 character display; H: 0 = basic instruction set plus standard instruction set, 1 = basic instruction set plus extended instruction set
    Wire.write(0x0C);                     // Display on   00001DCB D : 0 = Display Off, 1 = Display On; C : 0 = Underline Cursor Off, 1 = Underline Cursor On; B : 0 = Blinking Cursor Off, 1 = Blinking Cursor On
    Wire.write(0x06);                     // Cursor Move  000001IS I : 0 = DDRAM or CGRAM address decrements by 1, cursor moves to the left, 1 = DDRAM or CGRAM address increments by 1, cursor moves to the right; S : 0 = display does not shift,  1 = display does shifts
    Wire.endTransmission();
    lcdClear();         
  }
  void i2c_ETPP_send_cmd (byte c) {
    Wire.beginTransmission(0x3B);
    Wire.write(0x00);                     // ETPP command register
    Wire.write(c);
    Wire.endTransmission();
  }
  void i2c_ETPP_send_char (char c) {
    if (c > 0x0f) c |=  0x80;            // ETPP uses character set "R", which has A->z mapped same as ascii + high bit; don't mess with custom chars. 
    Wire.beginTransmission(0x3B);
    Wire.write(0x40);                     // ETPP data register
    Wire.write(c);
    Wire.endTransmission();
  }
  void i2c_ETPP_send_string (const char *s) {
    Wire.beginTransmission(0x3B);
    Wire.write(0x40);                     // ETPP data register
    while (*s) {byte c = *s++; if (c > 0x0f) c |=  0x80; Wire.write(c);}
    Wire.endTransmission();
  }
  void i2c_ETPP_set_cursor (byte addr) {  
    i2c_ETPP_send_cmd(0x80 | addr);    // High bit is "Set DDRAM" command, remaing bits are addr.  
  }
  void i2c_ETPP_set_cursor (byte col, byte row) {  
    row = min(row,1);
    col = min(col,15);  
    byte addr = col + row * 0x40;      // Why 0x40? RAM in this controller has many more bytes than are displayed.  In particular, the start of the second line (line 1 char 0) is 0x40 in DDRAM. The bytes between 0x0F (last char of line 1) and 0x40 are not displayable (unless the display is placed in marquee scroll mode)
    i2c_ETPP_set_cursor(addr);          
  }
  void i2c_ETPP_create_char (byte idx, uint8_t* array) {
    i2c_ETPP_send_cmd(0x80);                   // CGRAM and DDRAM share an address register, but you can't set certain bits with the CGRAM address command.   Use DDRAM address command to be sure high order address bits are zero. 
    i2c_ETPP_send_cmd(0x40 | byte(idx * 8));   // Set CGRAM address 
    Wire.beginTransmission(0x3B);  
    Wire.write(0x40);                           // ETPP data register
    for (byte i = 0 ; i<8 ; i++) {Wire.write(*array); array++;}
    Wire.endTransmission();
  }
//*******************************************************************************************************************************************************************

// *********************
// Functions that mimic LiquidCrystal library functions and work with ETPP
// *********************
  void lcdClear() {
    i2c_ETPP_send_cmd(0x01);                              // Clear display command, which does NOT clear an Eagle Tree because character set "R" has a '>' at 0x20
    for (byte i = 0; i<80; i++) i2c_ETPP_send_char(' ');  // Blanks for all 80 bytes of RAM in the controller, not just the 2x16 display
  }
  void lcdBlink()                                                      {
    i2c_ETPP_send_cmd(0x0D);            // Display on   00001DCB D : 0 = Display Off, 1 = Display On; C : 0 = Underline Cursor Off, 1 = Underline Cursor On; B : 0 = Blinking Cursor Off, 1 = Blinking Cursor On
  }
  void lcdNoBlink()                                                      {
    i2c_ETPP_send_cmd(0x0C);            // Display on   00001DCB D : 0 = Display Off, 1 = Display On; C : 0 = Underline Cursor Off, 1 = Underline Cursor On; B : 0 = Blinking Cursor Off, 1 = Blinking Cursor On
  }
  void lcdCursor()                                                      {
    i2c_ETPP_send_cmd(0x0E);            // Display on   00001DCB D : 0 = Display Off, 1 = Display On; C : 0 = Underline Cursor Off, 1 = Underline Cursor On; B : 0 = Blinking Cursor Off, 1 = Blinking Cursor On
  }
  void lcdNoCursor()                                                      {
    i2c_ETPP_send_cmd(0x0C);            // Display on   00001DCB D : 0 = Display Off, 1 = Display On; C : 0 = Underline Cursor Off, 1 = Underline Cursor On; B : 0 = Blinking Cursor Off, 1 = Blinking Cursor On
  }
  void lcdCreateChar(int8_t idx, uint8_t* array)                       {i2c_ETPP_create_char(idx, array);}
  void lcdSetCursor(uint8_t pos)                                       {i2c_ETPP_set_cursor(pos % 16, pos / 16);}
  void lcdWrite(int8_t digit)                                          {i2c_ETPP_send_char(digit);}
  void lcdWrite(uint8_t pos, int8_t digit)                             {lcdSetCursor(pos); lcdWrite(digit);}
  void lcdPrint                  (char const      msg[])               {i2c_ETPP_send_string(msg);}
  void lcdPrint     (uint8_t pos, char const      msg[])               {lcdSetCursor(pos); lcdPrint(msg);}

//----------------


FastSerialPort0(Serial);

#ifdef MAVLINK_INJECT
uint32_t last_mavlinkInject_time = 0;
MavlinkFrameDetector frameDetector;
#endif
uint16_t rxerrors = 0;

uint8_t RF_channel = 0;

uint32_t lastPacketTimeUs = 0;
uint32_t lastRSSITimeUs = 0;
uint32_t linkLossTimeMs;

// Background noise variables
uint8_t  noise_count = 0;
uint32_t noise_sum = 0;
uint8_t  noise_smooth = 0;
uint32_t noise_timeBeforeNextNoiseSampleUs = 0;
//


uint32_t lastBeaconTimeMs;

uint8_t  RSSI_count = 0;
uint16_t RSSI_sum = 0;
uint8_t  lastRSSIvalue = 0;
uint8_t  smoothRSSI = 0;
uint8_t  compositeRSSI = 0;
uint16_t lastAFCCvalue = 0;

uint16_t linkQuality = 0;

uint8_t  ppmCountter = 0;
uint16_t ppmSync = 40000;
uint8_t  ppmChannels = 8;

volatile uint8_t disablePWM = 0;
volatile uint8_t disablePPM = 0;
uint8_t failsafeActive = 0;

uint16_t failsafePPM[PPM_CHANNELS];
uint8_t  failsafeIsValid = 0;

uint8_t linkAcquired = 0;
uint8_t numberOfLostPackets = 0;

boolean willhop = 0, fs_saved = 0;

pinMask_t chToMask[PPM_CHANNELS];
pinMask_t clearMask;

void outputUp(uint8_t no)
{
  PORTB |= chToMask[no].B;
  PORTC |= chToMask[no].C;
  PORTD |= chToMask[no].D;
}

void outputDownAll()
{
  PORTB &= clearMask.B;
  PORTC &= clearMask.C;
  PORTD &= clearMask.D;
}

volatile uint16_t nextICR1;

ISR(TIMER1_OVF_vect)
{
  if (ppmCountter < ppmChannels) {
    ICR1 = nextICR1;
    nextICR1 = servoBits2Us(PPM[ppmCountter]) * 2;
    ppmSync -= nextICR1;
    if (ppmSync < (rx_config.minsync * 2)) {
      ppmSync = rx_config.minsync * 2;
    }
    if ((disablePPM) || ((rx_config.flags & PPM_MAX_8CH) && (ppmCountter >= 8))) {
      OCR1A = 65535; //do not generate a pulse
    } else {
      OCR1A = nextICR1 - 600;
    }

    while (TCNT1 < 32);
    outputDownAll();
    if ((!disablePWM) && (ppmCountter > 0)) {
      outputUp(ppmCountter - 1);
    }

    ppmCountter++;
  } else {
    ICR1 = nextICR1;
    nextICR1 = ppmSync;
    if (disablePPM) {
      OCR1A = 65535; //do not generate a pulse
    } else {
      OCR1A = nextICR1 - 600;
    }
    ppmSync = 40000;

    while (TCNT1 < 32);
    outputDownAll();
    if (!disablePWM) {
      outputUp(ppmChannels - 1);
    }

    ppmCountter = 0 ;
  }
}

uint16_t RSSI2Bits(uint8_t rssi)
{
  uint16_t ret = (uint16_t)rssi << 2;
  if (ret < 12) {
    ret = 12;
  } else if (ret > 1012) {
    ret = 1012;
  }
  return ret;
}

void set_PPM_RSSI_output()
{
	if (rx_config.RSSIpwm < 16) {
		cli();
#if REVERSE_PPM_RSSI_SERVO == 1
		PPM[rx_config.RSSIpwm] = 1024 - RSSI2Bits(compositeRSSI);
#else
		PPM[rx_config.RSSIpwm] = RSSI2Bits(compositeRSSI);
#endif
		sei();
	}
}

void set_RSSI_output()
{
  uint8_t linkq = countSetBits(linkQuality & 0x7fff);
  if (linkq == 15) {
    // RSSI 0 - 255 mapped to 192 - ((255>>2)+192) == 192-255
    compositeRSSI = (smoothRSSI >> 2) + 192;
  } else {
    // linkquality gives 0 to 14*13 == 182
    compositeRSSI = linkq * 13;
  }

  set_PPM_RSSI_output();

  if (rx_config.pinMapping[RSSI_OUTPUT] == PINMAP_RSSI) {
    if ((compositeRSSI == 0) || (compositeRSSI == 255)) {
      TCCR2A &= ~(1 << COM2B1); // disable RSSI PWM output
      digitalWrite(OUTPUT_PIN[RSSI_OUTPUT], (compositeRSSI == 0) ? LOW : HIGH);
    } else {
      OCR2B = compositeRSSI;
      TCCR2A |= (1 << COM2B1); // enable RSSI PWM output
    }
  }
}

void failsafeSave(void)
{
  uint32_t start = millis();
  uint8_t ee_buf[20];

  for (int16_t i = 0; i < PPM_CHANNELS; i++) {
    failsafePPM[i] = PPM[i];
  }

  failsafeIsValid = 1;

  packChannels(6, failsafePPM, ee_buf);
  for (int16_t i = 0; i < 20; i++) {
    myEEPROMwrite(EEPROM_FAILSAFE_OFFSET + 4 + i, ee_buf[i]);
  }

  ee_buf[0] = 0xFA;
  ee_buf[1] = 0x11;
  ee_buf[2] = 0x5A;
  ee_buf[3] = 0xFE;
  for (int16_t i = 0; i < 4; i++) {
    myEEPROMwrite(EEPROM_FAILSAFE_OFFSET + i, ee_buf[i]);
  }

  // make this last at least 200ms for user to see it
  // needed as optimized eeprom code can be real fast if no changes are done
  start = millis() - start;
  if (start < 200) {
    delay(200 - start);
  }
}

void failsafeLoad(void)
{
  uint8_t ee_buf[20];

  for (int16_t i = 0; i < 4; i++) {
    ee_buf[i] = EEPROM.read(EEPROM_FAILSAFE_OFFSET + i);
  }

  if ((ee_buf[0] == 0xFA) && (ee_buf[1] == 0x11) && (ee_buf[2] == 0x5A) && (ee_buf[3] == 0xFE)) {
    for (int16_t i = 0; i < 20; i++) {
      ee_buf[i] = EEPROM.read(EEPROM_FAILSAFE_OFFSET + 4 + i);
    }
    unpackChannels(6, failsafePPM, ee_buf);
    failsafeIsValid = 1;
  } else {
    failsafeIsValid = 0;
  }
}

void failsafeApply()
{
  if (failsafeIsValid) {
    for (int16_t i = 0; i < PPM_CHANNELS; i++) {
      if (i != rx_config.RSSIpwm) {
        cli();
        PPM[i] = failsafePPM[i];
        sei();
      }
    }
  }
}

void setupOutputs()
{
  uint8_t i;

  ppmChannels = getChannelCount(&bind_data);
  if (rx_config.RSSIpwm == ppmChannels) {
    ppmChannels += 1;
  }

  for (i = 0; i < OUTPUTS; i++) {
    chToMask[i].B = 0;
    chToMask[i].C = 0;
    chToMask[i].D = 0;
  }
  clearMask.B = 0xff;
  clearMask.C = 0xff;
  clearMask.D = 0xff;
  for (i = 0; i < OUTPUTS; i++) {
    if (rx_config.pinMapping[i] < PPM_CHANNELS) {
      chToMask[rx_config.pinMapping[i]].B |= OUTPUT_MASKS[i].B;
      chToMask[rx_config.pinMapping[i]].C |= OUTPUT_MASKS[i].C;
      chToMask[rx_config.pinMapping[i]].D |= OUTPUT_MASKS[i].D;
      clearMask.B &= ~OUTPUT_MASKS[i].B;
      clearMask.C &= ~OUTPUT_MASKS[i].C;
      clearMask.D &= ~OUTPUT_MASKS[i].D;
    }
  }

  for (i = 0; i < OUTPUTS; i++) {
    switch (rx_config.pinMapping[i]) {
    case PINMAP_ANALOG:
      pinMode(OUTPUT_PIN[i], INPUT);
      break;
    case PINMAP_TXD:
    case PINMAP_RXD:
    case PINMAP_SDA:
    case PINMAP_SCL:
      break; //ignore serial/I2C for now
    default:
      if (i == RXD_OUTPUT) {
        UCSR0B &= 0xEF; //disable serial RXD
      }
      if (i == TXD_OUTPUT) {
        UCSR0B &= 0xF7; //disable serial TXD
      }
      pinMode(OUTPUT_PIN[i], OUTPUT); //PPM,PWM,RSSI,LBEEP
      break;
    }
  }

  if (rx_config.pinMapping[PPM_OUTPUT] == PINMAP_PPM) {
    digitalWrite(OUTPUT_PIN[PPM_OUTPUT], HIGH);
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
  } else {
    TCCR1A = (1 << WGM11);
  }

  disablePWM = 1;
  disablePPM = 1;

  if ((rx_config.pinMapping[RSSI_OUTPUT] == PINMAP_RSSI) ||
      (rx_config.pinMapping[RSSI_OUTPUT] == PINMAP_LBEEP)) {
    pinMode(OUTPUT_PIN[RSSI_OUTPUT], OUTPUT);
    digitalWrite(OUTPUT_PIN[RSSI_OUTPUT], LOW);
    if (rx_config.pinMapping[RSSI_OUTPUT] == PINMAP_RSSI) {
      TCCR2B = (1 << CS20);
      TCCR2A = (1 << WGM20);
    } else { // LBEEP
      TCCR2A = (1 << WGM21); // mode=CTC
      TCCR2B = (1 << CS22) | (1 << CS20); // prescaler = 128
      OCR2A = 62; // 1KHz
    }
  }

  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
  OCR1A = 65535;  // no pulse =)
  ICR1 = 2000; // just initial value, will be constantly updated
  ppmSync = 40000;
  nextICR1 = 40000;
  ppmCountter = 0;
  TIMSK1 |= (1 << TOIE1);

  if ((rx_config.flags & IMMEDIATE_OUTPUT) && failsafeIsValid) {
    failsafeApply();
    disablePPM=0;
    disablePWM=0;
  }

#ifdef OUTPUT_DEBUG_TIMING_ON_SDA
  pinMode(A4, OUTPUT);
#endif
}

void updateLBeep(boolean packetlost)
{
  if (rx_config.pinMapping[RSSI_OUTPUT] == PINMAP_LBEEP) {
    if (packetlost) {
      TCCR2A |= (1 << COM2B0); // enable tone
    } else {
      TCCR2A &= ~(1 << COM2B0); // disable tone
    }
  }
}

uint8_t bindReceive(uint32_t timeout)
{
  uint32_t start = millis();
  uint8_t  rxb;
  init_rfm(1);
  RF_Mode = Receive;
  to_rx_mode();
  Serial.println("Waiting bind\n");

  while ((!timeout) || ((millis() - start) < timeout)) {
    if (RF_Mode == Received) {
      Serial.println("Got pkt\n");
      spiSendAddress(0x7f);   // Send the package read command
      rxb = spiReadData();
      if (rxb == 'b') {
        for (uint8_t i = 0; i < sizeof(bind_data); i++) {
          *(((uint8_t*) &bind_data) + i) = spiReadData();
        }

        if (bind_data.version == BINDING_VERSION) {
          Serial.println("data good\n");
          rxb = 'B';
          tx_packet(&rxb, 1); // ACK that we got bound
          Green_LED_ON; //signal we got bound on LED:s
          return 1;
        }
      } else if ((rxb == 'p') || (rxb == 'i')) {
        uint8_t rxc_buf[sizeof(rx_config) + 1];
        if (rxb == 'p') {
          Serial.println(F("Sending RX config"));
          rxc_buf[0] = 'P';
          timeout = 0;
        } else {
          Serial.println(F("Reinit RX config"));
          rxInitDefaults(1);
          rxc_buf[0] = 'I';
        }
        memcpy(rxc_buf + 1, &rx_config, sizeof(rx_config));
        tx_packet(rxc_buf, sizeof(rx_config) + 1);
      } else if (rxb == 't') {
        uint8_t rxc_buf[sizeof(rxSpecialPins) + 5];
        Serial.println(F("Sending RX type info"));
        timeout = 0;
        rxc_buf[0] = 'T';
        rxc_buf[1] = (version >> 8);
        rxc_buf[2] = (version & 0xff);
        rxc_buf[3] = OUTPUTS;
        rxc_buf[4] = sizeof(rxSpecialPins) / sizeof(rxSpecialPins[0]);
        memcpy(rxc_buf + 5, &rxSpecialPins, sizeof(rxSpecialPins));
        tx_packet(rxc_buf, sizeof(rxSpecialPins) + 5);
      } else if (rxb == 'u') {
        for (uint8_t i = 0; i < sizeof(rx_config); i++) {
          *(((uint8_t*) &rx_config) + i) = spiReadData();
        }
        rxWriteEeprom();
        rxb = 'U';
        tx_packet(&rxb, 1); // ACK that we updated settings
      }
      RF_Mode = Receive;
      rx_reset();

    }
  }
  return 0;
}

int8_t checkIfConnected(uint8_t pin1, uint8_t pin2)
{
  int8_t ret = 0;
  pinMode(pin1, OUTPUT);
  digitalWrite(pin1, 1);
  digitalWrite(pin2, 1);
  delayMicroseconds(10);

  if (digitalRead(pin2)) {
    digitalWrite(pin1, 0);
    delayMicroseconds(10);

    if (!digitalRead(pin2)) {
      ret = 1;
    }
  }

  pinMode(pin1, INPUT);
  digitalWrite(pin1, 0);
  digitalWrite(pin2, 0);
  return ret;
}

uint8_t rx_buf[21]; // RX buffer (uplink)
// First byte of RX buf is
// MSB..LSB [1bit uplink seqno.] [1bit downlink seqno] [6bits type)
// type 0x00 normal servo, 0x01 failsafe set
// type 0x38..0x3f uplinkked serial data

uint8_t tx_buf[64]; // TX buffer (downlink)(type plus 8 x data)
// First byte is meta
// MSB..LSB [1 bit uplink seq] [1bit downlink seqno] [6b telemtype]
// 0x00 link info [RSSI] [AFCC]*2 etc...
// type 0x38-0x3f downlink serial data 1-8 bytes

uint8_t hopcount;

void setup()
{
  //LEDs
  pinMode(Green_LED, OUTPUT);
  pinMode(Red_LED, OUTPUT);

  setupSPI();

#ifdef SDN_pin
  pinMode(SDN_pin, OUTPUT);  //SDN
  digitalWrite(SDN_pin, 0);
#endif

  pinMode(0, INPUT);   // Serial Rx
  pinMode(1, OUTPUT);  // Serial Tx

  Serial.begin(115200, SERIAL_RX_BUFFERSIZE, SERIAL_TX_BUFFERSIZE);   //Serial Transmission

  rxReadEeprom();
  failsafeLoad();
  Serial.print("OpenLRSng RX starting ");
  printVersion(version);
  Serial.print(" on HW ");
  Serial.println(BOARD_TYPE);

  setupRfmInterrupt();

  sei();
  Red_LED_ON;

  if (checkIfConnected(OUTPUT_PIN[2], OUTPUT_PIN[3])) { // ch1 - ch2 --> force scannerMode
    while (1) {
      scannerMode();
    }
  }

  if (checkIfConnected(OUTPUT_PIN[0], OUTPUT_PIN[1]) || (!bindReadEeprom())) {
    Serial.print("EEPROM data not valid or bind jumpper set, forcing bind\n");

    if (bindReceive(0)) {
      bindWriteEeprom();
      Serial.println("Saved bind data to EEPROM\n");
      Green_LED_ON;
    }
    setupOutputs();
  } else {
    setupOutputs();
    if ((rx_config.flags & ALWAYS_BIND) && (!(rx_config.flags & SLAVE_MODE))) {
      if (bindReceive(500)) {
        bindWriteEeprom();
        Serial.println("Saved bind data to EEPROM\n");
        setupOutputs(); // parameters may have changed
        Green_LED_ON;
      }
    }
  }

  if ((rx_config.pinMapping[SDA_OUTPUT] == PINMAP_SDA) &&
      (rx_config.pinMapping[SCL_OUTPUT] == PINMAP_SCL)) {
    if (rx_config.flags & SLAVE_MODE) {
      Serial.println("I am slave");
      fatalBlink(5); // not implemented
      // not reached
    } else {
      Serial.println("Looking for slave, not implemented yet");
    }
  }

  Serial.print("Entering normal mode");

  init_rfm(0);   // Configure the RFM22B's registers for normal operation
  RF_channel = 0;
  rfmSetChannel(RF_channel);

  // Count hopchannels as we need it later
  hopcount = 0;
  while ((hopcount < MAXHOPS) && (bind_data.hopchannel[hopcount] != 0)) {
    hopcount++;
  }

  //################### RX SYNC AT STARTUP #################
  RF_Mode = Receive;
  to_rx_mode();

  /*
  if ((bind_data.flags & TELEMETRY_MASK) == TELEMETRY_FRSKY) {
    Serial.begin(9600);
  } else {
    Serial.begin(bind_data.serial_baudrate, SERIAL_RX_BUFFERSIZE, SERIAL_TX_BUFFERSIZE);
  }
  */

  while (Serial.available()) {
    Serial.read();
  }


  Wire.begin();            // Join the i2c bus as a host
  i2c_ETPP_init();       // Initalize the Eagle Tree Power Panel

  char buf[16];
  //sprintf(buf, "OpenLRSngRX v%d.%d", 3, 5); // 19626 - 18070 = 1556 (this line takes 1556 bytes in memory, but it's convenient, remove once memory starts to b
  lcdPrint(0, buf);       // Position zero (line 0, character 0) = start of line 1
  //lcdPrint(16,"on HW");      // Position 16   (line 1, character 0) = start of line 2
  //lcdWrite(16 + 7, BOARD_TYPE);


  linkAcquired = 0;
  lastPacketTimeUs = micros();

}


void sampleBackgroundNoise()
{
#ifdef OUTPUT_DEBUG_TIMING_ON_SDA
	digitalWrite(A4, HIGH);
#endif

	noise_timeBeforeNextNoiseSampleUs = 0;

	for (uint8_t i = 0; i < 20; i++)
	{
		uint8_t noise = rfmGetRSSI(); // Read the RSSI value
		noise_sum += noise;    // tally up for average
		noise_count++;
	}

	if (noise_count > hopcount * 20) {
		noise_sum /= noise_count;
		noise_smooth = (((uint16_t)noise_smooth * 3 + (uint16_t)noise_sum * 1) / 4);
		noise_sum = 0;
		noise_count = 0;
	}

#ifdef OUTPUT_DEBUG_TIMING_ON_SDA
	digitalWrite(A4, LOW);
#endif
}

//############ MAIN LOOP ##############
void loop()
{
  uint32_t timeUs, timeMs;

  if (spiReadRegister(0x0C) == 0) {     // detect the locked module and reboot
    Serial.println("RX hang");
    init_rfm(0);
    to_rx_mode();
  }

  timeUs = micros();

  if (RF_Mode == Received) {   // RFM22B int16_t pin Enabled by received Data

    lastPacketTimeUs = micros(); // record last package time
    numberOfLostPackets = 0;
    linkQuality <<= 1;
    linkQuality |= 1;

    Red_LED_OFF;
    Green_LED_ON;

    updateLBeep(false);

    spiSendAddress(0x7f);   // Send the package read command

    for (int16_t i = 0; i < getPacketSize(&bind_data); i++) {
      rx_buf[i] = spiReadData();
    }

    lastAFCCvalue = rfmGetAFCC();

    if ((rx_buf[0] & 0x3e) == 0x00) {
      cli();
      unpackChannels(bind_data.flags & 7, PPM, rx_buf + 1);
      set_PPM_RSSI_output(); // Override PPM from TX with RSSI value.
	  sei();
      if (rx_buf[0] & 0x01) {
        if (!fs_saved) {
          failsafeSave();
          fs_saved = 1;
        }
      } else if (fs_saved) {
        fs_saved = 0;
      }
    } else {
      // something else than servo data...
      if ((rx_buf[0] & 0x38) == 0x38) {
        if ((rx_buf[0] ^ tx_buf[0]) & 0x80) {
          // We got new data... (not retransmission)
          uint8_t i;
          tx_buf[0] ^= 0x80; // signal that we got it
          for (i = 0; i <= (rx_buf[0] & 7);) {
            i++;
            const uint8_t ch = rx_buf[i];
            Serial.write(ch);
#if MAVLINK_INJECT == 1
            // Check mavlink frames of incoming serial stream before injection of mavlink radio status packet.
            // Inject packet right after a completed packet
            if (frameDetector.Parse(ch) && timeUs - last_mavlinkInject_time > MAVLINK_INJECT_INTERVAL) {
              // Inject Mavlink radio modem status package.
              MAVLink_report(&Serial, smoothRSSI, noise_smooth, rxerrors, 0); // uint8_t RSSI_remote, uint16_t RSSI_local, uint16_t rxerrors)
              last_mavlinkInject_time = timeUs;
            }
#endif
          }
        }
      }
    }

    if (linkAcquired == 0) {
      linkAcquired = 1;
    }
    failsafeActive = 0;
    disablePWM = 0;
    disablePPM = 0;

    if (bind_data.flags & TELEMETRY_MASK) {
#if MAVLINK_INJECT == 0
      if ((tx_buf[0] ^ rx_buf[0]) & 0x40) {
        // resend last message
      } else {
        tx_buf[0] &= 0xc0;
        tx_buf[0] ^= 0x40; // swap sequence as we have new data
        if (Serial.available()) {
          uint8_t bytes = 0;
          while ((bytes < 8) && Serial.available()) {
            bytes++;
            Serial.readBytes((char*)&tx_buf[bytes], 1);
          }
          tx_buf[0] |= (0x37 + bytes);
        } else {
          // tx_buf[0] lowest 6 bits left at 0
          tx_buf[1] = lastRSSIvalue;

          if (rx_config.pinMapping[ANALOG0_OUTPUT] == PINMAP_ANALOG) {
            tx_buf[2] = analogRead(OUTPUT_PIN[ANALOG0_OUTPUT]) >> 2;
#ifdef ANALOG0_OUTPUT_ALT
          } else if (rx_config.pinMapping[ANALOG0_OUTPUT_ALT] == PINMAP_ANALOG) {
            tx_buf[2] = analogRead(OUTPUT_PIN[ANALOG0_OUTPUT_ALT]) >> 2;
#endif
          } else {
            tx_buf[2] = 0;
          }

          if (rx_config.pinMapping[ANALOG1_OUTPUT] == PINMAP_ANALOG) {
            tx_buf[3] = analogRead(OUTPUT_PIN[ANALOG1_OUTPUT]) >> 2;
#ifdef ANALOG1_OUTPUT_ALT
          } else if (rx_config.pinMapping[ANALOG1_OUTPUT_ALT] == PINMAP_ANALOG) {
            tx_buf[3] = analogRead(OUTPUT_PIN[ANALOG1_OUTPUT_ALT]) >> 2;
#endif
          } else {
            tx_buf[3] = 0;
          }
          tx_buf[4] = (lastAFCCvalue >> 8);
          tx_buf[5] = lastAFCCvalue & 0xff;
          tx_buf[6] = countSetBits(linkQuality & 0x7fff);
        }
      }
#else
      if (!((tx_buf[0] ^ rx_buf[0]) & 0x40)) { // If not true, resend last message
        tx_buf[0] &= 0xc0; // set 2 msb high
        tx_buf[0] ^= 0x40; // swap sequence bit7 as we have new data
        uint8_t bytes = 0;
        while ((bytes < bind_data.serial_downlink - 1) && Serial.available()) {
          bytes++;
          Serial.readBytes((char*)&tx_buf[bytes], 1);
        }
        tx_buf[0] |= (0x3F & bytes);
      }
#endif

#ifdef OUTPUT_DEBUG_TIMING_ON_SDA
	  digitalWrite(A4, HIGH);
#endif
      tx_packet_async(tx_buf, bind_data.serial_downlink);
      while(!tx_done()) {
        // DO stuff while transmitting.
      }
#ifdef OUTPUT_DEBUG_TIMING_ON_SDA
	  digitalWrite(A4, LOW);
#endif
    }

    RF_Mode = Receive;
    rx_reset();

    willhop = 1;

    Green_LED_OFF;
  }

  timeUs = micros();
  timeMs = millis();

  // Sample background noise while TX/RX is not transmitting
  // If telemetry is enabled, then there's a tiny window between RX transmits before TX starts transmitting of approx 2ms.
  if (noise_timeBeforeNextNoiseSampleUs != 0)
  {
	  if (bind_data.flags & TELEMETRY_MASK)
	  {
		  delayMicroseconds(800); // Hit time between after RX transmit above and before tx starts.
		  sampleBackgroundNoise();
	  }
	  else // No telemetry, sample 1ms after hopping to next channel
	  {
		  if ((timeUs - noise_timeBeforeNextNoiseSampleUs) > 1000)
		  {
			  sampleBackgroundNoise();
		  }
	  }
  }


#if MAVLINK_INJECT == 2
  if (timeUs - last_mavlinkInject_time > MAVLINK_INJECT_INTERVAL) {
	  last_mavlinkInject_time = timeUs;

	  // Inject Mavlink radio modem status package.
	  //MAVLink_report(&Serial, smoothRSSI, noise_smooth, rxerrors, 0); // uint8_t RSSI_remote, uint16_t RSSI_local, uint16_t rxerrors)
	  //Serial.print("noise: "); Serial.print(noise_smooth); Serial.print(" rssi: "); Serial.print(smoothRSSI); Serial.print(" rxerr: "); Serial.println(rxerrors);
  }
#endif


  // sample RSSI when packet is in the 'air'
  if ((numberOfLostPackets < 2) && (lastRSSITimeUs != lastPacketTimeUs) &&
      (timeUs - lastPacketTimeUs) > (getInterval(&bind_data) - 1500)) {
    lastRSSITimeUs = lastPacketTimeUs;
    lastRSSIvalue = rfmGetRSSI(); // Read the RSSI value
    RSSI_sum += lastRSSIvalue;    // tally up for average
    RSSI_count++;

    if (RSSI_count > 8) {
      RSSI_sum /= RSSI_count;
      smoothRSSI = (((uint16_t)smoothRSSI * 3 + (uint16_t)RSSI_sum * 1) / 4);
      set_RSSI_output();
      RSSI_sum = 0;
      RSSI_count = 0;
    }
  }

  if (linkAcquired) {
    if ((numberOfLostPackets < hopcount) && ((timeUs - lastPacketTimeUs) > (getInterval(&bind_data) + 1000))) {
      // we lost packet, hop to next channel
      linkQuality <<= 1;
      willhop = 1;
      if (numberOfLostPackets == 0) {
        linkLossTimeMs = timeMs;
        lastBeaconTimeMs = 0;
		rxerrors++;
      }
      numberOfLostPackets++;
      lastPacketTimeUs += getInterval(&bind_data);
      willhop = 1;
      Red_LED_ON;
      updateLBeep(true);
      set_RSSI_output();
    } else if ((numberOfLostPackets == hopcount) && ((timeUs - lastPacketTimeUs) > (getInterval(&bind_data) * hopcount))) {
      // hop slowly to allow resync with TX
      linkQuality = 0;
      willhop = 1;
	  smoothRSSI = noise_smooth;
      set_RSSI_output();
      lastPacketTimeUs = timeUs;
    }

    if (numberOfLostPackets) {
      if (rx_config.failsafeDelay && (!failsafeActive) && ((timeMs - linkLossTimeMs) > delayInMs(rx_config.failsafeDelay))) {
        failsafeActive = 1;
        failsafeApply();
        lastBeaconTimeMs = (timeMs + delayInMsLong(rx_config.beacon_deadtime)) | 1; //beacon activating...
      }
      if (rx_config.pwmStopDelay && (!disablePWM) && ((timeMs - linkLossTimeMs) > delayInMs(rx_config.pwmStopDelay))) {
        disablePWM = 1;
      }
      if (rx_config.ppmStopDelay && (!disablePPM) && ((timeMs - linkLossTimeMs) > delayInMs(rx_config.ppmStopDelay))) {
        disablePPM = 1;
      }

      if ((rx_config.beacon_frequency) && (lastBeaconTimeMs)) {
        if (((timeMs - lastBeaconTimeMs) < 0x80000000) && // last beacon is future during deadtime
            (timeMs - lastBeaconTimeMs) > (1000UL * rx_config.beacon_interval)) {
          beacon_send();
          init_rfm(0);   // go back to normal RX
          rx_reset();
          lastBeaconTimeMs = millis() | 1; // avoid 0 in time
        }
      }
    }
  } else {
    // Waiting for first packet, hop slowly
    if ((timeUs - lastPacketTimeUs) > (getInterval(&bind_data) * hopcount)) {
      lastPacketTimeUs = timeUs;
      willhop = 1;
    }
  }

  if (willhop == 1) {
    RF_channel++;

    if ((RF_channel == MAXHOPS) || (bind_data.hopchannel[RF_channel] == 0)) {
      RF_channel = 0;
    }
    rfmSetChannel(RF_channel);
	noise_timeBeforeNextNoiseSampleUs = lastPacketTimeUs;
    willhop = 0;
  }

}
