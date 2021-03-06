#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <EEPROM.h>
#include <vector>
#include<Hash.h>

using namespace std;

 struct person 
{
  char username[10];
  char pass_hash[20];
  char salt[12];
  char perm;
};

#define user_start  78
#define user_step  sizeof(person)
#define passAP_start  68
#define passAP_stop  77 
#define permission_start 120 

// declare the Variables
char* ssid = "JohnAndWillow";
char* password = "IamSuperProgrammer";
char passAp[10];
char* ssidAp="isdoor";

person list[10];
vector<String> commands;

vector<String> splitString(String line, char c);
void writePassAp(String pass);
void writeWifi(String passWifi,String ssidWifi);
char* salt_random();
void readPerson(uint index, person* p);
void writePerson(int start, person* p);
char ishavingPermission(String username1);
void printlist();
void printt(char* s,int n);

//create wifi server listen on a given port
WiFiServer server(3030);
//create Wifi client
WiFiClient client;


int button = 0;
String str,hsh;
bool buttonState;
// eeprom memory byte
int passwifi_start = 0, passwifi_stop = 31;
int ssidwifi_start = 32, ssidwidi_stop = 63;
int salt_start = 64, salt_stop = 67;

void setup() {
 
  //for(int i=0;i<10;i++) readPerson(i, &list[i]);
  
  pinMode(button, INPUT);
  Serial.begin(115200);
  WiFi.softAP(ssidAp, passAp);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  //Print status to Serial Monitor
  Serial.print("connected to: "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  server.begin();
}

void loop()
{
  client = server.available();
  if (client)
  {
    Serial.println("connected to client");
    client.setTimeout(10000);
    str=client.readStringUntil('\n');
    commands=splitString(str,' ');
  
      //registrirane
      //v
      if(commands[0]=="signup")
      {
        bool flag=0,flag1=1;
        int j=0;
        person p;
        
        commands[1].toCharArray(p.username,sizeof(commands[1]));
        
        char salt[13];
        for(int i=0;i<12;i++){
        salt[i]=char(random(33,126));
        }
        salt[12]=0;
        //printt(salt,12);
       
        commands[2]+=salt;
        sha1(commands[2]).toCharArray(p.pass_hash,20);
        
        int ind,br=0;
        for(ind=0;ind<10;ind++) 
        {
          if( list[ind].username[0]!='\0')
          {
            br++;
            if(!strncmp(p.username,list[ind].username,10))break;
          }
        }
        if(br==0)p.perm='a';
        else p.perm='d';
        if(ind==10)
        {
         for(int i=0;i<10;i++)
         {
           if(list[i].username[0]=='\0')
           {
            strncpy(list[i].username,p.username,10);
            strncpy(list[i].pass_hash,p.pass_hash,20);
            //printt(salt,12);
            strncpy(list[i].salt,salt,12);
            //printt(list[i].salt,12);
            list[i].perm=p.perm;
            //writePerson(user_start+i*user_step,&p);
            break;
           }
         }
         printlist();
         client.println("truesignup");
        }
        else client.println("errorsignup");
      }
      //vlizane
      //v
      else if(commands[0]=="signin")
      {
        char perm1;
        bool flag=0;
        person p;
        char passh[20];
        commands[1].toCharArray(p.username,10);
        String pass=commands[2];
        char salt[12];
                
        for(int i=0;i<10;i++)
       {
        
          
            if(!strncmp(list[i].username,p.username,10))flag=1;
            else {flag=0;}
          
          if(flag==1)
          {
            strncpy(salt,list[i].salt,12);
            //printt(salt,12);
            pass+=salt;
            sha1(pass).toCharArray(passh,20);
            
            if(!strncmp(passh,list[i].pass_hash,20)){ perm1=list[i].perm;}
            else flag=0;
            break;
          }
         }
      if(flag==1) client.println(perm1);
      else client.println("errorsignin");
      }
      //v
      else  if(commands[0]=="setWifi")
      {
       commands[1].toCharArray(ssid,32);
       commands[2].toCharArray(password,32);
       
       writeWifi(commands[1],commands[2]); 
      }
      //v  
      else if(commands[0]=="setAP")
      {
       writePassAp(commands[1],passAP_start);
      }
      else if(commands[0]=="list")
      {
        char nameuser[10];
        bool flag=false;
        commands[1].toCharArray(nameuser,10);
        String str;
       for(int i=0;i<10;i++)
       {
         if(!strncmp(list[i].username,nameuser,10) && list[i].perm=='a') flag=true;
         if(list[i].username[0]!='\0' && flag==true){
            str=str+list[i].username;
           str+=' ';
           str+=list[i].perm;
           client.println(str);
           str="";
         }
       }
        client.println("stop");
      }
      //v
      else if(commands[0]=="setPermission")
      {
        bool flag=0;
        String username1=commands[1];
        String permission=commands[2];
        char perm1=permission[0];
        for(int i=0;i<10;i++)
        {
         if(list[i].username[0]==username1[0] && strlen(list[i].username)==username1.length()) {
          for(int j=1;j<strlen(list[i].username);j++)
          {
            if(list[i].username[j]==username1[j])flag=1;
            else {flag=0;break;}
          }
          if(flag==1)
          {
            list[i].perm=perm1;
           // EEPROM.write(permission_start+i*user_step,perm1);
            break;
          }
         }
       }
        
      }
      //v
      else if(commands[0]=="del")
      {
        String username1=commands[1];
         bool flag=0;
        for(int i=0;i<10;i++)
       {
         if(list[i].username[0]==username1[0] && strlen(list[i].username)==username1.length()) {
          for(int j=1;j<strlen(list[i].username);j++)
          {
            if(list[i].username[j]==username1[j])flag=1;
            else {flag=0;break;}
          }
          if(flag==1)
          {
            list[i].username[0]=0;
           // EEPROM.write(user_start+i*user_step,0);
            for(int i=0;i<10;i++) readPerson(i, &list[i]);
            break;
          }
        }
       }
      
      }
      
      //v
      else if (commands[0] == "take")
      {
       // read the state of the pushbutton value:
          buttonState = digitalRead(button);
          if (buttonState == HIGH)
          {
           client.println("open!");
          }
          else
          {
            client.println("close");
          }
        
      }
      else
      {
      client.println("error");
      }
      
    client.flush();
    client.stop();
  }
}
void writePerson(int start, person* p)
{
  for ( int i = 0; i < sizeof(person); i++); //EEPROM.write(start+i, *((byte*)p+i));
}
//v
void readPerson(uint index, person* p)
{
  if (index<10) {
    int start = sizeof(person)*index+user_start;
    //for ( int i = 0; i < sizeof(person); i++) *((byte*)p+i) = EEPROM.read(start + i);
  }
}

//v
void writeWifi(String passWifi,String ssidWifi)
{
  
  for(int i=0;i<passWifi.length();i++)
  {
    
    //EEPROM.write(i,passWifi[i]);    
  }
  for(int i=0;i<ssidWifi.length();i++)
  {
    
    //EEPROM.write(i+32,ssidWifi[i]);
  }
}
//v
char* salt_random()
{
  
  int random_number;
  static char salt[12];
  for(int i=0;i<12;i++){
  random_number=random(33,126);
  salt[i]=char(random_number);
  }
  return salt;
}
//v
void writePassAp(String pass,int start)
{
  int j=0;
   for(int i=start;i<pass.length()+start;i++)
   {
    
     //EEPROM.write(i,pass[j]);
     j++;
   }
}
//v
vector<String> splitString(String line, char c)
{
    vector<String> str;
    
    String curr_string = "";
    
    for(int i = 0; i < line.length(); i++)
    {
      if(line[i] == c)
      {

        str.push_back(curr_string);
        curr_string = "";
      }
      else curr_string+=line[i];
    }

    str.push_back(curr_string);
    return str;
}
char ishavingPermission(String username1)
{
 bool flag=0;
 char perm='d';
  
  for(int i=0;i<10;i++)
  {
    if(list[i].username[0]==username1[0])
    {
      for(int j=1;j<username1.length();j++)
      {
          if(list[i].username[j]==username1[j]){flag=1;perm=list[i].perm;}
          else {flag=0;perm='d';break;}
          
      }
    }
  }
  return perm;
}
void printlist()
{
  for(int i=0;i<10;i++)
  {
    if(list[i].username[0]!='\0'){
      Serial.print("User:");
      Serial.println(i);
      printt(list[i].username,10);
      printt(list[i].pass_hash,20);
      printt(list[i].salt,12);
      printt(&list[i].perm,1);
    }
  }
}
void printt(char* s,int n)
{
  for(int i=0;i<n;i++)
  {
    if(s[i]==0)break;
    Serial.print(s[i]);
  }
  Serial.println();
}


