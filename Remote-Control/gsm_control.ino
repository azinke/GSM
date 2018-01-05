#include <Sim800l.h>
#include <SoftwareSerial.h>

/* OUTPUT */
#define led_ 9
#define output_1_ 7
#define output_2_ 8

/* INPUT */
#define _input_1 2 // Interrupt Enable
#define _input_2 3 // Interrupt Enable

#define SERIAL_NUMBER "IUT21102017\0"

#define SMS 0
#define CALL 1



Sim800l gsm;

/*
 Varibale definition
*/
uint8_t index = 1;
String sms, number, response;
bool error = 0, ans = 0;
volatile bool _state_1 = 0, _state_2 = 0, reply = 0;
uint8_t idx;
String buffer_;
char buffer_0[25];
char buffer_1[25];

/* Super Admin number */
const String superAdmin = String("66514725");

/* Initialisation function */
void setup(){
  // Debug
  Serial.begin(9600);
  
   gsm.begin();
   delay(1500);

   /* PORT init */
   pinMode(output_1_, OUTPUT);
   pinMode(led_, OUTPUT);
   pinMode(output_2_, OUTPUT);

   pinMode(_input_1, INPUT_PULLUP);
   pinMode(_input_2, INPUT_PULLUP);

   /* Attach interrupt to inputs */
   attachInterrupt(digitalPinToInterrupt(_input_1), outpout_1ISR, FALLING);
   attachInterrupt(digitalPinToInterrupt(_input_2), outpout_2ISR, FALLING);

   delay(5000);
   sms = gsm.readStoSms(1);

   //Debug
   Serial.print("sms: ");
   Serial.println(sms);
   
   if(sms != ""){
     _state_1 = bool(sms.charAt(0)) >= 1 ? 1:0;
     _state_2 = bool(sms.charAt(1)) >= 1 ? 1:0;
     reply = bool(sms.charAt(2)) >= 1 ? 1:0;
     
	   digitalWrite(output_1_, _state_1);
     digitalWrite(led_, _state_1);
	   digitalWrite(output_2_, _state_2);
	}else{
		digitalWrite(output_1_, LOW);
    digitalWrite(led_, LOW);
	  digitalWrite(output_2_, LOW);
	}
   sms = "";
}

/*
  @function: clear_buffer
  @var: none
  @return: none
  @Summary: update all output after processing

*/

void clear_buffer(){
  uint8_t i = 0;
  for(i = 0; i <25; i++){
    buffer_0[i] = ' ';
    buffer_1[i] = ' ';
  }
}

/* 
	@function: process
	@var: 
		String: _data 
	@return:
		0: input empty
		1: Output 1 (ON)
		2: Output 2 (ON)
		3: Both 	(ON)
		10: Output 1 (OFF)
		20: Output 2 (OFF)
		30: Both 	(OFF)
		4: checking ...
		5: Add phone number # ADD:xxxxxxxx*name#  || to set a person as admin put _adm to it's name
		6: delete phone number # DEL:name#
		7: Enable/Disable reply #REP:x#
		8: return serial Id number

	@Summary: SMS processing functions 
*/
uint8_t process(String _data){
  	if(_data == "") { return 0; }
  	else if(_data.indexOf("ON1") != -1) { return 1; }
  	else if(_data.indexOf("ON2") != -1) { return 2; }
  	else if(_data.indexOf("ON")!= -1) { return 3; }
  	else if(_data.indexOf("OFF1") != -1) { return 10; }
  	else if(_data.indexOf("OFF2") != -1) { return 20; }
  	else if(_data.indexOf("OFF")!= -1) { return 30; }
  	else if(_data.indexOf("ON?")!= -1) { return 4; }
  	else if(_data.indexOf("ADD")!= -1){ return 5; }
  	else if(_data.indexOf("DEL")!= -1) { return 6; }
  	else if(_data.indexOf("REP")!= -1) { return 7; }
  	else if(_data.indexOf("IDN")!= -1) { return 8; }
	  else { gsm.delAllSms(); return 0; }
}

