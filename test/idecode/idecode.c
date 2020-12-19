//*****************************************************************************
// idecode.c
//
// This program reads icedrifter chunk data, reconstricts the original data
// record as it was built by the icedrifter code, saves that data to a file,
// and then decodes and prints the data.  Optionally, an email containing
// the reconstructed data record and the printed data can be sent out.
//
// It also has the ability to read in a reconstructed data record and print
// that data.
//
//*****************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "../icedrifter/icedrifter.h"
#include "../icedrifter/rockblock.h"

icedrifterData idData; // structure that defines the icedrifter record.

#define BUFF_SIZE 2048  // size of the buffer used to decode character data.
#define FILE_NAME_SIZE  1024  // size of buffers used for file names.
#define GPS_TIME_SIZE 16  // size of the buffer used to decode gps time and date.

bool mailResultsSwitch; // switch to indicate an email should be sent.
char emailAddress[256]; // email address to send the email to.

int getDataByChunk(char**, int);
void saveData(char* fileName);
int getDataByFile(char**);
int getDataByChar(char**, int);
void decodeData(char* fileName);
char convertCharToHex(char);
void convertBigEndianToLittleEndian(char* sPtr, int size);
float convertTempToC(short temp);
void printHelp(void);

//*****************************************************************************
//
// main entry point to program.  The options are evaluated and the proper
// routines called.
//
//*****************************************************************************

int main(int argc, char** argv) {
  int argIx;
  struct stat fileStat;

// we are expecting at least one argument.
  if (argc == 1) {
    printf("\nError: No arguments specified!!!\n\n");
    printHelp();
    exit(1);
  }

  argIx = 1;
  mailResultsSwitch = false;

// check the arguments and invoke the proper routines.
  if (argv[argIx][0] == '-') {
    while (1) {
      switch (argv[argIx][1]) {
        case 'm':
          mailResultsSwitch = true;
          ++argIx;
          strcpy(emailAddress, argv[argIx]);
          ++argIx;
          break;
        case 'c':
          if (stat(argv[argIx + 1], &fileStat) < 0) {
            printf("No files found.\n");
            return (0);
          }
          if (getDataByChunk(&argv[argIx + 1], argc - argIx - 1) != 0) {
            exit(1);
          }
          printf("Decode sucessful.\n");
          return (0);
        case 'f':
          if (stat(argv[argIx + 1], &fileStat) < 0) {
            printf("No file found.\n");
            exit(1);
          }
          if (getDataByFile(&argv[argIx + 1]) != 0) {
            exit(1);
          }
          return (0);
        case 'h':
          printHelp();
          exit(0);
        default:
          printf("Invalid switch!!!\n\n");
          printHelp();
          exit(1);
      }
    }
  } else {
    if (getDataByChar(&argv[1], argc - 1) != 0) {
      exit(1);
    }
  }
}

//*****************************************************************************
//
// getDataByChunk
//
// This routine reads in 1 to 3 chunk files, verifies that the data is all
// associated with one idrifterData record and then rebuilds the data record.
//
// It then decodes the data and send the data in a humand readable format
// to the console and writes out the idrifter data record to the current
// directory with the file name of Rockblock id number followed by a '-'
// followed by the date and time the sample was taken, and has an extension
// of '.dat'.
//
// Optionally, the decoded data and the idrifterData file can be sent by email
// to the email address specified.
//
// fnl is a pointer to the first chunk name specified on the command line.
//
// cnt is the number of files names speciried on the command line.
//
// returns 0 for good completion and non-zero if an error is detected.
//
//*****************************************************************************

