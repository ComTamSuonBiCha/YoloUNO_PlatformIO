// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// LED States
var led1State = false;
var led2State = false;

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
        console.log("📤 Gửi:", data);
    } else {
        console.warn("⚠️ WebSocket chưa sẵn sàng!");
        alert("⚠️ WebSocket chưa kết nối!");
    }
}

function onMessage(event) {
    console.log("📩 Nhận:", event.data);
    try {
        var data = JSON.parse(event.data);
        
        // Handle sensor data updates
        if (data.page === "sensor") {
            console.log("🌡️ Sensor data received - Temp:", data.temperature, "Humi:", data.humidity);
            
            // Update temperature gauge
            if (data.temperature !== undefined) {
                const tempValue = document.getElementById('temp-value');
                if (tempValue) {
                    tempValue.textContent = data.temperature.toFixed(1);
                    console.log("✅ Temperature updated:", data.temperature.toFixed(1));
                }
            }
            
            // Update humidity gauge
            if (data.humidity !== undefined) {
                const humiValue = document.getElementById('humi-value');
                if (humiValue) {
                    humiValue.textContent = data.humidity.toFixed(1);
                    console.log("✅ Humidity updated:", data.humidity.toFixed(1));
                }
            }
        }
        
        // Handle TinyML inference results (separate message)
        if (data.page === "tinyml") {
            console.log("🤖 TinyML data received - Anomaly:", data.is_anomaly, "Score:", data.anomaly_score);
            
            // Update anomaly status
            if (data.is_anomaly !== undefined) {
                const anomalyStatus = document.getElementById('anomaly-status');
                const anomalyScoreText = document.getElementById('anomaly-score-text');
                const anomalyGauge = document.getElementById('anomaly-gauge');
                const anomalyCard = document.getElementById('anomaly-card');
                
                if (anomalyStatus && anomalyScoreText && anomalyGauge) {
                    const isAnomaly = data.is_anomaly;
                    anomalyStatus.textContent = isAnomaly ? "⚠️ ANOMALY" : "✓ NORMAL";
                    anomalyScoreText.textContent = "Score: " + (data.anomaly_score !== undefined ? data.anomaly_score.toFixed(4) : "--");
                    
                    // Change color based on status
                    if (isAnomaly) {
                        anomalyGauge.style.background = "linear-gradient(135deg, #ff6b6b 0%, #ee5a6f 100%)";
                        anomalyCard.style.borderLeft = "4px solid #ff6b6b";
                    } else {
                        anomalyGauge.style.background = "linear-gradient(135deg, #11998e 0%, #38ef7d 100%)";
                        anomalyCard.style.borderLeft = "4px solid #11998e";
                    }
                    
                    console.log("✅ TinyML status updated:", isAnomaly ? "ANOMALY" : "NORMAL");
                }
            }
            
            // Update confidence
            if (data.confidence !== undefined) {
                const confidenceValue = document.getElementById('confidence-value');
                if (confidenceValue) {
                    confidenceValue.textContent = data.confidence.toFixed(1);
                    console.log("✅ Confidence updated:", data.confidence.toFixed(1) + "%");
                }
            }
            
            // Update inference time
            if (data.inference_time !== undefined) {
                const inferenceTime = document.getElementById('inference-time');
                if (inferenceTime) {
                    inferenceTime.textContent = data.inference_time;
                    console.log("✅ Inference time updated:", data.inference_time, "μs");
                }
            }
        }
        
        // Handle device status updates
        if (data.page === "device") {
            if (data.device === "LED1") {
                updateLED1UI(data.status === "ON");
            } else if (data.device === "LED2") {
                updateLED2UI(data.status === "ON");
            }
        }
    } catch (e) {
        console.warn("Không phải JSON hợp lệ:", event.data);
    }
}


// ==================== UI NAVIGATION ====================
let relayList = [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = id === 'settings' ? 'flex' : 'block';
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}


// ==================== HOME GAUGES ====================
// Initialize gauges after all libraries are loaded
window.onload = function () {
    console.log("🚀 Page loaded, initializing WebSocket...");
    
    // Initialize WebSocket
    initWebSocket();
};


// ==================== DEVICE CONTROL FUNCTIONS ====================
function toggleLED1() {
    led1State = !led1State;
    updateLED1UI(led1State);
    
    const command = JSON.stringify({
        page: "device",
        device: "LED1",
        gpio: 2,
        status: led1State ? "ON" : "OFF"
    });
    
    Send_Data(command);
}

function toggleLED2() {
    led2State = !led2State;
    updateLED2UI(led2State);
    
    const command = JSON.stringify({
        page: "device",
        device: "LED2",
        gpio: 4,
        status: led2State ? "ON" : "OFF"
    });
    
    Send_Data(command);
}

