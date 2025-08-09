let currentRules = [];
let editingRuleId = null;
let debugMode = false; // Debug mode flag

// Load and display active rule
async function loadActiveRule() {
    try {
        // Fetch active rule data
        const ruleResponse = await fetch('/api/rules/active');
        let ruleData = {};
        if (ruleResponse.ok) {
            ruleData = await ruleResponse.json();
        }
        
        // Fetch current temperature
        const tempResponse = await fetch('/api/temp');
        let tempData = {};
        if (tempResponse.ok) {
            tempData = await tempResponse.json();
        }
        
        // Fetch system info for current time
        const systemResponse = await fetch('/api/system');
        let systemData = {};
        if (systemResponse.ok) {
            systemData = await systemResponse.json();
        }
        
        // Combine all data
        const combinedData = {
            ...ruleData,
            currentTemp: tempData.temp,
            currentHour: systemData.currentHour
        };
        
        updateActiveRuleDisplay(combinedData);
    } catch (error) {
        document.getElementById('active-rule-status').innerHTML = '<span style="color: red;">❌ 加载激活规则时出错</span>';
    }
}

function updateActiveRuleDisplay(data) {
    const statusDiv = document.getElementById('active-rule-status');
    let statusHtml = `<div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px;">`;
    
    // Display temperature with fallback
    const tempDisplay = data.currentTemp !== undefined ? 
        `${data.currentTemp.toFixed(1)}°C` : 
        '<span style="color: #999;">获取中...</span>';
    statusHtml += `<div><strong>🌡️ 当前温度：</strong> ${tempDisplay}</div>`;
    
    // Display time with fallback
    const timeDisplay = data.currentHour !== undefined ? 
        `${data.currentHour}:00` : 
        '<span style="color: #999;">获取中...</span>';
    statusHtml += `<div><strong>🕐 当前时间：</strong> ${timeDisplay}</div>`;
    statusHtml += `</div>`;
    
    if (data.activeRuleId && data.activeRuleId !== -1) {
        // If we have an activeRuleId, try to get the rule details
        let rule = data.activeRule;
        
        // If activeRule wasn't provided by the API, try to find it in currentRules
        if (!rule && currentRules.length > 0) {
            rule = currentRules.find(r => r.id === data.activeRuleId);
        }
        
        if (rule) {
            statusHtml += `<div style="margin-top: 15px; padding: 15px; background: #e8f5e8; border-radius: 8px; border-left: 4px solid #28a745;">`;
            statusHtml += `<h4 style="margin: 0 0 10px 0; color: #28a745;">✅ ${rule.name} (ID: ${rule.id})</h4>`;
            
            const conditionsHtml = [];
            if (rule.startHour !== -1 && rule.endHour !== -1) {
                conditionsHtml.push(`🕐 ${rule.startHour}:00-${rule.endHour}:00`);
            }
            if (rule.minTemp !== -999 || rule.maxTemp !== -999) {
                const minStr = rule.minTemp !== -999 ? `≥${rule.minTemp}°C` : '';
                const maxStr = rule.maxTemp !== -999 ? `≤${rule.maxTemp}°C` : '';
                conditionsHtml.push(`🌡️ ${minStr}${minStr && maxStr ? ' & ' : ''}${maxStr}`);
            }
            
            if (conditionsHtml.length > 0) {
                statusHtml += `<p><strong>📋 条件：</strong> ${conditionsHtml.join(', ')}</p>`;
            }
            
            statusHtml += `<p><strong>❄️ 空调动作：</strong> ${rule.acOn ? '✅ 开机' : '❌ 关机'}`;
            if (rule.acOn) {
                statusHtml += ` → 🌡️ ${rule.setTemp}°C, 💨 ${['🔄 自动', '🌪️ 低速', '💨 中速', '🌀 高速'][rule.fanSpeed]}, 🎛️ ${['❄️ 制冷', '🔥 制热', '💧 除湿', '💨 送风', '🔄 自动'][rule.mode]}`;
            }
            statusHtml += `</p></div>`;
        } else {
            // We have activeRuleId but couldn't find rule details
            statusHtml += `<div style="margin-top: 15px; padding: 15px; background: #fff3cd; border-radius: 8px; border-left: 4px solid #ffc107;">`;
            statusHtml += `<h4 style="margin: 0; color: #856404;">⚠️ 规则 ID ${data.activeRuleId} 激活中 - 规则详情加载中...</h4>`;
            statusHtml += `</div>`;
        }
    } else {
        statusHtml += `<div style="margin-top: 15px; padding: 15px; background: #fff3cd; border-radius: 8px; border-left: 4px solid #ffc107;">`;
        statusHtml += `<h4 style="margin: 0; color: #856404;">💤 No active rule - AC unchanged</h4>`;
        statusHtml += `</div>`;
    }
    
    statusDiv.innerHTML = statusHtml;
}

