#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>

#define SS_PIN 5
#define RST_PIN 33
#define BUZZER_PIN 14

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

// Configuration WiFi
const char* ssid = "ssidname";
const char* password = "motdepasse";

// UID autorisés
String allowedUID = "D5D4A4AC";

// Stockage des logs (max 50 entrées)
struct LogEntry {
  String uid;
  String timestamp;
  bool authorized;
};

const int MAX_LOGS = 50;
LogEntry logEntries[MAX_LOGS];
int logCount = 0;
int accessGranted = 0;
int accessDenied = 0;

// Configuration NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

void beepOK() {
  tone(BUZZER_PIN, 2000, 200);
}

void beepError() {
  tone(BUZZER_PIN, 400, 500);
}

bool isAuthorized(String uid) {
  return (uid == allowedUID);
}

String getTimeStamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Non disponible";

  char buffer[30];
  strftime(buffer, 30, "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

void addLog(String uid, bool authorized) {
  if (logCount >= MAX_LOGS) {
    // Décaler les logs (supprimer le plus ancien)
    for (int i = 0; i < MAX_LOGS - 1; i++) {
      logEntries[i] = logEntries[i + 1];
    }
    logCount = MAX_LOGS - 1;
  }
  
  logEntries[logCount].uid = uid;
  logEntries[logCount].timestamp = getTimeStamp();
  logEntries[logCount].authorized = authorized;
  logCount++;
  
  if (authorized) {
    accessGranted++;
  } else {
    accessDenied++;
  }
}

// Page d'accueil avec design moderne
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Système RFID - Contrôle d'Accès</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .header {
            background: white;
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            margin-bottom: 30px;
            text-align: center;
        }
        
        .header h1 {
            color: #667eea;
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        
        .header p {
            color: #666;
            font-size: 1.1em;
        }
        
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .stat-card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
            transition: transform 0.3s;
        }
        
        .stat-card:hover {
            transform: translateY(-5px);
        }
        
        .stat-card h3 {
            color: #666;
            font-size: 0.9em;
            margin-bottom: 10px;
            text-transform: uppercase;
        }
        
        .stat-value {
            font-size: 2.5em;
            font-weight: bold;
            margin-bottom: 5px;
        }
        
        .stat-card.success .stat-value {
            color: #10b981;
        }
        
        .stat-card.danger .stat-value {
            color: #ef4444;
        }
        
        .stat-card.info .stat-value {
            color: #3b82f6;
        }
        
        .logs-section {
            background: white;
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }
        
        .logs-section h2 {
            color: #333;
            margin-bottom: 20px;
            padding-bottom: 15px;
            border-bottom: 3px solid #667eea;
        }
        
        .log-table {
            width: 100%;
            border-collapse: collapse;
            overflow-x: auto;
            display: block;
        }
        
        .log-table thead {
            background: #f8f9fa;
        }
        
        .log-table th {
            padding: 15px;
            text-align: left;
            color: #666;
            font-weight: 600;
            text-transform: uppercase;
            font-size: 0.85em;
        }
        
        .log-table td {
            padding: 15px;
            border-bottom: 1px solid #eee;
        }
        
        .log-table tbody tr:hover {
            background: #f8f9fa;
        }
        
        .badge {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.85em;
            font-weight: 600;
        }
        
        .badge-success {
            background: #d1fae5;
            color: #065f46;
        }
        
        .badge-danger {
            background: #fee2e2;
            color: #991b1b;
        }
        
        .uid-text {
            font-family: 'Courier New', monospace;
            background: #f3f4f6;
            padding: 5px 10px;
            border-radius: 5px;
            font-size: 0.9em;
        }
        
        .refresh-info {
            text-align: center;
            margin-top: 20px;
            color: #666;
            font-size: 0.9em;
        }
        
        .no-logs {
            text-align: center;
            padding: 40px;
            color: #999;
            font-style: italic;
        }
        
        @media (max-width: 768px) {
            .header h1 {
                font-size: 1.8em;
            }
            
            .stat-value {
                font-size: 2em;
            }
            
            .log-table {
                font-size: 0.85em;
            }
            
            .log-table th, .log-table td {
                padding: 10px;
            }
        }
    </style>
    <script>
        // Actualisation automatique toutes les 3 secondes
        setTimeout(function() {
            location.reload();
        }, 3000);
    </script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔐 Système de Contrôle d'Accès RFID</h1>
            <p>Surveillance en temps réel</p>
        </div>
        
        <div class="stats-grid">
            <div class="stat-card success">
                <h3>Accès Autorisés</h3>
                <div class="stat-value">)rawliteral" + String(accessGranted) + R"rawliteral(</div>
                <p>✓ Validés</p>
            </div>
            
            <div class="stat-card danger">
                <h3>Accès Refusés</h3>
                <div class="stat-value">)rawliteral" + String(accessDenied) + R"rawliteral(</div>
                <p>✗ Rejetés</p>
            </div>
            
            <div class="stat-card info">
                <h3>Total Tentatives</h3>
                <div class="stat-value">)rawliteral" + String(logCount) + R"rawliteral(</div>
                <p>📊 Enregistrées</p>
            </div>
        </div>
        
        <div class="logs-section">
            <h2>📋 Historique des Accès</h2>
            )rawliteral";
  
  if (logCount == 0) {
    html += R"rawliteral(
            <div class="no-logs">
                Aucune activité enregistrée pour le moment
            </div>
    )rawliteral";
  } else {
    html += R"rawliteral(
            <table class="log-table">
                <thead>
                    <tr>
                        <th>#</th>
                        <th>Statut</th>
                        <th>UID de la carte</th>
                        <th>Date et Heure</th>
                    </tr>
                </thead>
                <tbody>
    )rawliteral";
    
    // Afficher les logs en ordre inverse (plus récent d'abord)
    for (int i = logCount - 1; i >= 0; i--) {
      html += "<tr>";
      html += "<td>" + String(logCount - i) + "</td>";
      
      if (logEntries[i].authorized) {
        html += "<td><span class='badge badge-success'>✓ AUTORISÉ</span></td>";
      } else {
        html += "<td><span class='badge badge-danger'>✗ REFUSÉ</span></td>";
      }
      
      html += "<td><span class='uid-text'>" + logEntries[i].uid + "</span></td>";
      html += "<td>" + logEntries[i].timestamp + "</td>";
      html += "</tr>";
    }
    
    html += R"rawliteral(
                </tbody>
            </table>
    )rawliteral";
  }
  
  html += R"rawliteral(
            <div class="refresh-info">
                🔄 Actualisation automatique toutes les 3 secondes
            </div>
        </div>
    </div>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initialisation");
  lcd.setCursor(0, 1);
  lcd.print("Systeme RFID");

  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connexion WiFi");
  lcd.setCursor(0, 1);
  lcd.print("En cours...");

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connecte");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());

    Serial.println("\n=================================");
    Serial.println("WiFi connecté avec succès!");
    Serial.print("Adresse IP du serveur: ");
    Serial.println(WiFi.localIP());
    Serial.println("=================================");

    // Configuration de l'heure
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Initialisation RFID
    SPI.begin();
    rfid.PCD_Init();

    // Démarrage du serveur web
    server.on("/", handleRoot);
    server.begin();
    Serial.println("Serveur web démarré sur le port 80");

    beepOK();
    delay(2000);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Erreur WiFi");
    lcd.setCursor(0, 1);
    lcd.print("Verifier config");
    beepError();
    Serial.println("Échec de connexion WiFi!");
  }

  lcd.clear();
}

void loop() {
  server.handleClient();

  lcd.setCursor(0, 0);
  lcd.print("Presentez votre");
  lcd.setCursor(0, 1);
  lcd.print("carte RFID      ");

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Lecture de l'UID
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  lcd.clear();
  bool authorized = isAuthorized(uid);
  
  if (authorized) {
    lcd.setCursor(0, 0);
    lcd.print("** BIENVENUE **");
    lcd.setCursor(0, 1);
    lcd.print("Acces autorise");
    beepOK();
    Serial.println("[ACCÈS AUTORISÉ] UID: " + uid);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("*** REFUSE ***");
    lcd.setCursor(0, 1);
    lcd.print("Acces refuse");
    beepError();
    Serial.println("[ACCÈS REFUSÉ] UID: " + uid);
  }

  // Ajouter au log
  addLog(uid, authorized);

  delay(2500);
  lcd.clear();
  
  // Arrêter la lecture RFID
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
