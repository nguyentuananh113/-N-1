#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>


// ================= UART =================
HardwareSerial mySerial(1);

#define RX 16
#define TX 17

// ================= WIFI =================
const char ssid[] = "TNH_G";
const char pass[] = "123123123";


// ================= WEB SERVER =================

WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);
// ================= DATA FROM STM32 =================
int nightMode = 0;
int laneState[4];
int vehicleCount[4];
int sensorActive[4];
int currentPair = 0;
int trafficMode = 0;
int dataReceived = 0;
unsigned long lastDataMs = 0;

// ================= BUFFER =================
char buffer[128];
int idx = 0;

// ================= HTML =================
String getDashboardHTML()
{
    String html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>SMART TRAFFIC MONITORING</title>"
    "<style>"
    "*{box-sizing:border-box;margin:0;padding:0}"
    "body{"
    "font-family:Segoe UI,Roboto,Arial,sans-serif;"
    "background:#0f172a;"
    "color:#f8fafc;"
    "min-height:100vh;"
    "}"
    ".header{"
    "background:linear-gradient(135deg,#0f766e,#0891b2);"
    "padding:14px 24px;"
    "display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:8px;"
    "box-shadow:0 4px 20px rgba(0,0,0,0.5);"
    "}"
    ".header h1{font-size:20px;letter-spacing:1px}"
    ".header p{font-size:12px;color:#bae6fd;margin-top:1px}"
    ".header-badges{display:flex;gap:6px;flex-wrap:wrap;align-items:center}"
    ".badge{padding:4px 12px;border-radius:999px;font-size:10px;font-weight:700;letter-spacing:0.5px}"
    ".badge.wifi{background:#1e3a5f;color:#93c5fd;border:1px solid #3b82f6}"
    ".badge.night{background:#1e1b4b;color:#c4b5fd;border:1px solid #7c3aed}"
    ".badge.mode{background:#064e3b;color:#6ee7b7;border:1px solid #10b981}"
    ".badge.mode.p1{background:#064e3b;color:#6ee7b7;border-color:#10b981}"
    ".badge.mode.p2{background:#1e3a5f;color:#93c5fd;border-color:#3b82f6}"
    ".badge.data-ok{background:#064e3b;color:#6ee7b7;border:1px solid #10b981}"
    ".badge.data-wait{background:#7f1d1d;color:#fca5a5;border:1px solid #ef4444}"
    ".main-layout{"
    "display:grid;"
    "grid-template-columns:280px 1fr 280px;"
    "gap:14px;"
    "max-width:1380px;"
    "margin:14px auto;"
    "padding:0 14px;"
    "align-items:start;"
    "}"
    ".card{"
    "background:#1e293b;"
    "border:1px solid #334155;"
    "border-radius:16px;"
    "padding:14px;"
    "}"
    ".card-title{"
    "font-size:9px;font-weight:700;color:#64748b;"
    "text-transform:uppercase;letter-spacing:1.5px;"
    "margin-bottom:10px;"
    "padding-bottom:8px;border-bottom:1px solid #334155;"
    "}"
    ".sidebar-left{display:flex;flex-direction:column;gap:12px}"
    ".light-cards{display:grid;grid-template-columns:1fr 1fr;gap:8px}"
    ".light-card{"
"background:#0f172a;border:1px solid #334155;border-radius:12px;"
    "padding:10px;display:flex;flex-direction:column;align-items:center;gap:6px;"
    "transition:border-color 0.3s;"
    "}"
    ".light-card.red-card{border-color:rgba(239,68,68,0.4)}"
    ".light-card.yellow-card{border-color:rgba(234,179,8,0.4)}"
    ".light-card.green-card{border-color:rgba(34,197,94,0.4)}"
    ".light-housing{"
    "background:#111827;border:2px solid #334155;border-radius:10px;"
    "padding:5px 8px;display:flex;flex-direction:column;gap:5px;"
    "}"
    ".bulb-h{width:20px;height:20px;border-radius:50%;transition:0.2s;opacity:0.2}"
    ".red-h{background:#ef4444}"
    ".yellow-h{background:#eab308}"
    ".green-h{background:#22c55e}"
    ".red-h.on-h{opacity:1;box-shadow:0 0 14px #ef4444,0 0 28px rgba(239,68,68,0.3)}"
    ".yellow-h.on-h{opacity:1;box-shadow:0 0 14px #eab308,0 0 28px rgba(234,179,8,0.3)}"
    ".green-h.on-h{opacity:1;box-shadow:0 0 14px #22c55e,0 0 28px rgba(34,197,94,0.3)}"
    ".light-info{text-align:center}"
    ".light-name{font-size:9px;font-weight:700;color:#64748b;letter-spacing:1px}"
    ".light-state{font-size:12px;font-weight:800;margin:2px 0}"
    ".light-state.red-s{color:#ef4444}"
    ".light-state.yellow-s{color:#eab308}"
    ".light-state.green-s{color:#22c55e}"
    ".light-veh{font-size:11px;color:#94a3b8}"
    ".light-veh span{font-weight:800;color:#f8fafc}"
    ".light-timer{font-size:9px;color:#64748b;margin-top:1px}"
    ".light-timer span{font-weight:700;color:#22d3ee}"
    ".light-sens{"
    "font-size:9px;font-weight:700;padding:2px 8px;border-radius:999px;"
    "letter-spacing:0.5px;"
    "}"
    ".sens-off{background:#1e293b;color:#475569;border:1px solid #334155}"
    ".sens-on{background:#064e3b;color:#22c55e;border:1px solid #10b981}"
    ".lps-red{color:#ef4444}"
    ".lps-yellow{color:#eab308}"
    ".lps-green{color:#22c55e}"
    ".lps-off{color:#475569}"
    ".lps-on{color:#22c55e}"
    ".stats-row{display:grid;grid-template-columns:1fr 1fr;gap:7px}"
    ".stat-box{background:#0f172a;border:1px solid #334155;border-radius:10px;padding:9px;text-align:center}"
    ".stat-box span{display:block;font-size:9px;color:#64748b;font-weight:700;text-transform:uppercase;letter-spacing:0.8px}"
    ".stat-box b{display:block;font-size:18px;font-weight:800;margin-top:2px}"
    ".stat-box b.green{color:#22c55e}"
    ".stat-box b.yellow{color:#eab308}"
    ".stat-box b.cyan{color:#22d3ee}"
    ".timer-display{"
    "text-align:center;padding:10px;background:#0f172a;border-radius:10px;"
    "border:1px solid #334155;margin-top:7px;"
    "}"
    ".timer-display span{font-size:9px;color:#64748b;font-weight:700;text-transform:uppercase}"
    ".timer-display b{display:block;font-size:26px;margin-top:2px;color:#22d3ee}"
    ".raw-data{"
    "background:#020617;border:1px solid #1e3a5f;border-radius:8px;"
    "padding:7px;font-family:monospace;font-size:9px;color:#22d3ee;"
"word-break:break-all;line-height:1.4;max-height:55px;overflow:auto;margin-top:7px;"
    "}"
    ".sidebar-right{display:flex;flex-direction:column;gap:12px}"
    ".lane-prog-card{"
    "border-radius:10px;padding:9px;margin-bottom:7px;transition:0.3s;"
    "}"
    ".lane-prog-card:last-child{margin-bottom:0}"
    ".lane-prog-card.red-p{background:#7f1d1d;border:1px solid #b91c1c}"
    ".lane-prog-card.yellow-p{background:#713f12;border:1px solid #a16207}"
    ".lane-prog-card.green-p{background:#14532d;border:1px solid #15803d}"
    ".lane-prog-top{display:flex;align-items:center;justify-content:space-between;margin-bottom:5px}"
    ".lane-prog-name{font-size:11px;font-weight:700}"
    ".lane-prog-count{font-size:20px;font-weight:800}"
    ".lane-prog-bar-bg{height:5px;background:rgba(255,255,255,0.1);border-radius:999px;overflow:hidden}"
    ".lane-prog-bar{height:100%;border-radius:999px;transition:width 0.5s}"
    ".lane-prog-bar.red-b{background:linear-gradient(90deg,#ef4444,#f87171)}"
    ".lane-prog-bar.yellow-b{background:linear-gradient(90deg,#eab308,#facc15)}"
    ".lane-prog-bar.green-b{background:linear-gradient(90deg,#22c55e,#4ade80)}"
    ".map-card{height:100%}"
    ".map-wrap{border-radius:12px;overflow:hidden;border:1px solid #334155}"
    "#map{width:100%;height:580px;border-radius:12px}"
    ".footer{text-align:center;padding:14px;font-size:11px;color:#475569}"
    "@media(max-width:1100px){.main-layout{grid-template-columns:1fr}}"
    ".leaflet-container{background:#1e293b}"
    ".lane-popup{font-family:Segoe UI,Arial,sans-serif}"
    ".lane-popup .lp-title{font-size:13px;font-weight:700;margin-bottom:8px;color:#22d3ee}"
    ".lane-popup .lp-row{display:flex;justify-content:space-between;gap:12px;padding:3px 0;border-bottom:1px solid #334155;font-size:11px}"
    ".lane-popup .lp-row:last-child{border-bottom:none}"
    ".lane-popup .lp-row span{color:#64748b}"
    ".lane-popup .lp-row b{color:#f8fafc;font-weight:700}"
    ".leaflet-popup-content-wrapper{background:#1e293b;border:1px solid #334155;border-radius:10px;padding:0}"
    ".leaflet-popup-content{margin:12px;color:#f8fafc}"
    ".leaflet-popup-tip{background:#1e293b}"
    ".leaflet-tooltip{background:#1e293b;border:1px solid #334155;color:#f8fafc;font-size:11px;font-family:Segoe UI,Arial,sans-serif;padding:6px 10px;border-radius:8px;box-shadow:0 4px 12px rgba(0,0,0,0.5)}"
    ".leaflet-tooltip-top:before{border-top-color:#334155}"
    ".leaflet-control-zoom a{background:#1e293b;color:#f8fafc;border-color:#334155}"
    ".leaflet-control-zoom a:hover{background:#334155}"
    ".leaflet-control-attribution{background:rgba(15,23,42,0.8);color:#475569;font-size:9px}"
    "</style>"
    "<link rel='stylesheet' href='https://unpkg.com/leaflet@1.9.4/dist/leaflet.css'/>"
    "<script src='https://unpkg.com/leaflet@1.9.4/dist/leaflet.js'></script>"
    "</head>"
    "<body>"
    "<div class='header'>"
    "<div>"
    "<h1>SMART TRAFFIC MONITORING</h1>"
"<p>He thong giam sat giao thong thong minh</p>"
    "</div>"
    "<div class='header-badges'>"
    "<div id='wsStatus' class='badge wifi'>DANG KET NOI...</div>"
    "<div id='nightBadge' class='badge night'>DAY MODE</div>"
    "<div id='modeBadge' class='badge mode'>BINH THUONG</div>"
    "<div id='pairBadge0' class='badge mode p1'>L1 + L3</div>"
    "<div id='pairBadge1' class='badge mode p2'>L2 + L4</div>"
    "<div id='dataStatus' class='badge data-wait'>DOI DU LIEU...</div>"
    "</div>"
    "</div>"
    "<div class='main-layout'>"
    "<div class='sidebar-left'>"
    "<div class='card'>"
    "<div class='card-title'>Trang thai den giao thong</div>"
    "<div class='light-cards'>"
    "<div id='lc0' class='light-card'>"
    "<div class='light-housing'>"
    "<div id='lb-r0' class='bulb-h red-h on-h'></div>"
    "<div id='lb-y0' class='bulb-h yellow-h'></div>"
    "<div id='lb-g0' class='bulb-h green-h'></div>"
    "</div>"
    "<div class='light-info'>"
    "<div class='light-name'>LANE 1</div>"
    "<div id='lc-state0' class='light-state red-s'>RED</div>"
    "<div id='lc-veh0' class='light-veh'><span id='lc-cnt0'>0</span> xe</div>"
    "<div class='light-timer'>TG: <span id='lc-timer0'>--</span>s</div>"
    "</div>"
    "<div id='lc-sens0' class='light-sens sens-off'>IR</div>"
    "</div>"
    "<div id='lc1' class='light-card'>"
    "<div class='light-housing'>"
    "<div id='lb-r1' class='bulb-h red-h on-h'></div>"
    "<div id='lb-y1' class='bulb-h yellow-h'></div>"
    "<div id='lb-g1' class='bulb-h green-h'></div>"
    "</div>"
    "<div class='light-info'>"
    "<div class='light-name'>LANE 2</div>"
    "<div id='lc-state1' class='light-state red-s'>RED</div>"
    "<div id='lc-veh1' class='light-veh'><span id='lc-cnt1'>0</span> xe</div>"
    "<div class='light-timer'>TG: <span id='lc-timer1'>--</span>s</div>"
    "</div>"
    "<div id='lc-sens1' class='light-sens sens-off'>IR</div>"
    "</div>"
    "<div id='lc2' class='light-card'>"
    "<div class='light-housing'>"
    "<div id='lb-r2' class='bulb-h red-h on-h'></div>"
    "<div id='lb-y2' class='bulb-h yellow-h'></div>"
    "<div id='lb-g2' class='bulb-h green-h'></div>"
    "</div>"
    "<div class='light-info'>"
    "<div class='light-name'>LANE 3</div>"
    "<div id='lc-state2' class='light-state red-s'>RED</div>"
    "<div id='lc-veh2' class='light-veh'><span id='lc-cnt2'>0</span> xe</div>"
    "<div class='light-timer'>TG: <span id='lc-timer2'>--</span>s</div>"
    "</div>"
    "<div id='lc-sens2' class='light-sens sens-off'>IR</div>"
    "</div>"
    "<div id='lc3' class='light-card'>"
    "<div class='light-housing'>"
    "<div id='lb-r3' class='bulb-h red-h on-h'></div>"
    "<div id='lb-y3' class='bulb-h yellow-h'></div>"
    "<div id='lb-g3' class='bulb-h green-h'></div>"
    "</div>"
    "<div class='light-info'>"
    "<div class='light-name'>LANE 4</div>"
    "<div id='lc-state3' class='light-state red-s'>RED</div>"
"<div id='lc-veh3' class='light-veh'><span id='lc-cnt3'>0</span> xe</div>"
    "<div class='light-timer'>TG: <span id='lc-timer3'>--</span>s</div>"
    "</div>"
    "<div id='lc-sens3' class='light-sens sens-off'>IR</div>"
    "</div>"
    "</div>"
    "</div>"
    "<div class='card'>"
    "<div class='card-title'>Thong ke</div>"
    "<div class='stats-row'>"
    "<div class='stat-box'><span>Tong xe</span><b id='totalVeh' class='green'>0</b></div>"
    "<div class='stat-box'><span>Lane ban</span><b id='busyLane' class='cyan'>---</b></div>"
    "<div class='stat-box'><span>Trang thai</span><b id='sysStatus' class='yellow'>DOI DATA</b></div>"
    "<div class='stat-box'><span>Do tre</span><b id='delayMs' class='cyan'>---</b></div>"
    "</div>"
    "<div class='timer-display'>"
    "<span>Thoi gian cho</span>"
    "<b id='waitTimer'>---</b>"
    "</div>"
    "<div id='rawData' class='raw-data'>Dang cho du lieu UART...</div>"
    "</div>"
    "</div>"
    "<div class='map-card card'>"
    "<div class='card-title'>Ban do vi tri</div>"
    "<div class='map-wrap'>"
    "<div id='map'></div>"
    "</div>"
    "</div>"
    "<div class='sidebar-right'>"
    "<div class='card' style='height:100%'>"
    "<div class='card-title'>Xe trong lane</div>"
    "<div id='laneProgs'>"
    "<div id='lp0' class='lane-prog-card red-p'>"
    "<div class='lane-prog-top'>"
    "<span class='lane-prog-name' style='color:#fca5a5'>Lane 1</span>"
    "<span id='lpc0' class='lane-prog-count' style='color:#fca5a5'>0</span>"
    "</div>"
    "<div class='lane-prog-bar-bg'>"
    "<div id='lpb0' class='lane-prog-bar red-b' style='width:0%'></div>"
    "</div>"
    "</div>"
    "<div id='lp1' class='lane-prog-card red-p'>"
    "<div class='lane-prog-top'>"
    "<span class='lane-prog-name' style='color:#fca5a5'>Lane 2</span>"
    "<span id='lpc1' class='lane-prog-count' style='color:#fca5a5'>0</span>"
    "</div>"
    "<div class='lane-prog-bar-bg'>"
    "<div id='lpb1' class='lane-prog-bar red-b' style='width:0%'></div>"
    "</div>"
    "</div>"
    "<div id='lp2' class='lane-prog-card red-p'>"
    "<div class='lane-prog-top'>"
    "<span class='lane-prog-name' style='color:#fca5a5'>Lane 3</span>"
    "<span id='lpc2' class='lane-prog-count' style='color:#fca5a5'>0</span>"
    "</div>"
    "<div class='lane-prog-bar-bg'>"
    "<div id='lpb2' class='lane-prog-bar red-b' style='width:0%'></div>"
    "</div>"
    "</div>"
    "<div id='lp3' class='lane-prog-card red-p'>"
    "<div class='lane-prog-top'>"
    "<span class='lane-prog-name' style='color:#fca5a5'>Lane 4</span>"
    "<span id='lpc3' class='lane-prog-count' style='color:#fca5a5'>0</span>"
    "</div>"
    "<div class='lane-prog-bar-bg'>"
    "<div id='lpb3' class='lane-prog-bar red-b' style='width:0%'></div>"
    "</div>"
    "</div>"
    "</div>"
    "</div>"
    "</div>"
    "</div>"
    "<div class='footer'>ESP32 + STM32 Realtime Traffic System</div>"
    "<script>"
    "let lastUpdate=0;"
    "let map,markers=[];"
"let lanePositions=["
    "  {lat:10.85890,lng:106.77780,name:'Lane 1'},"
    "  {lat:10.85890,lng:106.77784,name:'Lane 2'},"
    "  {lat:10.85889,lng:106.77779,name:'Lane 3'},"
    "  {lat:10.85894,lng:106.77789,name:'Lane 4'}"
    "];"
    "function initMap(){"
    "  map=L.map('map',{preferCanvas:true}).setView([10.85890,106.77782],17);"
    "  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',{"
    "    attribution:'&copy; OpenStreetMap contributors',"
    "    maxZoom:19"
    "  }).addTo(map);"
    "  let laneColors=['#ef4444','#22c55e','#22c55e','#ef4444'];"
    "  let stateNames=['RED','YELLOW','GREEN'];"
    "  for(let i=0;i<4;i++){"
    "    let m=L.circleMarker([lanePositions[i].lat,lanePositions[i].lng],{"
    "      radius:12,"
    "      fillColor:laneColors[i],"
    "      color:'white',"
    "      weight:3,"
    "      fillOpacity:1"
    "    }).addTo(map);"
    "    m.bindTooltip(lanePositions[i].name+'<br>0 xe | RED',{"
    "      permanent:false,"
    "      direction:'top',"
    "      className:'lane-tooltip'"
    "    });"
    "    let popupContent='<div class=\"lane-popup\">'"
    "      +'<div class=\"lp-title\">'+lanePositions[i].name+'</div>'"
    "      +'<div class=\"lp-row\"><span>Trang thai</span><b id=\"lp-state'+i+'\">RED</b></div>'"
    "      +'<div class=\"lp-row\"><span>Luong xe</span><b id=\"lp-count'+i+'\">0 xe</b></div>'"
    "      +'<div class=\"lp-row\"><span>TG den do</span><b id=\"lp-redt'+i+'\">--s</b></div>'"
    "      +'<div class=\"lp-row\"><span>TG den xanh</span><b id=\"lp-grnt'+i+'\">--s</b></div>'"
    "      +'<div class=\"lp-row\"><span>Sensor IR</span><b id=\"lp-sens'+i+'\" class=\"lps-off\">OFF</b></div>'"
    "      +'</div>';"
    "    m.bindPopup(popupContent,{maxWidth:200,className:'lane-popup-container'});"
    "    m.addEventListener('click',function(){"
    "      window.open('/lane?i=' + i, '_blank');"
    "    });"
    "    markers.push(m);"
    "  }"
    "  map.on('click',function(e){"
    "  });"
    "}"
    "function updatePopupContent(idx){"
    "  let s=window['state'+(idx+1)];"
    "  let c=window['count'+(idx+1)]||0;"
    "  let sa=window['sensor'+(idx+1)]||0;"
    "  let stateNames=['RED','YELLOW','GREEN'];"
    "  let stEl=document.getElementById('lp-state'+idx);"
    "  let ctEl=document.getElementById('lp-count'+idx);"
    "  let seEl=document.getElementById('lp-sens'+idx);"
    "  let rtEl=document.getElementById('lp-redt'+idx);"
    "  let gtEl=document.getElementById('lp-grnt'+idx);"
    "  if(stEl){"
    "    stEl.innerText=stateNames[s]||'RED';"
    "    stEl.className='';"
    "    if(s==0)stEl.className='lps-red';"
    "    if(s==1)stEl.className='lps-yellow';"
    "    if(s==2)stEl.className='lps-green';"
    "  }"
    "  if(ctEl)ctEl.innerText=c+' xe';"
    "  if(seEl){seEl.innerText=sa?'ON':'OFF';seEl.className='lps-off'+(sa?' lps-on':'');}"
    "  if(rtEl){"
    "    var prevState=window['prevState'+(idx+1)];"
"    var stateStart=window['stateStart'+(idx+1)]||Date.now();"
    "    if(prevState!==s){window['stateStart'+(idx+1)]=Date.now();window['prevState'+(idx+1)]=s;}"
    "    var elapsed=Math.round((Date.now()-stateStart)/1000);"
    "    if(s==0)rtEl.innerText=elapsed+'s';"
    "    else rtEl.innerText='--';"
    "    if(s==2)gtEl.innerText=elapsed+'s';"
    "    else gtEl.innerText='--';"
    "  }"
    "  let tooltipContent=lanePositions[idx].name+'<br>'+c+' xe | '+(stateNames[s]||'RED');"
    "  markers[idx].setTooltipContent(tooltipContent);"
    "}"
    "function updateMarkers(){"
    "  let stateColors=['#ef4444','#eab308','#22c55e'];"
    "  let stateNames=['RED','YELLOW','GREEN'];"
    "  for(let i=0;i<4;i++){"
    "    let s=window['state'+(i+1)];"
    "    let c=window['count'+(i+1)]||0;"
    "    let color=stateColors[s]||'#ef4444';"
    "    markers[i].setStyle({fillColor:color,color:'white',radius:10});"
    "    markers[i].setTooltipContent(lanePositions[i].name+'<br>'+c+' xe | '+(stateNames[s]||'RED'));"
    "  }"
    "}"
    "function getModeName(m){"
    "  if(m==1)return'UU TIEN 1&3';"
    "  if(m==2)return'UU TIEN 2&4';"
    "  return'BINH THUONG';"
    "}"
    "function updateSVG(d){"
    "  let svIds=['svL1','svL2','svL3','svL4'];"
    "  let svColors=['#ef4444','#22c55e','#22c55e','#ef4444'];"
    "  let stateColors=['#ef4444','#eab308','#22c55e'];"
    "  for(let i=0;i<4;i++){"
    "    let s=d['state'+(i+1)];"
    "    let el=document.getElementById(svIds[i]);"
    "    if(!el)continue;"
    "    let c=stateColors[s]||'#ef4444';"
    "    let fill='rgba('+parseInt(c.slice(1,3),16)+','+parseInt(c.slice(3,5),16)+','+parseInt(c.slice(5,7),16)+',0.3)';"
    "    el.setAttribute('fill',fill);"
    "    el.setAttribute('stroke',c);"
    "  }"
    "}"
    "function updateUI(d){"
    "  if(d.night==1){"
    "    document.body.style.background='#000';"
    "    document.getElementById('nightBadge').innerText='NIGHT MODE';"
    "  }else{"
    "    document.body.style.background='#0f172a';"
    "    document.getElementById('nightBadge').innerText='DAY MODE';"
    "  }"
    "  var mName=getModeName(d.trafficMode||0);"
    "  var mEl=document.getElementById('modeBadge');"
    "  mEl.innerText=mName;"
    "  if(d.trafficMode==1)mEl.style.cssText='background:#064e3b;color:#6ee7b7;border:1px solid #10b981';"
    "  else if(d.trafficMode==2)mEl.style.cssText='background:#78350f;color:#fed7aa;border:1px solid #f97316';"
    "  else mEl.style.cssText='background:#064e3b;color:#6ee7b7;border:1px solid #10b981';"
    "  var cp=d.currentPair||0;"
    "  var p0=document.getElementById('pairBadge0');"
    "  var p1=document.getElementById('pairBadge1');"
    "  if(cp==0){"
    "    p0.style.cssText='background:#064e3b;color:#6ee7b7;border:1px solid #10b981';p0.className='badge mode p1';"
    "    p1.style.cssText='background:#1e3a5f;color:#93c5fd;border:1px solid #3b82f6';p1.className='badge mode p2';"
    "  }else{"
"    p0.style.cssText='background:#1e3a5f;color:#93c5fd;border:1px solid #3b82f6';p0.className='badge mode p2';"
    "    p1.style.cssText='background:#064e3b;color:#6ee7b7;border:1px solid #10b981';p1.className='badge mode p1';"
    "  }"
    "  var counts=[];var busyIdx=0;"
    "  for(var i=0;i<4;i++){"
    "    var s=d['state'+(i+1)];"
    "    var c=d['count'+(i+1)]||0;"
    "    var sa=d['sensor'+(i+1)]||0;"
    "    window['state'+(i+1)]=s;"
    "    window['count'+(i+1)]=c;"
    "    window['sensor'+(i+1)]=sa;"
    "    counts.push(c);"
    "    if(c>(counts[busyIdx]||0))busyIdx=i;"
    "    var lc=document.getElementById('lc'+i);"
    "    if(s==0){lc.className='light-card red-card';}"
    "    if(s==1){lc.className='light-card yellow-card';}"
    "    if(s==2){lc.className='light-card green-card';}"
    "    for(var b=0;b<3;b++){"
    "      var el=document.getElementById('lb-'+(['r','y','g'][b])+i);"
    "      el.className='bulb-h '+(['red-h','yellow-h','green-h'][b])+(s==b?' on-h':'');"
    "    }"
    "    var stEl=document.getElementById('lc-state'+i);"
    "    if(s==0){stEl.innerText='RED';stEl.className='light-state red-s';}"
    "    if(s==1){stEl.innerText='YELLOW';stEl.className='light-state yellow-s';}"
    "    if(s==2){stEl.innerText='GREEN';stEl.className='light-state green-s';}"
    "    document.getElementById('lc-cnt'+i).innerText=c;"
    "    var seEl=document.getElementById('lc-sens'+i);"
    "    seEl.className='light-sens '+(sa?'sens-on':'sens-off');"
    "    seEl.innerText=sa?'IR ON':'IR OFF';"
    "    var prevS=window['prevState'+(i+1)];"
    "    if(prevS!==s){window['stateStart'+(i+1)]=Date.now();window['prevState'+(i+1)]=s;}"
    "    var stStart=window['stateStart'+(i+1)]||Date.now();"
    "    var elapsed=Math.round((Date.now()-stStart)/1000);"
    "    var tEl=document.getElementById('lc-timer'+i);"
    "    tEl.innerText=elapsed;"
    "    var lp=document.getElementById('lp'+i);"
    "    var lpc=document.getElementById('lpc'+i);"
    "    var lpb=document.getElementById('lpb'+i);"
    "    if(s==0){lp.className='lane-prog-card red-p';lpc.style.color='#fca5a5';lpc.innerText=c;lpb.className='lane-prog-bar red-b';}"
    "    if(s==1){lp.className='lane-prog-card yellow-p';lpc.style.color='#fde047';lpc.innerText=c;lpb.className='lane-prog-bar yellow-b';}"
    "    if(s==2){lp.className='lane-prog-card green-p';lpc.style.color='#86efac';lpc.innerText=c;lpb.className='lane-prog-bar green-b';}"
    "    var pct=Math.min(c/20*100,100);"
    "    lpb.style.width=pct+'%';"
    "  }"
    "  var total=counts.reduce(function(a,b){return a+b},0);"
    "  document.getElementById('totalVeh').innerText=total;"
    "  document.getElementById('busyLane').innerText='Lane '+(busyIdx+1);"
    "  var now=Date.now();"
    "  document.getElementById('delayMs').innerText=(lastUpdate?Math.round(now-lastUpdate):0)+'ms';"
    "  lastUpdate=now;"
    "  if(d.dataReceived){"
"    document.getElementById('sysStatus').innerText='HOAT DONG';"
    "    document.getElementById('sysStatus').style.color='#22c55e';"
    "    document.getElementById('dataStatus').innerText='OK';"
    "    document.getElementById('dataStatus').className='badge data-ok';"
    "  }else{"
    "    document.getElementById('sysStatus').innerText='DOI DATA';"
    "    document.getElementById('sysStatus').style.color='#eab308';"
    "    document.getElementById('dataStatus').innerText='DOI DU LIEU...';"
    "    document.getElementById('dataStatus').className='badge data-wait';"
    "  }"
    "  if(d._raw)document.getElementById('rawData').innerText=d._raw;"
    "  if(typeof updateMarkers==='function')updateMarkers();"
    "  if(typeof updateSVG==='function')updateSVG(d);"
    "  for(let i=0;i<4;i++)updatePopupContent(i);"
    "}"
    "window.addEventListener('load',function(){"
    "  if(typeof L!=='undefined')initMap();"
    "});"
    "let ws = new WebSocket('ws://' + location.hostname + ':81/');"
    "ws.onopen = function(){"
    " document.getElementById('wsStatus').innerText='WS CONNECTED';"
    "};"
    "ws.onmessage = function(event){"
    " var data = JSON.parse(event.data);"
    " updateUI(data);"
    "};"
    "ws.onclose = function(){"
    " document.getElementById('wsStatus').innerText='WS DISCONNECTED';"
    "};"
    "ws.onerror = function(){"
    " document.getElementById('wsStatus').innerText='WS ERROR';"
    "};"  
    "setInterval(function(){"
    "  for(var i=0;i<4;i++){"
    "    var s=window['state'+(i+1)];"
    "    var rtEl=document.getElementById('lp-redt'+i);"
    "    var gtEl=document.getElementById('lp-grnt'+i);"
    "    if(!rtEl||!gtEl)continue;"
    "    var stateStart=window['stateStart'+(i+1)]||Date.now();"
    "    var prevS=window['prevState'+(i+1)];"
    "    if(prevS!==s){window['stateStart'+(i+1)]=Date.now();window['prevState'+(i+1)]=s;}"
    "    var elapsed=Math.round((Date.now()-stateStart)/1000);"
    "    if(s==0)rtEl.innerText=elapsed+'s';"
    "    if(s==2)gtEl.innerText=elapsed+'s';"
    "  }"
    "},1000);"
    "fetch('/data').then(function(r){return r.json()}).then(function(d){updateUI(d)});"
    "document.getElementById('wsStatus').innerText='KET NOI THANH CONG';"
    "document.getElementById('wsStatus').style.cssText='background:#064e3b;color:#6ee7b7;border:1px solid #10b981';"
    "</script>"
    "</body>"
    "</html>";
    return html;
}

