
//ffmpeg -i yin-mad-dansumegumindansu.mp4 - vf "fps=30,scale=-1:180:flags=lanczos,crop=320:in_h:(in_w-320)/2:0" -q:v 9 cancan1.mjpeg
//ffmpeg -i yin-mad-dansumegumindansu.mp4 -ar 44100 -ac 1 -q:a 9 CANCAN1.mp3
//#define MP3_FILENAME "/001.mp3"
#define FPS 30
#define MJPEG_BUFFER_SIZE (320 * 240 * 2 / 4)
#define volumeIconWidth 20
#define volumeIconHeight 20
#include <WiFi.h>
#include <SD_MMC.h>
#include <XPT2046_Touchscreen.h>
/* Arduino_GFX */
#include <Arduino_GFX_Library.h>
#include "SerialMP3Player.h"
#define TFT_BRIGHTNESS 128
Arduino_DataBus *bus = new Arduino_ESP32SPI(27 /* DC */, 5 /* CS */, 4 /* SCK */, 22 /* MOSI */, 16 /* MISO */);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, 33, 3 /* rotation */, false /* IPS */);
#define TFT_BL 12

#define CS_PIN  17
#define TIRQ_PIN  25
#define TX 21
#define RX 26
SerialMP3Player mp3(RX, TX);
 int bs=1;
 int bf =1;
 int pt=0;
 int as;
 int k;
 int msd=0;
 int m;
 int s=0;
 int fm=0;
 int gbreak=0;
 int voice=20;
 unsigned long t=0;
 unsigned long x=0;
 unsigned long Mtime =0;
const int imageWidth = 320;  
const int imageHeight = 240; 
/* MJPEG Video */
#include "MjpegClass.h"
static MjpegClass mjpeg;
uint8_t *mjpeg_buf;
/* variables */
static unsigned long total_play_audio, total_read_video, total_decode_video, total_show_video;
static unsigned long start_ms, curr_ms, next_frame_ms;
static int skipped_frames, next_frame, time_used, total_frames;

// pixel drawing callback
static int drawMCU(JPEGDRAW *pDraw)
{

  unsigned long s = millis();
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  total_show_video += millis() - s;
  return 1;
} 
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);
void display_stat();
void Chose_file();
int Mode();

void setup()
{
  
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  mp3.showDebug(1);
  mp3.begin(9600);
  mp3.sendCommand(CMD_SEL_DEV, 0, 2);
   pinMode(13,OUTPUT);
   digitalWrite(13,LOW);
  
  gfx->begin();
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(10, 10);
  
  delay(1000);
  gfx->fillScreen(BLACK);
  ts.begin();
    
  ts.setRotation(1);
#ifdef TFT_BL
  ledcAttachPin(TFT_BL, 1);     
  ledcSetup(1, 12000, 8);       
  ledcWrite(1, TFT_BRIGHTNESS); 
#endif
  mp3.setVol(voice);
  
}

