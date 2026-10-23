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
            max-width: 1400px;
            margin: 0 auto;
            padding: 25px;
            position: relative;
            z-index: 1;
        }
        
        .header {
            background: rgba(255, 255, 255, 0.98);
            backdrop-filter: blur(25px);
            color: #333;
            padding: 30px;
            border-radius: 25px;
            margin-bottom: 30px;
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
            font-size: 2.5em;
            color: #2196F3;
            font-weight: 700;
            text-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        .user-info {
            float: right;
            font-size: 16px;
            color: #666;
            font-weight: 500;
            background: rgba(255,255,255,0.8);
            padding: 8px 16px;
            border-radius: 20px;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.98);
            backdrop-filter: blur(25px);
            padding: 30px;
            margin: 25px 0;
            border-radius: 25px;
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
            transform: translateY(-5px);
            box-shadow: 
                0 25px 50px rgba(0,0,0,0.15),
                0 0 0 1px rgba(255,255,255,0.3);
        }
        
        .card h2 {
            color: #2196F3;
            margin-bottom: 20px;
            font-size: 1.8em;
            font-weight: 700;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .logout-btn {
            background: linear-gradient(45deg, #ff6b6b, #ee5a52);
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 15px;
            cursor: pointer;
            float: right;
            font-weight: 700;
            font-size: 1em;
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            text-transform: uppercase;
            letter-spacing: 0.5px;
            box-shadow: 0 8px 25px rgba(255, 107, 107, 0.3);
        }
        
        .logout-btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 15px 35px rgba(255, 107, 107, 0.4);
        }
        
        .family-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 30px;
            margin: 30px 0;
        }
        
        .status-value {
            font-size: 1.4em;
            font-weight: 700;
            color: #333;
            margin: 15px 0;
            padding: 12px 20px;
            background: rgba(255,255,255,0.8);
            border-radius: 15px;
            border-left: 5px solid #2196F3;
            transition: all 0.3s ease;
        }
        
        .status-value:hover {
            transform: translateX(5px);
            box-shadow: 0 5px 15px rgba(33, 150, 243, 0.2);
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
            gap: 20px;
            margin: 30px 0;
            flex-wrap: wrap;
            justify-content: center;
        }
        
        .action-btn {
            background: linear-gradient(45deg, #2196F3, #1976D2);
            color: white;
            padding: 20px 40px;
            border: none;
            border-radius: 20px;
            cursor: pointer;
            font-weight: 700;
            text-decoration: none;
            display: inline-flex;
            align-items: center;
            gap: 12px;
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            font-size: 1.2em;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            box-shadow: 0 10px 30px rgba(33, 150, 243, 0.3);
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
            transform: translateY(-5px) scale(1.05);
            box-shadow: 0 20px 40px rgba(33, 150, 243, 0.4);
        }
        
        .emergency-btn {
            background: linear-gradient(45deg, #F44336, #D32F2F);
            box-shadow: 0 10px 30px rgba(244, 67, 54, 0.3);
            animation: pulse 2s ease-in-out infinite;
        }
        
        .emergency-btn:hover {
            box-shadow: 0 20px 40px rgba(244, 67, 54, 0.5);
        }
        
        .contact-btn {
            background: linear-gradient(45d, #4CAF50, #388E3C);
            box-shadow: 0 10px 30px rgba(76, 175, 80, 0.3);
        }
        
        .contact-btn:hover {
            box-shadow: 0 20px 40px rgba(76, 175, 80, 0.4);
        }
        
        .simple-info {
            font-size: 1.4em;
            line-height: 1.8;
            color: #555;
        }
        
        .simple-info p {
            margin-bottom: 12px;
            padding: 10px 15px;
            background: rgba(255,255,255,0.6);
            border-radius: 10px;
            border-left: 4px solid #2196F3;
        }
        
        .refresh-info {
            text-align: center;
            color: #666;
            font-size: 1em;
            margin-top: 40px;
            padding: 25px;
            background: rgba(255,255,255,0.8);
            border-radius: 20px;
            box-shadow: 0 8px 25px rgba(0,0,0,0.1);
        }
        
        .refresh-info p {
            margin-bottom: 8px;
            font-weight: 500;
        }
        
        /* Health status indicators */
        .health-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
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
                gap: 20px;
            }
            
            .action-buttons {
                flex-direction: column;
                align-items: center;
            }
            
            .action-btn {
                width: 100%;
                max-width: 300px;
                justify-content: center;
                padding: 18px 30px;
            }
            
            .header h1 {
                font-size: 2em;
            }
            
            .user-info {
                font-size: 14px;
                float: none;
                margin-top: 10px;
                display: block;
                text-align: center;
            }
        }
        
        @media (max-width: 480px) {
            .header {
                padding: 20px;
            }
            
            .card {
                padding: 20px;
            }
            
            .action-btn {
                font-size: 1em;
                padding: 15px 25px;
            }
        }
    </style>
        
        .header {
            background: rgba(255, 255, 255, 0.98);
            backdrop-filter: blur(25px);
            color: #333;
            padding: 30px;
            border-radius: 25px;
            margin-bottom: 30px;
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
            font-size: 2.5em;
            color: #2196F3;
            font-weight: 700;
            text-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        .user-info {
            float: right;
            font-size: 16px;
            color: #666;
            font-weight: 500;
            background: rgba(255,255,255,0.8);
            padding: 8px 16px;
            border-radius: 20px;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.98);
            backdrop-filter: blur(25px);
            padding: 30px;
            margin: 25px 0;
            border-radius: 25px;
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
            transform: translateY(-5px);
            box-shadow: 
                0 25px 50px rgba(0,0,0,0.15),
                0 0 0 1px rgba(255,255,255,0.3);
        }
        
        .card h2 {
            color: #2196F3;
            margin-bottom: 20px;
            font-size: 1.8em;
            font-weight: 700;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .logout-btn {
            background: linear-gradient(45deg, #ff6b6b, #ee5a52);
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 15px;
            cursor: pointer;
            float: right;
            font-weight: 700;
            font-size: 1em;
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            text-transform: uppercase;
            letter-spacing: 0.5px;
            box-shadow: 0 8px 25px rgba(255, 107, 107, 0.3);
        }
        
        .logout-btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 15px 35px rgba(255, 107, 107, 0.4);
        }
        
        .family-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 30px;
            margin: 30px 0;
        }
        
        .status-value {
            font-size: 1.4em;
            font-weight: 700;
            color: #333;
            margin: 15px 0;
            padding: 12px 20px;
            background: rgba(255,255,255,0.8);
            border-radius: 15px;
            border-left: 5px solid #2196F3;
            transition: all 0.3s ease;
        }
        
        .status-value:hover {
            transform: translateX(5px);
            box-shadow: 0 5px 15px rgba(33, 150, 243, 0.2);
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
            gap: 20px;
            margin: 30px 0;
            flex-wrap: wrap;
            justify-content: center;
        }
        
        .action-btn {
            background: linear-gradient(45deg, #2196F3, #1976D2);
            color: white;
            padding: 20px 40px;
            border: none;
            border-radius: 20px;
            cursor: pointer;
            font-weight: 700;
            text-decoration: none;
            display: inline-flex;
            align-items: center;
            gap: 12px;
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            font-size: 1.2em;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            box-shadow: 0 10px 30px rgba(33, 150, 243, 0.3);
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
            transform: translateY(-5px) scale(1.05);
            box-shadow: 0 20px 40px rgba(33, 150, 243, 0.4);
        }
        
        .emergency-btn {
            background: linear-gradient(45deg, #F44336, #D32F2F);
            box-shadow: 0 10px 30px rgba(244, 67, 54, 0.3);
            animation: pulse 2s ease-in-out infinite;
        }
        
        .emergency-btn:hover {
            box-shadow: 0 20px 40px rgba(244, 67, 54, 0.5);
        }
        
        .contact-btn {
            background: linear-gradient(45deg, #4CAF50, #388E3C);
            box-shadow: 0 10px 30px rgba(76, 175, 80, 0.3);
        }
        
        .contact-btn:hover {
            box-shadow: 0 20px 40px rgba(76, 175, 80, 0.4);
        }
        
        .simple-info {
            font-size: 1.4em;
            line-height: 1.8;
            color: #555;
        }
        
        .simple-info p {
            margin-bottom: 12px;
            padding: 10px 15px;
            background: rgba(255,255,255,0.6);
            border-radius: 10px;
            border-left: 4px solid #2196F3;
        }
        
        .refresh-info {
            text-align: center;
            color: #666;
            font-size: 1em;
            margin-top: 40px;
            padding: 25px;
            background: rgba(255,255,255,0.8);
            border-radius: 20px;
            box-shadow: 0 8px 25px rgba(0,0,0,0.1);
        }
        
        .refresh-info p {
            margin-bottom: 8px;
            font-weight: 500;
        }
        
        /* Health status indicators */
        .health-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
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
                gap: 20px;
            }
            
            .action-buttons {
                flex-direction: column;
                align-items: center;
            }
            
            .action-btn {
                width: 100%;
                max-width: 300px;
                justify-content: center;
                padding: 18px 30px;
            }
            
            .header h1 {
                font-size: 2em;
            }
            
            .user-info {
                font-size: 14px;
                float: none;
                margin-top: 10px;
                display: block;
                text-align: center;
            }
        }
        
        @media (max-width: 480px) {
            .header {
                padding: 20px;
            }
            
            .card {
                padding: 20px;
            }
            
            .action-btn {
                font-size: 1em;
                padding: 15px 25px;
            }
        }
    </style>
    <script>
        // Enhanced auto refresh with loading animation
        let refreshTimer;
        let refreshCount = 0;
        
        function startRefreshTimer() {
            refreshTimer = setTimeout(() => {
                refreshCount++;
                
                // Add a subtle loading indication
                const refreshInfo = document.querySelector('.refresh-info');
                if (refreshInfo) {
                    refreshInfo.style.opacity = '0.7';
                    refreshInfo.innerHTML = `
                        <p>🔄 <strong>Refreshing...</strong> Getting latest updates</p>
                        <p>📊 <strong>Update #${refreshCount}:</strong> Keeping data current</p>
                    `;
                }
                
                // Refresh after a short delay to show the loading message
                setTimeout(() => {
                    location.reload();
                }, 1000);
            }, 30000);
        }
        
        // Add animation delays to cards
        function animateCards() {
            const cards = document.querySelectorAll('.card');
            cards.forEach((card, index) => {
                card.style.animationDelay = `${index * 0.2}s`;
            });
        }
        
        // Add interactive effects to action buttons
        function enhanceButtons() {
            const actionBtns = document.querySelectorAll('.action-btn');
            actionBtns.forEach(btn => {
                btn.addEventListener('mouseenter', function() {
                    this.style.transform = 'translateY(-5px) scale(1.05)';
                });
                
                btn.addEventListener('mouseleave', function() {
                    this.style.transform = 'translateY(0) scale(1)';
                });
                
                btn.addEventListener('click', function(e) {
                    // Add click effect
                    this.style.transform = 'translateY(-2px) scale(0.98)';
                    
                    // Create ripple effect
                    const ripple = document.createElement('div');
                    ripple.style.cssText = `
                        position: absolute;
                        border-radius: 50%;
                        background: rgba(255,255,255,0.6);
                        transform: scale(0);
                        animation: ripple 0.6s linear;
                        left: ${e.offsetX - 10}px;
                        top: ${e.offsetY - 10}px;
                        width: 20px;
                        height: 20px;
                        pointer-events: none;
                    `;
                    
                    this.appendChild(ripple);
                    
                    setTimeout(() => {
                        ripple.remove();
                    }, 600);
                });
            });
        }
        
        // Add dynamic health indicators
        function updateHealthIndicators() {
            const indicators = document.querySelectorAll('.health-indicator');
            indicators.forEach(indicator => {
                // This would be updated based on actual health data
                const isHealthy = Math.random() > 0.2; // 80% chance of being healthy
                
                if (isHealthy) {
                    indicator.className = 'health-indicator health-good';
                } else {
                    indicator.className = 'health-indicator health-warning';
                }
            });
        }
        
        // Add ripple animation CSS
        const style = document.createElement('style');
        style.textContent = `
            @keyframes ripple {
                to {
                    transform: scale(4);
                    opacity: 0;
                }
            }
            
            .card:nth-child(1) { animation-delay: 0s; }
            .card:nth-child(2) { animation-delay: 0.2s; }
            .card:nth-child(3) { animation-delay: 0.4s; }
            .card:nth-child(4) { animation-delay: 0.6s; }
        `;
        document.head.appendChild(style);
        
        // Initialize enhancements when page loads
        document.addEventListener('DOMContentLoaded', function() {
            animateCards();
            enhanceButtons();
            updateHealthIndicators();
            startRefreshTimer();
            
            // Add subtle animations to status values
            const statusValues = document.querySelectorAll('.status-value');
            statusValues.forEach((value, index) => {
                value.style.animationDelay = `${index * 0.1}s`;
                value.style.animation = 'slideInUp 0.6s ease-out forwards';
            });
        });
        
        // Clean up timer when page unloads
        window.addEventListener('beforeunload', function() {
            if (refreshTimer) {
                clearTimeout(refreshTimer);
            }
        });
        
        // Add keyboard shortcuts for family members
        document.addEventListener('keydown', function(e) {
            if (e.ctrlKey) {
                switch(e.key) {
                    case '2':
                        e.preventDefault();
                        document.querySelector('a[href="/call-nurse"]').click();
                        break;
                    case '9':
                        e.preventDefault();
                        document.querySelector('a[href="/emergency"]').click();
                        break;
                }
            }
        });
        
        // Show keyboard shortcuts on first visit
        if (!localStorage.getItem('keyboardShortcutsShown')) {
            setTimeout(() => {
                alert('💡 Keyboard Shortcuts:\nCtrl+2: Call Nurse\nCtrl+9: Emergency');
                localStorage.setItem('keyboardShortcutsShown', 'true');
            }, 3000);
        }
    </script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>👨‍👩‍👧‍👦 Family Care Center</h1>
            <div class="user-info">👥 Family Access | 🕒 %TIME%</div>
            <div style="clear:both;"></div>
            <button class="logout-btn" onclick="location.href='/logout'">🚪 Logout</button>
        </div>

        <div class="action-buttons">
            <a href="/call-nurse" class="action-btn contact-btn">
                🏥 <span>Call Nurse</span>
            </a>
            <a href="/emergency" class="action-btn emergency-btn">
                🚨 <span>Emergency</span>
            </a>
        </div>

        <div class="family-grid">
            <div class="card">
                <h2>❤️ Patient Well-being</h2>
                <div class="simple-info">
                    <div class="status-value">
                        <span class="health-indicator health-good"></span>
                        Body Temperature: %PATIENT_TEMP%°C
                    </div>
                    <div class="status-value">
                        <span class="health-indicator health-good"></span>
                        Heart Rate: %HEART_RATE% BPM
                    </div>
                    <div class="status-value">
                        <span class="health-indicator health-good"></span>
                        Overall Status: %PATIENT_STATUS%
                    </div>
                </div>
            </div>

            <div class="card">
                <h2>💊 Medication Care</h2>
                <div class="simple-info">
                    <div class="status-value">⏰ Next Medication: %NEXT_PILL_TIME%</div>
                    <div class="status-value">📊 Pills Today: %PILLS_TAKEN%</div>
                    <div class="status-value">✅ Reminder Status: %PILL_STATUS%</div>
                </div>
            </div>

            <div class="card">
                <h2>🏠 Home Environment</h2>
                <div class="simple-info">
                    <div class="status-value">🌡️ Room Temperature: %ROOM_TEMP%°C</div>
                    <div class="status-value">💧 Humidity Level: %ROOM_HUMIDITY%%</div>
                    <div class="status-value">😌 Comfort Level: %COMFORT_STATUS%</div>
                </div>
            </div>

            <div class="card">
                <h2>�️ Family Support</h2>
                <div class="simple-info">
                    <p>💡 <strong>24/7 Monitoring:</strong> Always keeping watch</p>
                    <p>📱 <strong>Instant Alerts:</strong> Get notified immediately</p>
                    <p>🏥 <strong>Medical Support:</strong> Direct nurse contact</p>
                    <p>👨‍👩‍👧‍� <strong>Family Network:</strong> Stay connected</p>
                </div>
            </div>
        </div>

        <div class="refresh-info">
            <p>🔄 <strong>Live Updates:</strong> Information refreshes every 30 seconds</p>
            <p>❤️ <strong>Always Protected:</strong> Your loved one is in safe hands</p>
            <p>🌟 <strong>Peace of Mind:</strong> Advanced health monitoring technology</p>
        </div>
    </div>
</body>
</html>)rawliteral";

#endif
