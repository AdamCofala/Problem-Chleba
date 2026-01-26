#pragma once

// =============================================================================
// HTML PAGES - Sourdough Incubator Web Interface (Dark Bread Theme)
// =============================================================================
//
// This file contains the embedded HTML/CSS/JavaScript for the web interface.
// The interface provides:
// - Real-time sensor data display
// - WiFi configuration for email notifications
// - Email recipient configuration
//
// Stored in PROGMEM to save RAM on ESP32.
// =============================================================================

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='pl'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Inkubator Zakwasu</title>
  <style>
    :root {
      --bg-dark: #1a1410;
      --bg-card: #2a2218;
      --bg-card-hover: #3a3028;
      --primary: #d4a574;
      --primary-light: #e8c49a;
      --primary-dark: #a67c52;
      --accent: #8b5a2b;
      --text: #f5e6d3;
      --text-muted: #a89888;
      --success: #6b8e4e;
      --warning: #d4a04a;
      --danger: #c45c4a;
      --border: #3d3228;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: var(--bg-dark);
      background-image:
        radial-gradient(ellipse at top, #2a1f15 0%, transparent 50%),
        radial-gradient(ellipse at bottom, #1f1a14 0%, transparent 50%);
      color: var(--text);
      min-height: 100vh;
    }
    .navbar {
      background: linear-gradient(135deg, #2a1f15 0%, #1a1410 100%);
      border-bottom: 1px solid var(--border);
      color: var(--primary-light);
      padding: 15px 20px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      box-shadow: 0 4px 20px rgba(0,0,0,0.4);
    }
    .navbar h1 {
      font-size: 1.4em;
      font-weight: 600;
      letter-spacing: 0.5px;
    }
    .navbar .status {
      display: flex;
      align-items: center;
      gap: 8px;
      font-size: 0.85em;
      color: var(--text-muted);
    }
    .status-dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--success);
      box-shadow: 0 0 10px var(--success);
      animation: pulse 2s infinite;
    }
    .status-dot.offline {
      background: var(--danger);
      box-shadow: 0 0 10px var(--danger);
      animation: none;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; box-shadow: 0 0 10px var(--success); }
      50% { opacity: 0.6; box-shadow: 0 0 5px var(--success); }
    }
    .container {
      max-width: 800px;
      margin: 20px auto;
      padding: 0 15px;
    }
    .card {
      background: var(--bg-card);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 24px;
      margin-bottom: 20px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.3);
      transition: transform 0.2s, box-shadow 0.2s;
    }
    .card:hover {
      transform: translateY(-2px);
      box-shadow: 0 8px 30px rgba(0,0,0,0.4);
    }
    .card h2 {
      color: var(--primary);
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      gap: 10px;
      font-weight: 600;
      font-size: 1.2em;
    }
    .sensor-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
      gap: 15px;
    }
    .sensor-item {
      background: linear-gradient(145deg, #322a20, #2a2218);
      border: 1px solid var(--border);
      border-radius: 12px;
      padding: 20px 15px;
      text-align: center;
      transition: all 0.3s;
    }
    .sensor-item:hover {
      border-color: var(--primary-dark);
      background: linear-gradient(145deg, #3a3228, #322a20);
    }
    .sensor-item .label {
      font-size: 0.85em;
      color: var(--text-muted);
      margin-bottom: 8px;
    }
    .sensor-item .value {
      font-size: 2.2em;
      font-weight: 700;
      color: var(--primary-light);
      text-shadow: 0 2px 10px rgba(212,165,116,0.3);
    }
    .sensor-item .unit {
      font-size: 0.8em;
      color: var(--text-muted);
      margin-top: 4px;
    }
    .nav-tabs {
      display: flex;
      gap: 8px;
      margin-bottom: 20px;
      flex-wrap: wrap;
      background: var(--bg-card);
      padding: 8px;
      border-radius: 12px;
      border: 1px solid var(--border);
    }
    .nav-tabs button {
      flex: 1;
      min-width: 100px;
      padding: 12px 16px;
      border: none;
      background: transparent;
      color: var(--text-muted);
      border-radius: 8px;
      cursor: pointer;
      font-weight: 600;
      font-size: 0.9em;
      transition: all 0.3s;
    }
    .nav-tabs button:hover {
      background: var(--bg-card-hover);
      color: var(--text);
    }
    .nav-tabs button.active {
      background: linear-gradient(135deg, var(--primary-dark), var(--accent));
      color: #fdf6e3;
      box-shadow: 0 4px 15px rgba(212,165,116,0.3);
    }
    .tab-content { display: none; }
    .tab-content.active { display: block; }
    .form-group { margin-bottom: 20px; }
    .form-group label {
      display: block;
      margin-bottom: 8px;
      font-weight: 600;
      color: var(--primary);
      font-size: 0.9em;
    }
    .form-group input {
      width: 100%;
      padding: 14px 16px;
      border: 2px solid var(--border);
      border-radius: 10px;
      font-size: 1em;
      background: var(--bg-dark);
      color: var(--text);
      transition: all 0.3s;
    }
    .form-group input::placeholder {
      color: var(--text-muted);
    }
    .form-group input:focus {
      outline: none;
      border-color: var(--primary);
      box-shadow: 0 0 15px rgba(212,165,116,0.2);
    }
    .btn {
      display: inline-block;
      padding: 14px 28px;
      border: none;
      border-radius: 10px;
      cursor: pointer;
      font-weight: 600;
      font-size: 1em;
      transition: all 0.3s;
      text-decoration: none;
    }
    .btn-primary {
      background: linear-gradient(135deg, var(--primary), var(--primary-dark));
      color: var(--bg-dark);
      box-shadow: 0 4px 15px rgba(212,165,116,0.3);
    }
    .btn-primary:hover {
      background: linear-gradient(135deg, var(--primary-light), var(--primary));
      transform: translateY(-2px);
      box-shadow: 0 6px 20px rgba(212,165,116,0.4);
    }
    .btn-danger {
      background: linear-gradient(135deg, var(--danger), #a04a3a);
      color: white;
    }
    .btn-danger:hover {
      background: linear-gradient(135deg, #d46a5a, var(--danger));
      transform: translateY(-2px);
    }
    .btn-block { width: 100%; margin-top: 12px; }
    .alert {
      padding: 16px;
      border-radius: 10px;
      margin-bottom: 20px;
      border: 1px solid;
    }
    .alert-info {
      background: rgba(107,142,78,0.15);
      border-color: var(--success);
      color: #a8c898;
    }
    .alert-success {
      background: rgba(107,142,78,0.2);
      border-color: var(--success);
      color: #b8d8a8;
    }
    .alert-error {
      background: rgba(196,92,74,0.2);
      border-color: var(--danger);
      color: #e8a8a8;
    }
    .email-status {
      margin-top: 16px;
    }
    .sourdough-status {
      text-align: center;
      padding: 40px 20px;
    }
    .sourdough-icon {
      font-size: 5em;
      margin-bottom: 20px;
      filter: drop-shadow(0 4px 20px rgba(212,165,116,0.3));
    }
    .sourdough-state {
      font-size: 1.6em;
      font-weight: 700;
      color: var(--primary-light);
      margin-bottom: 10px;
      text-shadow: 0 2px 10px rgba(212,165,116,0.3);
    }
    .sourdough-tip {
      margin-top: 20px;
      padding: 20px;
      background: linear-gradient(145deg, #322a20, #2a2218);
      border: 1px solid var(--border);
      border-radius: 12px;
      color: var(--text-muted);
      font-style: italic;
      line-height: 1.6;
    }
    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 12px 0;
      border-bottom: 1px solid var(--border);
    }
    .info-row:last-child { border-bottom: none; }
    .info-row .label { color: var(--text-muted); }
    .info-row .value { color: var(--primary-light); font-weight: 600; }

    /* Mini sensor display in sourdough tab */
    .mini-sensors {
      display: flex;
      justify-content: center;
      gap: 20px;
      margin-top: 25px;
      flex-wrap: wrap;
    }
    .mini-sensor {
      background: linear-gradient(145deg, #322a20, #2a2218);
      border: 1px solid var(--border);
      border-radius: 10px;
      padding: 12px 20px;
      text-align: center;
      min-width: 100px;
    }
    .mini-sensor .value {
      font-size: 1.4em;
      font-weight: 700;
      color: var(--primary-light);
    }
    .mini-sensor .label {
      font-size: 0.75em;
      color: var(--text-muted);
      margin-top: 4px;
    }
  </style>
</head>
<body>
  <nav class="navbar">
    <h1>🍞 Inkubator Zakwasu</h1>
    <div class="status">
      <div class="status-dot" id="statusDot"></div>
      <span id="statusText">Połączony</span>
    </div>
  </nav>

  <div class="container">
    <div class="nav-tabs">
      <button class="active" onclick="showTab('status')">🥖 Zakwas</button>
      <button onclick="showTab('settings')">⚙️ Ustawienia</button>
    </div>

    <!-- Tab: Status Zakwasu (domyślna) -->
    <div id="tab-status" class="tab-content active">
      <div class="card">
        <div class="sourdough-status">
          <div class="sourdough-icon" id="sourdoughIcon">🫙</div>
          <div class="sourdough-state" id="sourdoughState">Monitorowanie...</div>
          <div class="mini-sensors">
            <div class="mini-sensor">
              <div class="value" id="miniTemp">--</div>
              <div class="label">🌡️ Temperatura</div>
            </div>
            <div class="mini-sensor">
              <div class="value" id="miniHum">--</div>
              <div class="label">💧 Wilgotność</div>
            </div>
            <div class="mini-sensor">
              <div class="value" id="miniDist">--</div>
              <div class="label">📏 Poziom</div>
            </div>
          </div>
          <div class="sourdough-tip" id="sourdoughTip">
            🧑‍🍳 Algorytm analizy zakwasu jest w przygotowaniu.<br>
            Obserwuj dane i naucz się rozpoznawać zachowanie swojego zakwasu!
          </div>
        </div>
      </div>
    </div>

    <!-- Tab: Ustawienia -->
    <div id="tab-settings" class="tab-content">
      <div class="card">
        <h2>📶 Konfiguracja WiFi</h2>
        <div class="alert alert-info">
          <strong>💡 Info:</strong> Ustawienia sieci domowej dla przyszłych funkcji.
          Dane są zapisywane w pamięci urządzenia.
        </div>
        <form id="wifiForm" onsubmit="saveWifi(event)">
          <div class="form-group">
            <label for="ssid">Nazwa sieci (SSID)</label>
            <input type="text" id="ssid" name="ssid" placeholder="Twoja sieć WiFi">
          </div>
          <div class="form-group">
            <label for="password">Hasło</label>
            <input type="password" id="wifiPassword" name="password" placeholder="Hasło do sieci">
          </div>
          <button type="submit" class="btn btn-primary btn-block">💾 Zapisz ustawienia</button>
        </form>
        <button onclick="clearSettings()" class="btn btn-danger btn-block">🗑️ Wyczyść dane</button>
      </div>

      <div class="card">
        <h2>📧 Ustawienia e-mail</h2>
        <div class="alert alert-info">
          <strong>Wysyłka testu:</strong> Po zapisaniu ustawień wyślemy testowy e-mail.
          Nadawca i serwer SMTP są stałe. Konfigurujesz tylko adres odbiorcy.
        </div>
        <form id="emailForm" onsubmit="saveEmail(event)">
          <div class="form-group">
            <label for="emailRecipient">E-mail odbiorcy</label>
            <input type="email" id="emailRecipient" name="recipient" placeholder="adres@docelowy.com">
          </div>
          <button type="submit" class="btn btn-primary btn-block">📨 Zapisz i wyślij test</button>
        </form>
        <div class="alert alert-success email-status" id="emailStatus" style="display:none;"></div>
      </div>
    </div>
  </div>

  <script>
    function showTab(tabName) {
      document.querySelectorAll('.tab-content').forEach(tab => tab.classList.remove('active'));
      document.querySelectorAll('.nav-tabs button').forEach(btn => btn.classList.remove('active'));
      document.getElementById('tab-' + tabName).classList.add('active');
      event.target.classList.add('active');
    }

    async function fetchData() {
      try {
        const response = await fetch('/api/data');
        const data = await response.json();

        // Mini sensors on status tab
        document.getElementById('miniTemp').textContent = data.temperature.toFixed(1) + '°C';
        document.getElementById('miniHum').textContent = data.humidity.toFixed(0) + '%';
        document.getElementById('miniDist').textContent = data.distance.toFixed(1) + 'cm';

        const statusDot = document.getElementById('statusDot');
        const statusText = document.getElementById('statusText');
        if (data.sensorConnected) {
          statusDot.classList.remove('offline');
          statusText.textContent = 'Sensor połączony';
        } else {
          statusDot.classList.add('offline');
          statusText.textContent = 'Brak danych';
        }
      } catch (error) {
        document.getElementById('statusDot').classList.add('offline');
        document.getElementById('statusText').textContent = 'Błąd połączenia';
      }
    }

    async function saveWifi(event) {
      event.preventDefault();
      const ssid = document.getElementById('ssid').value;
      const password = document.getElementById('wifiPassword').value;

      try {
        const response = await fetch('/api/wifi', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ ssid, password })
        });
        const result = await response.json();
        alert(result.message);
      } catch (error) {
        alert('Błąd zapisywania ustawień');
      }
    }

    async function clearSettings() {
      if (!confirm('Czy na pewno chcesz wyczyścić zapisane ustawienia WiFi?')) return;

      try {
        const response = await fetch('/api/wifi/clear', { method: 'POST' });
        const result = await response.json();
        alert(result.message);
        document.getElementById('ssid').value = '';
        document.getElementById('wifiPassword').value = '';
      } catch (error) {
        alert('Błąd czyszczenia ustawień');
      }
    }

    async function loadWifiSettings() {
      try {
        const response = await fetch('/api/wifi');
        const data = await response.json();
        document.getElementById('ssid').value = data.ssid || '';
      } catch (error) {}
    }

    async function loadEmailSettings() {
      try {
        const response = await fetch('/api/email');
        const data = await response.json();
        document.getElementById('emailRecipient').value = data.recipient || '';
      } catch (error) {}
    }

    async function saveEmail(event) {
      event.preventDefault();
      const payload = {
        recipient: document.getElementById('emailRecipient').value,
      };

      const statusBox = document.getElementById('emailStatus');
      statusBox.style.display = 'none';

      try {
        const response = await fetch('/api/email', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(payload)
        });
        const result = await response.json();
        statusBox.textContent = result.message || (result.sent ? 'Wysłano testowy e-mail' : 'Nie udało się wysłać e-maila');
        statusBox.style.display = 'block';
        statusBox.className = 'alert email-status ' + (result.sent ? 'alert-success' : 'alert-error');
        if (!result.sent && result.wifiConnected === false) {
          statusBox.textContent += ' (Brak połączenia WiFi)';
        }
      } catch (error) {
        statusBox.textContent = 'Błąd zapisu lub wysyłki';
        statusBox.style.display = 'block';
        statusBox.className = 'alert email-status alert-error';
      }
    }

    fetchData();
    loadWifiSettings();
    loadEmailSettings();
    setInterval(fetchData, 2000);
  </script>
</body>
</html>
)rawliteral";
