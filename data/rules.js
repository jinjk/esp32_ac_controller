let currentRules = [];
let editingRuleId = null;

// Load and display active rule
async function loadActiveRule() {
    try {
        const response = await fetch('/api/rules/active');
        if (response.ok) {
            const data = await response.json();
            updateActiveRuleDisplay(data);
        }
    } catch (error) {
        document.getElementById('active-rule-status').innerHTML = '<span style="color: red;">[错误] 加载激活规则时出错</span>';
    }
}

function updateActiveRuleDisplay(data) {
    const statusDiv = document.getElementById('active-rule-status');
    let statusHtml = `<div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px;">`;
    statusHtml += `<div><strong>[温度] 当前温度：</strong> ${data.currentTemp?.toFixed(1) || 'N/A'}°C</div>`;
    statusHtml += `<div><strong>[时间] 当前时间：</strong> ${data.currentHour || 'N/A'}:00</div>`;
    statusHtml += `</div>`;
    
    if (data.activeRuleId && data.activeRuleId !== -1 && data.activeRule) {
        const rule = data.activeRule;
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
        statusHtml += `<div style="margin-top: 15px; padding: 15px; background: #fff3cd; border-radius: 8px; border-left: 4px solid #ffc107;">`;
        statusHtml += `<h4 style="margin: 0; color: #856404;">[STANDBY] No active rule - AC unchanged</h4>`;
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
        document.getElementById('rules-list').innerHTML = '<span style="color: red;">[ERROR] Error loading rules</span>';
    }
}

function updateRulesDisplay() {
    const rulesDiv = document.getElementById('rules-list');
    if (currentRules.length === 0) {
        rulesDiv.innerHTML = '<div style="text-align: center; padding: 40px; color: #666;"><h3>[EMPTY] No rules configured</h3><p>Click "Add Rule" to create your first automation rule.</p></div>';
        return;
    }
    
    let rulesHtml = '';
    currentRules.forEach((rule, index) => {
        const enabledClass = rule.enabled ? 'active' : 'disabled';
        const statusClass = rule.enabled ? 'enabled' : 'disabled';
        const statusText = rule.enabled ? '[ON]' : '[OFF]';
        
        // Build condition display
        const conditions = [];
        if (rule.startHour !== -1 && rule.endHour !== -1) {
            conditions.push(`[TIME] ${rule.startHour}:00-${rule.endHour}:00`);
        }
        if (rule.minTemp !== -999 || rule.maxTemp !== -999) {
            const minStr = rule.minTemp !== -999 ? `≥${rule.minTemp}°C` : '';
            const maxStr = rule.maxTemp !== -999 ? `≤${rule.maxTemp}°C` : '';
            conditions.push(`[TEMP] ${minStr}${minStr && maxStr ? ' & ' : ''}${maxStr}`);
        }
        
        const conditionText = conditions.length > 0 ? conditions.join(' • ') : '[ANY] Always';
        const acAction = rule.acOn ? `[ON] Power ON → [TEMP] ${rule.setTemp}°C, [FAN] ${['Auto', 'Low', 'Med', 'High'][rule.fanSpeed]}` : '[OFF] Power OFF';
        
        rulesHtml += `
            <div class="rule-item ${enabledClass}">
                <div class="rule-header">
                    <h3 class="rule-title">[RULE] ${rule.name} (ID: ${rule.id})</h3>
                    <span class="rule-status ${statusClass}">${statusText}</span>
                </div>
                
                <div class="rule-details">
                    <p><strong>[CONDITIONS]</strong> ${conditionText}</p>
                    <p><strong>[ACTION]</strong> ${acAction}</p>
                </div>
                
                <div class="rule-actions">
                    <button onclick="editRule(${rule.id})" class="btn btn-primary">[EDIT] Edit</button>
                    <button onclick="toggleRule(${rule.id}, ${!rule.enabled})" class="btn ${rule.enabled ? 'btn-warning' : 'btn-success'}">
                        ${rule.enabled ? '[DISABLE] Disable' : '[ENABLE] Enable'}
                    </button>
                    <button onclick="deleteRuleConfirm(${rule.id})" class="btn btn-danger">[DELETE] Delete</button>
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
            showNotification('[ERROR] Failed to create rule', 'error');
        }
    } catch (error) {
        showNotification('[ERROR] Connection failed', 'error');
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
                showNotification('[SUCCESS] Rule saved successfully', 'success');
            } else {
                showNotification('[ERROR] Failed to save rule', 'error');
            }
        } catch (error) {
            showNotification('[ERROR] Connection failed', 'error');
        }
    });
});

// Delete current rule
async function deleteCurrentRule() {
    if (!editingRuleId || !confirm('[CONFIRM] Are you sure you want to delete this rule? This action cannot be undone.')) return;
    
    try {
        const response = await fetch(`/api/rules?id=${editingRuleId}`, { method: 'DELETE' });
        if (response.ok) {
            closeRuleEditor();
            await loadRules();
            await loadActiveRule();
            showNotification('[SUCCESS] Rule deleted successfully', 'success');
        } else {
            showNotification('[ERROR] Failed to delete rule', 'error');
        }
    } catch (error) {
        showNotification('[ERROR] Connection failed', 'error');
    }
}

// Confirm rule deletion
function deleteRuleConfirm(ruleId) {
    const rule = currentRules.find(r => r.id === ruleId);
    if (!rule) return;
    
    if (confirm(`[CONFIRM] Delete rule "${rule.name}"?\n\nThis action cannot be undone.`)) {
        deleteRuleById(ruleId);
    }
}

// Delete rule by ID
async function deleteRuleById(ruleId) {
    try {
        const response = await fetch(`/api/rules?id=${ruleId}`, { method: 'DELETE' });
        if (response.ok) {
            await loadRules();
            await loadActiveRule();
            showNotification('[SUCCESS] Rule deleted successfully', 'success');
        } else {
            showNotification('[ERROR] Failed to delete rule', 'error');
        }
    } catch (error) {
        showNotification('[ERROR] Connection failed', 'error');
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
            showNotification(`[SUCCESS] Rule ${enabled ? 'enabled' : 'disabled'}`, 'success');
        } else {
            showNotification('[ERROR] Failed to update rule', 'error');
        }
    } catch (error) {
        showNotification('[ERROR] Connection failed', 'error');
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

// Initialize page
window.addEventListener('load', function() {
    loadRules();
    loadActiveRule();
    setInterval(loadActiveRule, 30000); // Refresh active rule every 30 seconds
});