int getDataByChunk(char** fnl, int cnt) {
  char** argIx;
  iceDrifterChunk* idcPtr;
  FILE* fd;
  char* wkPtr;
  char* fnPtr;
  int recordTime;
  int recordSize;
  int i;
  time_t tempTime;
  struct tm* timeInfo;
  bool firstTime;
  bool dashFound;
  bool zeroRecordFound;
  char fileBuffer[MAX_RECORD_LENGTH];
  char tempHold[FILE_NAME_SIZE];
  char fileName[FILE_NAME_SIZE];
  char datName[FILE_NAME_SIZE];
  char txtName[FILE_NAME_SIZE];
  char gpsTime[GPS_TIME_SIZE];

  wkPtr = (char*)&idData;
  argIx = fnl;
  firstTime = true;
  fileName[0] = 0;

  // Initialize the icedrifterData structure to zeros.

  for (i = 0; i < (BASE_RECORD_LENGTH + TEMP_DATA_SIZE + LIGHT_DATA_SIZE); ++i) {
    *wkPtr = 0;
    ++wkPtr;
  }

  zeroRecordFound = false;

  // Extract the Rockblock ID number from the file name and make sure
  // all IDs match.
  for (i = 0; i < cnt; ++i) {

    strcpy(tempHold, argIx[i]);
    wkPtr = tempHold;
    fnPtr = tempHold;
    dashFound = false;

    while (*wkPtr != 0) {
      // if the file specification contains a path we want to ignore that.
      if ((*wkPtr == '/') || (*wkPtr == '\\')) {
        fnPtr = wkPtr + 1;
      } else {
        if (*wkPtr == '-') {
          dashFound = true;
          *wkPtr = 0;
          if (fileName[0] == 0) {
            strcpy(fileName, fnPtr);
            printf("Processing data for Rockblock %s.\n", fileName);
            break;
          } else {
            // If the Rockblock ID's don't match we can not continue.
            if (strcmp(fileName, fnPtr) != 0) {
              printf("Error: File %d Rockblock name not equal previous file(s)!", i);
              printf("idecode terminating.\n");
              exit(1);
            }
            break;
          }
        }
      }
      ++wkPtr;
    }

    // If no dash was found we don't know what type of file this is
    // so we quit.
    if (dashFound == false) {
      printf("Error: Invalid Icedrifter file name = %s!", argIx[i]);
      printf("idecode terminating.\n");
      exit(1);
    }

    if ((fd = fopen(argIx[i], "r")) == NULL) {
      printf("Error opening input file %s!\n", argIx[0]);
      printf("idecode terminating.\n");
      fclose(fd);
      exit(1);
    }

    // Get the length of this chunk.
    fseek(fd, 0, SEEK_END);

    recordSize = ftell(fd);

    if ((recordSize == 0) || (recordSize > MAX_CHUNK_LENGTH)) {
      printf("Error: Record size zero or too long = %d!!!\n", recordSize);
      printf("idecode terminating.\n");
      fclose(fd);
      exit(1);
    }

    fseek(fd, 0, SEEK_SET);

    if (fread(fileBuffer, recordSize, 1, fd) == 0) {
      printf("Error reading data file!\n");
      printf("idecode terminating.\n");
      fclose(fd);
      exit(1);
    }

    idcPtr = (iceDrifterChunk*)&fileBuffer;

    // Check that is is really an icedrifter chunk.
    if (!((idcPtr->idcRecordType[0] == 'I') &&
          (idcPtr->idcRecordType[1] == 'D'))) {
      printf("Invalid record! Chunk header - not \"IDxx\"!\n");
      exit(1);
    }

    // Pick up the report time from the first chunk processed and check
    // that the other chunks have the same report time.
    if (firstTime) {
      recordTime = idcPtr->idcSendTime;
      firstTime = false;
    } else {
      if (idcPtr->idcSendTime != recordTime) {
        printf("Error: Sent time of record %d does not equal sent time of first record!\n", i + 1);
        printf("idecode terminating.\n");
        fclose(fd);
        exit(1);
      }
    }

    // Copy the chunk data to the proper location within the icedrifterData structure.
    if (idcPtr->idcRecordNumber == 0) {
      wkPtr = (char*)&idData;
      memmove(wkPtr, (char*)&idcPtr->idcBuffer, recordSize - CHUNK_HEADER_SIZE);
      zeroRecordFound = true;
    } else if (idcPtr->idcRecordNumber == 1) {
      wkPtr = (char*)&idData;
      wkPtr += MAX_CHUNK_DATA_LENGTH;
      memmove(wkPtr, (char*)&idcPtr->idcBuffer, recordSize - CHUNK_HEADER_SIZE);
    } else if (idcPtr->idcRecordNumber == 2) {
      wkPtr = (char*)&idData;
      wkPtr += (MAX_CHUNK_DATA_LENGTH * 2);
      memmove(wkPtr, (char*)&idcPtr->idcBuffer, recordSize - CHUNK_HEADER_SIZE);
    } else {
      printf("Invalid record number!!! Record number = %d!!!\n", idcPtr->idcRecordNumber);
      fclose(fd);
      exit(1);
    }

    fclose(fd);
  }

  // If no Zero record was found it doesn't make sense to continue.
  if (zeroRecordFound == false) {
    printf("Error: No record 0 found.  Can not continue!\n");
    printf("idecode terminating.\n");
    exit(1);
  }

  // The temperature and light probes return their data in big endien format so
  // we need to convert that data to little endien.
  convertBigEndianToLittleEndian((char*)&idData.idChainData, sizeof(idData.idChainData));

  // Build the file names that will be used to output the data.
  // The file names will be <Rockblock ID>-<report date and time>.
  strcpy(datName, fileName);
  strcat(datName, "-");
  tempTime = (time_t)idData.idGPSTime;
  timeInfo = gmtime(&tempTime);
  gpsTime[0] = 0;
  strftime(gpsTime, sizeof(gpsTime), "%Y%m%d%H%M%S", timeInfo);
  strcat(datName, gpsTime);
  strcpy(txtName, datName);
  strcat(datName, ".dat");
  strcat(txtName, ".txt");

  // Print the icedrifter data in human readable format to the .txt file.
  decodeData(txtName);

  // Save the data to disk.
  saveData(datName);

  // If the user wants to send this data out by email, use mutt to do it.
  if (mailResultsSwitch == true) {
    sprintf(tempHold, "mutt -s \"Decoded data for %s\" -a %s %s -- %s < %s",
            fileName, datName, txtName, emailAddress, txtName);

    if (system(tempHold) != 0) {
      printf("Error returned from mutt command!!!\n");
      printf("idecode terminating.\n");
      exit(1);
    }
  }
}

