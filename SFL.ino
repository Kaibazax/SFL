// #include <SPI.h>
// #define TEST_STEP_MOTOR
// #define TEST_LED
// #define VERBOSE
#include "Preferences.h"
Preferences preferences;
#define S_ENA 15
#define S_DIR 4
#define S_PUL 2
#define S_Ms 100

#define NO 0X1
#define NC 0X0

#define LM_P 12
#define BTN_P 14
#define DOOR_P 27
#define MU_P 0
#define MD_P 4

#define Led0 25
#define Led1 26

uint32_t loopi = 0;
uint32_t ms = 0;
uint32_t lms = 0;
uint8_t No_Ps[] = {22, 21, 18, 19, 23, 5, 13};

uint8_t Max = 3;
uint8_t state = 0;
#define s_idle 0
#define s_up 1
#define s_down 2
#define s_complete 3
#define s_door 4
#define s_error 5

uint8_t Led0EnCode[] = {
    0b1111110, 0b11000, 0b1101101, 0b111101,
    0b11011, 0b110111, 0b1110111,
    0b11100, 0b1111111, 0b111111};
uint8_t Led1EnCode[] = {
    0, 0b11011, 0b111100, 0b11000,
    0b100111, 0b101, 0b100010,
    0b1000000, 0b1000000, 0b1000000};

uint8_t cur = 1; // 0-4

uint8_t HBits = 0;
uint8_t DBits = 0;
uint8_t BBits = 0;

uint8_t HiBits = 0;
uint8_t DiBits = 0;
uint8_t BiBits = 0;

uint8_t HtCount = 0;
uint8_t displayNo = 0;
uint8_t dlState = 0;
uint8_t s = 0;
uint8_t request = 0xF;
uint8_t lrequest = 0xF;
uint8_t loopNo = 6;

#pragma region OTA
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>

const char *host = "esp32";
const char *ssid = "LanAnh";
const char *password = "83292164";
char timerOutCode[100];
#define LED_BUILTIN 2
WebServer server(80);
uint8_t WState = 0;
const char *serverindex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "<div id='prg'>progress: 0%</div>"
    "<script>"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";
