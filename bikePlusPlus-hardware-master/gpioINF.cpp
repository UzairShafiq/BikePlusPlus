#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ugpio/ugpio.h>
#include <fstream>
#include <string.h>

#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////STRUCTS/////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

struct Accelerometer {
  float x;
  float y;
  float z;
};

struct Gyroscope {
  float yaw;
  float pitch;
  float roll;
};

struct Date {
  int month;
  int day;
  int year;
};

struct Time {
  int hour;
  int minute;
  int second;
};

struct TheFloat {
  int floatNegative = 1;
  bool decimalPoint = false;
  bool minus = false;
  bool plus = false;
  bool exponential = false; // set to true if 'e' shows up
  int exponent = 0; // this is the exponent
  int exponentNegative = 1; // set to -1 if part after 'e' is negative
  float postexp = 0;
  float preDecimal = 0; // the number before the decimal
  float postDecimal = 0; // the number after the decimal
  float postDecimalDigits = 0; // # of digits postDecimal has
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////Declaration /////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
Time getTime ();
Date getDate ();
Time timeElapsed (const Time startTime, const Date startDate, const Time time1, const Date date1);
Time timeElapsed (const Time startTime, const Time time1);
void averageInclination (const Gyroscope gyro, float avgIncline[], Gyroscope& previousGyro);
void maximumIncline (const Gyroscope gyro, float& max);
void minimumIncline (const Gyroscope gyro, float& min);
float totalTime (float avgIncline[]);
float calcAvg (int avgIncline, float totalTime);
int getSeconds();

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////GLOBAL VARIABLES////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

int startingTime = getSeconds();
Date startingDate = getDate();
int offset = 10;
unsigned int usecs = 100000;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////FUNCTIONS///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

Time getTime () {
  time_t now;
  struct tm nowLocal;
  now = time(NULL);
  nowLocal = *localtime(&now);

  Time timeStamp;
  timeStamp.hour = nowLocal.tm_hour;
  timeStamp.minute = nowLocal.tm_min;
  timeStamp.second = nowLocal.tm_sec;

  return timeStamp;
}

int getSeconds () {
  time_t now;
  struct tm nowLocal;
  now = time(NULL);
  nowLocal = *localtime(&now);

  return nowLocal.tm_sec;
}

Date getDate () {
  time_t now;
  struct tm nowLocal;
  now = time(NULL);
  nowLocal = *localtime(&now);

  Date dateStamp;
  dateStamp.month = nowLocal.tm_mon + 1;
  dateStamp.day = nowLocal.tm_mday;
  dateStamp.year = nowLocal.tm_year + 1900;

  return dateStamp;
}

Time timeElapsed (const Time startTime, const Date startDate, const Time time1, const Date date1) {
  int hours = 0;
  int minutes = 0;
  int startTimeInMinutes = 0;
  int timeInMinutes = 0;
  int timeElapsed = 0;

  if (startDate.month == date1.month && startDate.day == date1.day && startDate.year == date1.year) {
    startTimeInMinutes = (startTime.hour * 60) + (startTime.minute);
    timeInMinutes = (time1.hour * 60) + (time1.minute);
    timeElapsed = timeInMinutes - startTimeInMinutes;
    hours = timeElapsed/60;
    minutes = timeElapsed - hours*60;
  } else if (startDate.month == date1.month && startDate.day != date1.day && startDate.year == date1.year) {
    int dayDifference = date1.day - startDate.day;
    startTimeInMinutes = (startTime.hour * 60) + (startTime.minute);
    timeInMinutes = (time1.hour * 60) + (time1.minute) + (dayDifference*24*60);
    timeElapsed = timeInMinutes - startTimeInMinutes;
    hours = timeElapsed/60;
    minutes = timeElapsed - hours*60;
  }

  Time timeChange = {
    .hour = hours,
    .minute = minutes
  };

  return timeChange;
};

Time timeElapsed (const Time startTime, const Time time1) {
  int hours = 0;
  int minutes = 0;
  int startTimeInMinutes = 0;
  int timeInMinutes = 0;
  int timeElapsed = 0;

  if (startTime.hour <= time1.hour) {
    startTimeInMinutes = (startTime.hour * 60) + (startTime.minute);
    timeInMinutes = (time1.hour * 60) + (time1.minute);
    timeElapsed = timeInMinutes - startTimeInMinutes;
    hours = timeElapsed/60;
    minutes = timeElapsed - hours*60;
  } else if (startTime.hour > time1.hour) {
    int dayDifference = 1;
    startTimeInMinutes = (startTime.hour * 60) + (startTime.minute);
    timeInMinutes = (time1.hour * 60) + (time1.minute) + (dayDifference*24*60);
    timeElapsed = timeInMinutes - startTimeInMinutes;
    hours = timeElapsed/60;
    minutes = timeElapsed - hours*60;
  }

  Time timeChange = {
    .hour = hours,
    .minute = minutes
  };

  return timeChange;
}

void averageInclination (const Gyroscope gyro, float avgIncline[], Gyroscope& previousGyro) {
  if ((gyro.pitch + offset) > previousGyro.pitch && (gyro.pitch - offset) < previousGyro.pitch) {
    avgIncline[0] += 1;
  } else if ((gyro.pitch + offset) > previousGyro.pitch && (gyro.pitch - offset) > previousGyro.pitch) {
    avgIncline[1] += 1;
  } else if ((gyro.pitch + offset) < previousGyro.pitch && (gyro.pitch - offset) < previousGyro.pitch) {
    avgIncline[2] += 1;
  }

  previousGyro = gyro;
}

void maximumIncline (const Gyroscope gyro, float& max) {
  if (gyro.pitch > max) {
    max = gyro.pitch;
  }
}

void minimumIncline (const Gyroscope gyro, float& min) {
  if (gyro.pitch < min) {
    min = gyro.pitch;
  }
}

float totalTime (float avgIncline[]) {
  int sum = 0;
  for (int i = 0; i < 3; i++) {
    sum += avgIncline[i];
  }
  float tTime = (float(sum))/60;
  return tTime;
}

float calcAvg (int avgIncline, float totalTime) {
  float avgI = float(avgIncline)/60;
  float avg = avgI/totalTime;
  return avg;
}

bool stringToFloat(const char input[], float& value) {

  // have the line of input in the char array
  int i = 0;
  TheFloat f; // created new float struct, where i will keep the property info of the float
  value = 0;
  while(input[i]!=0) {
    // input[i] is current character

    /*
      What are the possible cases?
      1. current char is '-'
      2. current char is '+'
      3. current char is a number in [0,9]
      4. current char is 'e'
      5. current char is '.'
      6. current char is something else
    */

    if (input[i]=='-' && !f.minus)
    {
	  f.minus=true;
      // ****make sure this sign is in the right place tho
        // can only be BEFORE preDecimal or BEFORE exponent
      if (!f.preDecimal || (f.preDecimal&&f.exponential&&!f.exponent))
      {// either the float itself can be negative, or the exponent can be negative
      if (f.exponential)
        f.exponentNegative=-1;
      else if (!f.exponential)
        f.floatNegative=-1;
      }
      else
        return false;
    } else if (input[i]=='-' && f.minus){
		return false;
	} else if (input[i]=='+' && !f.plus)
    {
	  f.plus = true;
      // make sure this sign is in the right place tho
      if (!f.preDecimal || (f.preDecimal&&f.exponential&&!f.exponent));
      // nothing needed cuz this is the default
      else
        return false;
    } else if (input[i]=='+' && f.plus){
		return false;
	} else if (input[i]>='0' && input[i]<='9')
    {
      // can either be preDecimal num, postDecimal num, or exponent num
      if (!f.decimalPoint)
      {
        // append to preDecimal
        f.preDecimal = 10*f.preDecimal+(input[i]-'0');
      }
      else if (f.decimalPoint && !f.exponential)
      {
        // append to postDecimal
        f.postDecimal = 10*f.postDecimal+(input[i]-'0');
        f.postDecimalDigits++;
      }
      else if (f.exponential)
      {
		f.postexp = 2;
        // append to exponent
        f.exponent = 10*f.exponent+(input[i]-'0');
      }
    }
    else if (input[i]=='e' || input[i]=='E')
    {
      f.exponential=true;
    }
    else if (input[i]=='.' && !f.decimalPoint)
    {
      f.decimalPoint=true;
    } else if(input[i]=='.' && f.decimalPoint){
		return false;
	}

    i++;
  }

  float floatPre = (float)f.preDecimal;
  float dividingTerm = 1;
  for (int i = 0; i < f.postDecimalDigits; i++)
  {
    dividingTerm*=10;
  }
  float floatPost = ((float)f.postDecimal)/dividingTerm;
  float multiplier = 1;
  for (int i = 0; i<f.exponent;i++)
  {
    if(f.exponentNegative>0)
    {
      multiplier*=10.0;
    }
    else if (f.exponentNegative<0)
    {
      multiplier/=10.0;
    }
  }
  if((!f.postDecimalDigits&&f.decimalPoint) || (!f.postexp&&f.exponential)){
	  return false;
  }
  value = f.floatNegative*(floatPre+floatPost)*multiplier;
  return true;
}

int stringToInt(char input[]) {
	int value = 0;
	int i = 0;
	bool run = true;
	bool neg = false;
	while(input[i] && run){
			switch(input[i]){
			case '0':
				if(value == 0){ //first digit
					value = 0;
				} else { //not first digit and positive
					value*=10;
					value+= 0;
				}
				break;
			case '1':
				if(value == 0){ //first digit
					value = 1;
				} else { //not first digit and positive
					value*=10;
					value+= 1;
				}
				break;
			case '2':
				if(value == 0){ //first digit
					value = 2;
				} else { //not first digit and positive
					value*=10;
					value+= 2;
				}
				break;
			case '3':
				if(value == 0){ //first digit
					value = 3;
				} else { //not first digit and positive
					value*=10;
					value+= 3;
				}
				break;
			case '4':
				if(value == 0){ //first digit
					value = 4;
				} else { //not first digit and positive
					value*=10;
					value+= 4;
				}
				break;
			case '5':
				if(value == 0){ //first digit
					value = 5;
				} else { //not first digit and positive
					value*=10;
					value+= 5;
				}
				break;
			case '6':
				if(value == 0){ //first digit
					value = 6;
				} else { //not first digit and positive
					value*=10;
					value+= 6;
				}
				break;
			case '7':
				if(value == 0){ //first digit
					value = 7;
				} else { //not first digit and positive
					value*=10;
					value+= 7;
				}
				break;
			case '8':
				if(value == 0){ //first digit
					value = 8;
				} else { //not first digit and positive
					value*=10;
					value+= 8;
				}
				break;
			case '9':
				if(value == 0){ //first digit
					value = 9;
				} else { //not first digit and positive
					value*=10;
					value+= 9;
				}
				break;
			case ' ':
				break;
			case '-':
			//cout << "ME";
				neg = true;
				break;
			case '\t':
				break;
			default:
				break;


		}
			++i;
		}


	if(run == false){
		return -1;
	}
	if(neg){
		value*=-1;

	}
	return value;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

int main () {
  float avgIncline[3]; //index0: horizontal, index1: incline, index2: decline
  for (int i = 0; i < 3; i++) {
    avgIncline[i] = 0;
  }

  //create variables
  float max = 0;
  float min = 0;
  float totalT = 0;

  Gyroscope previousGyro = { //base value
    .yaw = 0,
    .pitch = 0,
    .roll = 0
  };

  int startSeconds = getSeconds();

  //file read stuff
  time_t t = time(0);
  struct tm * now = localtime( & t );
	char logname[] = "LOGFILE.txt";

  int i = 0;

  int rq1, rv1, rq2, rq3, rv2, rv3, read1;
  int gpio1, gpio2, gpio3, gpio4;
  gpio1 = 1;
  gpio2 = 2;
  gpio3 = 6;
  gpio_free(gpio1);
  gpio_free(gpio2);
  gpio_free(gpio3);
  // check if gpio is already exported
  if ((rq1 = gpio_is_requested(gpio1)) < 0 || (rq2 = gpio_is_requested(gpio2)) < 0 || (rq3 = gpio_is_requested(gpio3)) < 0)
  {
    perror("gpio_is_requested");
    return EXIT_FAILURE;
  }
  // export the gpio
  if (!rq1) {
    printf("> exporting gpio\n");
    if ((rv1 = gpio_request(gpio1, NULL)) < 0 || (rv2 = gpio_request(gpio2, NULL)) < 0 || (rv3 = gpio_request(gpio3, NULL)) < 0)
    {
      perror("gpio_request");
      return EXIT_FAILURE;
    }
  }

  // set to input direction

  printf("> setting to input\n");

  if ((rv1 = gpio_direction_output(gpio1, 0)) < 0 || ((rv2 = gpio_direction_output(gpio2, 0)) < 0) || (rv3 = gpio_direction_output(gpio3, 0)) < 0){
    perror("gpio_direction_input");
  }
  bool val = true;
  //int terminate = 20;
  int analoguesize = 20;

  while (true) {
    Gyroscope gyr;
    FILE * bytes;
    bytes = fopen("/dev/ttyS1", "r");

    char line[analoguesize];
    //char line;

    //int c = fgetc(bytes);

    char* read = fgets(line, analoguesize, bytes);
    char cpot[6];
    char cYaw[6];
    char cRoll[6];
    char cPitch[6];
    int tempIndex = 0;
    int pot = -1;
    for(int x = 0; x < 6; x++){
      cpot[x] = 0;
      cYaw[x] = 0;
      cRoll[x] = 0;
      cPitch[x] = 0;
    }

    // if success then read is not null, else read also contains same val as line
    if(read){
       int i = 0;

      while(line[i]){
        //cout << line[i];
        if(line[i] == 'q'){ //pot value found
          int x = i;
          while(line[x] != 'Y' && line[x]){ //keep on adding chars to cpot until you reach 'Y'. extra error check in case you reach end of line
            x++;
            cpot[tempIndex] = line[x];
            tempIndex++;
          }
        pot = stringToInt(cpot);
        tempIndex = 0;
        // cout << "POT: " << pot;
        } else if(line[i] == 'Y'){
          int x = i;
          while(line[x] != 'P' && line[x]){ //keep on adding chars to cpot until you reach 'Y'. extra error check in case you reach end of line
            x++;
            cYaw[tempIndex] = line[x];
            tempIndex++;
          }
          stringToFloat(cYaw, gyr.yaw);
          tempIndex = 0;
          // cout << "Yaw: " << gyr.yaw;
        } else if(line[i] == 'P'){
          int x = i;
          while(line[x] != 'R' && line[x]){ //keep on adding chars to cpot until you reach 'Y'. extra error check in case you reach end of line
            x++;
            cPitch[tempIndex] = line[x];
            tempIndex++;
          }
          stringToFloat(cPitch, gyr.pitch);
          tempIndex = 0;
          // cout << "Pitch: " << gyr.pitch;
        } else if(line[i] == 'R'){
          int x = i;
          while(line[x] != 'A' && line[x]){ //keep on adding chars to cpot until you reach 'Y'. extra error check in case you reach end of line
            x++;
            cRoll[tempIndex] = line[x];
            tempIndex++;
          }
          stringToFloat(cRoll, gyr.roll);
          tempIndex = 0;
          // cout << "Roll: " << gyr.roll;
        }
        i++;

		if(pot != -1){
		if(pot >500){
			//cout << "pot: " << pot << endl;
		int setval1 = gpio_set_value(gpio1, 1);
		int setval2 = gpio_set_value(gpio2, 0);

		//int setval3 = gpio_set_value(gpio1, 0);


	  } else if(pot < 300){
		 // cout << "pot: " << pot << endl;
		 int setval1 = gpio_set_value(gpio1, 0);
		int setval2 = gpio_set_value(gpio2, 1);

	//	int setval3 = gpio_set_value(gpio2, 0);
	  } else {
		//  cout << "pot: " << pot << endl;
		int setval1 = gpio_set_value(gpio1, 0);
		int setval2 = gpio_set_value(gpio2, 0);
	  }

		}

        //stats stuff
        int tempSeconds = getSeconds();
        if (tempSeconds != startSeconds) {
          int currentTime = getSeconds();
         // Time elapsed = timeElapsed(startingTime, currentTime);

          averageInclination(gyr, avgIncline, previousGyro);
          maximumIncline(gyr, max);
          minimumIncline(gyr, min);

          totalT = totalTime(avgIncline);

          float avIncline = calcAvg(avgIncline[1], totalT);
          float avDecline = calcAvg(avgIncline[2], totalT);
          float avFlat = calcAvg(avgIncline[0], totalT);
	  int nameWidth = 25;
 	  int numWidth = 25;
          char separator = ' ';
          cout << left << setw(nameWidth) << setfill(separator) << "Time Elapsed (min)"
          << left << setw(nameWidth) << setfill(separator) << "Average Inclination (\%)"
          << left << setw(nameWidth) << setfill(separator) << "Average Declination (\%)"
          << left << setw(nameWidth) << setfill(separator) << "Average Flat (\%)"
          << left << setw(nameWidth) << setfill(separator) << "Maximum Inclination (deg)"
          << left << setw(nameWidth) << setfill(separator) << "Minimum Inclination (deg)" << endl;

          cout << left << setw(numWidth) << setfill(separator) << totalT
          << left << setw(numWidth) << setfill(separator) << avIncline*100
          << left << setw(numWidth) << setfill(separator) << avDecline*100
          << left << setw(numWidth) << setfill(separator) << avFlat*100
          << left << setw(numWidth) << setfill(separator) << max
          << left << setw(numWidth) << setfill(separator) << min << endl;

          previousGyro = gyr;
          startingTime = currentTime;
          startSeconds = tempSeconds;
        }
      }

	  //do gpio stuff here:


    } else {
		//log some error
    }
    val = !val;
  }

  // unexport the gpio
  gpio_free(gpio1);
  gpio_free(gpio2);
  gpio_free(gpio3);
  gpio_free(gpio4);

  return 0;
}