//*****************************************************************************
//
// saveData
//
// fileName is the name that will be given the file.
//
// returns nothing.
//
// This function writes the icedrifterData structure to disk.
//
//*****************************************************************************

void saveData(char* fileName) {
  FILE* fd;

  if ((fd = fopen(fileName, "w")) == NULL) {
    printf("Error opening input file %s!\n", fileName);
    printf("idecode terminating.\n");
    fclose(fd);
    exit(1);
  }

  if (fwrite((char*)&idData, sizeof(idData), 1, fd) != 1) {
    printf("Error writing output %s!\n", fileName);
    printf("idecode terminating.\n");
    fclose(fd);
    exit(1);
  }

  fclose(fd);
}

//*****************************************************************************
//
// getDataByFile
//
// fnl is a pointer to the arg list that contains the name of the file to print.
//
// Returns 0 for good completion or non-zero for failure.
//
// This function opens and reads the file name passed to it and then displays
// the data contained in the file in human readable format on sysout.
//
//*****************************************************************************

int getDataByFile(char** fnl) {
  size_t recordSize;
  int i;
  char** argIx;
  FILE* fd;
  char* wkPtr;

  wkPtr = (char*)&idData;
  argIx = fnl;

  // Initialize the icedrifterData structure.
  for (i = 0; i < (BASE_RECORD_LENGTH + TEMP_DATA_SIZE + LIGHT_DATA_SIZE); ++i) {
    *wkPtr - 0;
  }

  if ((fd = fopen(argIx[0], "r")) == NULL) {
    printf("Error opening input file %s!\n", argIx[0]);
    printf("idecode terminating.\n");
    fclose(fd);
    exit(1);
  }

  if ((fread((char*)&idData, sizeof(idData), 1, fd) == 0)) {
    printf("Error reading data file %s!\n", argIx[0]);
    printf("idecode2 terminating.\n");
    fclose(fd);
    exit(1);
  }

  fclose(fd);

  // Write the data in human readable forman to sysout.
  // Passing a NULL to the decodeData function tells it to write to sysout instead
  // of a disk file.
  decodeData(NULL);
  return (0);
}

