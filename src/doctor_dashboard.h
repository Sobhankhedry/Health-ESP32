#ifndef DOCTOR_DASHBOARD_H
#define DOCTOR_DASHBOARD_H

const char DOCTOR_DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>       
    <meta charset='UTF-8'>
    <title>🏥 Doctor Dashboard</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .header {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            color: #333;
            padding: 20px;
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }
        
        .header h1 {
            margin: 0;
            font-size: 2em;
            color: #667eea;
        }
        
        .user-info {
            float: right;
            font-size: 14px;
            color: #666;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            padding: 20px;
            margin: 15px 0;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
        }
        
        .card h2 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: 1.5em;
        }
        
        .logout-btn {
            background: linear-gradient(45deg, #ff6b6b, #ee5a52);
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            float: right;
            font-weight: 600;
            transition: transform 0.3s ease;
        }
        
        .logout-btn:hover {
            transform: translateY(-2px);
        }
        
        .sensor-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        
        .sensor-value {
            font-size: 1.2em;
            font-weight: bold;
            color: #333;
            margin: 10px 0;
        }
        
        .status-normal { color: #4CAF50; }
        .status-warning { color: #FF9800; }
        .status-critical { color: #F44336; }
        
        .navigation-buttons {
            display: flex;
            gap: 15px;
            margin: 20px 0;
            flex-wrap: wrap;
        }
        
        .nav-btn {
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            font-weight: 600;
            text-decoration: none;
            display: inline-block;
            transition: transform 0.3s ease;
        }
        
        .nav-btn:hover {
            transform: translateY(-2px);
        }
        
        .refresh-info {
            text-align: center;
            color: #666;
            font-size: 0.9em;
            margin-top: 20px;
        }
    </style>
    <script>
        // Auto refresh every 30 seconds
        setTimeout(() => {
            location.reload();
        }, 30000);
        
        // Function to reset alert counters
        function resetAlertCounters() {
            if (confirm('🔄 RESET ALERT COUNTERS\\n\\nThis will reset all alert counters to zero and start a new monitoring period.\\n\\nAre you sure you want to continue?')) {
                // Create a form and submit it
                var form = document.createElement('form');
                form.method = 'POST';
                form.action = '/reset-counters';
                document.body.appendChild(form);
                form.submit();
            }
        }
    </script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>👨‍⚕️ Doctor Dashboard</h1>
            <div class="user-info">Logged in as: Doctor | Time: %TIME%</div>
            <div style="clear:both;"></div>
            <button class="logout-btn" onclick="location.href='/logout'">🚪 Logout</button>
        </div>

        <div class="navigation-buttons">
            <a href="/pill-settings" class="nav-btn">💊 Pill Settings</a>
            <a href="/prescriptions" class="nav-btn">📋 Write Prescription</a>
            <a href="/messages" class="nav-btn">� Messages %MESSAGE_BADGE%</a>
            <a href="/weekly-report" class="nav-btn">📊 Weekly Report</a>
        </div>

        <div class="sensor-grid">
            <div class="card">
                <h2>🏠 Room Environment</h2>
                <div class="sensor-value">Temperature: %ROOM_TEMP%°C</div>
                <div class="sensor-value">Humidity: %ROOM_HUMIDITY%%</div>
            </div>

            <div class="card">
                <h2>🤒 Patient Vitals</h2>
                <div class="sensor-value">Temperature: %PATIENT_TEMP%°C</div>
                <div class="sensor-value">Humidity: %PATIENT_HUMIDITY%%</div>
                <div class="sensor-value">Heart Rate: %HEART_RATE% BPM</div>
            </div>

            <div class="card">
                <h2>💊 Medication Status</h2>
                <div class="sensor-value">Next Pill Time: %NEXT_PILL_TIME%</div>
                <div class="sensor-value">Pills Today: %PILLS_TAKEN%</div>
                <div class="sensor-value">Status: %PILL_STATUS%</div>
            </div>

            <div class="card">
                <h2>📊 System Status</h2>
                <div class="sensor-value">WiFi: %WIFI_STATUS%</div>
                <div class="sensor-value">Uptime: %UPTIME%</div>
                <div class="sensor-value">Last Update: %LAST_UPDATE%</div>
            </div>
        </div>

        
    </div>
</body>
</html>)rawliteral";

#endif
