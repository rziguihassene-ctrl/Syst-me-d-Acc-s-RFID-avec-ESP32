# 🔐 Système de Contrôle d'Accès RFID avec ESP32

Un système de contrôle d'accès complet utilisant ESP32, RFID-RC522 et interface web moderne pour la surveillance en temps réel.

![ESP32 RFID System](https://img.shields.io/badge/ESP32-RFID-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Version](https://img.shields.io/badge/version-1.0-orange)

## ✨ Fonctionnalités

- ✅ Authentification par carte/tag RFID
- 📊 Interface web moderne avec statistiques en temps réel
- 📱 Design responsive (PC, tablette, mobile)
- 🔔 Feedback sonore (buzzer) et visuel (LCD)
- 📝 Historique des 50 dernières tentatives d'accès
- 🕐 Horodatage précis via NTP
- 🔄 Actualisation automatique de la page web

## 📋 Matériel Nécessaire

| Composant | Description |
|-----------|-------------|
| **ESP32** | Carte de développement |
| **Module RFID-RC522** | Lecteur RFID 13.56MHz |
| **Écran LCD I2C 16x2** | Affichage (adresse I2C: 0x27 ou 0x3F) |
| **Buzzer actif** | Feedback sonore |
| **2 Tags/Cartes RFID** | Pour tester le système |
| **Câbles de connexion** | Jumper wires mâle-femelle |
| **Breadboard** | (Optionnel) Pour le prototypage |

## 🔌 Schéma de Connexion

### RFID-RC522 → ESP32

| RFID-RC522 | ESP32 |
|------------|-------|
| SDA (SS) | Pin 5 |
| SCK | Pin 18 |
| MOSI | Pin 23 |
| MISO | Pin 19 |
| IRQ | Non connecté |
| GND | GND |
| RST | Pin 33 |
| 3.3V | 3.3V |

### LCD I2C 16x2 → ESP32

| LCD I2C | ESP32 |
|---------|-------|
| GND | GND |
| VCC | 5V |
| SDA | Pin 21 (GPIO 21) |
| SCL | Pin 22 (GPIO 22) |

> **⚠️ Note:** Si le LCD ne fonctionne pas, l'adresse I2C peut être **0x3F** au lieu de **0x27**. Utilisez un scanner I2C pour vérifier l'adresse correcte.

### Buzzer → ESP32

| Buzzer | ESP32 |
|--------|-------|
| Pin Positif (+) | 3.3V |
| I/O | Pin 14 |
| Pin Négatif (-) | GND |

## 📦 Installation

### 1. Prérequis

Installez l'IDE Arduino et les bibliothèques suivantes via le Gestionnaire de bibliothèques :

- **MFRC522** (par GithubCommunity)
- **LiquidCrystal I2C** (par Frank de Brabander)
- **ESP32 Board** (via le Gestionnaire de cartes)

### 2. Configuration du code

Avant de téléverser le code, **vous devez modifier** les paramètres suivants :

#### 🔑 Configuration WiFi

Modifiez ces lignes avec vos identifiants WiFi :

```cpp
const char* ssid = "VOTRE_NOM_WIFI";        // Remplacez par votre SSID
const char* password = "VOTRE_MOT_DE_PASSE"; // Remplacez par votre mot de passe
```

#### 🏷️ Configuration de l'UID RFID

**Important :** Vous devez obtenir l'UID de votre propre carte RFID !

**Comment obtenir l'UID de votre carte :**

1. Ouvrez le **Moniteur Série** dans l'IDE Arduino
2. Scannez votre carte RFID
3. L'UID s'affichera dans le format : `[ACCÈS REFUSÉ] UID: XXXXXXXX`
4. Copiez cet UID et modifiez cette ligne :

```cpp
String allowedUID = "XXXXXXXX";  // Remplacez par l'UID de votre carte
```

**Exemple :**
```cpp
// Si votre UID est D5A3B2C1
String allowedUID = "D5A3B2C1";
```

#### 🌍 Configuration du fuseau horaire (optionnel)

Par défaut, le système utilise GMT+1. Pour modifier :

```cpp
const long gmtOffset_sec = 3600;      // GMT+1 = 3600, GMT+2 = 7200, etc.
const int daylightOffset_sec = 0;     // Heure d'été (3600 si applicable)
```

### 3. Téléversement

1. Sélectionnez votre carte ESP32 dans **Outils → Type de carte**
2. Sélectionnez le port COM approprié
3. Cliquez sur **Téléverser**
4. Attendez la fin du téléversement

## 🚀 Utilisation

### 1. Démarrage

Après le téléversement :
- L'ESP32 se connecte au WiFi
- L'adresse IP s'affiche sur le LCD et dans le Moniteur Série
- Le système est prêt à scanner les cartes

### 2. Accès à l'interface web

1. Notez l'adresse IP affichée (exemple: `192.168.1.100`)
2. Ouvrez un navigateur web
3. Entrez l'adresse IP : `http://192.168.1.100`
4. L'interface web s'affiche avec les statistiques en temps réel

### 3. Scanner une carte

- Approchez une carte RFID du lecteur
- Le système vérifie l'UID
- **Carte autorisée** : 
  - LCD affiche "BIENVENUE"
  - Buzzer émet un bip court
  - Log marqué en vert sur l'interface web
- **Carte refusée** :
  - LCD affiche "REFUSE"
  - Buzzer émet un bip long
  - Log marqué en rouge sur l'interface web

### Interface Web
- Statistiques en temps réel (autorisés/refusés/total)
- Historique des 50 dernières tentatives
- Design moderne et responsive
- Actualisation automatique toutes les 3 secondes

## 🛠️ Dépannage

### Le LCD n'affiche rien
- Vérifiez les connexions SDA/SCL
- L'adresse I2C est peut-être 0x3F au lieu de 0x27
- Modifiez cette ligne : `LiquidCrystal_I2C lcd(0x3F, 16, 2);`

### Le WiFi ne se connecte pas
- Vérifiez le SSID et le mot de passe
- Assurez-vous que le réseau est en 2.4GHz (ESP32 ne supporte pas 5GHz)
- Vérifiez la portée du signal WiFi

### Le RFID ne lit pas les cartes
- Vérifiez toutes les connexions
- Assurez-vous que le module est alimenté en 3.3V
- La distance maximale de lecture est d'environ 3-5cm

### L'heure ne s'affiche pas correctement
- Vérifiez la connexion Internet
- Le serveur NTP peut prendre quelques secondes pour se synchroniser
- Vérifiez le fuseau horaire dans la configuration

## 📝 Ajouter plusieurs cartes autorisées

Pour autoriser plusieurs cartes, modifiez le code comme suit :

```cpp
// Remplacez cette ligne :
String allowedUID = "D5D4A4AC";

// Par un tableau de UIDs :
String allowedUIDs[] = {"D5D4A4AC", "A1B2C3D4", "E5F6G7H8"};
int numAllowedUIDs = 3;

// Et modifiez la fonction isAuthorized :
bool isAuthorized(String uid) {
  for (int i = 0; i < numAllowedUIDs; i++) {
    if (uid == allowedUIDs[i]) return true;
  }
  return false;
}


⭐ **Si ce projet vous a été utile, n'oubliez pas de mettre une étoile !**
