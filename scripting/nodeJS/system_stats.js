const os = require('os');
const fs = require('fs');
const { execSync } = require('child_process');
const mysql = require('mysql2');

// MySQL config
const db = mysql.createPool({
    host: 'localhost',
    user: '',
    password: '',
    database: '',
    port: 6969
});

// Cache to store last known values
const lastValues = {
    memory_used_gib: null,
    memory_total_gib: null,
    cpu_usage_percent: null,
    cpu_frequency_x10: null,
    cpu_temperature_c: null
};

function getMemoryInfoGiB() {
    const totalGiB = Math.round(os.totalmem() / 1073741824);
    const usedGiB = Math.round((os.totalmem() - os.freemem()) / 1073741824);
    return { totalGiB, usedGiB };
}

function getCPUFrequencyMultiplied() {
    const freqMHz = os.cpus()[0].speed;
    const freqGHz = freqMHz / 1000;
    return Math.round(freqGHz * 10); // e.g., 3.5 GHz → 35
}

function getCPUTemperatureC() {
    try {
        const output = execSync('sensors').toString();
        const match = output.match(/Tctl:\s+\+?(\d+(\.\d+)?)/) ||
                      output.match(/Tdie:\s+\+?(\d+(\.\d+)?)/) ||
                      output.match(/\+(\d+(\.\d+)?)°C/);
        if (match) {
            return Math.round(parseFloat(match[1]));
        }
    } catch (_) {
        try {
            const raw = fs.readFileSync('/sys/class/thermal/thermal_zone0/temp', 'utf8');
            return Math.round(parseInt(raw) / 1000);
        } catch (_) {
            return 0;
        }
    }
    return 0;
}

function getCPUUsage(callback) {
    const start = os.cpus();

    setTimeout(() => {
        const end = os.cpus();
        let idleDiff = 0;
        let totalDiff = 0;

        for (let i = 0; i < start.length; i++) {
            const s = start[i].times;
            const e = end[i].times;
            const idle = e.idle - s.idle;
            const total = (e.user - s.user) + (e.nice - s.nice) +
                          (e.sys - s.sys) + (e.irq - s.irq) + idle;

            idleDiff += idle;
            totalDiff += total;
        }

        const usage = Math.round(100 - (idleDiff / totalDiff) * 100);
        callback(usage);
    }, 1000);
}

function updateIfChanged(table, newValue) {
    if (lastValues[table] !== newValue) {
        lastValues[table] = newValue;
        const query = `UPDATE ${table} SET value = ? WHERE id = 1`;
        db.query(query, [newValue], (err) => {
            if (err) console.error(`DB error updating ${table}:`, err.message);
        });
    }
}

function collectAndUpdateStats() {
    getCPUUsage((cpuUsage) => {
        const { usedGiB, totalGiB } = getMemoryInfoGiB();
        const cpuFreq = getCPUFrequencyMultiplied();
        const cpuTemp = getCPUTemperatureC();

        updateIfChanged('memory_used_gib', usedGiB);
        updateIfChanged('memory_total_gib', totalGiB);
        updateIfChanged('cpu_usage_percent', cpuUsage);
        updateIfChanged('cpu_frequency_x10', cpuFreq);
        updateIfChanged('cpu_temperature_c', cpuTemp);
    });
}

setInterval(collectAndUpdateStats, 10000); // every 10 seconds