TaskHandle_t t_ota;
uint32_t wlms = 0;
uint32_t timerOut[7][7]; // 8*7*7=392
void ota_loop(void *parameter)
{
    for (;;)
    {
        bool sbegin = 0;
        switch (WState)
        {
        case 0: // Start
            // Connect to WiFi network
            WiFi.begin(ssid, password);
#ifdef VERBOSE
            Serial.println("Connecting");
#endif
            wlms = ms;
            WState = 1;
            break;
        case 1: // Wait conn
            digitalWrite(LED_BUILTIN, loopi % 2 == 0);
            if (WiFi.status() == WL_CONNECTED)
            {
                WState = 2;
#ifdef VERBOSE
                Serial.println("");
                Serial.print("Connected to ");
                Serial.println(ssid);
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());
#endif
                sbegin = 1;
            }
            else if (60 * 1000 < ms - wlms && ms > wlms)
            {
                WState = 3;
                WiFi.softAP("esp-32");
#ifdef VERBOSE
                Serial.print("AP esp-32");
#endif
                sbegin = 1;
            }
            if (sbegin)
            { /*return index page which is stored in serverIndex */
                server.on("/", HTTP_GET, []()
                          {
                    wlms = ms;
                    server.sendHeader("Connection", "close");
                    server.send(200, "text/html", serverindex); });
                sprintf(timerOutCode, "<table>"
                                      "<tr><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td></tr>"
                                      "<tr><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td></tr>"
                                      "<tr><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td></tr>"
                                      "<tr><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td></tr>"
                                      "<tr><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td></tr>"
                                      "<tr><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td></tr>"
                                      "<tr><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td><td>%d<td></tr>"
                                      "</table>",
                        timerOut[0][0], timerOut[0][1], timerOut[0][2], timerOut[0][3], timerOut[0][4], timerOut[0][5], timerOut[0][6],
                        timerOut[1][0], timerOut[1][1], timerOut[1][2], timerOut[1][3], timerOut[1][4], timerOut[1][5], timerOut[1][6],
                        timerOut[2][0], timerOut[2][1], timerOut[2][2], timerOut[2][3], timerOut[2][4], timerOut[2][5], timerOut[2][6],
                        timerOut[3][0], timerOut[3][1], timerOut[3][2], timerOut[3][3], timerOut[3][4], timerOut[3][5], timerOut[3][6],
                        timerOut[4][0], timerOut[4][1], timerOut[4][2], timerOut[4][3], timerOut[4][4], timerOut[4][5], timerOut[4][6],
                        timerOut[5][0], timerOut[5][1], timerOut[5][2], timerOut[5][3], timerOut[5][4], timerOut[5][5], timerOut[5][6],
                        timerOut[6][0], timerOut[6][1], timerOut[6][2], timerOut[6][3], timerOut[6][4], timerOut[6][5], timerOut[6][6]);
                server.on("/timeout", HTTP_GET, []()
                          {
                    wlms = ms;
                    server.sendHeader("Connection", "close");
                    server.send(200, "text/html", timerOutCode); });
                server.on("/timeout-clear", HTTP_GET, []()
                          {
                    wlms = ms;
                    server.sendHeader("Connection", "close");
                    preferences.clear(); });
                /*handling uploading firmware file */
                server.on(
                    "/update", HTTP_POST, []()
                    {
                    wlms = ms;
                    server.sendHeader("Connection", "close");
                    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
                    ESP.restart(); },
                    []()
                    {
                        HTTPUpload &upload = server.upload();
                        if (upload.status == UPLOAD_FILE_START)
                        {
                            Serial.printf("Update: %s\n", upload.filename.c_str());
                            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                            { // start with max available size
                                Update.printError(Serial);
                            }
                        }
                        else if (upload.status == UPLOAD_FILE_WRITE)
                        {
                            /* flashing firmware to ESP*/
                            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                            {
                                Update.printError(Serial);
                            }
                        }
                        else if (upload.status == UPLOAD_FILE_END)
                        {
                            if (Update.end(true))
                            { // true to set the size to the current progress
                                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                            }
                            else
                            {
                                Update.printError(Serial);
                            }
                        }
                    });
                server.begin();
                wlms = ms;
            }
            break;
        case 2:
            server.handleClient();
            digitalWrite(LED_BUILTIN, loopi % 4 != 0);
            if (300000 < ms - wlms && ms > wlms)
            {
                WState = 4;
                server.stop();
            }
            break;
        case 3: // AP
            server.handleClient();
            digitalWrite(LED_BUILTIN, loopi % 8 == 1 || loopi % 8 == 3 || loopi % 8 == 5);
            if (300000 < ms - wlms && ms > wlms)
            {
                WState = 4;
                server.stop();
#ifdef VERBOSE
                Serial.print("Stoped");
#endif
            }
            break;
        case 4:
            digitalWrite(LED_BUILTIN, LOW);
            break;
        }
        delay(1);
    }
}
#pragma endregion
#pragma region NNTimer
void initOutTimer()
{
    preferences.begin("Elv7Seg", false);
    char str[5];
    for (int8_t i = 0; i < 7; i++)
    {
        for (int8_t j = 0; j < 7; j++)
        {
            sprintf(str, "to%d%d", i, j);
            timerOut[i][j] = preferences.getULong(str, 0);
        }
    }
}
uint32_t to_ms = 0;
uint32_t to_MaxMs = 0;
uint8_t to_cur = 0xF;
uint8_t to_req = 0xF;
bool checkTimeOut(uint8_t cur, uint8_t req, uint32_t ms)
{
    if (req == 0xF)
    {
        if (to_cur < 7 && to_req < 7)
        {
            if (to_MaxMs == 0 || to_ms < to_MaxMs)
            {
                timerOut[to_cur][to_req] = to_ms;
                char str[5];
                sprintf(str, "to%d%d", to_cur, to_req);
                preferences.putULong(str, to_ms);
            }
        }
        to_cur = cur;
        to_req = 0xF;
        to_ms = 0;
    }
    else if (to_req == 0xF)
    {
        to_req = req;
        to_ms = ms;
        if (to_cur < 7 && to_req < 7)
        {
            to_MaxMs = timerOut[to_cur][to_req] * 1.1;
        }
    }
    else
    {
        int32_t cms = ms - to_ms;
        if (cms > 0 && cms > to_MaxMs)
            return true;
    }
    return false;
}
#pragma endregion
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(MU_P, OUTPUT);
    pinMode(MD_P, OUTPUT);
    pinMode(Led0, OUTPUT);
    pinMode(Led1, OUTPUT);
    digitalWrite(MU_P, NO);
    digitalWrite(MD_P, NO);
    pinMode(LM_P, INPUT_PULLUP);
    pinMode(BTN_P, INPUT_PULLUP);
    pinMode(DOOR_P, INPUT_PULLUP);

    for (uint8_t i = 6; i < 7; i--)
    {
        pinMode(No_Ps[i], OUTPUT);
    }
    delay(10);
    // for (uint8_t i = 4; i > 1; i--)
    // {
    //     if (!digitalRead(No_Ps[i]))
    //     {
    //         Max = i - 1;
    //     }
    // }
    xTaskCreatePinnedToCore(
        ota_loop, /* Function to implement the task */
        "t_ota",  /* Name of the task */
        10000,    /* Stack size in words */
        NULL,     /* Task input parameter */
        0,        /* Priority of the task */
        &t_ota,   /* Task handle. */
        0);       /* Core where the task should run */
    initOutTimer();
#ifdef VERBOSE
    Serial.begin(115200);
#endif
}

