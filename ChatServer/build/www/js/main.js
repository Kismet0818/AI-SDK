const API_BASE = ''; // 因为前端放在 www 下，由后端提供服务，所以可以使用相对路径

let currentSessionId = null;
let models = [];
let sessions = [];

// 初始化
document.addEventListener('DOMContentLoaded', () => {
    fetchSessions();
    initEventListeners();
});

// 事件监听
function initEventListeners() {
    // 新建对话按钮
    document.getElementById('new-chat-btn-top').onclick = openModelModal;
    document.getElementById('new-chat-btn-main').onclick = openModelModal;

    // 模态框按钮
    document.getElementById('modal-cancel').onclick = closeModal;
    document.getElementById('modal-confirm').onclick = createSession;

    // 消息输入
    const msgInput = document.getElementById('message-input');
    msgInput.oninput = (e) => {
        const len = e.target.value.length;
        document.getElementById('char-count').innerText = `${len}/2000`;
        document.getElementById('send-btn').disabled = len === 0;
    };

    msgInput.onkeydown = (e) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    };

    // 发送按钮
    document.getElementById('send-btn').onclick = sendMessage;
}

// 获取会话列表
async function fetchSessions() {
    try {
        const res = await fetch(`${API_BASE}/api/sessions`);
        const result = await res.json();
        if (result.success) {
            sessions = result.data;
            renderSessionList();
        }
    } catch (err) {
        console.error('获取会话失败:', err);
    }
}

// 渲染会话列表
function renderSessionList() {
    const list = document.getElementById('session-list');
    list.innerHTML = '';
    sessions.forEach(session => {
        const div = document.createElement('div');
        div.className = `session-item ${session.id === currentSessionId ? 'active' : ''}`;
        div.onclick = () => selectSession(session.id);
        
        const time = new Date(session.updated_at * 1000).toLocaleString();
        div.innerHTML = `
            <div class="session-info">
                <span class="session-time">${time}</span>
                <span class="session-preview">${session.first_user_message || '新对话'}</span>
                <span class="session-model">${session.model}</span>
            </div>
            <button class="btn-delete" onclick="deleteSession(event, '${session.id}')">...</button>
        `;
        list.appendChild(div);
    });
}

// 打开模型选择模态框
async function openModelModal() {
    try {
        const res = await fetch(`${API_BASE}/api/models`);
        const result = await res.json();
        if (result.success) {
            models = result.data;
            renderModelGrid();
            document.getElementById('model-modal').classList.remove('hidden');
        }
    } catch (err) {
        alert('获取模型列表失败');
    }
}

function renderModelGrid() {
    const grid = document.getElementById('model-grid');
    grid.innerHTML = '';
    models.forEach((model, index) => {
        const item = document.createElement('div');
        item.className = `model-item ${index === 0 ? 'selected' : ''}`;
        item.onclick = () => {
            document.querySelectorAll('.model-item').forEach(i => i.classList.remove('selected'));
            item.classList.add('selected');
            item.querySelector('input').checked = true;
        };
        item.innerHTML = `
            <input type="radio" name="model-select" value="${model.name}" ${index === 0 ? 'checked' : ''}>
            <div class="model-info">
                <h4>${model.name}</h4>
                <p>${model.desc}</p>
            </div>
        `;
        grid.appendChild(item);
    });
}

function closeModal() {
    document.getElementById('model-modal').classList.add('hidden');
}

// 创建新会话
async function createSession() {
    const selectedModel = document.querySelector('input[name="model-select"]:checked').value;
    try {
        const res = await fetch(`${API_BASE}/api/session`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ model: selectedModel })
        });
        const result = await res.json();
        if (result.success) {
            closeModal();
            currentSessionId = result.data.session_id;
            await fetchSessions();
            selectSession(currentSessionId);
        }
    } catch (err) {
        alert('创建会话失败');
    }
}

// 选择并加载会话
async function selectSession(sessionId) {
    currentSessionId = sessionId;
    document.getElementById('welcome-view').classList.add('hidden');
    document.getElementById('chat-view').classList.remove('hidden');
    renderSessionList();
    
    // 加载历史记录
    try {
        const res = await fetch(`${API_BASE}/api/session/${sessionId}/history`);
        const result = await res.json();
        if (result.success) {
            const chatMessages = document.getElementById('chat-messages');
            chatMessages.innerHTML = '';
            result.data.forEach(msg => {
                appendMessage(msg.role, msg.content, msg.timestamp);
            });
            scrollToBottom();
        }
    } catch (err) {
        console.error('加载历史记录失败:', err);
    }
}

