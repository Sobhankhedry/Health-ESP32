#ifndef LOGIN_H
#define LOGIN_H

const char LOGIN_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🏥 Health Monitor - Login</title>
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
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
            position: relative;
            overflow: hidden;
        }
        
        /* Animated background elements */
        body::before {
            content: '';
            position: absolute;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            background: radial-gradient(circle, rgba(255,255,255,0.1) 1px, transparent 1px);
            background-size: 50px 50px;
            animation: float 20s ease-in-out infinite;
            z-index: 0;
        }
        
        @keyframes float {
            0%, 100% { transform: translateY(0px) rotate(0deg); }
            50% { transform: translateY(-20px) rotate(180deg); }
        }
        
        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.05); }
        }
        
        @keyframes slideIn {
            from { 
                opacity: 0; 
                transform: translateY(30px);
            }
            to { 
                opacity: 1; 
                transform: translateY(0);
            }
        }
        
        @keyframes glow {
            0%, 100% { box-shadow: 0 0 20px rgba(102, 126, 234, 0.3); }
            50% { box-shadow: 0 0 30px rgba(102, 126, 234, 0.6); }
        }
        
        .login-container {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            border-radius: 25px;
            padding: 50px;
            box-shadow: 
                0 25px 50px rgba(0,0,0,0.15),
                0 0 0 1px rgba(255,255,255,0.2);
            max-width: 450px;
            width: 100%;
            text-align: center;
            position: relative;
            z-index: 1;
            animation: slideIn 0.8s ease-out;
        }
        
        .login-container::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: linear-gradient(45deg, rgba(102, 126, 234, 0.1), rgba(118, 75, 162, 0.1));
            border-radius: 25px;
            z-index: -1;
        }
        
        .logo {
            font-size: 4em;
            margin-bottom: 15px;
            background: linear-gradient(45deg, #667eea, #764ba2);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            animation: pulse 2s ease-in-out infinite;
            text-shadow: 0 0 30px rgba(102, 126, 234, 0.3);
        }
        
        .title {
            font-size: 1.8em;
            color: #333;
            margin-bottom: 35px;
            font-weight: 700;
            letter-spacing: 0.5px;
        }
        
        .subtitle {
            color: #666;
            margin-bottom: 30px;
            font-size: 1.1em;
            font-weight: 400;
        }
        
        .user-type-selector {
            display: flex;
            gap: 20px;
            margin-bottom: 30px;
            justify-content: center;
        }
        
        .user-type-btn {
            flex: 1;
            padding: 20px;
            border: 3px solid transparent;
            border-radius: 20px;
            background: linear-gradient(white, white) padding-box,
                        linear-gradient(45deg, #ddd, #eee) border-box;
            cursor: pointer;
            transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            font-size: 1.2em;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 12px;
            position: relative;
            overflow: hidden;
        }
        
        .user-type-btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255,255,255,0.4), transparent);
            transition: left 0.5s;
        }
        
        .user-type-btn:hover::before {
            left: 100%;
        }
        
        .user-type-btn:hover {
            transform: translateY(-5px) scale(1.05);
            box-shadow: 
                0 15px 35px rgba(102, 126, 234, 0.3),
                0 5px 15px rgba(0,0,0,0.1);
            border: 3px solid transparent;
            background: linear-gradient(white, white) padding-box,
                        linear-gradient(45deg, #667eea, #764ba2) border-box;
        }
        
        .user-type-btn.active {
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            border: 3px solid #667eea;
            transform: translateY(-3px) scale(1.02);
            box-shadow: 
                0 20px 40px rgba(102, 126, 234, 0.4),
                0 10px 20px rgba(0,0,0,0.1);
            animation: glow 2s ease-in-out infinite;
        }
        
        .user-icon {
            font-size: 2.5em;
            filter: drop-shadow(0 2px 4px rgba(0,0,0,0.1));
        }
        
        .user-type-btn.active .user-icon {
            filter: drop-shadow(0 2px 4px rgba(0,0,0,0.2));
        }
        
        .password-group {
            margin-bottom: 30px;
            text-align: left;
        }
        
        .password-group label {
            display: block;
            margin-bottom: 12px;
            color: #555;
            font-weight: 600;
            font-size: 1.1em;
        }
        
        .password-input-wrapper {
            position: relative;
        }
        
        .password-input {
            width: 100%;
            padding: 18px 20px;
            border: 3px solid transparent;
            border-radius: 15px;
            font-size: 1.1em;
            transition: all 0.3s ease;
            background: linear-gradient(white, white) padding-box,
                        linear-gradient(45deg, #ddd, #eee) border-box;
            outline: none;
        }
        
        .password-input:focus {
            border: 3px solid transparent;
            background: linear-gradient(white, white) padding-box,
                        linear-gradient(45deg, #667eea, #764ba2) border-box;
            box-shadow: 0 0 20px rgba(102, 126, 234, 0.3);
            transform: translateY(-2px);
        }
        
        .password-input::placeholder {
            color: #aaa;
            font-style: italic;
        }
        
        .login-btn {
            width: 100%;
            padding: 18px;
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            border: none;
            border-radius: 15px;
            font-size: 1.2em;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            position: relative;
            overflow: hidden;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .login-btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent);
            transition: left 0.5s;
        }
        
        .login-btn:hover::before {
            left: 100%;
        }
        
        .login-btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 15px 35px rgba(102, 126, 234, 0.4);
        }
        
        .login-btn:active {
            transform: translateY(-1px);
        }
        
        .login-btn:disabled {
            opacity: 0.6;
            cursor: not-allowed;
            transform: none;
            background: #ccc;
        }
        
        .login-btn:disabled:hover {
            box-shadow: none;
        }
        
        .error-message {
            background: linear-gradient(45deg, #ff6b6b, #ee5a52);
            color: white;
            padding: 15px;
            border-radius: 12px;
            margin-bottom: 25px;
            display: none;
            font-weight: 600;
            box-shadow: 0 8px 25px rgba(255, 107, 107, 0.3);
            animation: slideIn 0.5s ease-out;
        }
        
        .footer {
            margin-top: 30px;
            color: #666;
            font-size: 0.95em;
            line-height: 1.6;
        }
        
        .footer p {
            margin-bottom: 8px;
        }
        
        .security-badge {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            background: rgba(76, 175, 80, 0.1);
            color: #4CAF50;
            padding: 8px 16px;
            border-radius: 20px;
            font-size: 0.9em;
            font-weight: 600;
            margin-top: 15px;
        }
        
        .loading-overlay {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: rgba(255, 255, 255, 0.9);
            display: none;
            align-items: center;
            justify-content: center;
            border-radius: 25px;
            z-index: 10;
        }
        
        .loading-spinner {
            width: 40px;
            height: 40px;
            border: 4px solid #f3f3f3;
            border-top: 4px solid #667eea;
            border-radius: 50%;
            animation: spin 1s linear infinite;
        }
        
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        /* Responsive design */
        @media (max-width: 480px) {
            .login-container {
                padding: 30px 25px;
                margin: 10px;
            }
            
            .user-type-selector {
                flex-direction: column;
                gap: 15px;
            }
            
            .user-type-btn {
                padding: 15px;
            }
            
            .logo {
                font-size: 3em;
            }
            
            .title {
                font-size: 1.5em;
            }
        }
    </style>
</head>
<body>
    <div class="login-container">
        <div class="loading-overlay" id="loadingOverlay">
            <div class="loading-spinner"></div>
        </div>
        
        <div class="logo">🏥</div>
        <div class="title">Health Monitor System</div>
        <div class="subtitle">🔐 Secure Patient Care Portal</div>
        
        <div id="errorMessage" class="error-message"></div>
        
        <form id="loginForm" method="POST" action="/login">
            <div class="user-type-selector">
                <div class="user-type-btn" onclick="selectUserType('doctor')">
                    <div class="user-icon">👨‍⚕️</div>
                    <div>Doctor</div>
                </div>
                <div class="user-type-btn" onclick="selectUserType('family')">
                    <div class="user-icon">👨‍👩‍👧‍👦</div>
                    <div>Family</div>
                </div>
            </div>
            
            <input type="hidden" id="userType" name="userType" value="">
            
            <div class="password-group">
                <label for="password">🔑 Enter Password:</label>
                <div class="password-input-wrapper">
                    <input type="password" id="password" name="password" class="password-input" 
                           placeholder="Your secure password" required>
                </div>
            </div>
            
            <button type="submit" class="login-btn" id="loginBtn" disabled>
                🔐 Secure Login
            </button>
        </form>
        
        <div class="footer">
            <div class="security-badge">
                �️ SSL Encrypted Connection
            </div>
            <p><strong>🏥 Advanced Health Monitoring</strong></p>
            <p>💝 ESP32 Based Patient Care System</p>
            <p>⚡ Real-time Vital Signs & Alerts</p>
        </div>
    </div>

    <script>
        let selectedUserType = '';
        
        function selectUserType(type) {
            selectedUserType = type;
            document.getElementById('userType').value = type;
            
            // Update visual selection with animation
            document.querySelectorAll('.user-type-btn').forEach(btn => {
                btn.classList.remove('active');
            });
            
            const selectedBtn = event.target.closest('.user-type-btn');
            selectedBtn.classList.add('active');
            
            // Add a subtle vibration effect
            selectedBtn.style.animation = 'none';
            setTimeout(() => {
                selectedBtn.style.animation = 'glow 2s ease-in-out infinite';
            }, 10);
            
            // Enable login button
            updateLoginButton();
            
            // Show user type specific message
            showUserTypeMessage(type);
        }
        
        function showUserTypeMessage(type) {
            const messages = {
                doctor: '👨‍⚕️ Welcome Doctor! Please enter your secure password.',
                family: '👨‍👩‍👧‍👦 Hello Family! Please enter your access password.'
            };
            
            // You could display this message if needed
            console.log(messages[type]);
        }
        
        function updateLoginButton() {
            const password = document.getElementById('password').value;
            const loginBtn = document.getElementById('loginBtn');
            
            if (selectedUserType && password.length > 0) {
                loginBtn.disabled = false;
                loginBtn.style.opacity = '1';
            } else {
                loginBtn.disabled = true;
                loginBtn.style.opacity = '0.6';
            }
        }
        
        function showLoading() {
            document.getElementById('loadingOverlay').style.display = 'flex';
        }
        
        function hideLoading() {
            document.getElementById('loadingOverlay').style.display = 'none';
        }
        
        // Enhanced form submission with loading animation
        document.getElementById('loginForm').addEventListener('submit', function(e) {
            showLoading();
            
            // Add a small delay to show the loading animation
            setTimeout(() => {
                // The form will submit naturally after this
            }, 500);
        });
        
        document.getElementById('password').addEventListener('input', updateLoginButton);
        
        // Add enter key support for password field
        document.getElementById('password').addEventListener('keypress', function(e) {
            if (e.key === 'Enter' && !document.getElementById('loginBtn').disabled) {
                document.getElementById('loginForm').submit();
            }
        });
        
        // Check for error parameter and show enhanced error messages
        const urlParams = new URLSearchParams(window.location.search);
        if (urlParams.get('error') === '1') {
            const errorMsg = document.getElementById('errorMessage');
            errorMsg.innerHTML = '❌ <strong>Access Denied!</strong> Invalid credentials. Please check your password and try again.';
            errorMsg.style.display = 'block';
            
            // Add shake animation to the container
            document.querySelector('.login-container').style.animation = 'shake 0.5s ease-in-out';
        }
        if (urlParams.get('timeout') === '1') {
            const errorMsg = document.getElementById('errorMessage');
            errorMsg.innerHTML = '⏰ <strong>Session Expired!</strong> For security reasons, please login again.';
            errorMsg.style.display = 'block';
        }
        
        // Add shake animation keyframe
        const style = document.createElement('style');
        style.textContent = `
            @keyframes shake {
                0%, 100% { transform: translateX(0); }
                25% { transform: translateX(-10px); }
                75% { transform: translateX(10px); }
            }
        `;
        document.head.appendChild(style);
        
        // Auto refresh every 60 seconds to show current time
        setTimeout(() => {
            if (!selectedUserType) {
                location.reload();
            }
        }, 60000);
        
        // Add some interactive effects
        document.querySelectorAll('.user-type-btn').forEach(btn => {
            btn.addEventListener('mouseenter', function() {
                if (!this.classList.contains('active')) {
                    this.style.transform = 'translateY(-5px) scale(1.05)';
                }
            });
            
            btn.addEventListener('mouseleave', function() {
                if (!this.classList.contains('active')) {
                    this.style.transform = 'translateY(0) scale(1)';
                }
            });
        });
        
        // Initialize page
        hideLoading();
    </script>
</body>
</html>)rawliteral";

#endif
