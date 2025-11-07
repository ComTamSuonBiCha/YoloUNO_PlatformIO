// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onLoad);

const STORAGE_KEY = 'relays_v1';
let deleteMode = false;

function onLoad(event) {
    initWebSocket();
}

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

        // Có thể thêm xử lý riêng nếu cần (ví dụ cập nhật trạng thái)
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
// window.onload = function () {
//     const gaugeTemp = new JustGage({
//         id: "gauge_temp",
//         value: 26,
//         min: -10,
//         max: 50,
//         donut: true,
//         pointer: false,
//         gaugeWidthScale: 0.25,
//         gaugeColor: "transparent",
//         levelColorsGradient: true,
//         levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
//     });

//     const gaugeHumi = new JustGage({
//         id: "gauge_humi",
//         value: 60,
//         min: 0,
//         max: 100,
//         donut: true,
//         pointer: false,
//         gaugeWidthScale: 0.25,
//         gaugeColor: "transparent",
//         levelColorsGradient: true,
//         levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
//     });

//     setInterval(() => {
//         gaugeTemp.refresh(Math.floor(Math.random() * 15) + 20);
//         gaugeHumi.refresh(Math.floor(Math.random() * 40) + 40);
//     }, 3000);
// };

window.onload = function () {
  window._gaugeTemp = new JustGage({
    id: "gauge_temp",
    value: 26, min: -10, max: 50, donut: true, pointer: false,
    gaugeWidthScale: 0.25, gaugeColor: "transparent",
    levelColorsGradient: true,
    levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
  });

  window._gaugeHumi = new JustGage({
    id: "gauge_humi",
    value: 60, min: 0, max: 100, donut: true, pointer: false,
    gaugeWidthScale: 0.25, gaugeColor: "transparent",
    levelColorsGradient: true,
    levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
  });

  setInterval(() => {
    _gaugeTemp.refresh(Math.floor(Math.random() * 15) + 20);
    _gaugeHumi.refresh(Math.floor(Math.random() * 40) + 40);
  }, 3000);
};



// ==================== DEVICE FUNCTIONS ====================
// function openAddRelayDialog() {
//     document.getElementById('addRelayDialog').style.display = 'flex';
// }

function openAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'flex';
}
// function closeAddRelayDialog() {
//     document.getElementById('addRelayDialog').style.display = 'none';
// }
function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'none';
}

function saveRelay() {
  const name = document.getElementById('relayName').value.trim();
  const gpioStr = document.getElementById('relayGPIO').value.trim();
  const gpio = Number(gpioStr);

  if (!name || gpioStr === '' || Number.isNaN(gpio)) {
    alert("⚠️ Please enter a name and a valid GPIO number!");
    return;
  }

  // block duplicate GPIO
  const exists = relayList.some(r => Number(r.gpio) === gpio);
  if (exists) {
    alert("⚠️ A relay with this GPIO already exists.");
    return;
  }

  relayList.push({ id: Date.now(), name, gpio, state: false });
  saveRelays();
  renderRelays();
  closeAddRelayDialog();

  // clear form
  document.getElementById('relayName').value = '';
  document.getElementById('relayGPIO').value = '';
}

// function renderRelays() {
//     const container = document.getElementById('relayContainer');
//     container.innerHTML = "";
//     relayList.forEach(r => {
//         const card = document.createElement('div');
//         card.className = 'device-card';
//         card.innerHTML = `
//       <i class="fa-solid fa-bolt device-icon"></i>
//       <h3>${r.name}</h3>
//       <p>GPIO: ${r.gpio}</p>
//       <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">
//         ${r.state ? 'ON' : 'OFF'}
//       </button>
//       <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.id})"></i>
//     `;
//         container.appendChild(card);
//     });
// }

function loadRelays() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    relayList = raw ? JSON.parse(raw) : [];
    if (!Array.isArray(relayList)) relayList = [];
  } catch (e) {
    relayList = [];
  }
}

function saveRelays() {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(relayList));
  // (optional) also notify firmware so it can persist to NVS if you want
  try {
    Send_Data(JSON.stringify({ page: "device", value: { relays: relayList } }));
  } catch(e){}
}

// function renderRelays() {
//   const container = document.getElementById('relayContainer');
//   container.innerHTML = "";
//   relayList.forEach(r => {
//     const card = document.createElement('div');
//     card.className = 'device-card' + (r.state ? ' on' : '');
//     card.innerHTML = `
//       <div class="relay-row">
//         <span class="relay-name">${r.name || 'Relay'}</span>
//         <button class="toggle-chip ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">
//           ${r.state ? 'ON' : 'OFF'}
//         </button>
//       </div>
//       <div class="relay-gpio">
//         <span class="chip">GPIO: ${r.gpio ?? 'N/A'}</span>
//       </div>
//     `;
//     container.appendChild(card);
//   });
// }