// 删除会话
async function deleteSession(event, sessionId) {
    event.stopPropagation();
    if (!confirm('确定要删除这个会话吗？')) return;
    
    try {
        const res = await fetch(`${API_BASE}/api/session/${sessionId}`, {
            method: 'DELETE'
        });
        const result = await res.json();
        if (result.success) {
            // 乐观更新：立即从本地数组中删除并重新渲染
            sessions = sessions.filter(s => s.id !== sessionId);
            renderSessionList();

            if (currentSessionId === sessionId) {
                currentSessionId = null;
                document.getElementById('chat-view').classList.add('hidden');
                document.getElementById('welcome-view').classList.remove('hidden');
            }
            // fetchSessions(); // 不再需要重新获取，本地已经同步
        }
    } catch (err) {
        alert('删除失败');
    }
}

// 发送消息
async function sendMessage() {
    const input = document.getElementById('message-input');
    const content = input.value.trim();
    if (!content || !currentSessionId) return;

    input.value = '';
    document.getElementById('char-count').innerText = '0/2000';
    document.getElementById('send-btn').disabled = true;

    // 先在界面显示用户消息
    appendMessage('user', content, Math.floor(Date.now() / 1000));
    
    // 创建 AI 消息占位符
    const aiMsgId = 'ai-' + Date.now();
    const aiWrapper = appendMessage('assistant', '', Math.floor(Date.now() / 1000), aiMsgId);
    const contentDiv = aiWrapper.querySelector('.message-content');
    
    let fullContent = '';

    try {
        const response = await fetch(`${API_BASE}/api/message/async`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                session_id: currentSessionId,
                message: content
            })
        });

        const reader = response.body.getReader();
        const decoder = new TextDecoder();

        while (true) {
            const { done, value } = await reader.read();
            if (done) break;

            const chunk = decoder.decode(value);
            const lines = chunk.split('\n');
            
            for (const line of lines) {
                if (line.startsWith('data: ')) {
                    let data = line.slice(6);
                    if (data === '[DONE]') {
                        // 完成后更新左侧预览
                        fetchSessions();
                        break;
                    }
                    
                    // 如果数据被引号包裹（来自旧版后端的残留或特殊格式），则尝试解析它
                    if (data.startsWith('"') && data.endsWith('"')) {
                        try {
                            data = JSON.parse(data);
                        } catch (e) {
                            // 解析失败则保持原样
                        }
                    }

                    fullContent += data;
                    // 使用 marked 解析 Markdown 并高亮
                    contentDiv.innerHTML = marked.parse(fullContent);
                    contentDiv.querySelectorAll('pre code').forEach((el) => {
                        hljs.highlightElement(el);
                        addCopyButton(el);
                    });
                    scrollToBottom();
                }
            }
        }
    } catch (err) {
        contentDiv.innerText = '发送失败，请检查网络连接';
    }
}

function appendMessage(role, content, timestamp, id) {
    const chatMessages = document.getElementById('chat-messages');
    const wrapper = document.createElement('div');
    wrapper.className = `message-wrapper message-${role === 'user' ? 'user' : 'ai'}`;
    if (id) wrapper.id = id;

    const timeStr = new Date(timestamp * 1000).toLocaleTimeString();
    
    const contentDiv = document.createElement('div');
    contentDiv.className = 'message-content';
    contentDiv.innerHTML = role === 'user' ? content : marked.parse(content);
    
    // 如果是 AI 消息且有内容，添加代码高亮和复制按钮
    if (role !== 'user') {
        contentDiv.querySelectorAll('pre code').forEach((el) => {
            hljs.highlightElement(el);
            addCopyButton(el);
        });
    }

    const timeDiv = document.createElement('div');
    timeDiv.className = 'message-time';
    timeDiv.innerText = timeStr;

    wrapper.appendChild(contentDiv);
    wrapper.appendChild(timeDiv);
    chatMessages.appendChild(wrapper);
    scrollToBottom();
    return wrapper;
}

function addCopyButton(codeEl) {
    const pre = codeEl.parentElement;
    if (pre.querySelector('.code-header')) return;

    const header = document.createElement('div');
    header.className = 'code-header';
    const lang = codeEl.className.replace('language-', '') || 'code';
    header.innerHTML = `<span>${lang}</span><span class="copy-btn"><i class="far fa-copy"></i> 复制</span>`;
    
    header.querySelector('.copy-btn').onclick = () => {
        navigator.clipboard.writeText(codeEl.innerText).then(() => {
            header.querySelector('.copy-btn').innerHTML = '<i class="fas fa-check"></i> 已复制';
            setTimeout(() => {
                header.querySelector('.copy-btn').innerHTML = '<i class="far fa-copy"></i> 复制';
            }, 2000);
        });
    };

    pre.insertBefore(header, codeEl);
}

function scrollToBottom() {
    const chatMessages = document.getElementById('chat-messages');
    chatMessages.scrollTop = chatMessages.scrollHeight;
}