// ================= JSON API =================
void handleData()
{
    unsigned long now = millis();
    unsigned long sinceLast = dataReceived ? (now - lastDataMs) : 999999;

    String json = "{";
    json += "\"night\":" + String(nightMode) + ",";
    json += "\"currentPair\":" + String(currentPair) + ",";
    json += "\"trafficMode\":" + String(trafficMode) + ",";
    json += "\"dataReceived\":" + String(dataReceived) + ",";
    json += "\"sinceLast\":" + String(sinceLast) + ",";

    for (int i = 0; i < 4; i++)
    {
        json += "\"state" + String(i + 1) + "\":" + String(laneState[i]);
        json += ",\"count" + String(i + 1) + "\":" + String(vehicleCount[i]);
        json += ",\"sensor" + String(i + 1) + "\":" + String(sensorActive[i]);
        if (i < 3) json += ",";
    }

    json += "}";
    webSocket.broadcastTXT(json);
server.send(200, "application/json", json);
}

// ================= PARSE UART =================
void parseData(char* data)
{
    Serial.print("[UART RX] ");
    Serial.println(data);

    int n, s1, c1, s2, c2, s3, c3, s4, c4;
    int ok = sscanf(data,
        "N:%d;S1:%d;C1:%d;S2:%d;C2:%d;S3:%d;C3:%d;S4:%d;C4:%d",
        &n, &s1, &c1, &s2, &c2, &s3, &c3, &s4, &c4);

    if (ok == 9)
    {
        nightMode = n;

        laneState[0] = s1;
        vehicleCount[0] = c1;

        laneState[1] = s2;
        vehicleCount[1] = c2;

        laneState[2] = s3;
        vehicleCount[2] = c3;

        laneState[3] = s4;
        vehicleCount[3] = c4;

        for (int i = 0; i < 4; i++)
        {
            sensorActive[i] = (laneState[i] == 2) ? 1 : 0;
        }

        lastDataMs = millis();
        dataReceived = 1;

        Serial.printf("[PARSE OK] night=%d S=[%d,%d,%d,%d] C=[%d,%d,%d,%d]\n",
            nightMode, s1, s2, s3, s4, c1, c2, c3, c4);
    }
    else
    {
        Serial.printf("[PARSE FAIL] expected 9, got %d | data: '%s'\n", ok, data);
    }
}

