#ifndef BEENHERE
#include "SDT.h"
#endif

//DB2OO, 29-AUG-23: Don't use the overall VERSION for the EEPROM structure version information, but use a combination of an EEPROM_VERSION with the size of the EEPROMData variable.
// The "EEPROM_VERSION" should only be changed, if the structure config_t EEPROMData has changed!
// For V049.1 the new version in EEPROM will be "V049_808", for V049.2 it will be "V049_812"
// For V050.2 the version will be "V050_1028"; added arrays for TX & RX attenuator values for V12 hardware
#define EEPROM_VERSION "V050"
static char version_size[10];

/*****
  Purpose: void EEPROMSetVersion()

  Parameter list:
    

  Return value;
    char* pointer to EEPROM version string of the form "V049_808"
*****/
static char *EEPROMSetVersion(void) {
  size_t l;
  strncpy(version_size, EEPROM_VERSION, sizeof(version_size));
  l = strlen(version_size);
  //enough space to append '_' and 4 characters for size
  if ((sizeof(version_size) - l) > 5) {
    version_size[l] = '_';
    itoa(sizeof(EEPROMData), version_size + l + 1, 10);
  }
  return version_size;
}

/*****
  Purpose: void EEPROMRead()

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/
FLASHMEM void EEPROMRead() {
  int i;
  int v049_version = 0;  //DB2OO, 10-SEP-23
#define MORSE_STRING_DISPLAY(s) \
  { \
    size_t j; \
    for (j = 0; j < strlen(s); j++) MorseCharacterDisplay(s[j]); \
  }

  //DB2OO, 25-AUG-23: don't read the EEPROM before you are sure, that it is in T41-SDT format!!
  //DB2OO, 25-AUG-23: first read only the version string and compare it with the current version. The version string must also be at the beginning of the EEPROMData structure!
  for (i = 0; i < 10; i++) { versionSettings[i] = EEPROM.read(EEPROM_BASE_ADDRESS + i); }
#ifdef DEBUG1
  //display version in EEPROM in last line of display
  MORSE_STRING_DISPLAY(F("EEPROMVersion "));
  if (strlen(versionSettings) < 10) {
    MORSE_STRING_DISPLAY(versionSettings);
  } else {
    MORSE_STRING_DISPLAY(F("<<INVALID>>"));
  }
  MyDelay(1000);
#endif
  //Do we have V049.1 or V049.2 structure in EEPROM?
  if (strcmp("V049.1", versionSettings) == 0) v049_version = 1;
  if (strcmp("V049.2", versionSettings) == 0) v049_version = 2;

  if (v049_version > 0) {
    //DB2OO, 29-AUG-23: allow "V049.1" or "V049.2" instead of the Version with size for a migration to the new format
    strcpy(versionSettings, EEPROMSetVersion());  // set new version format
    for (i = 0; i < 10; i++) { EEPROM.write(EEPROM_BASE_ADDRESS + i, versionSettings[i]); }
  }
  if (strncmp(EEPROMSetVersion(), versionSettings, 10) != 0) {
    //Different version in EEPROM: set the EEPROM values for THIS version
#ifdef DEBUG1
    const char *wrong_ver = "EEPROMRead(): different version, calling EEPROMSaveDefaults2()";
    MORSE_STRING_DISPLAY(wrong_ver);
    Serial.println(wrong_ver);
    MyDelay(1000);
#endif
    EEPROMSaveDefaults2();
    // and write it into the EEPROM
    EEPROM.put(EEPROM_BASE_ADDRESS, EEPROMData);
    // after this we will read the default values for this version
  } else {
#ifdef DEBUG1
    MORSE_STRING_DISPLAY(F("-->Reading EEPROM content..."));
    MyDelay(1000);
#endif
  }
#ifdef DEBUG
  //clear the Morse character buffer
  MorseCharacterClear();
#endif

  EEPROM.get(EEPROM_BASE_ADDRESS, EEPROMData);  // Read as one large chunk

  strncpy(versionSettings, EEPROMData.versionSettings, 10);  // KF5N
  AGCMode = EEPROMData.AGCMode;
  audioVolume = EEPROMData.audioVolume;  // 4 bytes
  rfGainAllBands = EEPROMData.rfGainAllBands;
  spectrumNoiseFloor = EEPROMData.spectrumNoiseFloor;  // AFP 09-26-22
  tuneIndex = EEPROMData.tuneIndex;
  stepFineTune = EEPROMData.stepFineTune;
  transmitPowerLevel = EEPROMData.powerLevel;
  xmtMode = EEPROMData.xmtMode;                // AFP 09-26-22
  nrOptionSelect = EEPROMData.nrOptionSelect;  // 1 byte
  currentScale = EEPROMData.currentScale;
  spectrum_zoom = EEPROMData.spectrum_zoom;
  spectrum_display_scale = EEPROMData.spectrum_display_scale;  // 4 bytes

  CWFilterIndex = EEPROMData.CWFilterIndex;  // Off
  paddleDit = EEPROMData.paddleDit;
  paddleDah = EEPROMData.paddleDah;
  decoderFlag = EEPROMData.decoderFlag;
  keyType = EEPROMData.keyType;                  // straight key = 0, keyer = 1
  currentWPM = EEPROMData.currentWPM;            // 4 bytes
  sidetoneVolume = EEPROMData.sidetoneVolume;    // 4 bytes
  cwTransmitDelay = EEPROMData.cwTransmitDelay;  // 4 bytes

  activeVFO = EEPROMData.activeVFO;          // 2 bytes
  freqIncrement = EEPROMData.freqIncrement;  // 4 bytes

  currentBand = EEPROMData.currentBand;                        // 4 bytes
  currentBandA = EEPROMData.currentBandA;                      // 4 bytes
  currentBandB = EEPROMData.currentBandB;                      // 4 bytes
  currentFreqA = EEPROMData.lastFrequencies[currentBandA][0];  // JJP 7/17/23
  currentFreqB = EEPROMData.lastFrequencies[currentBandB][1];  // JJP 7/17/23

  for (int i = 0; i < EQUALIZER_CELL_COUNT; i++) {
    recEQ_Level[i] = EEPROMData.equalizerRec[i];  // 4 bytes each
  }
  for (int i = 0; i < EQUALIZER_CELL_COUNT; i++) {
    xmtEQ_Level[i] = EEPROMData.equalizerXmt[i];
  }

  currentMicThreshold = EEPROMData.currentMicThreshold;  // AFP 09-22-22
  currentMicCompRatio = EEPROMData.currentMicCompRatio;
  currentMicAttack = EEPROMData.currentMicAttack;
  currentMicRelease = EEPROMData.currentMicRelease;
  currentMicGain = EEPROMData.currentMicGain;

  //  Note: switch values are read and written to EEPROM only
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    switchThreshholds[i] = EEPROMData.switchValues[i];
  }

  LPFcoeff = EEPROMData.LPFcoeff;  // 4 bytes
  NR_PSI = EEPROMData.NR_PSI;      // 4 bytes
  NR_alpha = EEPROMData.NR_alpha;  // 4 bytes
  NR_beta = EEPROMData.NR_beta;    // 4 bytes
  omegaN = EEPROMData.omegaN;      // 4 bytes
  pll_fmax = EEPROMData.pll_fmax;  // 4 bytes

  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    powerOutCW[i] = EEPROMData.powerOutCW[i];
    powerOutSSB[i] = EEPROMData.powerOutSSB[i];
    CWPowerCalibrationFactor[i] = EEPROMData.CWPowerCalibrationFactor[i];
    SSBPowerCalibrationFactor[i] = EEPROMData.SSBPowerCalibrationFactor[i];
    IQAmpCorrectionFactor[i] = EEPROMData.IQAmpCorrectionFactor[i];
    IQPhaseCorrectionFactor[i] = EEPROMData.IQPhaseCorrectionFactor[i];
    IQXAmpCorrectionFactor[i] = EEPROMData.IQXAmpCorrectionFactor[i];
    IQXPhaseCorrectionFactor[i] = EEPROMData.IQXPhaseCorrectionFactor[i];
    IQXRecAmpCorrectionFactor[i] = EEPROMData.IQXRecAmpCorrectionFactor[i];
    IQXRecPhaseCorrectionFactor[i] = EEPROMData.IQXRecPhaseCorrectionFactor[i];
    XAttenCW[i] = EEPROMData.XAttenCW[i];
    XAttenSSB[i] = EEPROMData.XAttenSSB[i];
    RAtten[i] = EEPROMData.RAtten[i];
    antennaSelection[i] = EEPROMData.antennaSelection[i];
    SWR_PowerAdj[i] = EEPROMData.SWR_PowerAdj[i];
    SWRSlopeAdj[i] = EEPROMData.SWRSlopeAdj[i];
  }
  for (int i = 0; i < MAX_FAVORITES; i++) {
    favoriteFrequencies[i] = EEPROMData.favoriteFreqs[i];
  }

  // V12HW
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    lastFrequencies[i][0] = EEPROMData.lastFrequencies[i][0];
    lastFrequencies[i][1] = EEPROMData.lastFrequencies[i][1];
  }

  centerFreq = EEPROMData.lastFrequencies[currentBand][activeVFO];  // 4 bytes
  TxRxFreq = centerFreq;                                            // Need to assign TxRxFreq here or numerous subtle frequency bugs will happen.  KF5N August 7, 2023

  //strncpy(EEPROMData.mapFileName, MAP_FILE_NAME, 50);  KF5N
  strncpy(mapFileName, EEPROMData.mapFileName, 50);     // KF5N
  strncpy(myCall, EEPROMData.myCall, 10);               // KF5N
  strncpy(myTimeZone, EEPROMData.myTimeZone, 10);       // KF5N
  freqSeparationChar = EEPROMData.separationCharacter;  //   JJP  7/25/23  KF5N

  paddleFlip = EEPROMData.paddleFlip;            //   JJP  7/27/23.  Was setting to symbolic constant PADDLE_FLIP.  KF5N August 8, 2023
  sdCardPresent = EEPROMData.sdCardPresent = 0;  //   JJP  7/27/23

  myLat = EEPROMData.myLat;
  myLong = EEPROMData.myLong;
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    currentNoiseFloor[i] = EEPROMData.currentNoiseFloor[i];
  }
  //DB2OO, 10-SEP-23: We have V049.1 or V049.2 in EEPROM --> just initialize new/changed EEPROMData structure elements
  switch (v049_version) {
    case 1:  //V049.1 --> there is no "compressorFlag" yet in the structure --> initalize it, to overwrite junk from EEPROM
      EEPROMData.compressorFlag = 0;
      //fall through
    case 2:  //V049.2: compressorFlag is there, but we might have to add other inits (not required yet)
      break;
  }
  compressorFlag = EEPROMData.compressorFlag;  // JJP 8/28/23
  receiveEQFlag = EEPROMData.receiveEQFlag;
  xmitEQFlag = EEPROMData.xmitEQFlag;
  CWToneIndex = EEPROMData.CWToneIndex;

  transmitPowerLevelCW = EEPROMData.TransmitPowerLevelCW;    // Power level factors by mode
  transmitPowerLevelSSB = EEPROMData.TransmitPowerLevelSSB;  // Power level factors by mode

}


/*****
  Purpose: To save the configuration data (working variables) to EEPROM

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/
FLASHMEM void EEPROMWrite() {
  strncpy(EEPROMData.versionSettings, EEPROMSetVersion(), 9);  // KF5N

  EEPROMData.AGCMode = AGCMode;
  EEPROMData.CWFilterIndex = CWFilterIndex;
  EEPROMData.nrOptionSelect = nrOptionSelect;
  EEPROMData.rfGainAllBands = rfGainAllBands;

  EEPROMData.activeVFO = activeVFO;  // 2 bytes

  EEPROMData.audioVolume = audioVolume;  // 4 bytes
  EEPROMData.currentBand = currentBand;  // 4 bytes
  EEPROMData.currentBandA = currentBandA;
  EEPROMData.currentBandB = currentBandB;
  EEPROMData.currentFreqA = currentFreqA;  // JJP 7/17/23
  EEPROMData.currentFreqB = currentFreqB;  // JJP 7/17/23
  EEPROMData.decoderFlag = decoderFlag;

  for (int i = 0; i < EQUALIZER_CELL_COUNT; i++) {
    EEPROMData.equalizerRec[i] = recEQ_Level[i];  // 4 bytes each
    EEPROMData.equalizerXmt[i] = xmtEQ_Level[i];
  }

  EEPROMData.freqIncrement = freqIncrement;              // 4 bytes
  EEPROMData.keyType = keyType;                          // straight key = 0, keyer = 1
  EEPROMData.currentMicThreshold = currentMicThreshold;  // 4 bytes      // AFP 09-22-22
  EEPROMData.currentMicCompRatio = currentMicCompRatio;
  EEPROMData.currentMicAttack = currentMicAttack;
  EEPROMData.currentMicRelease = currentMicRelease;
  EEPROMData.currentMicGain = currentMicGain;

  EEPROMData.paddleDit = paddleDit;
  EEPROMData.paddleDah = paddleDah;
  EEPROMData.spectrumNoiseFloor = spectrumNoiseFloor;  // AFP 09-26-22

  EEPROMData.tuneIndex = tuneIndex;
  EEPROMData.stepFineTune = stepFineTune;

  EEPROMData.powerLevel = transmitPowerLevel;
  EEPROMData.currentWPM = currentWPM;  // 4 bytes
  EEPROMData.xmtMode = xmtMode;        // AFP 09-26-22

  EEPROMData.currentScale = currentScale;
  EEPROMData.spectrum_zoom = spectrum_zoom;
  EEPROMData.spectrum_display_scale = spectrum_display_scale;  // 4 bytes
  EEPROMData.sidetoneVolume = sidetoneVolume;                  // 4 bytes
  EEPROMData.cwTransmitDelay = cwTransmitDelay;                // 4 bytes

  EEPROMData.LPFcoeff = LPFcoeff;  // 4 bytes
  EEPROMData.NR_PSI = NR_PSI;      // 4 bytes
  EEPROMData.NR_alpha = NR_alpha;  // 4 bytes
  EEPROMData.NR_beta = NR_beta;    // 4 bytes
  EEPROMData.omegaN = omegaN;      // 4 bytes
  EEPROMData.pll_fmax = pll_fmax;  // 4 bytes

  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    EEPROMData.CWPowerCalibrationFactor[i] = CWPowerCalibrationFactor[i];    // 0.019;   //AFP 10-29-22
    EEPROMData.SSBPowerCalibrationFactor[i] = SSBPowerCalibrationFactor[i];  // 0.008;   //AFP 10-29-22
    EEPROMData.powerOutCW[i] = powerOutCW[i];                                // 4 bytes //AFP 10-21-22
    EEPROMData.powerOutSSB[i] = powerOutSSB[i];                              // 4 bytes AFP 10-21-22
    EEPROMData.IQAmpCorrectionFactor[i] = IQAmpCorrectionFactor[i];
    EEPROMData.IQPhaseCorrectionFactor[i] = IQPhaseCorrectionFactor[i];
    EEPROMData.IQXAmpCorrectionFactor[i] = IQXAmpCorrectionFactor[i];
    EEPROMData.IQXPhaseCorrectionFactor[i] = IQXPhaseCorrectionFactor[i];
    EEPROMData.IQXRecAmpCorrectionFactor[i] = IQXRecAmpCorrectionFactor[i];
    EEPROMData.IQXRecPhaseCorrectionFactor[i] = IQXRecPhaseCorrectionFactor[i];
    EEPROMData.XAttenCW[i] = XAttenCW[i];
    EEPROMData.XAttenSSB[i] = XAttenSSB[i];
    EEPROMData.RAtten[i] = RAtten[i];
    EEPROMData.antennaSelection[i] = antennaSelection[i];
    EEPROMData.SWR_PowerAdj[i] = SWR_PowerAdj[i];
    EEPROMData.SWR_R_Offset[i] = SWRSlopeAdj[i];
    EEPROMData.SWR_R_Offset[i] = SWR_R_Offset[i];
    
  }
  //  Note:favoriteFreqs are written as they are saved.

  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    EEPROMData.lastFrequencies[i][0] = lastFrequencies[i][0];
    EEPROMData.lastFrequencies[i][1] = lastFrequencies[i][1];
  }

  EEPROMData.lastFrequencies[currentBand][activeVFO] = currentFreq;  // 4 bytes
  EEPROMData.lastFrequencies[currentBandA][VFO_A] = currentFreqA;    // 4 bytes
  EEPROMData.lastFrequencies[currentBandB][VFO_B] = currentFreqB;    // 4 bytes
  //EEPROMData.freqCorrectionFactor = freqCorrectionFactor;

  strncpy(EEPROMData.mapFileName, mapFileName, 49);  // 1 smaller to allow for null
  strncpy(EEPROMData.myCall, myCall, 9);
  strncpy(EEPROMData.myTimeZone, myTimeZone, 9);
  EEPROMData.separationCharacter = (int)freqSeparationChar;

  EEPROMData.paddleFlip = paddleFlip;
  EEPROMData.sdCardPresent = sdCardPresent;

  EEPROMData.myLat = myLat;
  EEPROMData.myLong = myLong;
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    EEPROMData.currentNoiseFloor[i] = currentNoiseFloor[i];
  }
  EEPROMData.compressorFlag = compressorFlag;  // JJP 8/28/23
  EEPROMData.receiveEQFlag = receiveEQFlag;
  EEPROMData.xmitEQFlag = xmitEQFlag;
  EEPROMData.CWToneIndex = CWToneIndex;


  EEPROMData.TransmitPowerLevelCW = transmitPowerLevelCW;    // Power level factors by mode
  EEPROMData.TransmitPowerLevelSSB = transmitPowerLevelSSB;  // Power level factors by mode
 


  EEPROM.put(EEPROM_BASE_ADDRESS, EEPROMData);
  // CopyEEPROMToSD();                                               // JJP 7/26/23
  syncEEPROM = 0;  // SD EEPROM different that memory EEPROM
}  // end void eeProm SAVE

/*****
  Purpose: To show the current EEPROM values. Used for debugging

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/
FLASHMEM void EEPROMShow() {
  int i;

  Serial.println(F("----- EEPROM Parameters: -----"));

  Serial.print(F("Version                         = "));
  Serial.println(EEPROMData.versionSettings);
  Serial.print(F("AGCMode                         = "));
  Serial.println(EEPROMData.AGCMode);
  Serial.print(F("audioVolume                     = "));
  Serial.println(EEPROMData.audioVolume);
  Serial.print(F("rfGainAllBands                  = "));
  Serial.println(EEPROMData.rfGainAllBands);
  Serial.print(F("spectrumNoiseFloor              = "));
  Serial.println(EEPROMData.spectrumNoiseFloor);
  Serial.print(F("tuneIndex                       = "));
  Serial.println(EEPROMData.tuneIndex);
  Serial.print(F("stepFineTne                     = "));
  Serial.println(EEPROMData.stepFineTune);
  Serial.print(F("powerLevel                      = "));
  Serial.println(EEPROMData.powerLevel);
  Serial.print(F("xmtMode                         = "));
  Serial.println(EEPROMData.xmtMode);
  Serial.print(F("nrOptionSelect                  = "));
  Serial.println(EEPROMData.nrOptionSelect);
  Serial.print(F("currentScale                    = "));
  Serial.println(EEPROMData.currentScale);
  Serial.print(F("spectrum_zoom                   = "));
  Serial.println(EEPROMData.spectrum_zoom);
  Serial.print(F("spect_display_scale             = "));
  Serial.println(EEPROMData.spectrum_display_scale);
  Serial.println(F("----- CW Parameters: -----"));
  Serial.print(F("CWFilterIndex                   = "));
  Serial.println(EEPROMData.CWFilterIndex);
  Serial.print(F("paddleDah                       = "));
  Serial.println(EEPROMData.paddleDah);
  Serial.print(F("paddleDit                       = "));
  Serial.println(EEPROMData.paddleDit);
  Serial.print(F("decoderFlag                     = "));
  Serial.println(EEPROMData.decoderFlag);
  Serial.print(F("keyType                         = "));
  Serial.println(EEPROMData.keyType);
  Serial.print(F("wordsPerMinute                  = "));
  Serial.println(EEPROMData.currentWPM);
  Serial.print(F("sidetoneVolume                  = "));
  Serial.println(EEPROMData.sidetoneVolume, 5);
  Serial.print(F("cwTransmitDelay                 = "));
  Serial.println(EEPROMData.cwTransmitDelay);
  Serial.println(F("----- Current Frequencies & Bands: -----"));
  Serial.print(F("activeVFO                       = "));
  Serial.println(EEPROMData.activeVFO);
  Serial.print(F("freqIncrement                   = "));
  Serial.println(EEPROMData.freqIncrement);
  Serial.print(F("currentBand                     = "));
  Serial.println(EEPROMData.currentBand);
  Serial.print(F("currentBandA                    = "));
  Serial.println(EEPROMData.currentBandA);
  Serial.print(F("currentBandB                    = "));
  Serial.println(EEPROMData.currentBandB);
  Serial.print(F("currentFreqA                    = "));
  Serial.println(EEPROMData.currentFreqA);
  Serial.print(F("currentFreqB                    = "));
  Serial.println(EEPROMData.currentFreqB);
  Serial.print(F("freqCorrectionFactor            = "));
  Serial.println(EEPROMData.freqCorrectionFactor);
  Serial.println(F("----- Equalizer Parameters -----"));
  for (i = 0; i < EQUALIZER_CELL_COUNT; i++) {
    if (i < 10) {
      Serial.print(F(" "));
    }
    Serial.print(F("               equalizerRec["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.equalizerRec[i]);
  }
  Serial.println(F(" "));
  for (i = 0; i < EQUALIZER_CELL_COUNT; i++) {
    if (i < 10) {
      Serial.print(F(" "));
    }
    Serial.print(F("               equalizerXmt["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.equalizerXmt[i]);
  }
  Serial.println(F("----- Mic Compressor Parameters -----"));
  Serial.print(F("micCompression                  = "));
  Serial.println(EEPROMData.currentMicThreshold);
  Serial.print(F("currentMicCompRatio             = "));
  Serial.println(EEPROMData.currentMicCompRatio);
  Serial.print(F("currentMicAttack                = "));
  Serial.println(EEPROMData.currentMicAttack);
  Serial.print(F("currentMicRelease               = "));
  Serial.println(EEPROMData.currentMicRelease);
  Serial.print(F("currentMicGain                  = "));
  Serial.println(EEPROMData.currentMicGain);
  Serial.println(F("----- Switch Matrix Values -----"));
  for (i = 0; i < NUMBER_OF_SWITCHES; i++) {
    if (i < 10) {
      Serial.print(F(" "));
    }
    Serial.print(F("               switchValues["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.switchValues[i]);
  }
  Serial.println(F("----- Filter Parameters -----"));
  Serial.print(F("LPF coeff                       = "));
  Serial.println(EEPROMData.LPFcoeff);
  Serial.print(F("NR_PSI                          = "));
  Serial.println(EEPROMData.NR_PSI);
  Serial.print(F("NR_alpha                        = "));
  Serial.println(EEPROMData.NR_alpha);
  Serial.print(F("NR_beta                         = "));
  Serial.println(EEPROMData.NR_beta);
  Serial.print(F("NR_omega                        = "));
  Serial.println(EEPROMData.omegaN);
  Serial.print(F("pll_fmax                        = "));
  Serial.println(EEPROMData.pll_fmax);
  Serial.println(F("----- Power Out Calibration Parameters -----"));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("                  powerOutCW["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.powerOutCW[i], 5);  //AFP 10-13-22
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("                 powerOutSSB["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.powerOutSSB[i], 5);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("            CWPowerCalFactor["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.CWPowerCalibrationFactor[i], 5);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("           SSBPowerCalFactor["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.SSBPowerCalibrationFactor[i], 5);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("           XAttenCW["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.XAttenCW[i], 5);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("           XAttenSSB["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.XAttenSSB[i], 5);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("           RAtten["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.RAtten[i], 5);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("           antennaSelection["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.antennaSelection[i], 5);
    Serial.println(EEPROMData.SWR_PowerAdj[i], 5);
    Serial.println(EEPROMData.SWRSlopeAdj[i], 5);
    Serial.println(EEPROMData.SWR_R_Offset[i], 5);
    
  }
  Serial.println(F(" "));
  Serial.println(F("----- I/Q Calibration Parameters -----"));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F(" IQAmplitudeCorrectionFactor["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.IQAmpCorrectionFactor[i], 3);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("     IQPhaseCorrectionFactor["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.IQPhaseCorrectionFactor[i], 3);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("IQXAmplitudeCorrectionFactor["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.IQXAmpCorrectionFactor[i], 3);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("    IQXPhaseCorrectionFactor["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.IQXPhaseCorrectionFactor[i], 3);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("IQXRecAmplitudeCorrectionFactor["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.IQXRecAmpCorrectionFactor[i], 3);
  }
  Serial.println(F(" "));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("    IQXRecPhaseCorrectionFactor["));
    Serial.print(i);
    Serial.print(F("] = "));
    Serial.println(EEPROMData.IQXRecPhaseCorrectionFactor[i], 3);
  }
  Serial.println(F(" "));
  Serial.println(F("----- Favorite Frequencies -----"));
  for (i = 0; i < MAX_FAVORITES; i++) {
    if (i < 10) {
      Serial.print(F(" "));
    }
    Serial.print(F("              favoriteFreqs["));
    Serial.print(i);
    Serial.print(F("] = "));
    if (i < 4) {
      Serial.print(F(" "));
    }
    Serial.println(EEPROMData.favoriteFreqs[i]);
  }
  Serial.println(F("----- Last Frequencies -----"));
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("          lastFrequencies["));
    Serial.print(i);
    Serial.print(F("][0] = "));
    if (i < 2) {
      Serial.print(F(" "));
    }
    Serial.println(EEPROMData.lastFrequencies[i][0]);
  }

  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("          lastFrequencies["));
    Serial.print(i);
    Serial.print(F("][1] = "));
    if (i < 2) {
      Serial.print(F(" "));
    }
    Serial.println(EEPROMData.lastFrequencies[i][1]);
  }
  Serial.println(F(" "));
  Serial.print(F("centerFreq                      = "));
  Serial.println((long)EEPROMData.centerFreq);

  Serial.print(F("mapFileName                     = "));
  Serial.println(EEPROMData.mapFileName);

  Serial.print(F("myCall                          = "));
  Serial.println(EEPROMData.myCall);

  Serial.print(F("myTimeZone                      = "));
  Serial.println(EEPROMData.myTimeZone);

  Serial.print(F("separationCharacter             = "));
  Serial.println((char)EEPROMData.separationCharacter);

  Serial.println(F(" "));  // JJP 7-3-23

  Serial.print(F("paddleFlip                      = "));
  Serial.println(EEPROMData.paddleFlip);
  Serial.print(F("sdCardPresent                   = "));
  Serial.println(EEPROMData.sdCardPresent);
  Serial.print(F("myLat                           = "));
  Serial.println(EEPROMData.myLat);
  Serial.print(F("myLong                          = "));
  Serial.println(EEPROMData.myLong);

  Serial.println(F(" "));  // JJP 7-3-23

  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    Serial.print(F("        currentNoiseFloor["));
    Serial.print(i);
    Serial.print(F("]    = "));
    Serial.println(EEPROMData.currentNoiseFloor[i]);
  }
  Serial.print(F("compressorFlag                  = "));
  Serial.println(EEPROMData.compressorFlag);

  Serial.print(F("receiveEQFlag                   = "));
  Serial.println(EEPROMData.receiveEQFlag);
  Serial.print(F("xmitEQFlag                      = "));
  Serial.println(EEPROMData.xmitEQFlag);
  Serial.print(F("CWToneIndex                     = "));
  Serial.println(EEPROMData.CWToneIndex);

  Serial.print(F("TransmitPowerLevelCW            = "));
  Serial.println(EEPROMData.TransmitPowerLevelCW);
  Serial.print(F("TransmitPowerLevelSSB           = "));
  Serial.println(EEPROMData.TransmitPowerLevelSSB);
  //Serial.println(EEPROMData.SWR_R_Offset);
  Serial.println(F("----- End EEPROM Parameters -----"));
}

/*****
  Purpose: Read default favorite frequencies

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/
void EEPROMStuffFavorites(unsigned long current[]) {
  int i;

  for (i = 0; i < MAX_FAVORITES; i++) {
    current[i] = EEPROMData.favoriteFreqs[i];
  }
}

/*****
  Purpose: Used to save a favortie frequency to EEPROM

  Parameter list:

  Return value;
    void

  CAUTION: This code assumes you have set the curently active VFO frequency to the new
           frequency you wish to save. You them use the menu encoder to scroll through
           the current list of stored frequencies. Stop on the one that you wish to
           replace and press Select to save in EEPROM. The currently active VFO frequency
           is then stored to EEPROM.
*****/
void SetFavoriteFrequency() {
  int index;
  int val;

  tft.setFontScale((enum RA8875tsize)1);

  index = 0;
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
  tft.print(EEPROMData.favoriteFreqs[index]);
  while (true) {
    if (filterEncoderMove != 0) {  // Changed encoder?
      index += filterEncoderMove;  // Yep
      if (index < 0) {
        index = MAX_FAVORITES - 1;  // Wrap to last one
      } else {
        if (index > MAX_FAVORITES)
          index = 0;  // Wrap to first one
      }
      tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
      tft.print(EEPROMData.favoriteFreqs[index]);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    MyDelay(150L);
    if (val == MENU_OPTION_SELECT) {  // Make a choice??
      EraseMenus();
      EEPROMData.favoriteFreqs[index] = TxRxFreq;
      syncEEPROM = 0;  // SD EEPROM different that memory EEPROM
      if (activeVFO == VFO_A) {
        currentFreqA = TxRxFreq;
      } else {
        currentFreqB = TxRxFreq;
      }
      //      EEPROMWrite();
      SetFreq();
      BandInformation();
      ShowBandwidth();
      FilterBandwidth();
      ShowFrequency();
      break;
    }
  }
}

/*****
  Purpose: Used to fetch a favortie frequency as stored in EEPROM. It then copies that
           frequency to the currently active VFO

  Parameter list:

  Return value;
    void
*****/
void GetFavoriteFrequency() {
  int index = 0;
  int val;
  int currentBand2 = 0;
  tft.setFontScale((enum RA8875tsize)1);

  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
  tft.print(EEPROMData.favoriteFreqs[index]);
  while (true) {
    if (filterEncoderMove != 0) {  // Changed encoder?
      index += filterEncoderMove;  // Yep
      if (index < 0) {
        index = MAX_FAVORITES - 1;  // Wrap to last one
      } else {
        if (index > MAX_FAVORITES)
          index = 0;  // Wrap to first one
      }
      tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
      tft.print(EEPROMData.favoriteFreqs[index]);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    MyDelay(150L);

    centerFreq = EEPROMData.favoriteFreqs[index];  // current frequency  AFP 09-27-22
    if (centerFreq >= bands[BAND_80M].fBandLow && centerFreq <= bands[BAND_80M].fBandHigh) {
      currentBand2 = BAND_80M;
      //} else if (centerFreq >= bands[BAND_80M].fBandHigh && centerFreq <= bands[BAND_80M].fBandHigh) {  // covers 5MHz WWV AFP 11-03-22
      // currentBand2 = BAND_80M;

    } else if (centerFreq >= bands[BAND_60M].fBandLow && centerFreq <= bands[BAND_60M].fBandHigh) {  // covers 5MHz WWV AFP 11-03-22
      currentBand2 = BAND_60M;

    } else if (centerFreq >= bands[BAND_40M].fBandLow && centerFreq <= bands[BAND_40M].fBandHigh) {
      currentBand2 = BAND_40M;
      //} else if (centerFreq >= bands[BAND_40M].fBandLow && centerFreq <= bands[BAND_40M].fBandHigh) {
      // currentBand2 = BAND_40M;

    } else if (centerFreq >= bands[BAND_30M].fBandLow && centerFreq <= bands[BAND_30M].fBandHigh) {  // covers 10MHz WWV AFP 11-03-22
      currentBand2 = BAND_30M;

      //} else if (centerFreq >= bands[BAND_80M].fBandHigh && centerFreq <= 7000000L) {  // covers 5MHz WWV AFP 11-03-22
      // currentBand2 = BAND_80M;
    } else if (centerFreq >= bands[BAND_20M].fBandLow && centerFreq <= bands[BAND_20M].fBandHigh) {
      currentBand2 = BAND_20M;
      //} else if (centerFreq >= 14000000L && centerFreq <= 18000000L) {  // covers 15MHz WWV AFP 11-03-22
      //currentBand2 = BAND_20M;
    } else if (centerFreq >= bands[BAND_17M].fBandLow && centerFreq <= bands[BAND_17M].fBandHigh) {
      currentBand2 = BAND_17M;
    } else if (centerFreq >= bands[BAND_15M].fBandLow && centerFreq <= bands[BAND_15M].fBandHigh) {
      currentBand2 = BAND_15M;
    } else if (centerFreq >= bands[BAND_12M].fBandLow && centerFreq <= bands[BAND_12M].fBandHigh) {
      currentBand2 = BAND_12M;
    } else if (centerFreq >= bands[BAND_10M].fBandLow && centerFreq <= bands[BAND_10M].fBandHigh) {
      currentBand2 = BAND_10M;

    } else if (centerFreq >= bands[BAND_6M].fBandLow && centerFreq <= bands[BAND_6M].fBandHigh) {
      currentBand2 = BAND_6M;
      // } else if (centerFreq >= bands[BAND_4M].fBandLow && centerFreq <= bands[BAND_4M].fBandHigh) {
      //   currentBand2 = BAND_4M;
      // } else if (centerFreq >= bands[BAND_2M].fBandLow && centerFreq <= bands[BAND_2M].fBandHigh) {
      //   currentBand2 = BAND_2M;
    }
    currentBand = currentBand2;


    if (val == MENU_OPTION_SELECT) {  // Make a choice??
      switch (activeVFO) {
        case VFO_A:
          if (currentBandA == NUMBER_OF_BANDS) {  // Incremented too far?
            currentBandA = 0;                     // Yep. Roll to list front.
          }
          currentBandA = currentBand2;
          TxRxFreq = centerFreq + NCOFreq;
          lastFrequencies[currentBand][VFO_A] = TxRxFreq;
          break;

        case VFO_B:
          if (currentBandB == NUMBER_OF_BANDS) {  // Incremented too far?
            currentBandB = 0;                     // Yep. Roll to list front.
          }                                       // Same for VFO B
          currentBandB = currentBand2;
          TxRxFreq = centerFreq + NCOFreq;
          lastFrequencies[currentBand][VFO_B] = TxRxFreq;
          break;
      }
    }
    if (val == MENU_OPTION_SELECT) {

      EraseSpectrumDisplayContainer();
      currentMode = bands[currentBand].mode;
      DrawSpectrumDisplayContainer();
      DrawFrequencyBarValue();
      SetBand();
      SetFreq();
      ShowFrequency();
      ShowSpectrumdBScale();
      EraseMenus();
      ResetTuning();
      FilterBandwidth();
      BandInformation();
      NCOFreq = 0L;
      DrawBandWidthIndicatorBar();  // AFP 10-20-22
      digitalWrite(bandswitchPins[currentBand], LOW);
      SetFreq();
      ShowSpectrumdBScale();
      ShowSpectrum();
      //bands[currentBand].mode = currentBand;
      return;
    }
  }
}

//===================================================================
/*****
  Purpose: Copy EEPROM data into working variables

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/


/*****
  Purpose: To save the default setting for EEPROM variables

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    void
*****/

FLASHMEM void EEPROMSaveDefaults2() {
  Serial.println(String(__FUNCTION__));
  strcpy(EEPROMData.versionSettings, EEPROMSetVersion());  // Update version

  EEPROMData.AGCMode = 1;
  EEPROMData.audioVolume = 40;  // 4 bytes
  EEPROMData.rfGainAllBands = 0;
  EEPROMData.spectrumNoiseFloor = SPECTRUM_NOISE_FLOOR;
  EEPROMData.tuneIndex = 5;
  EEPROMData.stepFineTune = 50L;
  EEPROMData.powerLevel = 10;
  EEPROMData.xmtMode = 0;
  EEPROMData.nrOptionSelect = 0;  // 1 byte
  EEPROMData.currentScale = 1;
  EEPROMData.spectrum_zoom = 1;
  EEPROMData.spectrum_display_scale = 20.0;  // 4 bytes

  EEPROMData.CWFilterIndex = 5;  // Off
  EEPROMData.paddleDit = 36;
  EEPROMData.paddleDah = 35;
  EEPROMData.decoderFlag = 0;
  EEPROMData.keyType = 0;            // straight key = 0, keyer = 1
  EEPROMData.currentWPM = 15;        // 4 bytes
  EEPROMData.sidetoneVolume = 50.0;  // 4 bytes.  Changed to default 50.  KF5N October 7, 2023.
  EEPROMData.cwTransmitDelay = 750;  // 4 bytes

  EEPROMData.activeVFO = 0;            // 2 bytes, 0 = VFOa
  EEPROMData.freqIncrement = 5;        // 4 bytes
  EEPROMData.currentBand = BAND_40M;   // 4 bytes // V12HW changed from 1
  EEPROMData.currentBandA = BAND_40M;  // 4 bytes // V12HW changed from 1
  EEPROMData.currentBandB = BAND_40M;  // 4 bytes // V12HW changed from 1
  //DB2OO, 23-AUG-23 7.1MHz for Region 1
#if defined(ITU_REGION) && ITU_REGION == 1
  EEPROMData.currentFreqA = 7100000;
#else
  EEPROMData.currentFreqA = 7200000;
#endif
  EEPROMData.currentFreqB = 7030000;
  //DB2OO, 23-AUG-23: with TCXO needs to be 0
#ifdef TCXO_25MHZ
  EEPROMData.freqCorrectionFactor = 0;  //68000;
#else
  //Conventional crystal with freq offset needs a correction factor
  EEPROMData.freqCorrectionFactor = 0;
#endif

  for (int i = 0; i < EQUALIZER_CELL_COUNT; i++) {
    EEPROMData.equalizerRec[i] = 100;  // 4 bytes each
  }
  // Use transmit equalizer profile in struct initializer list in SDT.h.  KF5N November 2, 2023

  EEPROMData.currentMicThreshold = -10;  // 4 bytes      // AFP 09-22-22
  EEPROMData.currentMicCompRatio = 8.0;  // Changed to 8.0 from 5.0 based on Neville's tests.  KF5N November 2, 2023
  EEPROMData.currentMicAttack = 0.1;
  EEPROMData.currentMicRelease = 0.1;  // Changed to 0.1 from 2.0 based on Neville's tests.  KF5N November 2, 2023
  //DB2OO, 23-AUG-23: MicGain 20
  EEPROMData.currentMicGain = 20;

  EEPROMData.switchValues[0] = 924;
  EEPROMData.switchValues[1] = 870;
  EEPROMData.switchValues[2] = 817;
  EEPROMData.switchValues[3] = 769;
  EEPROMData.switchValues[4] = 713;
  EEPROMData.switchValues[5] = 669;
  EEPROMData.switchValues[6] = 616;
  EEPROMData.switchValues[7] = 565;
  EEPROMData.switchValues[8] = 513;
  EEPROMData.switchValues[9] = 459;
  EEPROMData.switchValues[10] = 407;
  EEPROMData.switchValues[11] = 356;
  EEPROMData.switchValues[12] = 298;
  EEPROMData.switchValues[13] = 242;
  EEPROMData.switchValues[14] = 183;
  EEPROMData.switchValues[15] = 131;
  EEPROMData.switchValues[16] = 67;
  EEPROMData.switchValues[17] = 10;

  EEPROMData.LPFcoeff = 0.0;     // 4 bytes
  EEPROMData.NR_PSI = 0.0;       // 4 bytes
  EEPROMData.NR_alpha = 0.0;     // 4 bytes
  EEPROMData.NR_beta = 0.0;      // 4 bytes
  EEPROMData.omegaN = 0.0;       // 4 bytes
  EEPROMData.pll_fmax = 4000.0;  // 4 bytes


  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    EEPROMData.XAttenCW[i] = 0;
    EEPROMData.XAttenSSB[i] = 10;
    EEPROMData.RAtten[i] = 0;
    EEPROMData.antennaSelection[i] = 0;
    EEPROMData.SWR_PowerAdj[i] = 0;
    EEPROMData.SWRSlopeAdj[i] = 0;
    EEPROMData.SWR_R_Offset[i] = 0;
  }

  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    EEPROMData.powerOutCW[i] = 0.0;
  }

  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    EEPROMData.powerOutSSB[i] = 1.0;
  }

  EEPROMData.CWPowerCalibrationFactor[0] = 0.023;  //V12HW
  EEPROMData.CWPowerCalibrationFactor[1] = 0.023;  //V12HW
  EEPROMData.CWPowerCalibrationFactor[2] = 0.023;  //AFP 10-29-22
  EEPROMData.CWPowerCalibrationFactor[3] = 0.023;  //AFP 10-29-22
  EEPROMData.CWPowerCalibrationFactor[4] = 0.038;  //AFP 10-29-22
  EEPROMData.CWPowerCalibrationFactor[5] = 0.052;  //AFP 10-29-22
  EEPROMData.CWPowerCalibrationFactor[6] = 0.051;  //AFP 10-29-22
  EEPROMData.CWPowerCalibrationFactor[7] = 0.028;  //AFP 10-29-22
  EEPROMData.CWPowerCalibrationFactor[8] = 0.028;  //AFP 10-29-22
  EEPROMData.CWPowerCalibrationFactor[9] = 0.028;  //V12HW
  if (NUMBER_OF_BANDS > 10) {
    EEPROMData.CWPowerCalibrationFactor[10] = 0.028;  //V12HW
    EEPROMData.CWPowerCalibrationFactor[11] = 0.028;  //V12HW
    EEPROMData.CWPowerCalibrationFactor[12] = 0.028;  //V12HW
    EEPROMData.CWPowerCalibrationFactor[13] = 0.028;  //V12HW
    EEPROMData.CWPowerCalibrationFactor[14] = 0.028;  //V12HW
    EEPROMData.CWPowerCalibrationFactor[15] = 0.028;  //V12HW
    EEPROMData.CWPowerCalibrationFactor[16] = 0.028;  //V12HW
    EEPROMData.CWPowerCalibrationFactor[17] = 0.028;  //V12HW
  }


  EEPROMData.SSBPowerCalibrationFactor[0] = 0.017;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[1] = 0.017;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[2] = 0.017;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[3] = 0.019;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[4] = 0.017;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[5] = 0.019;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[6] = 0.021;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[7] = 0.020;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[8] = 0.022;  //AFP 10-29-22
  EEPROMData.SSBPowerCalibrationFactor[9] = 0.022;  //V12HW
  if (NUMBER_OF_BANDS > 10) {
    EEPROMData.SSBPowerCalibrationFactor[10] = 0.022;  //V12HW
    EEPROMData.SSBPowerCalibrationFactor[11] = 0.022;  //V12HW
    EEPROMData.SSBPowerCalibrationFactor[12] = 0.022;  //V12HW
    EEPROMData.SSBPowerCalibrationFactor[13] = 0.022;  //V12HW
    EEPROMData.SSBPowerCalibrationFactor[14] = 0.022;  //V12HW
    EEPROMData.SSBPowerCalibrationFactor[15] = 0.022;  //V12HW
    EEPROMData.SSBPowerCalibrationFactor[16] = 0.022;  //V12HW
    EEPROMData.SSBPowerCalibrationFactor[17] = 0.022;  //V12HW
  }

  // simplified - V12HW
  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    EEPROMData.IQAmpCorrectionFactor[i] = 1.0;
    EEPROMData.IQPhaseCorrectionFactor[i] = 0.0;
    EEPROMData.IQXAmpCorrectionFactor[i] = 1.0;
    EEPROMData.IQXPhaseCorrectionFactor[i] = 0.0;
    EEPROMData.IQXRecAmpCorrectionFactor[i] = 1.0;
    EEPROMData.IQXRecPhaseCorrectionFactor[i] = 0.0;
  }

  EEPROMData.favoriteFreqs[0] = 3560000L;  // These are CW/SSB calling frequencies for HF bands
  EEPROMData.favoriteFreqs[1] = 5300000L;
  EEPROMData.favoriteFreqs[2] = 7030000L;
  EEPROMData.favoriteFreqs[3] = 7200000L;
  EEPROMData.favoriteFreqs[4] = 10100000L;
  EEPROMData.favoriteFreqs[5] = 14200000L;
  EEPROMData.favoriteFreqs[6] = 21060000L;
  EEPROMData.favoriteFreqs[7] = 21285000L;
  EEPROMData.favoriteFreqs[8] = 28060000L;
  EEPROMData.favoriteFreqs[9] = 28365000L;
  EEPROMData.favoriteFreqs[10] = 50100000L;
  EEPROMData.favoriteFreqs[11] = 5000000L;
  EEPROMData.favoriteFreqs[12] = 10000000L;
  EEPROMData.favoriteFreqs[13] = 15000000L;


