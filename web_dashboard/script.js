const SAVE_FILE = '../saves/savegame.json';
const AUTO_REFRESH = 5000; // 5 seconds

// Achievement definitions
const ACHIEVEMENTS = [
    { id: 'first-kill', name: 'First Blood', condition: (data) => data.enemiesKilled >= 1, icon: '🎯' },
    { id: 'level-5', name: 'Rising Hero', condition: (data) => data.playerLevel >= 5, icon: '⭐' },
    { id: 'level-10', name: 'Legendary', condition: (data) => data.playerLevel >= 10, icon: '👑' },
    { id: 'score-1k', name: 'Millionaire', condition: (data) => data.score >= 1000, icon: '💰' },
    { id: 'kills-10', name: 'Slayer', condition: (data) => data.enemiesKilled >= 10, icon: '⚔️' },
    { id: 'floor-5', name: 'Deep Delver', condition: (data) => data.highestFloor >= 5, icon: '🏰' },
];

function loadGameData() {
    // Read saves/savegame.json
    fetch(SAVE_FILE)
        .then(response => {
            if (!response.ok) {
                console.log('Save file not found yet. Showing default stats.');
                return null;
            }
            return response.json();
        })
        .then(data => {
            if (data) {
                updateDashboard(data);
            }
        })
        .catch(error => console.log('Save file loading:', error));
}

function updateDashboard(data) {
    // Player Stats
    document.getElementById('level').textContent = data.playerLevel || 1;

    const health = data.playerHealth || 150;
    const maxHealth = data.playerMaxHealth || 150;
    document.getElementById('health').textContent = `${health}/${maxHealth}`;

    // Update health bar
    const healthPercentage = (health / maxHealth) * 100;
    document.getElementById('health-bar').style.width = `${healthPercentage}%`;

    document.getElementById('experience').textContent = data.playerExperience || 0;
    document.getElementById('weapon').textContent = data.currentWeapon || 'Wooden Sword';

    // Game Progress
    document.getElementById('floor').textContent = data.currentFloor || 1;
    document.getElementById('score').textContent = data.score || 0;
    document.getElementById('enemies-killed').textContent = data.enemiesKilled || 0;
    document.getElementById('playtime').textContent = formatTime(data.playTime || 0);

    // Inventory
    if (data.potions) {
        document.getElementById('health-potions').textContent = data.potions.health || 0;
        document.getElementById('speed-potions').textContent = data.potions.speed || 0;
        document.getElementById('stealth-potions').textContent = data.potions.stealth || 0;
        document.getElementById('rage-potions').textContent = data.potions.rage || 0;
    }

    // Statistics
    document.getElementById('damage-dealt').textContent = data.totalDamageDealt || 0;
    document.getElementById('damage-taken').textContent = data.totalDamageTaken || 0;
    document.getElementById('potions-used').textContent = data.potionsUsed || 0;
    document.getElementById('highest-floor').textContent = data.highestFloor || 1;

    // Last Save
    document.getElementById('last-save').textContent = data.lastSaveTime || 'Never';

    // Achievements
    updateAchievements(data);

    // Update timestamp
    document.getElementById('update-time').textContent = new Date().toLocaleTimeString();
}

function updateAchievements(data) {
    const container = document.getElementById('achievements-list');
    container.innerHTML = '';

    ACHIEVEMENTS.forEach(achievement => {
        const unlocked = achievement.condition(data);
        const div = document.createElement('div');
        div.className = `achievement ${unlocked ? 'unlocked' : ''}`;
        div.title = unlocked ? achievement.name : 'Locked';
        div.textContent = achievement.icon;
        container.appendChild(div);
    });
}

function formatTime(seconds) {
    const hrs = Math.floor(seconds / 3600);
    const mins = Math.floor((seconds % 3600) / 60);
    const secs = Math.floor(seconds % 60);

    if (hrs > 0) {
        return `${hrs}h ${mins}m`;
    } else if (mins > 0) {
        return `${mins}m ${secs}s`;
    } else {
        return `${secs}s`;
    }
}

function openGameFolder() {
    alert('📂 Save files are located in: saves/savegame.json\n\nYou can open this folder from your game directory.');
}

function launchGame() {
    alert('🎮 To launch the game:\n\n1. Navigate to your game folder\n2. Run DungeonCrawler2.exe\n\nOr create a shortcut on your desktop!');
    // In a real app, you could use electron or file:// protocol handlers
}

// Load data on page load
window.addEventListener('load', () => {
    loadGameData();
    // Auto-refresh every 5 seconds
    setInterval(loadGameData, AUTO_REFRESH);
});

// Allow manual refresh with R key
document.addEventListener('keydown', (e) => {
    if (e.key === 'r' || e.key === 'R') {
        loadGameData();
    }
});