// ================= UART READ =================
void readSTM32()
{
    while (mySerial.available())
    {
        char c = mySerial.read();

        if (c == '\n')
        {
            buffer[idx] = '\0';
            if (idx > 0) parseData(buffer);
            idx = 0;
        }
        else if (c != '\r')
        {
            if (idx < sizeof(buffer) - 1)
                buffer[idx++] = c;
            else
                idx = 0;
        }
    }
}

// ================= SETUP =================
void setup()
{
    Serial.begin(115200);
    Serial.println("=== ESP32 Traffic Dashboard ===");

    mySerial.begin(115200, SERIAL_8N1, RX, TX);
    Serial.println("UART1 ready (RX=16, TX=17)");

    WiFi.begin(ssid, pass);
    Serial.print("Connecting WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", []()
    {
        server.send(200, "text/html", getDashboardHTML());
    });

    server.on("/data", handleData);

    server.begin();
    Serial.println("Web Server Started!");
    server.on("/lane", []() {
    if (!server.hasArg("i")) {
        server.send(400, "text/plain", "Missing lane index");
        return;
    }

    int i = server.arg("i").toInt();

    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Lane " + String(i+1) + "</title>";

    // ===== CSS =====
    html += "<style>";
    html += "body{font-family:Arial;background:#0f172a;color:#fff;text-align:center;padding:20px}";
    html += ".card{background:#1e293b;padding:20px;border-radius:15px;max-width:300px;margin:auto}";
    html += ".title{font-size:22px;font-weight:bold;margin-bottom:10px}";
    html += ".state{font-size:30px;font-weight:bold;margin:15px 0}";
    html += ".red{color:#ef4444}";
    html += ".yellow{color:#eab308}";
    html += ".green{color:#22c55e}";
    html += ".info{margin:8px 0;font-size:18px}";
    html += "a{display:inline-block;margin-top:15px;color:#38bdf8;text-decoration:none}";
    html += "</style>";

    html += "</head><body>";

    html += "<div class='card'>";
    html += "<div class='title'>LANE " + String(i+1) + "</div>";

    html += "<div id='state' class='state'>--</div>";
    html += "<div class='info'>Xe: <span id='count'>0</span></div>";
    html += "<div class='info'>Sensor: <span id='sensor'>OFF</span></div>";

    html += "<a href='/'>⬅ Quay lại</a>";
    html += "</div>";

    // ===== JS =====
    html += "<script>";
    html += "let lane=" + String(i) + ";";

    html += "function update(d){";
    html += " let s=d['state'+(lane+1)];";
    html += " let c=d['count'+(lane+1)];";
    html += " let sen=d['sensor'+(lane+1)];";

    html += " let st=document.getElementById('state');";

    html += " if(s==0){st.innerText='RED';st.className='state red';}";
    html += " if(s==1){st.innerText='YELLOW';st.className='state yellow';}";
    html += " if(s==2){st.innerText='GREEN';st.className='state green';}";

    html += " document.getElementById('count').innerText=c;";
    html += " document.getElementById('sensor').innerText=sen?'ON':'OFF';";
    html += "}";

    html += "setInterval(()=>{";
    html += " fetch('/data').then(r=>r.json()).then(update);";
    html += "},1000);";

    html += "</script>";

    html += "</body></html>";

    server.send(200, "text/html", html);
});
webSocket.begin();
webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length){
    if(type == WStype_CONNECTED){
        Serial.println("WS Connected");
    }
});
}

// ================= LOOP =================
void loop()
{
    server.handleClient();
    readSTM32();
    webSocket.loop();
}