#if defined(ITU_REGION) && ITU_REGION == 1
  EEPROMData.lastFrequencies[0][0] = 3560000L;   // 80 CW
  EEPROMData.lastFrequencies[1][0] = 5300000L;   // 60 CW
  EEPROMData.lastFrequencies[2][0] = 7200000L;   // 40
  EEPROMData.lastFrequencies[3][0] = 10100000L;  // 30
  EEPROMData.lastFrequencies[4][0] = 14060000L;  // 20
  EEPROMData.lastFrequencies[5][0] = 18096000L;  // 17
  EEPROMData.lastFrequencies[6][0] = 21060000L;  // 15
  EEPROMData.lastFrequencies[7][0] = 24906000L;  // 12
  EEPROMData.lastFrequencies[8][0] = 28060000L;  // 10
  EEPROMData.lastFrequencies[9][0] = 50100000L;  // 6
#else
  EEPROMData.lastFrequencies[0][0] = 3560000L;   // 80 CW
  EEPROMData.lastFrequencies[1][0] = 5300000L;   // 60 CW
  EEPROMData.lastFrequencies[2][0] = 7200000L;   // 40
  EEPROMData.lastFrequencies[3][0] = 10100000L;  // 30
  EEPROMData.lastFrequencies[4][0] = 14060000L;  // 20
  EEPROMData.lastFrequencies[5][0] = 18096000L;  // 17
  EEPROMData.lastFrequencies[6][0] = 21060000L;  // 15
  EEPROMData.lastFrequencies[7][0] = 24906000L;  // 12
  EEPROMData.lastFrequencies[8][0] = 28060000L;  // 10
  EEPROMData.lastFrequencies[9][0] = 50100000L;
