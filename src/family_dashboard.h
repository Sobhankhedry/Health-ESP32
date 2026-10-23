#ifndef FAMILY_DASHBOARD_H
#define FAMILY_DASHBOARD_H

const char FAMILY_DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>👨‍👩‍👧‍👦 Family Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background: linear-gradient(135deg, #84fab0 0%, #8fd3f4 100%);
            min-height: 100vh;
            position: relative;
            overflow-x: hidden;
        }
        
        /* Animated background hearts for family theme */
        body::before {
            content: '';
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-image: 
                radial-gradient(circle at 20% 20%, rgba(255,182,193,0.1) 2px, transparent 2px),
                radial-gradient(circle at 80% 40%, rgba(255,182,193,0.1) 2px, transparent 2px),
                radial-gradient(circle at 40% 80%, rgba(255,182,193,0.1) 2px, transparent 2px);
            background-size: 100px 100px, 150px 150px, 200px 200px;
            animation: floatHearts 30s ease-in-out infinite;
            z-index: 0;
        }
        
        @keyframes floatHearts {
            0%, 100% { transform: translateY(0px); opacity: 0.3; }
            50% { transform: translateY(-20px); opacity: 0.6; }
        }
        
        @keyframes slideInUp {
            from { 
                opacity: 0; 
                transform: translateY(50px);
            }
            to { 
                opacity: 1; 
                transform: translateY(0);
            }
        }
        
        @keyframes heartbeat {
            0%, 100% { transform: scale(1); }
            25% { transform: scale(1.1); }
            75% { transform: scale(0.95); }
        }
        
        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.02); }
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            position: relative;
            z-index: 1;
        }
        
        .header {
            background: rgba(255, 255, 255, 0.98);
            backdrop-filter: blur(25px);
            color: #333;
            padding: 20px;
            border-radius: 20px;
            margin-bottom: 20px;
            box-shadow: 
                0 15px 40px rgba(0,0,0,0.15),
                0 0 0 1px rgba(255,255,255,0.2);
            position: relative;
            overflow: hidden;
            animation: slideInUp 0.8s ease-out;
        }
        
        .header::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: linear-gradient(45deg, rgba(132, 250, 176, 0.1), rgba(143, 211, 244, 0.1));
            z-index: -1;
        }
        
        .header h1 {
            margin: 0;
            font-size: 1.4em;
            color: #2196F3;
            font-weight: 700;
            text-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        .user-info {
            float: right;
            font-size: 12px;
            color: #666;
            font-weight: 500;
            background: rgba(255,255,255,0.8);
            padding: 5px 10px;
            border-radius: 12px;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.98);
            backdrop-filter: blur(25px);
            padding: 20px;
            margin: 15px 0;
            border-radius: 20px;
            box-shadow: 
                0 15px 40px rgba(0,0,0,0.1),
                0 0 0 1px rgba(255,255,255,0.2);
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            position: relative;
            overflow: hidden;
            animation: slideInUp 0.8s ease-out;
        }
        
        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: linear-gradient(45deg, rgba(132, 250, 176, 0.05), rgba(143, 211, 244, 0.05));
            z-index: -1;
        }
        
        .card:hover {
            transform: translateY(-3px);
            box-shadow: 
                0 20px 40px rgba(0,0,0,0.15),
                0 0 0 1px rgba(255,255,255,0.3);
        }
        
        .card h2 {
            color: #2196F3;
            margin-bottom: 15px;
            font-size: 1.1em;
            font-weight: 700;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .logout-btn {
            background: linear-gradient(45deg, #ff6b6b, #ee5a52);
            color: white;
            padding: 8px 16px;
            border: none;
            border-radius: 12px;
            cursor: pointer;
            float: right;
            font-weight: 700;
            font-size: 0.85em;
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            text-transform: uppercase;
            letter-spacing: 0.5px;
            box-shadow: 0 6px 20px rgba(255, 107, 107, 0.3);
        }
        
        .logout-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 25px rgba(255, 107, 107, 0.4);
        }
        
        .family-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        
        .status-value {
            font-size: 1em;
            font-weight: 700;
            color: #333;
            margin: 10px 0;
            padding: 8px 15px;
            background: rgba(255,255,255,0.8);
            border-radius: 12px;
            border-left: 4px solid #2196F3;
            transition: all 0.3s ease;
        }
        
        .status-value:hover {
            transform: translateX(3px);
            box-shadow: 0 4px 12px rgba(33, 150, 243, 0.2);
        }
        
        .status-good { 
            color: #4CAF50; 
            border-left-color: #4CAF50;
        }
        .status-concern { 
            color: #FF9800; 
            border-left-color: #FF9800;
            animation: pulse 2s ease-in-out infinite;
        }
        .status-urgent { 
            color: #F44336; 
            border-left-color: #F44336;
            animation: heartbeat 1.5s ease-in-out infinite;
        }
        
        .action-buttons {
            display: flex;
            gap: 15px;
            margin: 20px 0;
            flex-wrap: wrap;
            justify-content: center;
        }
        
        .action-btn {
            background: linear-gradient(45deg, #2196F3, #1976D2);
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 15px;
            cursor: pointer;
            font-weight: 700;
            text-decoration: none;
            display: inline-flex;
            align-items: center;
            gap: 8px;
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            box-shadow: 0 8px 20px rgba(33, 150, 243, 0.3);
            position: relative;
            overflow: hidden;
        }
        
        .action-btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent);
            transition: left 0.5s;
        }
        
        .action-btn:hover::before {
            left: 100%;
        }
        
        .action-btn:hover {
            transform: translateY(-3px) scale(1.03);
            box-shadow: 0 12px 30px rgba(33, 150, 243, 0.4);
        }
        
        .emergency-btn {
            background: linear-gradient(45deg, #F44336, #D32F2F);
            box-shadow: 0 8px 20px rgba(244, 67, 54, 0.3);
            animation: pulse 2s ease-in-out infinite;
        }
        
        .emergency-btn:hover {
            box-shadow: 0 12px 30px rgba(244, 67, 54, 0.5);
        }
        
        .simple-info {
            font-size: 1em;
            line-height: 1.6;
            color: #555;
        }
        
        .simple-info p {
            margin-bottom: 8px;
            padding: 8px 12px;
            background: rgba(255,255,255,0.6);
            border-radius: 8px;
            border-left: 3px solid #2196F3;
        }
        
        .prescriptions-container {
            max-height: 400px;
            overflow-y: auto;
        }
        
        .prescription {
            background: rgba(255,255,255,0.8);
            border-radius: 12px;
            padding: 15px;
            margin-bottom: 15px;
            border-left: 4px solid #4CAF50;
            box-shadow: 0 4px 15px rgba(0,0,0,0.1);
        }
        
        .prescription-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 10px;
        }
        
        .prescription-medication {
            font-size: 1.2em;
            font-weight: bold;
            color: #4CAF50;
        }
        
        .prescription-time {
            font-size: 0.9em;
            color: #666;
        }
        
        .prescription-doctor {
            font-style: italic;
            color: #2196F3;
            margin-bottom: 8px;
        }
        
        .prescription-details {
            line-height: 1.6;
            color: #555;
        }
        
        .no-prescriptions {
            text-align: center;
            color: #999;
            font-style: italic;
            padding: 30px;
            background: rgba(255,255,255,0.5);
            border-radius: 12px;
        }
        
        .notification-badge {
            position: absolute;
            top: -5px;
            right: -5px;
            background: #FF5722;
            color: white;
            border-radius: 50%;
            width: 20px;
            height: 20px;
            font-size: 0.8em;
            display: flex;
            align-items: center;
            justify-content: center;
            font-weight: bold;
            animation: pulse 2s ease-in-out infinite;
        }
        
        .refresh-info {
            text-align: center;
            color: #666;
            font-size: 0.85em;
            margin-top: 25px;
            padding: 15px;
            background: rgba(255,255,255,0.8);
            border-radius: 15px;
            box-shadow: 0 6px 20px rgba(0,0,0,0.1);
        }
        
        .refresh-info p {
            margin-bottom: 5px;
            font-weight: 500;
        }
        
        /* Health status indicators */
        .health-indicator {
            display: inline-block;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            margin-right: 6px;
            animation: pulse 2s ease-in-out infinite;
        }
        
        .health-good { background-color: #4CAF50; }
        .health-warning { background-color: #FF9800; }
        .health-critical { background-color: #F44336; }
        
        /* Responsive design */
        @media (max-width: 768px) {
            .container {
                padding: 15px;
            }
            
            .family-grid {
                grid-template-columns: 1fr;
                gap: 15px;
            }
            
            .action-buttons {
                flex-direction: column;
                align-items: center;
            }
            
            .action-btn {
                width: 100%;
                max-width: 300px;
                justify-content: center;
                padding: 15px 25px;
            }
            
            .header h1 {
                font-size: 1.2em;
            }
            
            .user-info {
                font-size: 11px;
                padding: 4px 8px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="user-info">👥 Family Access | ⏰ %TIME%</div>
            <h1>🏠 Family Care Center</h1>
            <button class="logout-btn" onclick="window.location.href='/logout'">LOGOUT</button>
        </div>

        <div class="card">
            <h2>� Quick Actions</h2>
            <div class="action-buttons">
                <a href="/call-nurse" class="action-btn">
                    👩‍⚕️ Call Nurse
                </a>
                <a href="/emergency" class="action-btn emergency-btn">
                    🚨 Emergency
                </a>
                <a href="/pill-settings" class="action-btn">
                    💊 Pill Settings
                </a>
                <a href="/messages" class="action-btn">
                    💬 Messages %MESSAGE_BADGE%
                </a>
            </div>
        </div>

        <div class="card">
            <h2>�📊 Current Health Status</h2>
            <div class="family-grid">
                <div class="simple-info">
                    <p><span class="health-indicator health-good"></span><strong>Heart Rate:</strong> %HEART_RATE% BPM</p>
                    <p><span class="health-indicator health-good"></span><strong>Patient Temperature:</strong> %PATIENT_TEMP%°C</p>
                    <p><span class="health-indicator health-good"></span><strong>Patient Humidity:</strong> %PATIENT_HUMIDITY%%</p>
                    <p><span class="health-indicator health-good"></span><strong>Patient Status:</strong> %PATIENT_STATUS%</p>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>🏠 Room Environment</h2>
            <div class="family-grid">
                <div class="simple-info">
                    <p><span class="health-indicator health-good"></span><strong>Room Temperature:</strong> %ROOM_TEMP%°C</p>
                    <p><span class="health-indicator health-good"></span><strong>Room Humidity:</strong> %ROOM_HUMIDITY%%</p>
                    <p><span class="health-indicator health-good"></span><strong>Air Quality:</strong> %AIR_QUALITY% (%GAS_LEVEL%%)</p>
                    <p><span class="health-indicator health-good"></span><strong>Comfort Status:</strong> %COMFORT_STATUS%</p>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>💊 Medication Reminder</h2>
            <div class="family-grid">
                <div class="simple-info">
                    <p><span class="health-indicator health-good"></span><strong>Next Pill Time:</strong> %NEXT_PILL_TIME%</p>
                    <p><span class="health-indicator health-good"></span><strong>Pills Status:</strong> %PILL_STATUS%</p>
                </div>
            </div>
        </div>

        <div class="card">
            <h2> Medical Prescriptions</h2>
            <div class="family-grid">
                <div class="prescriptions-container">
                    %PRESCRIPTIONS_HTML%
                </div>
            </div>
        </div>

        <div class="refresh-info">
            <p><strong>💡 Keyboard Shortcuts:</strong></p>
            <p>Ctrl+2: Call Nurse<br>Ctrl+9: Emergency</p>
            <p>🔄 Health data updates automatically every 30 seconds</p>
        </div>
    </div>

    <script>
        // Keyboard shortcuts
        document.addEventListener('keydown', function(e) {
            if (e.ctrlKey) {
                switch(e.key) {
                    case '2':
                        e.preventDefault();
                        window.location.href = '/call-nurse';
                        break;
                    case '9':
                        e.preventDefault();
                        if (confirm('⚠️ EMERGENCY ALERT ⚠️\n\nThis will send an immediate emergency notification.\nAre you sure you want to proceed?')) {
                            window.location.href = '/emergency';
                        }
                        break;
                }
            }
        });



        // Auto-refresh health data every 30 seconds
        setInterval(function() {
            location.reload();
        }, 60000);
    </script>
</body>
</html>
)rawliteral";

#endif