// Load and display all rules
async function loadRules() {
    try {
        const response = await fetch('/api/rules');
        if (response.ok) {
            const data = await response.json();
            currentRules = data.rules || [];
            updateRulesDisplay();
        }
    } catch (error) {
        document.getElementById('rules-list').innerHTML = '<span style="color: red;">❌ Error loading rules</span>';
    }
}

function updateRulesDisplay() {
    const rulesDiv = document.getElementById('rules-list');
    if (currentRules.length === 0) {
        rulesDiv.innerHTML = '<div style="text-align: center; padding: 40px; color: #666;"><h3>📭 No rules configured</h3><p>Click "➕ Add Rule" to create your first automation rule.</p></div>';
        return;
    }
    
    let rulesHtml = '';
    currentRules.forEach((rule, index) => {
        const enabledClass = rule.enabled ? 'active' : 'disabled';
        const statusClass = rule.enabled ? 'enabled' : 'disabled';
        const statusText = rule.enabled ? '✅ ON' : '❌ OFF';
        
        // Build condition display
        const conditions = [];
        if (rule.startHour !== -1 && rule.endHour !== -1) {
            conditions.push(`🕐 ${rule.startHour}:00-${rule.endHour}:00`);
        }
        if (rule.minTemp !== -999 || rule.maxTemp !== -999) {
            const minStr = rule.minTemp !== -999 ? `≥${rule.minTemp}°C` : '';
            const maxStr = rule.maxTemp !== -999 ? `≤${rule.maxTemp}°C` : '';
            conditions.push(`🌡️ ${minStr}${minStr && maxStr ? ' & ' : ''}${maxStr}`);
        }
        
        const conditionText = conditions.length > 0 ? conditions.join(' • ') : '🌐 未定义';
        const modeNames = ['制冷', '制热', '除湿', '送风', '自动'];
        const acAction = rule.acOn ? 
            `🔛 开启 → 🌡️ ${rule.setTemp}°C, 模式: ${modeNames[rule.mode]}, 风速: ${['自动', '低', '中', '高'][rule.fanSpeed]}` : 
            '🔴 关闭';
        
        rulesHtml += `
            <div class="rule-item ${enabledClass}">
                <div class="rule-header">
                    <h3 class="rule-title">⚙️ 规则 ${rule.name} (编号: ${rule.id})</h3>
                    <span class="rule-status ${statusClass}">${statusText}</span>
                </div>
                
                <div class="rule-details">
                    <p><strong>📋 条件</strong> ${conditionText}</p>
                    <p><strong>⚡ 动作</strong> ${acAction}</p>
                </div>
                
                <div class="rule-actions">
                    <button onclick="editRule(${rule.id})" class="btn btn-primary">✏️ 修改</button>
                    <button onclick="toggleRule(${rule.id}, ${!rule.enabled})" class="btn ${rule.enabled ? 'btn-warning' : 'btn-success'}">
                        ${rule.enabled ? '⏸️ 停用' : '▶️ 启用'}
                    </button>
                    <button onclick="deleteRuleConfirm(${rule.id})" class="btn btn-danger">🗑️ 删除</button>
                </div>
            </div>
        `;
    });
    
    rulesDiv.innerHTML = rulesHtml;
}