void loop()
{
  
  if(s==0){
  m=Mode();
  }
  gfx->fillScreen(BLACK);
  if(s!=1&&s!=2){
    if(m==2||m==1){
      Chose_file();
    }
    
  }
  if(fm==1){return;}
  
 
  gfx->setTextSize(1);
  gfx->setCursor(10,190);
  gfx->setTextColor(RED);
  File vFile;
  if(m==1){
    char MP3_FILENAME[15];
  sprintf(MP3_FILENAME,"/00%d.mp3",pt);
  gfx->println(MP3_FILENAME);
  }
  else if(m==2){
  char MJPEG_FILENAME[15];
  sprintf(MJPEG_FILENAME,"/cancan%d.mjpeg",pt);
  vFile = SD_MMC.open(MJPEG_FILENAME);
  gfx->println(MJPEG_FILENAME);
  }
  
  
  gfx->drawRect(210, 195, 40, 40, RED);

  gfx->drawLine(10,205,50,205,RED);
  gfx->drawLine(10,220,50,220,RED);
  gfx->drawLine(10,235,50,235,RED);
  
  
  gfx->drawLine(190, 195, 170, 215, RED);
  gfx->drawLine(170, 215, 190, 235, RED);
  gfx->drawLine(170, 195, 150, 215, RED);
  gfx->drawLine(150, 215, 170, 235, RED);
  
  gfx->drawLine(270, 195, 290, 215, RED);
  gfx->drawLine(290, 215, 270, 235, RED);
  gfx->drawLine(290, 195, 310, 215, RED);
  gfx->drawLine(310, 215, 290, 235, RED);

  gfx->drawLine(125, 200, 125, 230, RED);
  gfx->drawLine(110, 200, 110, 230, RED);

  gfx->setCursor(70,210);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->println(voice);
  gfx->setCursor(10,190);
  gfx->setTextColor(RED);

  if(m==2){
      if (!vFile || vFile.isDirectory())
  { 
    
    if(pt!=0){
    gfx->println(F("ERROR: Failed to open file for reading"));
    }
    exit(1);
  }

  Serial.println(F("PCM audio MJPEG video start"));
  
  mjpeg.setup(&vFile, mjpeg_buf, drawMCU, true, true);
  s=1;
  
  mp3.play(pt);
  mp3.play(pt);
  skipped_frames = 0;
  total_play_audio = 0;
  total_read_video = 0;
  total_decode_video = 0;
  total_show_video = 0;
  next_frame = 0;
  t=0;
  start_ms = millis();
  curr_ms = start_ms;
  next_frame_ms = start_ms + (++next_frame * 1000 / FPS);
  gbreak =0;
  while (vFile.available() && mjpeg.readMjpegBuf()) // Read video
  {
    
    if(ts.touched()){
      TS_Point p = ts.getPoint();
      Serial.print(", x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.println(p.y);
      
      if(p.y>=300&&p.y<=800){
        if(p.x<=900&&p.x>=400){s=3;mp3.stop();return;}
        else if(p.x<=3100&&p.x>=2700){s=2;break;}
        else if(p.x<=2500&&p.x>=2100){pt--;s=2;break;}
        else if (p.x<=3800&&p.x>=3300){pt++;s=2;break;}
        else if (p.x<=1800&&p.x>=1300){
            x=millis();
            gfx->fillRect(105, 200, 30, 30, BLACK);
            gfx->drawTriangle(105,200,105,230,131,215,RED);
            mp3.pause();
            while(ts.touched()){};
             delay(300);
             for(;;){
              if(ts.touched()){
                TS_Point p = ts.getPoint();
                if(p.y>=300&&p.y<=800){
                  if(p.x<=900&&p.x>=400){s=3;mp3.stop();return;}
                else if(p.x<=3100&&p.x>=2700){s=2;gbreak=1;break;}
                else if(p.x<=2500&&p.x>=2100){pt--;s=2;gbreak=1;break;}
                else if (p.x<=3800&&p.x>=3300){pt++;s=2;gbreak=1;break;}
                else if (p.x<=1800&&p.x>=1300){
                    while(ts.touched()){};
                    gfx->fillRect(105, 200, 30, 30, BLACK);
                    gfx->drawLine(125, 200, 125, 230, RED);
                    gfx->drawLine(110, 200, 110, 230, RED);
                    mp3.play();
                    t=millis()-x+t;
                    break;
                }
              }
                }
              }
          }
        }
        
    }
    if(gbreak){break;}
    total_read_video += millis() - curr_ms;
    curr_ms = millis();
    
          
      
      mjpeg.drawJpg();
      
      total_decode_video += millis() - curr_ms;
    
    curr_ms = millis();

    

    while (millis() < next_frame_ms)
    {
      vTaskDelay(1);
    }

    curr_ms = millis();
    next_frame_ms = start_ms + (++next_frame * 1000 / FPS) + t;
  }
  mp3.stop();
  time_used = millis() - start_ms - t;
  Serial.println(F("PCM audio MJPEG video end"));
  vFile.close();
  

  display_stat();
  }
  else if(m==1){
    
      Serial.println(F("MP3 start"));
      gbreak=0;
      s=1;
      
      Mtime=millis();
      mp3.play(pt);
      mp3.play(pt);
  for(;;){
  if(ts.touched()){
      TS_Point p = ts.getPoint();
      Serial.print(", x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.println(p.y);
      
      if(p.y>=300&&p.y<=800){
        if(p.x<=900&&p.x>=400){s=3;mp3.stop();return;}
        else if(p.x<=3100&&p.x>=2700){s=2;break;}
        else if(p.x<=2500&&p.x>=2100){pt--;s=2;break;}
        else if (p.x<=3800&&p.x>=3300){pt++;s=2;break;}
        else if (p.x<=1800&&p.x>=1300){
            //x=millis();
            gfx->fillRect(105, 200, 30, 30, BLACK);
            gfx->drawTriangle(105,200,105,230,131,215,RED);
            mp3.pause();
            while(ts.touched()){};
             delay(300);
             for(;;){
              if(ts.touched()){
                TS_Point p = ts.getPoint();
                if(p.y>=300&&p.y<=800){
                  if(p.x<=900&&p.x>=400){s=3;mp3.stop();return;}
                else if(p.x<=3100&&p.x>=2700){s=2;gbreak=1;break;}
                else if(p.x<=2500&&p.x>=2100){pt--;s=2;gbreak=1;break;}
                else if (p.x<=3800&&p.x>=3300){pt++;s=2;gbreak=1;break;}
                else if (p.x<=1800&&p.x>=1300){
                    while(ts.touched()){};
                    gfx->fillRect(105, 200, 30, 30, BLACK);
                    gfx->drawLine(125, 200, 125, 230, RED);
                    gfx->drawLine(110, 200, 110, 230, RED);
                    mp3.play();
                    //t=millis()-x+t;
                    break;
                }
              }
                }
              }
          }
        }
       
    }
    if(gbreak){break;}
  }
    mp3.stop();
    Mtime=millis()-Mtime;
    Serial.println(F("Music end"));
    display_stat();
  }

  
  delay(5000);
  //bs =digitalRead(26);
  
  gfx->fillScreen(BLACK);
}

#define CHART_MARGIN 24
#define LEGEND_A_COLOR 0xE0C3
#define LEGEND_B_COLOR 0x33F7
#define LEGEND_C_COLOR 0x4D69
#define LEGEND_D_COLOR 0x9A74
#define LEGEND_E_COLOR 0xFBE0
#define LEGEND_F_COLOR 0xFFE6
#define LEGEND_G_COLOR 0xA2A5


  


int Mode(){
  int n;
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->drawRect(70, 40, 180, 60, WHITE);
  gfx->drawRect(50, 140, 220, 60, WHITE);
  gfx->setTextSize(3);
  gfx->setCursor(135, 60);
  gfx->println("MP3");
  gfx->setCursor(80, 160);
  gfx->println("MJPEG+MP3");
  for(;;){
    if(ts.touched()){
      TS_Point p = ts.getPoint();
      if(p.x<=3100&&p.x>=1100&&p.y<=3200&&p.y>=2400){
        gfx->setTextColor(BLACK);
        gfx->fillRect(70, 40, 180, 60, RED);
        gfx->setTextSize(3);
        gfx->setCursor(135, 60);
        gfx->println("MP3");
        if(!ts.touched()){
        delay(500);
        n=1;
        break;
        }
      }
      else if(p.x<=3300&&p.x>=1000&&p.y<=1600&&p.y>=800){
        gfx->setTextColor(BLACK);
        gfx->fillRect(50, 140, 220, 60, RED);
        gfx->setTextSize(3);
        gfx->setCursor(80, 160);
        gfx->println("MJPEG+MP3");
        if(!ts.touched()){
        delay(500);
        n=2;
        break;
        }
      }
    }
  }
  fm=0;
  return n;
}

void Chose_file(){
gfx->setTextColor(WHITE);
  
  gfx->fillTriangle(53,95,10,120,53,145,RED);
  gfx->fillTriangle(267,95,310,120,267,145,RED);
  gfx->drawRect(90, 160, 140, 50, WHITE);
  gfx->setCursor(110, 175);
  gfx->setTextSize(3);
  gfx->println("Finish");
  gfx->setCursor(10, 20);
  gfx->println("<");
  gfx->fillTriangle(100,30,120,10,120,50,WHITE);
  gfx->fillRect(100,20,20,20,WHITE);
  gfx->fillArc(120, 30, 10, 13, 0, 50, WHITE);
  gfx->fillArc(120, 30, 18, 21, 0, 50, WHITE);
  gfx->fillArc(120, 30, 26, 29, 0, 50, WHITE);

  gfx->fillArc(120, 30, 10, 13, 310, 0, WHITE);
  gfx->fillArc(120, 30, 18, 21, 310, 0, WHITE);
  gfx->fillArc(120, 30, 26, 29, 310, 0, WHITE);
  gfx->fillRoundRect(160, 23, 150, 14, 7, WHITE);
  gfx->fillRoundRect(160, 23, voice*5, 14, 7, RED);
  gfx->fillRect(50,20,40,40,BLACK);
  gfx->setCursor(50, 20);
  gfx->setTextSize(2);
  gfx->println(voice);
  char as[30];
  for(;;){
    //bs =digitalRead(26);
    //bs==0&&bf==1
    if(ts.touched()){ 
      TS_Point p = ts.getPoint();
      Serial.println(p.x + " , "+ p.y);
      if(p.x<3700&&p.x>3300&&p.y<2200&&p.y>1800){pt++;}
      else if(p.x<900&&p.x>500&&p.y<2200&&p.y>1800){pt--;}
      else if(p.x>=2100&&p.x<3800&&p.y<3800&&p.y>3300){
        float v=(float)(p.x-2100)/56.6;
        voice=round(v);
        if(voice>=30){voice=30;}                              
        gfx->fillRoundRect(160, 23, 150, 14, 7, WHITE);
        gfx->fillRoundRect(160, 23, voice*5, 14, 7, RED);
        gfx->fillRect(50,20,40,40,BLACK);
        gfx->setCursor(50, 20);
        gfx->println(voice);
        mp3.setVol(voice); 
      }
      else if(p.x<600&&p.x>100&&p.y<3800&&p.y>3300){
        gfx->fillRect(0, 10, 40, 40, RED);
        gfx->setTextColor(WHITE);
        gfx->setTextSize(3);
        gfx->setCursor(10, 20);
        gfx->println("<");
        delay(500);
        s=0;fm=1;return;
        }
      //Serial.println(pt);
      gfx->fillRect(75, 70, 180, 50, BLACK); 
      gfx->setTextSize(2);
      gfx->setCursor(75, 70);
      //char as[30];
      if(m==2){
      sprintf(as,"/cancan%d.mjpeg",pt);
      }
      else if(m==1){
        sprintf(as,"/00%d.mp3",pt);
      }
      if(pt==0){
        gfx->println(F("cancanbox begin"));
      }
      else{
        gfx->println(as);
      }
      if(p.x<2900&&p.x>1400&&p.y<1300&&p.y>700){
        gfx->fillRect(90, 160, 140, 50, RED);
        gfx->setCursor(110, 175);
        gfx->setTextSize(3);
        gfx->setTextColor(BLACK);
        gfx->println("Finish");
        delay(500);
        break;
        }
      delay(100);
  }
  
  delay(50);
  }
    
    gfx->setTextSize(2);
    gfx->setCursor(75, 70);
    gfx->setTextColor(RED);
    if(pt==0){
        gfx->println(F("cancanbox begin"));
      }
      else{
        gfx->println(as);
      }
    delay(500); 
    Serial.println(pt);
    digitalWrite(13,HIGH);
    delay(100);
    
    gfx->fillScreen(BLACK);
  
  gfx->setTextColor(WHITE);
  if(m==2){
  if ((!SD_MMC.begin("/sdcard", true)) && (!SD_MMC.begin("/sdcard", true))) 
  {
    Serial.println(F("ERROR: SD card mount failed!"));
    gfx->println(F("ERROR: SD card mount failed!"));
    exit(1);
  }
  }

  

  mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
  if (!mjpeg_buf)
  {
    Serial.println(F("mjpeg_buf malloc failed!"));
    gfx->println(F("mjpeg_buf malloc failed!"));
    exit(1);
  }
}

void display_stat()
{
  int total_frames = next_frame - 1;
  int played_frames = total_frames - skipped_frames;
  float fps = 1000.0 * played_frames / time_used;
  
  if(m==2){
  gfx->setCursor(0, 0);
  gfx->setTextColor(WHITE);
  gfx->printf("Played frames: %d\n", played_frames);
  gfx->printf("Skipped frames: %d (%0.1f %%)\n", skipped_frames, 100.0 * skipped_frames / total_frames);
  gfx->printf("Actual FPS: %0.1f\n\n", fps);
  }
  else if(m==1){
    gfx->setCursor(0, 0);
    gfx->setTextColor(WHITE);
    gfx->printf("Time used: %d\n",Mtime/1000);
  }
  
}