//*****************************************************************************
//
// getDataByChar
//
// data: A pointer to an arg entry containing the character data to be
//       converted into an icedrifterData structure.
//
// cnt:  The number of arg entries to process.
//
// This function takes character data clipped out of the email sent from
// Rockblock.  The data is converted to hex data and copied into the structure.
// The structure is then printed in human readable format.  This feature is
// not documented in the help listing and is used mainly for testing.
//
//*****************************************************************************

int getDataByChar(char** data, int cnt) {
  char* cunkPtr;
  char* wkPtr;
  iceDrifterChunk* idcPtr;
  char* recPtr;
  int recNum;
  int dataIx;
  int i;
  int recLen;
  char hb1;
  char hb2;
  char buffHold;
  struct tm* timeInfo;
  uint32_t tempTime;
  uint32_t timeHold;
  bool gotDate;
  char buff[BUFF_SIZE];

  wkPtr = buff;
  gotDate = false;

  for (recNum = 0; recNum < cnt; ++recNum) {

    dataIx = 0;
    recLen = 0;

    if (*data[recNum] == 0) {
      break;
    }

    while (1) {
      hb1 = hb2 = 0;

      if (!((data[recNum][dataIx] == 0) ||
            (data[recNum][dataIx] == '\r') ||
            (data[recNum][dataIx] == '\n'))) {
        if ((hb1 = convertCharToHex(data[recNum][dataIx])) == 0xFF) {
          printf("\nInvalid hex charactor found at location %d!!!\n", dataIx);
          exit(1);
        }
      } else {
        break;
      }

      if (!((data[recNum][dataIx + 1] == 0) ||
            (data[recNum][dataIx + 1] == '\r') ||
            (data[recNum][dataIx + 1] == '\n'))) {
        if ((hb2 = convertCharToHex(data[recNum][dataIx + 1])) == 0xFF) {
          printf("\nInvalid hex charactor found at location %d!!!\n", (i + 1));
          exit(1);
        }
      }

      *wkPtr = (hb1 << 4) | hb2;
      ++wkPtr;
      ++recLen;
      dataIx += 2;
    }

    idcPtr = (iceDrifterChunk*)buff;

    if (!((idcPtr->idcRecordType[0] == 'I') && (idcPtr->idcRecordType[1] == 'D'))) {
      printf("Record ID not = \"ID\"!!!\n");
      exit(1);
    }

    if (gotDate) {
      if (idcPtr->idcSendTime != timeHold) {
        printf("Time of record %d does not equal other record!!!", recNum);
        exit(1);
      }
    } else {
      timeHold = idcPtr->idcSendTime;
      gotDate = true;
    }

    recPtr = (char*)&idData;

    if (idcPtr->idcRecordNumber == 1) {
      recPtr += sizeof(idcPtr->idcBuffer);
    } else if (idcPtr->idcRecordNumber == 2) {
      recPtr += sizeof(idcPtr->idcBuffer) * 2;
    }

    wkPtr = (char*)idcPtr->idcBuffer;
    memmove(recPtr, wkPtr, recLen);
  }

  convertBigEndianToLittleEndian((char*)&idData.idChainData, sizeof(idData.idChainData));
  decodeData(NULL);
}

//*****************************************************************************
//
// decodeData
//
// fileName: The file name to write the decoded data in human readable forman.
//           If fileName is NULL the data will be written to sysour.
//
// The function decodes and prints in human readable format the data contained
// in the icddrifterData structure.
//
//*****************************************************************************