#endif

  EEPROMData.lastFrequencies[0][1] = 3560000L;   // 80 CW
  EEPROMData.lastFrequencies[1][1] = 5300000L;   // 60 CW
  EEPROMData.lastFrequencies[2][1] = 7200000L;   // 40
  EEPROMData.lastFrequencies[3][1] = 10100000L;  // 30
  EEPROMData.lastFrequencies[4][1] = 14060000L;  // 20
  EEPROMData.lastFrequencies[5][1] = 18096000L;  // 17
  EEPROMData.lastFrequencies[6][1] = 21060000L;  // 15
  EEPROMData.lastFrequencies[7][1] = 24906000L;  // 12
  EEPROMData.lastFrequencies[8][1] = 28060000L;  // 10
  EEPROMData.lastFrequencies[9][1] = 50100000L;


  EEPROMData.centerFreq = 7150000;

  strncpy(EEPROMData.mapFileName, MAP_FILE_NAME, 50);
  strncpy(EEPROMData.myCall, MY_CALL, 10);
  strncpy(EEPROMData.myTimeZone, MY_TIMEZONE, 10);
  EEPROMData.separationCharacter = (char)FREQ_SEP_CHARACTER;  //   JJP  7/25/23

  EEPROMData.paddleFlip = paddleFlip;
  EEPROMData.sdCardPresent = 0;  //   JJP  7/18/23

  EEPROMData.myLat = MY_LAT;
  EEPROMData.myLong = MY_LON;

  for (int i = 0; i < NUMBER_OF_BANDS; i++) {
    EEPROMData.currentNoiseFloor[i] = 0;
  }
  EEPROMData.compressorFlag = 0;  // Off by default JJP  8/28/23

  EEPROMData.receiveEQFlag = 0;  // JJP 2/29/2024
  EEPROMData.xmitEQFlag = 0;
  EEPROMData.CWToneIndex = 0;

  EEPROMData.TransmitPowerLevelCW = 0.0;
  EEPROMData.TransmitPowerLevelSSB = 0.0;
  //EEPROMData.SWR_R_Offset2 = 0;
}