// Create new rule
async function createNewRule() {
    try {
        const response = await fetch('/api/rules', { method: 'POST' });
        if (response.ok) {
            const data = await response.json();
            await loadRules();
            editRule(data.ruleId);
        } else {
            showNotification('❌ Failed to create rule', 'error');
        }
    } catch (error) {
        showNotification('❌ Connection failed', 'error');
    }
}

// Edit rule
function editRule(ruleId) {
    editingRuleId = ruleId;
    const rule = currentRules.find(r => r.id === ruleId);
    if (!rule) return;
    
    // Populate form
    document.getElementById('rule-id').value = rule.id;
    document.getElementById('rule-name').value = rule.name;
    document.getElementById('rule-enabled').checked = rule.enabled;
    document.getElementById('rule-start-hour').value = rule.startHour === -1 ? '' : rule.startHour;
    document.getElementById('rule-end-hour').value = rule.endHour === -1 ? '' : rule.endHour;
    document.getElementById('rule-min-temp').value = rule.minTemp === -999 ? '' : rule.minTemp;
    document.getElementById('rule-max-temp').value = rule.maxTemp === -999 ? '' : rule.maxTemp;
    document.getElementById('rule-ac-on').checked = rule.acOn;
    document.getElementById('rule-set-temp').value = rule.setTemp;
    document.getElementById('rule-fan-speed').value = rule.fanSpeed;
    document.getElementById('rule-mode').value = rule.mode;
    document.getElementById('rule-v-swing').value = rule.vSwing || 0;
    document.getElementById('rule-h-swing').value = rule.hSwing || 0;
    
    // Show modal
    document.getElementById('rule-editor').style.display = 'block';
    document.getElementById('delete-rule-btn').style.display = 'inline-block';
}

// Close rule editor
function closeRuleEditor() {
    document.getElementById('rule-editor').style.display = 'none';
    editingRuleId = null;
}

// Save rule
document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('rule-form').addEventListener('submit', async function(e) {
        e.preventDefault();
        
        const formData = new FormData(e.target);
        const params = new URLSearchParams();
        
        // Convert form data to URL parameters
        for (let [key, value] of formData.entries()) {
            if (key === 'enabled' || key === 'acOn') {
                params.append(key, value ? 'true' : 'false');
            } else if (key === 'minTemp' || key === 'maxTemp') {
                params.append(key, value === '' ? '-999' : value);
            } else if (key === 'startHour' || key === 'endHour') {
                params.append(key, value === '' ? '-1' : value);
            } else {
                params.append(key, value);
            }
        }
        
        try {
            const response = await fetch('/api/rules', {
                method: 'PUT',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: params
            });
            
            if (response.ok) {
                closeRuleEditor();
                await loadRules();
                await loadActiveRule();
                showNotification('✅ [SUCCESS] Rule saved successfully', 'success');
            } else {
                showNotification('❌ [ERROR] Failed to save rule', 'error');
            }
        } catch (error) {
            showNotification('🚫 [ERROR] Connection failed', 'error');
        }
    });
});

// Delete current rule
async function deleteCurrentRule() {
    if (!editingRuleId || !confirm('⚠️ CONFIRM Are you sure you want to delete this rule? This action cannot be undone.')) return;
    
    try {
        const params = new URLSearchParams();
        params.append('id', editingRuleId);
        
        const response = await fetch('/api/rules', {
            method: 'DELETE',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: params
        });
        
        if (response.ok) {
            closeRuleEditor();
            await loadRules();
            await loadActiveRule();
            showNotification('✅ Rule deleted successfully', 'success');
        } else {
            const errorData = await response.json();
            showNotification(`❌ Failed to delete rule: ${errorData.message}`, 'error');
        }
    } catch (error) {
        showNotification('❌ Connection failed', 'error');
    }
}

// Confirm rule deletion
function deleteRuleConfirm(ruleId) {
    const rule = currentRules.find(r => r.id === ruleId);
    if (!rule) return;
    
    if (confirm(`⚠️ CONFIRM Delete rule "${rule.name}"?\n\nThis action cannot be undone.`)) {
        deleteRuleById(ruleId);
    }
}