void decodeData(char* fileName) {

  struct tm* timeInfo;
  time_t tempTime;
  int i;
  FILE* fd;
  uint32_t ltClear;
  uint8_t rgbRed;
  uint8_t rgbGreen;
  uint8_t rgbBlue;



  if (fileName == NULL) {
    fd = stdout;
  } else {
    if ((fd = fopen(fileName, "w")) == NULL) {
      printf("Error opening input file %s!\n", fileName);
      printf("idecode terminating.\n");
      fclose(fd);
      exit(1);
    }
  }

  tempTime = (time_t)idData.idLastBootTime;
  timeInfo = gmtime(&tempTime);
  fprintf(fd, "Last Boot:   %s", asctime(timeInfo));

  tempTime = (time_t)idData.idGPSTime;
  timeInfo = gmtime(&tempTime);
  fprintf(fd, "GPS time:    %s", asctime(timeInfo));

  if (idData.idcdError == 0) {
    fprintf(fd, "No errors found.\n");
  } else {
    fprintf(fd, "\nError(s) found!!!\n");
    if (idData.idcdError & TEMP_CHAIN_TIMEOUT_ERROR) {
      fprintf(fd, "*** Temperature chain timeout.\n");
    }
    if (idData.idcdError & TEMP_CHAIN_OVERRUN_ERROR) {
      fprintf(fd, "*** Temperature chain overrun.\n");
    }
    if (idData.idcdError & LIGHT_CHAIN_TIMEOUT_ERROR) {
      fprintf(fd, "*** Light chain timeout.\n");
    }
    if (idData.idcdError & LIGHT_CHAIN_OVERRUN_ERROR) {
      fprintf(fd, "*** Light chain overrun.\n");
    }
    fprintf(fd, "\n");
  }

  fprintf(fd, "Temp chain bytes received %d\n", idData.idTempByteCount);
  fprintf(fd, "Light chain bytes received %d\n\n", idData.idLightByteCount);

  fprintf(fd, "latitude:    %f\n", idData.idLatitude);
  fprintf(fd, "longitude:   %f\n", idData.idLongitude);
  fprintf(fd, "temperature: %f C\n", idData.idTemperature);
  fprintf(fd, "pressure:    %f Pa\n", idData.idPressure);

  if (idData.idSwitches & PROCESS_REMOTE_TEMP_SWITCH) {
    fprintf(fd, "remote temp: %f C\n\n", idData.idRemoteTemp);
  }

  if (idData.idSwitches & PROCESS_CHAIN_DATA_SWITCH) {
    for (i = 0; i < TEMP_SENSOR_COUNT; ++i) {
      fprintf(fd, "Chain temperature sensor %3d = %f\n", i, convertTempToC(idData.idChainData.cdTempData[i]));
    }

    fprintf(fd, "\n");

    for (i = 0; i < LIGHT_SENSOR_COUNT; ++i) {
      if (idData.idChainData.cdLightData[i][0] == 0) {
        rgbRed = rgbGreen = rgbBlue = 0;
      } else {
        ltClear = (uint32_t)idData.idChainData.cdLightData[i][0];
        rgbRed = (float)idData.idChainData.cdLightData[i][1] / ltClear * 255.0;
        rgbGreen = (float)idData.idChainData.cdLightData[i][2] / ltClear * 255.0;
        rgbBlue = (float)idData.idChainData.cdLightData[i][3] / ltClear * 255.0;
      }

      fprintf(fd, "Chain light sensor %2d = %5hu %5hu %5hu %5hu  RGB %3d %3d %3d\n", i,
              idData.idChainData.cdLightData[i][0], idData.idChainData.cdLightData[i][1], idData.idChainData.cdLightData[i][2], idData.idChainData.cdLightData[i][3],
              rgbRed, rgbGreen, rgbBlue);
    }
  }
  if (fileName != NULL) {
    fclose(fd);
  }
}

//*****************************************************************************
//
// convertCharToHex
//
// chr: a charactor to be converted from charactor formant ot hex format.
//
// This function takes a charactor in the range of 0123456789abcdef and
// converts it to a byte containing the hex value.
//
//*****************************************************************************