void WriteNoBus(uint8_t no)
{
    for (uint8_t bi = 6; bi < 7; bi--)
    {
        digitalWrite(No_Ps[bi], bi == no);
    }
}
void loop()
{
    WriteNoBus(loopNo);
#ifdef TEST_LED
    if (Serial.available() > 0)
    {
        state = Serial.read();
        // while(!Serial.available() == 0);
        displayNo = state;
        Serial.printf("display %d %d", state, displayNo);
    }
#else
#ifdef VERBOSE
    if (loopi % 100 == 0)
        Serial.printf("%x %x %x s%d l%d c%x r%x \n", HBits, BBits, DBits, state, loopNo, cur, request);
#endif
#pragma region Input
    uint8_t iCur = 0xF;
    switch (loopNo)
    {
    case 2: // Cur
        HtCount = 0;
        for (uint8_t i = 0; i <= Max; i++)
        {
            if (bitRead(HBits, i))
            {
                iCur = i;
                HtCount++;
            }
        }
        if (HtCount > 1)
            state = s_error;
        if (iCur != 0xF)
            cur = iCur;
        break;
    case 3: // Btn
        for (uint8_t i = 0; i <= Max; i++)
        {
            if (state != s_idle && state != s_complete)
                break;
            if (bitRead(BBits, i))
            {
                request = i;
                state = s_idle;
                break;
            }
        }
        break;
    case 4: // Door
        for (uint8_t i = 0; i <= Max; i++)
        {
            if (bitRead(DBits, i))
            {
                iCur = i;
                break;
            }
        }
        if (iCur != 0xF && state < s_error)
        {
            state = s_door;
            request = 0xF;
            displayNo = iCur;
        }
        else if (state == s_complete)
        {
            state = s_idle;
        }
        break;
    }
#pragma endregion
#pragma region State
    if (state == s_error) // Error
    {
        // state = 7;
        digitalWrite(MU_P, NO);
        digitalWrite(MD_P, NO);
    }
    else if (state < s_error)
    {
        if (request == cur || request > Max)
        {
            request = 0xF;
            state = s_idle; // TODO:?
            digitalWrite(MU_P, NO);
            digitalWrite(MD_P, NO);
        }
        else if (request > cur)
        {
            if (state == s_down || checkTimeOut(cur, request, ms))
            {
                state = s_error;
                digitalWrite(MU_P, NO);
                digitalWrite(MD_P, NO);
            }
            else
            {
                state = s_up;
                digitalWrite(MU_P, NC);
                digitalWrite(MD_P, NO);
#ifdef TEST_STEP_MOTOR
                digitalWrite(S_DIR, NC);
#endif
            }
        }
        else
        {
            if (state == s_up || checkTimeOut(cur, request, ms))
            {
                state = s_error;
                digitalWrite(MU_P, NO);
                digitalWrite(MD_P, NO);
            }
            else
            {
                state = s_down;
                digitalWrite(MU_P, NO);
                digitalWrite(MD_P, NC);
#ifdef TEST_STEP_MOTOR
                digitalWrite(S_DIR, NO);
#endif
            }
        }
    }
    else
    {
        digitalWrite(MU_P, NO);
        digitalWrite(MD_P, NO);
    }
#pragma endregion
#pragma region Render
    if (state == s_error)
    {
        if (HtCount > 1)
        {
            uint8_t j = (loopi >> 2) % HtCount;
            for (uint8_t i = 0; i <= Max; i++)
            {
                if (j == 0)
                {
                    displayNo = i;
                    break;
                }
                if (bitRead(HBits, i))
                    j--;
            }
        }
        else
            displayNo = 8;
    }
    else if (state != s_door)
    {
        displayNo = cur;
    }
#pragma endregion
#endif
    dlState = state;
    digitalWrite(Led0, bitRead(Led0EnCode[displayNo], loopNo));
    if ((loopi % 4) < 2)
    {
        if (dlState == s_error)
            dlState++;
        else if (dlState == s_door)
            dlState--;
    }
    // if (state == s_complete && loopNo == cur)
    //     digitalWrite(Led1, HIGH);
    digitalWrite(Led1, bitRead(Led1EnCode[dlState], loopNo));
    delay(2);
    if (digitalRead(LM_P))
        bitSet(HiBits, loopNo);
    else
        bitClear(HiBits, loopNo);
    if (!digitalRead(BTN_P))
        bitSet(BiBits, loopNo);
    else
        bitClear(BiBits, loopNo);
    if (digitalRead(DOOR_P))
        bitSet(DiBits, loopNo);
    else
        bitClear(DiBits, loopNo);
    loopNo--;
    if (loopNo > 6)
    {
        loopNo = 6;
        HBits = HiBits;
        BBits = BiBits;
        DBits = DiBits;
    }
    ms = millis();
    if (ms - lms > 100)
    {
        loopi++;
        lms = ms;
    }
}