/*
	@function: update
	@var: none
	@return: none
	@Summary: update all output after processing

*/
void update(){
  String user_name = gsm.getNameSms(index);
  String num = gsm.getNumberSms(index);
	if((user_name != "") || ( num == superAdmin)){	
  	switch(process(sms)){
  		case 0:{
  
  			break;
  		}
  		case 1: {
  			digitalWrite(output_1_, HIGH);
        digitalWrite(led_, HIGH);
        _state_1 = 1;
        if(reply) { response = "EQ1ON"; }
  			break;
  		}
  		case 2: {
  			digitalWrite(output_2_, HIGH);
         _state_2 = 1;
        if(reply) { response = "EQ2ON"; }
  			break;
  		}
  		case 3: {
  			digitalWrite(output_1_, HIGH);
        digitalWrite(led_, HIGH);
  			digitalWrite(output_2_, HIGH);
         _state_1 = 1;
         _state_2 = 1;
        if(reply) { response = "EQON"; }
  			break;
  		}
  		case 10: {
  			digitalWrite(output_1_, LOW);
        digitalWrite(led_, LOW);
        _state_1 = 0;
        if(reply) { response = "EQ1OFF"; }
  			break;
  		}
  		case 20: {
  			digitalWrite(output_2_, LOW);
       _state_2 = 0;
        if(reply) { response = "EQ2OFF"; }
  			break;
  		}
  		case 30: {
  			digitalWrite(output_1_, LOW);
        digitalWrite(led_, LOW);
  			digitalWrite(output_2_, LOW);
        _state_1 = 0;
        _state_2 = 0;
        if(reply) { response = "EQOFF"; }
  			break;
  		}
  		case 4: {
  			//gsm.sendSms()
        if(_state_1){ response = "EQ1ON "; }
        else{ response = "EQ1OFF "; }

        if(_state_2){ response += "EQ2ON"; }
        else{ response += "EQ2OFF"; }
  			break;
  		}
  		case 5: { // Add phone number
        if((user_name.indexOf("_adm") != -1) || ( num == superAdmin ) ){
    			idx = sms.indexOf("ADD:");
    			buffer_ = sms.substring(idx+4,sms.indexOf("*",idx+5));
    			buffer_.toCharArray(buffer_0, buffer_.length()+1);
          idx = sms.indexOf("*", idx+4);
    			buffer_ = sms.substring(idx+1, sms.indexOf("#", idx+2));
    			buffer_.toCharArray(buffer_1, buffer_.length()+1);
          
    			ans = gsm.addNumber(buffer_0, buffer_1);
    			//gsm.listNumber(1, 5); // for debugging
        }else if(reply){
          response = "ADD_ERROR; Need admin credential!";
        }
        if(reply){ 
          if(ans){ response = "ADD_OK"; } 
          else{ response = "ADD_ERROR"; } 
        }
  			break;
  		}
  		case 6: { // Delete phone number
        if(( user_name.indexOf("_adm") != -1 ) || ( num == superAdmin ) ){
    			idx = sms.indexOf("DEL:");
    			buffer_ = sms.substring(idx+4, sms.indexOf("#", idx+5));
    			buffer_.toCharArray(buffer_0, buffer_.length()+1);
    
    			ans = gsm.delNumber(buffer_0);
          if(reply){ 
            if(ans){ response = "DEL_OK"; } 
            else{ response = "DEL_ERROR"; } 
          }
    			//gsm.listNumber(1, 5);
        }else if(reply){
          response = "ADD_ERROR; Need admin credential!";
        }
        
  			break;
  		}
  		case 7: { // Enable or disable reply
    		idx = sms.indexOf("REP:");
		    reply = sms.substring(idx+4, sms.indexOf("#", idx+5)).toInt();
        error = gsm.delAllSms();
        logs();
        if(reply){  response = "REP_ON"; } 
        else{ response = "REP_OFF"; }

  			break;
  		}
  		case 8: {
        number = gsm.getNumberSms(index);
        number.toCharArray(buffer_0, number.length());
  			error = gsm.sendSms(buffer_0, SERIAL_NUMBER);
  			break;
  		}
  	}
	}

  if(reply && (response != "")){ Response(SMS, response); }

  sms = "";
  response = "";
  return;
}

/*
  @function: Response
  @var:
    uint8_t type: (0: sms  -  1: call)
    String msg
  @return none
  @Summary: Answer back to the user
*/
void Response(uint8_t type, String msg){
  if(type){  number = gsm.getCallNumber(); }
  else{  number = gsm.getNumberSms(index); }

  number.toCharArray(buffer_0, number.length()+1);
  msg.toCharArray(buffer_1, msg.length()+1);
  
  error = gsm.sendSms(buffer_0, buffer_1);
  return;
}


/*
	@function: logs
	@var: none
	@return none
	@Summary: stored the last state of output device
*/
void logs(){
	char text[4];
  text[0] = String(_state_1).charAt(0);
  text[1] = String(_state_2).charAt(0);
  text[2] = String(reply).charAt(0);
  /* 
   *  text[0] = char(_state_1 + 48);
  text[1] = char(_state_2 + 48);
  text[2] = char(reply + 48);
   */
  text[3] = '\0';
  Serial.println(String(text));
	error = gsm.saveSms(text);
  return;
}

/*
  @function: outpout_1ISR
  @var: none
  @return none
  @Summary: Interrupt Service Routine for manual control of output_1
*/
void outpout_1ISR(){
  delay(500);
  _state_1 ^= 1;
  digitalWrite(output_1_, _state_1);
  delay(1500);
}

/*
  @function: outpout_2ISR
  @var: none
  @return none
  @Summary: Interrupt Service Routine for manual control of output_1
*/
void outpout_2ISR(){
  delay(500);
  _state_2 ^= 1;
  digitalWrite(output_2_, _state_2);
  delay(1500);
}

void loop(){
	sms = gsm.readSms(2);
 if(sms.length() >= 1){
  update();
  error = gsm.delAllSms();
  logs();
  clear_buffer();
  Serial.println("One SMS");
  sms = "";
 }
 
	/* phone call management routine */
	else if(gsm.getCallStatus() == 3){
          Serial.println("Ring ...");
	    	if(gsm.getCallName().length() > 1){
	    		_state_1 ^= 1;
	    		digitalWrite(output_1_, _state_1);
          digitalWrite(led_, _state_1);

          if(reply){ 
            if(_state_1){ response = String("EQ1ON"); }
            else{ response = String("EQ1OFF"); }

            Response(CALL, response); 
            response = "";
          }
	    	}
	    error = gsm.hangoffCall();
      Serial.println("HangOff");
      
      error = gsm.delAllSms();
      logs();
   }
 delay(100);
}

