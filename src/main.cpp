#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80); // Create a webserver object that listens for HTTP request on port 80

const int led = D0;
const char *ssid = "SprinklerSystem";
const char *password = "1234567890";

const char rootPage[] PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
  <title>Sprinker Wifi System</title>
</head>
<h2>Settings</h2>
<form action="/SPRINKLER" method="post">
  <label for="timer">Set Timer (seconds):</label>
  <input type="number" id="timer" name="timer" min="1" value="10">
  <br><br>
  <label for="duration">Set Duration of Sprinkler in (seconds):</label>
  <input type="number" id="duration" name="duration" min="1" value="20">
  <br><br>
  <input type="submit" value="Set Timer and Duration">
</form>
<div id="countdown"></div>
</body>
</html>
)";

const char statusPage[] PROGMEM = R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Status Page</title>
</head>
<body>
<h1>Status Page</h1>
<br>
<br>
<form action="/" method="GET">
  <input type="submit" value="Back to Settings">
</form>
<h3 id="timer"></h3> 
<div id="countdown"></div>
<h3 id="duration"></h3>
<div id="durationcountdown"></div>
  <script>
    // Function to extract URL parameters
    function getUrlParams() {
      const urlParams = new URLSearchParams(window.location.search);
      const timer = urlParams.get('timer');
      const duration = urlParams.get('duration');

      // Display the values in HTML
      document.getElementById('timer').textContent = `Timer is set to ${timer}`;
      document.getElementById('duration').textContent = `Duration of sprinkler is set to ${duration}`;

      // Start the countdownTimer
      startCountdownTimer(parseInt(timer), "countdown", ()=> startCountdownTimer(parseInt(duration), "durationcountdown", ()=> window.location.reload()) )
    }

    function startCountdownTimer(seconds, element, callback = () => {}) {
      var countdownDiv = document.getElementById(element);
      countdownDiv.innerText = seconds + " seconds remaining";

      var timer = setInterval(function() {
        seconds--;
        countdownDiv.innerText = seconds + " seconds remaining";
        if (seconds <= 0) {
          clearInterval(timer)
          countdownDiv.innerText = "Timer completed";
          callback();
          return ;
        }
      }, 1000);
    }

    // Call the function when the page loads
    window.onload = getUrlParams;
  </script>
</body>
</html>
)";

void handleRoot(); // function prototypes for HTTP handlers
void handleSprinklerSettings();
void handleStatusPage();
void handleNotFound();

void setup(void)
{
  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.softAP(ssid, password);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/STATUS", HTTP_GET, handleStatusPage);
  server.on("/SPRINKLER", HTTP_POST, handleSprinklerSettings);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void)
{
  server.handleClient();
}

void handleRoot()
{
  server.send(200, "text/html", rootPage);
}

void handleStatusPage()
{
  server.send(200, "text/html", statusPage);
}

void handleSprinklerSettings()
{
  Serial.println("EARL_DEBUG");
  if (server.args() > 0)
  {
    String timerStr = server.arg("timer");
    String durationStr = server.arg("duration");
    int timerValue = timerStr.toInt();
    int durationValue = durationStr.toInt();
    if (timerValue > 0 && durationValue > 0)
    {
      server.sendHeader("Location", "/STATUS?timer="+timerStr+"&duration="+durationStr);
      server.send(303);
    }
    else
    {
      server.sendHeader("Location", "/");
      server.send(400, "text/plain", "Invalid timer value");
    }
  }
  else
  {
    server.sendHeader("Location", "/");
    server.send(400, "text/plain", "No timer value received");
  }
}

void handleNotFound()
{
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