function updateLED1UI(isOn) {
    led1State = isOn;
    const btn = document.getElementById('led1-btn');
    const statusDot = document.querySelector('#led1-status .status-dot');
    const statusText = document.querySelector('#led1-status .status-text');
    
    if (isOn) {
        btn.classList.add('on');
        btn.textContent = 'ON';
        statusDot.classList.remove('off');
        statusDot.classList.add('on');
        statusText.textContent = 'Bật';
    } else {
        btn.classList.remove('on');
        btn.textContent = 'OFF';
        statusDot.classList.remove('on');
        statusDot.classList.add('off');
        statusText.textContent = 'Tắt';
    }
}

function updateLED2UI(isOn) {
    led2State = isOn;
    const btn = document.getElementById('led2-btn');
    const statusDot = document.querySelector('#led2-status .status-dot');
    const statusText = document.querySelector('#led2-status .status-text');
    
    if (isOn) {
        btn.classList.add('on');
        btn.textContent = 'ON';
        statusDot.classList.remove('off');
        statusDot.classList.add('on');
        statusText.textContent = 'Bật';
    } else {
        btn.classList.remove('on');
        btn.textContent = 'OFF';
        statusDot.classList.remove('on');
        statusDot.classList.add('off');
        statusText.textContent = 'Tắt';
    }
}

function turnAllOn() {
    updateLED1UI(true);
    updateLED2UI(true);
    
    const command1 = JSON.stringify({
        page: "device",
        device: "LED1",
        gpio: 2,
        status: "ON"
    });
    
    const command2 = JSON.stringify({
        page: "device",
        device: "LED2",
        gpio: 4,
        status: "ON"
    });
    
    Send_Data(command1);
    setTimeout(() => Send_Data(command2), 100);
}

function turnAllOff() {
    updateLED1UI(false);
    updateLED2UI(false);
    
    const command1 = JSON.stringify({
        page: "device",
        device: "LED1",
        gpio: 2,
        status: "OFF"
    });
    
    const command2 = JSON.stringify({
        page: "device",
        device: "LED2",
        gpio: 4,
        status: "OFF"
    });
    
    Send_Data(command1);
    setTimeout(() => Send_Data(command2), 100);
    
    // Turn off all custom relays
    relayList.forEach(relay => {
        relay.state = false;
    });
    renderRelays();
}


// ==================== CUSTOM DEVICE FUNCTIONS ====================
function openAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'flex';
}

function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'none';
    document.getElementById('relayName').value = '';
    document.getElementById('relayGPIO').value = '';
}

function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    
    if (!name || !gpio) {
        alert("⚠️ Vui lòng điền đầy đủ thông tin!");
        return;
    }
    
    relayList.push({ id: Date.now(), name, gpio: parseInt(gpio), state: false });
    renderRelays();
    closeAddRelayDialog();
}

function renderRelays() {
    const container = document.getElementById('relayContainer');
    container.innerHTML = "";
    
    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML = `
            <i class="fa-solid fa-bolt device-icon"></i>
            <h3>${r.name}</h3>
            <p>GPIO: ${r.gpio}</p>
            <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">
                ${r.state ? 'ON' : 'OFF'}
            </button>
            <div class="status-indicator">
                <span class="status-dot ${r.state ? 'on' : 'off'}"></span>
                <span class="status-text">${r.state ? 'Bật' : 'Tắt'}</span>
            </div>
            <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.id})"></i>
        `;
        container.appendChild(card);
    });
}

function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;
        
        const command = JSON.stringify({
            page: "device",
            device: relay.name,
            gpio: relay.gpio,
            status: relay.state ? "ON" : "OFF"
        });
        
        Send_Data(command);
        renderRelays();
    }
}

function showDeleteDialog(id) {
    deleteTarget = id;
    document.getElementById('confirmDeleteDialog').style.display = 'flex';
}

function closeConfirmDelete() {
    document.getElementById('confirmDeleteDialog').style.display = 'none';
}

function confirmDelete() {
    relayList = relayList.filter(r => r.id !== deleteTarget);
    renderRelays();
    closeConfirmDelete();
}


// ==================== SETTINGS FORM ====================
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();

    const ssid = document.getElementById("ssid").value.trim();
    const password = document.getElementById("password").value.trim();
    const token = document.getElementById("token").value.trim();
    const server = document.getElementById("server").value.trim();
    const port = document.getElementById("port").value.trim();

    const settingsJSON = JSON.stringify({
        page: "setting",
        value: {
            ssid: ssid,
            password: password,
            token: token,
            server: server,
            port: port
        }
    });

    Send_Data(settingsJSON);
    alert("✅ Cấu hình đã được gửi đến thiết bị!");
});