char convertCharToHex(char chr) {
  if ((chr >= '0') && (chr <= '9')) {
    return (chr - '0');
  } else if ((chr >= 'a') && (chr <= 'f')) {
    return ((chr - 'a') + 10);
  } else if ((chr >= 'A') && (chr <= 'F')) {
    return ((chr - 'A') + 10);
  }

  return (0xFF);
}

//*****************************************************************************
//
// convertBigEndianToLittleEndian
//
// sPtr: A pointer to the beginning of the area to convert.
//
// size: The number of bytes to convert.
//
// This function converts a list of halfwords from big endian to little endian
// by swapping the two bytes of the halfwords.
//
//*****************************************************************************
void convertBigEndianToLittleEndian(char* sPtr, int size) {
  char tmp;
  int i;

  for (i = 0; i < (size / 2); ++i) {
    tmp = *sPtr;
    *sPtr = *(sPtr + 1);
    *(sPtr + 1) = tmp;
    sPtr += 2;
  }
}
//*****************************************************************************
//
// convertTempToC
//
// temp: The sensor temperature to be converted to Celsius.
//
// returns: the sensor temperature in Celsius.
//
// Converts the sensor temperature data from it's native form to Celsius.
//
//*****************************************************************************
float convertTempToC(short temp) {

  //This formula is defined in the DSi8B20 sensor datasheet.
  if (temp & 0x8000) {
    return ((float)((temp & 0x7FFF) - 0x8000) / 128.0);
  }

  return ((float)temp / 128.0);
}

//*****************************************************************************
//
// Print out the following help information:
//
// Help for idecode2.
//
// idecode2 [-m <email address>] -c <file name list or *.bin>
// idecode2 -f <path and file name of a .dat file>
// idecode2 <character data from email(s) as a single string>
//
// -c Read one to three .bin chunk files, decode the data, display
//    human readable data on the console, write the human
//    readable data to a file with the file name of
//    <Rockblock id>-<yyyymmddhhmmss>.txt, and write the accumulated data
//    as an icedrifterData structured file with a file name of
//    <Rockblock id>-<yyyymmddhhmmss>.dat.  The file names should
//    not contain a path.
//
// -f Read in the specified file expecting it to be a .dat type
//    icedrifterData structured file and print it's data in a human
//    readable format to the console.
//
// -m Only used with -c.  Must be specified before the -c option.
//    Indicates that an email should be created and sent to <email address>
//    which contains the console output and attached .dat and .txt files.
//
// For the -c option, if more than one file is specified, these files
// should be a set of chunks from a single icedrifter report.  The
// Rockblock id number and sent times should all match.
//
//*****************************************************************************

void printHelp(void) {
  printf("Help for idecode2.\n\n");
  printf("idecode2 [-m <email address>] -c <file name list or *.bin>\n");
  printf("idecode2 -f <path and file name of a .dat file>\n");
  printf("idecode2 <character data from email(s) as a single string>\n\n");
  printf("-c Read one to three .bin chunk files, decode the data, display\n");
  printf("   human readable data on the console, write the human\n");
  printf("   readable data to a file with the file name of\n");
  printf("   <Rockblock id>-<yyyymmddhhmmss>.txt, and write the accumulated data\n");
  printf("   as an icedrifterData structured file with a file name of\n");
  printf("   <Rockblock id>-<yyyymmddhhmmss>.dat.  The file names should\n");
  printf("   not contain a path.\n\n");
  printf("-f Read in the specified file expecting it to be a .dat type\n");
  printf("   icedrifterData structured file and print it's data in a human\n");
  printf("   readable format to the console.\n\n");
  printf("-m Only used with -c.  Must be specified before the -c option.\n");
  printf("   Indicates that an email should be created and sent to <email address>\n");
  printf("   which contains the console output and attached .dat and .txt files.\n\n");
  printf("For the -c option, if more than one file is specified, these files\n");
  printf("should be a set of chunks from a single icedrifter report.  The\n");
  printf("Rockblock id number and sent times should all match.\n");
}