#if !defined(USE_JSON)
/*****
  Purpose: Read the EEPROM data from the SD card and assign it to the
           proper EEPROM members.

  Parameter list:
    struct config_t e[]       pointer to the EEPROM structure

  Return value;
    int                         0 unsuccessful, 1 ok
*****/
FLASHMEM int CopySDToEEPROM() {
  char character;
  char line[150];
  char *target;
  char temp[150];
  int index = 0;
  int lineCount = 0;

  if (!SD.begin(chipSelect)) {  // SD failed
    // don't do anything more:
    return 0;  // Go home and report it
  }

  File file = SD.open("SDEEPROMData.txt", FILE_READ);  // Try to open file...
  if (file == 0) {
    return 0;  // Could not get a file handle, so go home and report it
  }
  //  while (file.available() > 0) {
  while (true) {
    /*
    Serial.printf("lineCount = %d\n", lineCount);     // Greg's debug code
    if (lineCount == 100) {
      Serial.printf("Hit end of file!\n"));
      break;
    }
*/
    while (true) {
      character = file.read();
      if (character == EOF || lineCount > MAX_SD_ITEMS) {
        file.close();
        EEPROM.put(EEPROM_BASE_ADDRESS, EEPROMData);  // KF5N
        return 1;
      }
      line[index++] = character;
      if ((character == '\n')) {       // Have we read the complete line?
        line[index - 2] = '\0';        // Overwrite newline and semicolon, we have the complete line as string
        target = strstr(line, " = ");  // Look for token
        strcpy(temp, target + 3);      // Skip past the token
        break;
      }
    }
    index = 0;

    switch (lineCount) {                           // This is a hack, but it works
      case 0:                                      // Version
        strcpy(EEPROMData.versionSettings, temp);  // Real assignment from header file
        break;
      case 1:
        EEPROMData.AGCMode = atoi(temp);
        break;
      case 2:
        EEPROMData.audioVolume = atoi(temp);
        break;
      case 3:
        EEPROMData.rfGainAllBands = atoi(temp);
        break;
      case 4:
        EEPROMData.spectrumNoiseFloor = atoi(temp);
        break;
      case 5:
        EEPROMData.tuneIndex = atoi(temp);
        break;
      case 6:
        EEPROMData.stepFineTune = atol(temp);
        break;
      case 7:
        EEPROMData.powerLevel = atoi(temp);
        break;
      case 8:
        EEPROMData.xmtMode = atoi(temp);
        break;
      case 9:
        EEPROMData.nrOptionSelect = (int8_t)atoi(temp);
        break;
      case 10:
        EEPROMData.currentScale = (uint16_t)atoi(temp);
        break;
      case 11:
        EEPROMData.spectrum_zoom = (int32_t)atol(temp);
        break;
      case 12:
        EEPROMData.spectrum_display_scale = atof(temp);
        break;
      case 13:
        EEPROMData.CWFilterIndex = atoi(temp);
        break;
      case 14:
        EEPROMData.paddleDit = atoi(temp);
        break;
      case 15:
        EEPROMData.paddleDah = atoi(temp);
        break;
      case 16:
        EEPROMData.decoderFlag = atoi(temp);
        break;
      case 17:
        EEPROMData.keyType = atoi(temp);
        break;
      case 18:
        EEPROMData.currentWPM = atoi(temp);
        break;
      case 19:
        EEPROMData.sidetoneVolume = atof(temp);
        break;
      case 20:
        EEPROMData.cwTransmitDelay = atol(temp);
        break;
      case 21:
        EEPROMData.activeVFO = (int16_t)atoi(temp);
        break;
      case 22:
        EEPROMData.freqIncrement = atoi(temp);
        break;
      case 23:
        EEPROMData.currentBand = atoi(temp);
        break;
      case 24:
        EEPROMData.currentBandA = atoi(temp);
        break;
      case 25:
        EEPROMData.currentBandB = atoi(temp);
        break;
      case 26:
        EEPROMData.currentFreqA = atol(temp);
        break;
      case 27:
        EEPROMData.currentFreqB = atol(temp);
        break;
      case 28:
        EEPROMData.freqCorrectionFactor = (long)atol(temp);
        break;

      case 29:
        EEPROMData.equalizerRec[0] = atoi(temp);
        break;
      case 30:
        EEPROMData.equalizerRec[1] = atoi(temp);
        break;
      case 31:
        EEPROMData.equalizerRec[2] = atoi(temp);
        break;
      case 32:
        EEPROMData.equalizerRec[3] = atoi(temp);
        break;
      case 33:
        EEPROMData.equalizerRec[4] = atoi(temp);
        break;
      case 34:
        EEPROMData.equalizerRec[5] = atoi(temp);
        break;
      case 35:
        EEPROMData.equalizerRec[6] = atoi(temp);
        break;
      case 36:
        EEPROMData.equalizerRec[7] = atoi(temp);
        break;
      case 37:
        EEPROMData.equalizerRec[8] = atoi(temp);
        break;
      case 38:
        EEPROMData.equalizerRec[9] = atoi(temp);
        break;
      case 39:
        EEPROMData.equalizerRec[10] = atoi(temp);
        break;

      case 40:
        EEPROMData.equalizerRec[11] = atoi(temp);
        break;
      case 41:
        EEPROMData.equalizerRec[12] = atoi(temp);
        break;
      case 42:
        EEPROMData.equalizerRec[13] = atoi(temp);
        break;
      case 43:
        EEPROMData.equalizerXmt[0] = atoi(temp);
        break;
      case 44:
        EEPROMData.equalizerXmt[1] = atoi(temp);
        break;
      case 45:
        EEPROMData.equalizerXmt[2] = atoi(temp);
        break;
      case 46:
        EEPROMData.equalizerXmt[3] = atoi(temp);
        break;
      case 47:
        EEPROMData.equalizerXmt[4] = atoi(temp);
        break;
      case 48:
        EEPROMData.equalizerXmt[5] = atoi(temp);
        break;
      case 49:
        EEPROMData.equalizerXmt[6] = atoi(temp);
        break;
      case 50:
        EEPROMData.equalizerXmt[7] = atoi(temp);
        break;
      case 51:
        EEPROMData.equalizerXmt[8] = atoi(temp);
        break;
      case 52:
        EEPROMData.equalizerXmt[9] = atoi(temp);
        break;
      case 53:
        EEPROMData.equalizerXmt[10] = atoi(temp);
        break;
      case 54:
        EEPROMData.equalizerXmt[11] = atoi(temp);
        break;
      case 55:
        EEPROMData.equalizerXmt[12] = atoi(temp);
        break;
      case 56:
        EEPROMData.equalizerXmt[13] = atoi(temp);
        break;

      case 57:
        EEPROMData.currentMicThreshold = atoi(temp);
        break;
      case 58:
        EEPROMData.currentMicCompRatio = atof(temp);
        break;
      case 59:
        EEPROMData.currentMicAttack = atof(temp);
      case 60:
        EEPROMData.currentMicRelease = atof(temp);
        break;
      case 61:
        EEPROMData.currentMicGain = atoi(temp);
        break;
      case 62:
        EEPROMData.switchValues[0] = atoi(temp);
        break;
      case 63:
        EEPROMData.switchValues[1] = atoi(temp);
        break;
      case 64:
        EEPROMData.switchValues[2] = atoi(temp);
        break;
      case 65:
        EEPROMData.switchValues[3] = atoi(temp);
        break;
      case 66:
        EEPROMData.switchValues[4] = atoi(temp);
        break;
      case 67:
        EEPROMData.switchValues[5] = atoi(temp);
        break;
      case 68:
        EEPROMData.switchValues[6] = atoi(temp);
        break;
      case 69:
        EEPROMData.switchValues[7] = atoi(temp);
        break;
      case 70:
        EEPROMData.switchValues[8] = atoi(temp);
        break;
      case 71:
        EEPROMData.switchValues[9] = atoi(temp);
        break;
      case 72:
        EEPROMData.switchValues[10] = atoi(temp);
        break;
      case 73:
        EEPROMData.switchValues[11] = atoi(temp);
        break;
      case 74:
        EEPROMData.switchValues[12] = atoi(temp);
        break;
      case 75:
        EEPROMData.switchValues[13] = atoi(temp);
        break;
      case 76:
        EEPROMData.switchValues[14] = atoi(temp);
        break;
      case 77:
        EEPROMData.switchValues[15] = atoi(temp);
        break;
      case 78:
        EEPROMData.switchValues[16] = atoi(temp);
        break;
      case 79:
        EEPROMData.switchValues[17] = atoi(temp);
        break;
      case 80:
        EEPROMData.LPFcoeff = atof(temp);
        break;
      case 81:
        EEPROMData.NR_PSI = atof(temp);
        break;
      case 82:
        EEPROMData.NR_alpha = atof(temp);
        break;
      case 83:
        EEPROMData.NR_beta = atof(temp);
        break;
      case 84:
        EEPROMData.omegaN = atof(temp);
        break;
      case 85:
        EEPROMData.pll_fmax = atof(temp);
        break;
      case 86:
        EEPROMData.powerOutCW[0] = atof(temp);
        break;
      case 87:
        EEPROMData.powerOutCW[1] = atof(temp);
        break;
      case 88:
        EEPROMData.powerOutCW[2] = atof(temp);
        break;
      case 89:
        EEPROMData.powerOutCW[3] = atof(temp);
        break;
      case 90:
        EEPROMData.powerOutCW[4] = atof(temp);
        break;
      case 91:
        EEPROMData.powerOutCW[5] = atof(temp);
        break;
      case 92:
        EEPROMData.powerOutCW[6] = atof(temp);
        break;
      case 93:
        EEPROMData.powerOutSSB[0] = atof(temp);
        break;
      case 94:
        EEPROMData.powerOutSSB[1] = atof(temp);
        break;
      case 95:
        EEPROMData.powerOutSSB[2] = atof(temp);
        break;
      case 96:
        EEPROMData.powerOutSSB[3] = atof(temp);
        break;
      case 97:
        EEPROMData.powerOutSSB[4] = atof(temp);
        break;
      case 98:
        EEPROMData.powerOutSSB[5] = atof(temp);
        break;
      case 99:
        EEPROMData.powerOutSSB[6] = atof(temp);
        break;
      case 100:
        EEPROMData.CWPowerCalibrationFactor[0] = atof(temp);
        break;
      case 101:
        EEPROMData.CWPowerCalibrationFactor[1] = atof(temp);
        break;
      case 102:
        EEPROMData.CWPowerCalibrationFactor[2] = atof(temp);
        break;
      case 103:
        EEPROMData.CWPowerCalibrationFactor[3] = atof(temp);
        break;
      case 104:
        EEPROMData.CWPowerCalibrationFactor[4] = atof(temp);
        break;
      case 105:
        EEPROMData.CWPowerCalibrationFactor[5] = atof(temp);
        break;
      case 106:
        EEPROMData.CWPowerCalibrationFactor[6] = atof(temp);
        break;
      case 107:
        EEPROMData.SSBPowerCalibrationFactor[0] = atof(temp);
        break;
      case 108:
        EEPROMData.SSBPowerCalibrationFactor[1] = atof(temp);
        break;
      case 109:
        EEPROMData.SSBPowerCalibrationFactor[2] = atof(temp);
        break;
      case 110:
        EEPROMData.SSBPowerCalibrationFactor[3] = atof(temp);
        break;
      case 111:
        EEPROMData.SSBPowerCalibrationFactor[4] = atof(temp);
        break;
      case 112:
        EEPROMData.SSBPowerCalibrationFactor[5] = atof(temp);
        break;
      case 113:
        EEPROMData.SSBPowerCalibrationFactor[6] = atof(temp);
        break;
      case 114:
        EEPROMData.IQAmpCorrectionFactor[0] = atof(temp);
        break;
      case 115:
        EEPROMData.IQAmpCorrectionFactor[1] = atof(temp);
        break;
      case 116:
        EEPROMData.IQAmpCorrectionFactor[2] = atof(temp);
        break;
      case 117:
        EEPROMData.IQAmpCorrectionFactor[3] = atof(temp);
        break;
      case 118:
        EEPROMData.IQAmpCorrectionFactor[4] = atof(temp);
        break;
      case 119:
        EEPROMData.IQAmpCorrectionFactor[5] = atof(temp);
        break;
      case 120:
        EEPROMData.IQAmpCorrectionFactor[6] = atof(temp);
        break;
      case 121:
        EEPROMData.IQPhaseCorrectionFactor[0] = atof(temp);
        break;
      case 122:
        EEPROMData.IQPhaseCorrectionFactor[1] = atof(temp);
        break;
      case 123:
        EEPROMData.IQPhaseCorrectionFactor[2] = atof(temp);
        break;
      case 124:
        EEPROMData.IQPhaseCorrectionFactor[3] = atof(temp);
        break;
      case 125:
        EEPROMData.IQPhaseCorrectionFactor[4] = atof(temp);
        break;
      case 126:
        EEPROMData.IQPhaseCorrectionFactor[5] = atof(temp);
        break;
      case 127:
        EEPROMData.IQPhaseCorrectionFactor[6] = atof(temp);
        break;
      case 128:
        EEPROMData.IQXAmpCorrectionFactor[0] = atof(temp);
        break;
      case 129:
        EEPROMData.IQXAmpCorrectionFactor[1] = atof(temp);
        break;
      case 130:
        EEPROMData.IQXAmpCorrectionFactor[2] = atof(temp);
        break;
      case 131:
        EEPROMData.IQXAmpCorrectionFactor[3] = atof(temp);
        break;
      case 132:
        EEPROMData.IQXAmpCorrectionFactor[4] = atof(temp);
        break;
      case 133:
        EEPROMData.IQXAmpCorrectionFactor[5] = atof(temp);
        break;
      case 134:
        EEPROMData.IQXAmpCorrectionFactor[6] = atof(temp);
        break;
      case 135:
        EEPROMData.IQXPhaseCorrectionFactor[0] = atof(temp);
        break;
      case 136:
        EEPROMData.IQXPhaseCorrectionFactor[1] = atof(temp);
        break;
      case 137:
        EEPROMData.IQXPhaseCorrectionFactor[2] = atof(temp);
        break;
      case 138:
        EEPROMData.IQXPhaseCorrectionFactor[3] = atof(temp);
        break;
      case 139:
        EEPROMData.IQXPhaseCorrectionFactor[4] = atof(temp);
        break;
      case 140:
        EEPROMData.IQXPhaseCorrectionFactor[5] = atof(temp);
        break;
      case 141:
        EEPROMData.IQXPhaseCorrectionFactor[6] = atof(temp);
        break;
      case 142:
        EEPROMData.favoriteFreqs[0] = atol(temp);
        break;
      case 143:
        EEPROMData.favoriteFreqs[1] = atol(temp);
        break;
      case 144:
        EEPROMData.favoriteFreqs[2] = atol(temp);
        break;
      case 145:
        EEPROMData.favoriteFreqs[3] = atol(temp);
        break;
      case 146:
        EEPROMData.favoriteFreqs[4] = atol(temp);
        break;
      case 147:
        EEPROMData.favoriteFreqs[5] = atol(temp);
        break;
      case 148:
        EEPROMData.favoriteFreqs[6] = atol(temp);
        break;
      case 149:
        EEPROMData.favoriteFreqs[7] = atol(temp);
        break;
      case 150:
        EEPROMData.favoriteFreqs[8] = atol(temp);
        break;
      case 151:
        EEPROMData.favoriteFreqs[9] = atol(temp);
        break;
      case 152:
        EEPROMData.favoriteFreqs[10] = atol(temp);
        break;
      case 153:
        EEPROMData.favoriteFreqs[11] = atol(temp);
        break;
      case 154:
        EEPROMData.favoriteFreqs[12] = atol(temp);
        break;
      case 155:
        EEPROMData.lastFrequencies[0][0] = atol(temp);
        break;
      case 156:
        EEPROMData.lastFrequencies[1][0] = atol(temp);
        break;
      case 157:
        EEPROMData.lastFrequencies[2][0] = atol(temp);
        break;
      case 158:
        EEPROMData.lastFrequencies[3][0] = atol(temp);
        break;
      case 159:
        EEPROMData.lastFrequencies[4][0] = atol(temp);
        break;
      case 160:
        EEPROMData.lastFrequencies[5][0] = atol(temp);
        break;
      case 161:
        EEPROMData.lastFrequencies[6][0] = atol(temp);
        break;
      case 162:
        EEPROMData.lastFrequencies[0][1] = atol(temp);
        break;
      case 163:
        EEPROMData.lastFrequencies[1][1] = atol(temp);
        break;
      case 164:
        EEPROMData.lastFrequencies[2][1] = atol(temp);
        break;
      case 165:
        EEPROMData.lastFrequencies[3][1] = atol(temp);
        break;
      case 166:
        EEPROMData.lastFrequencies[4][1] = atol(temp);
        break;
      case 167:
        EEPROMData.lastFrequencies[5][1] = atol(temp);
        break;
      case 168:
        EEPROMData.lastFrequencies[6][1] = atol(temp);
        break;
      case 169:

        EEPROMData.centerFreq = atol(temp);
        break;
      case 170:
        strcpy(EEPROMData.mapFileName, temp);
        break;
      case 171:
        strcpy(EEPROMData.myCall, temp);
        break;
      case 172:
        strcpy(EEPROMData.myTimeZone, temp);
        break;
      case 173:
        EEPROMData.separationCharacter = atoi(temp);  //   JJP  7/25/23
        break;
      case 174:
        EEPROMData.paddleFlip = atoi(temp);
        break;
      case 175:
        EEPROMData.sdCardPresent = atoi(temp);  //   JJP  7/18/23
        break;
      case 176:
        EEPROMData.myLat = atof(temp);
        break;
      case 177:
        EEPROMData.myLong = atof(temp);
        break;
      case 178:
        EEPROMData.currentNoiseFloor[0] = atoi(temp);
        break;
      case 179:
        EEPROMData.currentNoiseFloor[1] = atoi(temp);
        break;
      case 180:
        EEPROMData.currentNoiseFloor[2] = atoi(temp);
        break;
      case 181:
        EEPROMData.currentNoiseFloor[3] = atoi(temp);
        break;
      case 182:
        EEPROMData.currentNoiseFloor[4] = atoi(temp);
        break;
      case 183:
        EEPROMData.currentNoiseFloor[5] = atoi(temp);
        break;
      case 184:
        EEPROMData.currentNoiseFloor[6] = atoi(temp);
        break;
      case 185:
        EEPROMData.compressorFlag = atoi(temp);  // JJP  8/28/23
        break;
      case 186:
        EEPROMData.receiveEQFlag = atoi("0");  // JJP 2/29/2024
        break;
      case 187:
        EEPROMData.xmitEQFlag = atoi("0");  // JJP 2/29/2024
        break;
      case 188:
        EEPROMData.CWToneIndex = atoi("2");

      case 189:
        EEPROMData.TransmitPowerLevelCW = atof(temp);
        break;
      case 190:
        EEPROMData.TransmitPowerLevelSSB = atof(temp);
        break;
      case 191:
        EEPROMDATA.SWR_R_Offset2 = atof(temp);
        break;
      default:
        Serial.print(F("Shouldn't be here: lineCount = "));
        Serial.println(lineCount);
        break;
    }
    //   }
    lineCount++;
  }

  file.close();
  //  EEPROM.put(0, EEPROMData);  // This rewrites the entire EEPROM struct as defined in SDT.h
  EEPROM.put(EEPROM_BASE_ADDRESS, EEPROMData);  // KF5N
                                                //  EEPROMShow();
                                                //  syncEEPROM = 1;  // SD EEPROM same as memory EEPROM  KF5N
  RedrawDisplayScreen();
  return 1;
}
#endif  // !USE_JSON