function renderRelays() {
  const container = document.getElementById('relayContainer');
  container.innerHTML = "";

  relayList.forEach(r => {
    const card = document.createElement('div');
    card.className = 'device-card' + (r.state ? ' on' : '') + (deleteMode ? ' delete-mode' : '');
    card.innerHTML = `
      <div class="relay-row">
        <span class="relay-name">${r.name || 'Relay'}</span>
        <button class="toggle-chip ${r.state ? 'on' : ''}" data-id="${r.id}">
          ${r.state ? 'ON' : 'OFF'}
        </button>
      </div>
      <div class="relay-gpio">
        <span class="chip">GPIO: ${r.gpio ?? 'N/A'}</span>
      </div>
    `;

    if (deleteMode) {
      // tap whole card to delete (with confirm)
      card.addEventListener('click', () => {
        if (confirm(`Delete "${r.name}" (GPIO ${r.gpio})?`)) {
          relayList = relayList.filter(x => x.id !== r.id);
          saveRelays();
          renderRelays();
        }
      });
    } else {
      // normal mode: only button toggles
      const btn = card.querySelector('.toggle-chip');
      btn.addEventListener('click', (ev) => {
        ev.stopPropagation();
        toggleRelay(r.id);
      });
    }

    container.appendChild(card);
  });
}

function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;
        const relayJSON = JSON.stringify({
            page: "device",
            value: {
                name: relay.name,
                status: relay.state ? "ON" : "OFF",
                gpio: relay.gpio
            }
        });
        Send_Data(relayJSON);
        renderRelays();
    }
}

// function toggleRelay(id) {
//   const relay = relayList.find(r => r.id === id);
//   if (!relay) return;

//   relay.state = !relay.state;

//   // notify device
//   Send_Data(JSON.stringify({
//     page: "device",
//     value: { name: relay.name, status: relay.state ? "ON" : "OFF", gpio: relay.gpio }
//   }));

//   saveRelays();
//   renderRelays();
// }

// Wire delete mode button
(function setupDeleteModeButton(){
  const btn = document.getElementById('btnDeleteMode');
  if (!btn) return;
  btn.addEventListener('click', () => {
    deleteMode = !deleteMode;
    btn.textContent = `🗑️ Delete mode: ${deleteMode ? 'ON' : 'OFF'}`;
    renderRelays();
  });
})();

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


// ==================== SETTINGS FORM (BỔ SUNG) ====================
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


// Sidebar hamburger
// (function () {
//   const btn = document.getElementById('hamburger');
//   const sidebar = document.getElementById('sidebar');
//   if (btn && sidebar) {
//     btn.addEventListener('click', () => sidebar.classList.toggle('open'));
//     // close sidebar when navigating
//     document.querySelectorAll('.nav-item').forEach(i =>
//       i.addEventListener('click', () => sidebar.classList.remove('open'))
//     );
//   }
// })();

document.addEventListener('DOMContentLoaded', () => {
  const btn = document.getElementById('hamburger');
  const sidebar = document.getElementById('sidebar');

  if (!btn || !sidebar) return;

  btn.addEventListener('click', (e) => {
    e.preventDefault();
    e.stopPropagation();
    const isOpen = sidebar.classList.toggle('open');
    btn.setAttribute('aria-expanded', isOpen ? 'true' : 'false');
    console.log('sidebar open:', isOpen);
  });

  // Close when navigating (good mobile UX)
  sidebar.querySelectorAll('.nav-item').forEach(i =>
    i.addEventListener('click', () => sidebar.classList.remove('open'))
  );

  // Optional: tap outside to close
  document.addEventListener('click', (e) => {
    if (window.innerWidth > 820) return;         // only on mobile
    if (!sidebar.classList.contains('open')) return;
    const clickedInside = sidebar.contains(e.target) || btn.contains(e.target);
    if (!clickedInside) sidebar.classList.remove('open');
  });
});

// Resize JustGage when viewport changes
(function () {
  let gauges = [];
  function init() {
    if (window._gaugeTemp && window._gaugeHumi) {
      gauges = [window._gaugeTemp, window._gaugeHumi];
    }
  }
  function refreshSize() {
    gauges.forEach(g => {
      const el = document.getElementById(g.config.id);
      if (!el) return;
      // force re-render by setting width/height from CSS box
      const sz = Math.min(el.clientWidth, el.clientHeight);
      if (sz > 0) g.refresh(g.txtLabel ? Number(g.txtLabel.text()) : g.config.value);
    });
  }
  window.addEventListener('load', () => { init(); setTimeout(refreshSize, 50); });
  window.addEventListener('resize', () => setTimeout(refreshSize, 20));
})();

function setGaugeValue(which, value){
  const g = (which === 'temp') ? window._gaugeTemp : window._gaugeHumi;
  const nd = document.getElementById(which === 'temp' ? 'nd_temp' : 'nd_humi');

  if (value === null || value === undefined || isNaN(value)) {
    if (nd) nd.style.display = 'flex';   // show N/A
    return;
  }
  if (nd) nd.style.display = 'none';     // hide N/A
  g.refresh(Number(value));
}


// Load & render once DOM is ready
window.addEventListener('DOMContentLoaded', () => {
  loadRelays();
  renderRelays(); 
});
// Test random values
// setInterval(() => {
//   setGaugeValue('temp', Math.floor(Math.random() * 15) + 20);
//   setGaugeValue('humi', Math.floor(Math.random() * 40) + 40);
// }, 3000);