// Delete rule by ID
async function deleteRuleById(ruleId) {
    try {
        const params = new URLSearchParams();
        params.append('id', ruleId);
        
        const response = await fetch('/api/rules', {
            method: 'DELETE',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: params
        });
        
        if (response.ok) {
            await loadRules();
            await loadActiveRule();
            showNotification('✅ Rule deleted successfully', 'success');
        } else {
            const errorData = await response.json();
            showNotification(`❌ Failed to delete rule: ${errorData.message}`, 'error');
        }
    } catch (error) {
        showNotification('❌ Connection failed', 'error');
    }
}

// Toggle rule enabled/disabled
async function toggleRule(ruleId, enabled) {
    try {
        const params = new URLSearchParams();
        params.append('id', ruleId);
        params.append('enabled', enabled ? 'true' : 'false');
        
        const response = await fetch('/api/rules', {
            method: 'PUT',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: params
        });
        
        if (response.ok) {
            await loadRules();
            await loadActiveRule();
            showNotification(`✅ Rule ${enabled ? 'enabled' : 'disabled'}`, 'success');
        } else {
            showNotification('❌ Failed to update rule', 'error');
        }
    } catch (error) {
        showNotification('❌ Connection failed', 'error');
    }
}

// Show notification
function showNotification(message, type) {
    const notification = document.createElement('div');
    notification.style.cssText = `
        position: fixed; top: 20px; right: 20px; z-index: 1001;
        padding: 15px 25px; border-radius: 8px; color: white; font-weight: bold;
        background: ${type === 'success' ? '#28a745' : '#dc3545'};
        box-shadow: 0 4px 12px rgba(0,0,0,0.3);
        transform: translateX(100%); transition: transform 0.3s;
    `;
    notification.textContent = message;
    document.body.appendChild(notification);
    
    setTimeout(() => notification.style.transform = 'translateX(0)', 100);
    setTimeout(() => {
        notification.style.transform = 'translateX(100%)';
        setTimeout(() => document.body.removeChild(notification), 300);
    }, 3000);
}

// Debug mode functions
function toggleDebugMode() {
    debugMode = document.getElementById('debug-mode-toggle').checked;
    updateDebugModeDisplay();
    saveDebugModeToServer();
}

function updateDebugModeDisplay() {
    const statusElement = document.getElementById('debug-mode-status');
    const textElement = document.getElementById('debug-mode-text');
    const descriptionElement = document.getElementById('debug-mode-description');
    
    if (debugMode) {
        statusElement.textContent = '调试模式';
        statusElement.style.background = '#ffc107';
        statusElement.style.color = '#212529';
        textElement.textContent = '🔧 调试模式：强制发送IR命令';
        descriptionElement.textContent = '开启时：忽略状态检查，每次都发送IR命令（测试模式）';
    } else {
        statusElement.textContent = '正常模式';
        statusElement.style.background = '#28a745';
        statusElement.style.color = 'white';
        textElement.textContent = '🚀 调试模式：强制发送IR命令';
        descriptionElement.textContent = '关闭时：仅在状态改变时发送IR命令（省电模式）';
    }
}

async function saveDebugModeToServer() {
    try {
        const response = await fetch('/api/debug/mode', {
            method: 'POST',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: `enabled=${debugMode ? 'true' : 'false'}`
        });
        
        if (response.ok) {
            showNotification(`✅ ${debugMode ? '调试模式已开启' : '调试模式已关闭'}`, 'success');
        } else {
            showNotification('❌ 无法保存调试模式设置', 'error');
        }
    } catch (error) {
        showNotification('❌ 连接失败', 'error');
    }
}

async function loadDebugMode() {
    try {
        const response = await fetch('/api/debug/mode');
        if (response.ok) {
            const data = await response.json();
            debugMode = data.debugMode || false;
            document.getElementById('debug-mode-toggle').checked = debugMode;
            updateDebugModeDisplay();
        }
    } catch (error) {
        console.log('Debug mode API not available, using default');
    }
}

// Initialize page
window.addEventListener('load', function() {
    loadRules();
    loadActiveRule();
    loadDebugMode(); // Load debug mode setting
    setInterval(loadActiveRule, 30000); // Refresh active rule every 30 seconds
});