/*****
  Purpose: Converts EEPROMData members and value to ASCII

  Parameter list:
    File file             file handle for the SD file
    char *buffer          pointer to the EEPROMData member
    int val               the current value of the member
    int whatDataType      1 = int, 2 = long, 3 = float, 4 = string

  Return value;
    void

*****/
FLASHMEM void ConvertForEEPROM(File file, char *buffer, int val, int whatDataType) {
  char temp[10];

  temp[0] = '\0';
  switch (whatDataType) {
    case 1:  // int
      itoa(val, temp, DEC);
      break;
    case 2:  // long
      ltoa(val, temp, DEC);
      break;
    case 3:                      // float
      dtostrf(val, 9, 4, temp);  //Field of up to 9 digits with 4 decimal places
      break;
    case 4:
      strcpy(temp, EEPROMSetVersion());
      break;
    default:
#if defined(DEBUG)
      Serial.println("Error");
#endif
      break;
  }
  strcat(buffer, " = ");
  strcat(buffer, temp);
#if defined(DEBUG1)
  Serial.println(buffer);
#endif
  file.println(buffer);
  buffer[0] = '\0';
}


#if !defined(USE_JSON)
/*****
  Purpose: Writes the current values of the working variable
           to the SD card as SDEEPROMData.txt

  Parameter list:
    void

  Return value;
    int               0 = no write, 1 = write
*****/
int CopyEEPROMToSD() {
  char buffer[100];
  char temp[15];
  char digits[10];
  int i;

  File file = SD.open("SDEEPROMData.txt", O_RDWR);  // Get a file handle
  if (!file) {                                      // Can't open the file
    return 0;                                       // Go home
  }
  file.seek(0L);  // Reset to BOF so we don't append
  strcpy(buffer, "EEPROMData.versionSettings = ");
  strcat(buffer, EEPROMSetVersion());
  file.println(buffer);

  strcpy(buffer, "EEPROMData.AGCMode = ");
  itoa(AGCMode, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.audioVolume = ");
  itoa(audioVolume, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.rfGainAllBands = ");
  itoa(rfGainAllBands, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.spectrumNoiseFloor = ");
  itoa(spectrumNoiseFloor, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.tuneIndex = ");
  itoa(tuneIndex, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.stepFineTune = ");
  ltoa(stepFineTune, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.powerLevel = ");
  itoa(transmitPowerLevel, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.xmtMode = 0");
  itoa(xmtMode, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.nrOptionSelect = ");  // KF5N
  itoa(nrOptionSelect, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentScale = ");
  itoa(currentScale, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.spectrum_zoom = ");  // long data type  KF5N
  ltoa(spectrum_zoom, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.spectrum_display_scale = ");  // float data type
  dtostrf(spectrum_display_scale, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.CWFilterIndex = ");
  itoa(CWFilterIndex, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.paddleDit = ");
  itoa(paddleDit, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.paddleDah = ");
  itoa(paddleDah, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.decoderFlag = ");
  itoa(decoderFlag, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.keyType = ");
  itoa(keyType, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentWPM = ");
  itoa(currentWPM, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);

  strcpy(buffer, "EEPROMData.sidetoneVolume = ");  // float data type
  dtostrf(sidetoneVolume, 6, 4, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.cwTransmitDelay = ");  // long data type
  ltoa(cwTransmitDelay, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.activeVFO = 0");
  itoa(activeVFO, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.freqIncrement = ");
  itoa(freqIncrement, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);

  strcpy(buffer, "EEPROMData.currentBand = ");
  itoa(currentBand, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentBandA = ");
  itoa(currentBandA, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentBandB = ");
  itoa(currentBandB, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentFreqA = ");  // long data type
  ltoa(currentFreqA, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentFreqB = ");  // long data type
  ltoa(currentFreqB, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.freqCorrectionFactor = ");  // long data type
  ltoa(freqCorrectionFactor, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);

  for (i = 0; i < EQUALIZER_CELL_COUNT; i++) {
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.equalizerRec[");  // long data type
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    itoa(recEQ_Level[i], temp, DEC);
    strcat(buffer, temp);
    file.println(buffer);
  }
  for (i = 0; i < EQUALIZER_CELL_COUNT; i++) {
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.equalizerXmt[");  // long data type
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    itoa(xmtEQ_Level[i], temp, DEC);
    strcat(buffer, temp);
    file.println(buffer);
  }
  strcpy(buffer, "EEPROMData.currentMicThreshold = ");
  itoa(currentMicThreshold, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentMicCompRatio = ");  // float data type
  dtostrf(currentMicCompRatio, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentMicAttack = ");  // float data type
  dtostrf(currentMicAttack, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentMicRelease = ");  // float data type
  dtostrf(currentMicRelease, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.currentMicGain = ");
  itoa(currentMicGain, temp, DEC);
  strcat(buffer, temp);
  file.println(buffer);

  for (i = 0; i < NUMBER_OF_SWITCHES; i++) {  // switch values
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.switchValues[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    itoa(switchThreshholds[i], temp, DEC);
    strcat(buffer, temp);
    file.println(buffer);
  }

  strcpy(buffer, "EEPROMData.LPFcoeff = ");  // float data type
  dtostrf(LPFcoeff, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EPROMData.NR_PSI = ");  // float data type
  dtostrf(NR_PSI, 6, 1, temp);            // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.NR_alpha = ");  // float data type
  dtostrf(NR_alpha, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.NR_beta = ");  // float data type
  dtostrf(NR_beta, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.omegaN = ");  // float data type
  dtostrf(omegaN, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);
  strcpy(buffer, "EEPROMData.pll_fmax = ");  // float data type
  dtostrf(pll_fmax, 6, 1, temp);             // Field of up to 6 digits with 1 decimal place
  strcat(buffer, temp);
  file.println(buffer);

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // powerOutCW
    itoa(i, digits, DEC);
    strcpy(buffer, "powerOutCW[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    dtostrf(EEPROMData.powerOutCW[i], 6, 3, temp);
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // powerOutSSB
    itoa(i, digits, DEC);
    strcpy(buffer, "powerOutSSB[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    dtostrf(EEPROMData.powerOutSSB[i], 6, 3, temp);
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // CW Calibration factor
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.CWPowerCalibrationFactor[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    dtostrf(EEPROMData.CWPowerCalibrationFactor[i], 6, 3, temp);
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // SSB Calibration factor
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.SSBPowerCalibrationFactor[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    dtostrf(EEPROMData.SSBPowerCalibrationFactor[i], 6, 3, temp);
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // IQ Amp Correction factor
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.IQAmpCorrectionFactor[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    dtostrf(EEPROMData.IQAmpCorrectionFactor[i], 6, 3, temp);
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // IQ Phase Correction factor
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.IQPhaseCorrectionFactor[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    dtostrf(EEPROMData.IQPhaseCorrectionFactor[i], 6, 3, temp);
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // IQX Amp Correction factor
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.IQXAmpCorrectionFactor[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    dtostrf(EEPROMData.IQXAmpCorrectionFactor[i], 6, 3, temp);
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // IQX Phase Correction factor
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.IQXPhaseCorrectionFactor[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    dtostrf(EEPROMData.IQXPhaseCorrectionFactor[i], 6, 3, temp);
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < MAX_FAVORITES; i++) {  // Last frequencies
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.favoriteFreqs[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    ltoa(EEPROMData.favoriteFreqs[i], temp, DEC);  // long
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // Last frequencies
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.lastFrequencies[");
    strcat(buffer, digits);
    strcat(buffer, "][0] = ");
    ltoa(EEPROMData.lastFrequencies[i][0], temp, DEC);  // long
    strcat(buffer, temp);
    file.println(buffer);
  }

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // Last frequencies
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.lastFrequencies[");
    strcat(buffer, digits);
    strcat(buffer, "][1] = ");
    ltoa(EEPROMData.lastFrequencies[i][1], temp, DEC);  // long
    strcat(buffer, temp);
    file.println(buffer);
  }

  strcpy(buffer, "EEPROMData.centerFreq = ");  // Center freq
  ltoa(EEPROMData.centerFreq, temp, DEC);      // long
  strcat(buffer, temp);
  file.println(buffer);

  //                                                      JJP 7/3/23
  strcpy(buffer, "EEPROMData.mapFileName = ");  // Map file name
  strncat(buffer, EEPROMData.mapFileName, 50);
  file.println(buffer);

  strcpy(buffer, "EEPROMData.myCall = ");  // Call
  strncat(buffer, EEPROMData.myCall, 10);
  file.println(buffer);

  strcpy(buffer, "EEPROMData.myTimeZone = ");  // Time zone
  strncat(buffer, EEPROMData.myTimeZone, 10);
  file.println(buffer);

  strcpy(buffer, "EEPROMData.separationCharacter = ");  // Separation character in freq display
  itoa(EEPROMData.separationCharacter, digits, DEC);
  strcat(buffer, digits);
  file.println(buffer);

  itoa(paddleFlip, digits, DEC);  // Paddle flip
  strcpy(buffer, "EEPROMData.paddleFlip = ");
  strcat(buffer, digits);
  file.println(buffer);

  itoa(sdCardPresent, temp, DEC);  // SD card
  strcpy(buffer, "EEPROMData.sdCardPresent = ");
  strcat(buffer, temp);
  file.println(buffer);

  strcpy(buffer, "EEPROMData.myLat = ");  // Latitude
  dtostrf(EEPROMData.myLat, 6, 4, temp);
  strcat(buffer, temp);
  file.println(buffer);

  strcpy(buffer, "EEPROMData.myLong = ");  // Longitude
  dtostrf(EEPROMData.myLong, 6, 4, temp);
  strcat(buffer, temp);
  file.println(buffer);

  for (i = 0; i < NUMBER_OF_BANDS; i++) {  // Noise floor
    itoa(i, digits, DEC);
    strcpy(buffer, "EEPROMData.currentNoiseFloor[");
    strcat(buffer, digits);
    strcat(buffer, "] = ");
    itoa(EEPROMData.currentNoiseFloor[i], temp, DEC);
    strcat(buffer, temp);
    file.println(buffer);
  }
  itoa(EEPROMData.compressorFlag, temp, DEC);  // JJP  8/28/23
  strcpy(buffer, "EEPROMData.compressorFlag = ");
  strcat(buffer, temp);
  file.println(buffer);

  itoa(EEPROMData.receiveEQFlag, temp, DEC);  // JJP  2/29/24
  strcpy(buffer, "EEPROMData.receiveEQFlag = ");
  strcat(buffer, temp);
  file.println(buffer);

  itoa(EEPROMData.xmitEQFlag, temp, DEC);  // JJP  2/29/24
  strcpy(buffer, "EEPROMData.xmitEQFlag = ");
  strcat(buffer, temp);
  file.println(buffer);

  itoa(EEPROMData.CWToneIndex, temp, DEC);  // JJP  2/29/24
  strcpy(buffer, "EEPROMData.CWToneIndex = ");
  strcat(buffer, temp);
  file.println(buffer);


  strcpy(buffer, "TransmitPowerLevelCW = ");  // Latitude
  dtostrf(EEPROMData.TransmitPowerLevelCW, 6, 4, temp);
  strcat(buffer, temp);
  file.println(buffer);

  strcpy(buffer, "TransmitPowerLevelSSB = ");  // Latitude
  dtostrf(EEPROMData.TransmitPowerLevelSSB, 6, 4, temp);
  strcat(buffer, temp);
  file.println(buffer);

  strcpy(buffer, "SWR_R_Offset2 = ");  // Latitude
  dtostrf(EEPROMData.SWR_R_Offset2, 6, 4, temp);
  strcat(buffer, temp);
  file.println(buffer);

  file.write(EOF);  // EOF marker
  file.close();
  RedrawDisplayScreen();
  return 1;
}
#endif  // !USE_JSON

/*****
  Purpose: See if the EEPROM has ever been set

  Parameter list:
    void

  Return value;
    int               1 = used before, 0 = nope
*****/
int ValidEEPROMData() {
  int val = EEPROMData.switchValues[0];
  if (val > 0 && val < 1023)
    return VALID_EEPROM_DATA;  // return 1
  else
    return INVALID_EEPROM_DATA;  // return 0
}

/*****
  Purpose: Update the version number only in EEPROM

  Parameter list:
    void

  Return value;
    void
*****/
void UpdateEEPROMVersionNumber() {
  strcpy(EEPROMData.versionSettings, EEPROMSetVersion());  // Copy the latest version to EEPROM
}

/*****
  Purpose: Reads the SD EEPROM data and writes it to the Serial object

  Parameter list:
    void

  Return value;
    int               0 = SD is untouched, 1 = has data
*****/
void SDEEPROMDump() {
  //  char c;
  //  int lines = 0;

  if (!SD.begin(chipSelect)) {
    Serial.print(F("SD card cannot be initialized."));
  }
  // open the file.
  Serial.println(F("----- Start SD EEPROM File Dump ------"));
#ifdef USE_JSON
  printFile("/config.txt");
#else
  File dataFile = SD.open("SDEEPROMData.txt");
  // if the file is available, read from it:
  if (dataFile) {
    //     while (dataFile.available()) {
    //      Serial.write(dataFile.read());
    while (true) {
      c = dataFile.read();
      if (c == 26 || lines > MAX_SD_ITEMS) {  // EOF Marker
        break;
      }
      if (c == '\n') {
        lines++;
      }
      Serial.print(c);
    }
    dataFile.close();
  } else {
    Serial.println(F("error opening SDEEPROMData.txt"));
  }
#endif
}


/*****
  Purpose: Clears the first 1K of emulated EEPROM to 0xff

  Parameter list:
    void

  Return value;
    void
*****/
void ClearEEPROM() {
  int i;
  for (i = 0; i < 1000; i++) {
    EEPROM.write(i, 0xFF);
  }
}

/*****
  Purpose: Read the EEPROM from: a) EEPROM memory, b) SD card memory, or c) defaults

  Parameter list:
    void

  Return value;
    void
*****/
void EEPROMStartup() {
  //  EEPROMSaveDefaults2();
  EEPROMRead();                                                       // Read current stored data
                                                                      //EEPROMShow();
  if (strcmp(EEPROMData.versionSettings, EEPROMSetVersion()) == 0) {  // Are the versions the same?
    return;                                                           // Yep. Go home and don't mess with the EEPROM
  }
  strcpy(EEPROMData.versionSettings, EEPROMSetVersion());  // Nope, this is a new the version, so copy new version title to EEPROM
  //                                                                     Check if calibration has not been done and/or switch values are wonky, okay to use defaults
  //                                                                     If the Teensy is unused, these EEPROM values are 0xFF or perhaps cleared to 0.
#if !defined(FRONTPANEL)
  if (switchThreshholds[9] < 440 || switchThreshholds[9] > 480) {
    EEPROMSaveDefaults2();     // At least give them some starting values
    switchThreshholds[9] = 0;  // This will force the next code block to set the switch values.
  }
  if (switchThreshholds[9] < 440 || switchThreshholds[9] > 480) {  // If the Teensy is unused, these EEPROM values are 0xFF or perhaps cleared to 0.
    SaveAnalogSwitchValues();                                      // In that case, we need to set the switch values.
  }
#endif  // FRONTPANEL
  //                                                                     If we get here, the switch values have been set, either previously or by the call to
  //                                                                     SaveAnalogSwitchValues() as has the rest of the EEPROM data. This avoids recalibration.

  EEPROMSaveDefaults2();


  EEPROM.put(0, EEPROMData);  // This rewrites the entire EEPROM struct as defined in SDT.h
  EEPROMRead();               // Read the EEPROM data, including new switch values. This also resets working variables

//  if (sdCardPresent) {  // If there's an SD card present...
//    CopyEEPROMToSD();   // ...copy the EEPROM data to the SD card, otherwise we're done  // Don't do this.  It could overwrite an existing file.  KF5N August 9, 2023
//  }
#ifdef DEBUG1
  SDEEPROMDump();  // Call this to observe EEPROM struct data
#endif
}
