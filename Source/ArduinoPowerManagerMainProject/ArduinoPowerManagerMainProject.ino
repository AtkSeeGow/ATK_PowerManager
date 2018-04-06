#include <Servo.h>
const int opticalCouplingPin = 9;
const char endKey = '\n';
Servo cameraServoX, cameraServoY;

void setup()
{
	Serial.begin(9600);
	pinMode(opticalCouplingPin,OUTPUT);
	cameraServoX.attach(7);cameraServoY.attach(8);
}

int inPutCount;
void loop()
{
	if((inPutCount = Serial.available())>0)
	{
		for (int ii = 0; ii < inPutCount; ii++) {
			OperationInPutString();
		}
	}

	PowerButtonSwtchControl();
	CameraServoControl();
}

bool powerButtonSwtch;
long powerButtonInterval;
long powerButtonPreviousTime;
void PowerButtonSwtchControl()
{
	unsigned long currentTime = millis();
	if(powerButtonSwtch)
	{
		digitalWrite(opticalCouplingPin,HIGH);
		if(currentTime - powerButtonPreviousTime > powerButtonInterval)
		{
			powerButtonSwtch = false;
			powerButtonPreviousTime = currentTime;
		}
	}
	else
	{
		digitalWrite(opticalCouplingPin,LOW);
	}
}

int cameraServoXValue = 90;
int cameraServoYValue = 90;
bool autoCameraServoX;
bool autoCameraServoY;
bool autoCameraServoXStatus;
bool autoCameraServoYStatus;
long autoCameraServoPreviousTime;
long autoCameraServoInterval = 50;
void CameraServoControl()
{
	unsigned long currentTime = millis();
	if(currentTime - autoCameraServoPreviousTime > autoCameraServoInterval)
	{
		if(autoCameraServoX)
		{
			if(autoCameraServoXStatus)
				cameraServoXValue++;
			else
				cameraServoXValue--;

			if(cameraServoXValue > 175 || cameraServoXValue < 5)
				autoCameraServoXStatus = !autoCameraServoXStatus;

			cameraServoX.write(cameraServoXValue);
		}

		if(autoCameraServoY)
		{
			if(autoCameraServoYStatus)
				cameraServoYValue++;
			else
				cameraServoYValue--;

			if(cameraServoYValue > 175 || cameraServoYValue < 5)
				autoCameraServoYStatus = !autoCameraServoYStatus;

			cameraServoY.write(cameraServoYValue);
		}
		autoCameraServoPreviousTime = currentTime;
	}
}

String inPutInformation = "";
void OperationInPutString()
{
	char value = Serial.read();
	if(value == endKey)
	{
		OperationInPutEvent(inPutInformation);
		inPutInformation = "";
	}
	else
		inPutInformation += value;
}

void OperationInPutEvent(String value)
{
	String actionCode = value.substring(0,1);
	String information = value.substring(2,value.length());
	if(actionCode == "S")
	{
		powerButtonSwtch = true;
		powerButtonInterval = information.toInt();
		powerButtonPreviousTime = millis();
	}
	else if(actionCode == "O")
	{
		int commaPosition;
		do
		{
			commaPosition = information.indexOf('#');
			if(commaPosition != -1)
			{

				UpdateServoMotor(information.substring(0,commaPosition));
				information = information.substring(commaPosition+1, information.length());
			}
			else
			{
				if(information.length() > 0 )
					UpdateServoMotor(information);
			}
		}while(commaPosition >= 0);
	}
}

void UpdateServoMotor(String value)
{
	String actionCode = value.substring(0,1);
	String information = value.substring(1,value.length());

	if(actionCode == "X")
	{
		cameraServoXValue = information.toInt();
		cameraServoX.write(cameraServoXValue);
	}	
	else if (actionCode == "Y")
	{
		cameraServoYValue = information.toInt();
		cameraServoY.write(cameraServoYValue);
	}
	else if (actionCode == "E")
	{
		autoCameraServoX = information.toInt();
	}
	else if (actionCode == "F")
	{
		autoCameraServoY = information.toInt();
	}
}

char outPutInformation[128];
void ReturnInformationValue(String value)
{
	value += endKey;
	value.toCharArray(outPutInformation, value.length()+2);
	Serial.write(outPutInformation);
}