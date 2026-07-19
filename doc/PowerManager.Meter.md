# PowerManager.Meter

`PowerManager.Meter` 是 ESP32 電表韌體，透過 RS-485 Modbus RTU 讀取 Schneider
PM5350，並透過 I²C 讀取 AM2320 溫濕度感測器。最新資料由 HTTP JSON API 提供。

## 本機設定

複製範例檔後再填入部署資訊：

```powershell
Copy-Item src/PowerManager.Meter/MeterConfig.example.h src/PowerManager.Meter/MeterConfig.h
```

`MeterConfig.h` 已由 Git 忽略。可設定 Wi-Fi、hostname、HTTP port、NTP、log
baud rate、讀取週期、Modbus retries、AM2320 retries 與 power-factor 判定門檻。

預設硬體設定：

| 項目 | 設定 |
| --- | --- |
| PM5350 slave ID | `2` |
| Modbus | `9600 8E1` |
| ESP32 RX / TX | GPIO `16` / `17` |
| MAX485 DE / RE | GPIO `19` / `18` |
| AM2320 SDA / SCL | GPIO `32` / `33` |
| AM2320 I²C clock | `100 kHz` |
| HTTP port | `80` |
| 讀取週期 | `5000 ms` |

## HTTP API

### API discovery

```http
GET /
```

```json
{"api":"/api/meter","method":"GET"}
```

### 最新讀值

```http
GET /api/meter
```

回應標頭：

```text
Content-Type: application/json; charset=utf-8
Access-Control-Allow-Origin: *
Cache-Control: no-store
```

回應範例：

```json
{
  "voltage_ln": 118.156,
  "current_a": 0.0,
  "current_b": 0.0,
  "current_c": null,
  "current_average": 0.0,
  "active_power_total_kw": 0.0,
  "power_factor_total": null,
  "power_factor_leading": null,
  "frequency_hz": 60.044,
  "active_energy_delivered_wh": 38491114,
  "active_energy_delivered_kwh": 38491.114,
  "temperature_c": 27.3,
  "humidity_percent": 68.2,
  "am2320_valid": true,
  "am2320_status": 0,
  "am2320_wire_status": 0,
  "am2320_failure_counts": {
    "not_initialized": 0,
    "command_failed": 0,
    "response_too_short": 0,
    "invalid_frame": 0,
    "crc_mismatch": 0
  },
  "am2320_retry_recovery_count": 0,
  "ntp_synchronized": true,
  "current_time_utc": "2026-07-20T10:00:00Z",
  "boot_started_at_utc": "2026-07-20T09:00:00Z",
  "boot_started_at_epoch": 1784538000,
  "uptime_ms": 3600000,
  "units": {
    "voltage": "V",
    "current": "A",
    "active_power": "kW",
    "frequency": "Hz",
    "active_energy": "Wh"
  },
  "valid": true,
  "modbus_valid": true,
  "updated_at_ms": 3599000,
  "modbus_status": 0,
  "pm5350_power_factor_invalid_count": 0
}
```

## 欄位說明

| 欄位 | 說明 |
| --- | --- |
| `voltage_ln` | 相線對中性線電壓，單位 V |
| `current_a/b/c`、`current_average` | 各相與平均電流，單位 A |
| `active_power_total_kw` | 總有效功率，單位 kW |
| `power_factor_total`、`power_factor_leading` | 總功率因數與 leading/lagging 狀態 |
| `frequency_hz` | 頻率，單位 Hz |
| `active_energy_delivered_wh/kwh` | 累積輸入有效電能 |
| `temperature_c`、`humidity_percent` | AM2320 溫度與相對濕度 |
| `am2320_valid` | 本次 AM2320 讀取是否有效 |
| `am2320_status`、`am2320_wire_status` | AM2320 與 Wire 狀態碼 |
| `am2320_failure_counts` | 依失敗原因累積的讀取失敗次數 |
| `am2320_retry_recovery_count` | retry 後成功的累積次數 |
| `ntp_synchronized` | NTP 是否已同步 |
| `current_time_utc`、`boot_started_at_utc` | 目前時間與啟動時間；未同步時為 `null` |
| `uptime_ms`、`updated_at_ms` | 系統 uptime 與最後一次更新時間 |
| `valid` | 完整讀值是否符合目前接線模式與有效性條件 |
| `modbus_valid`、`modbus_status` | Modbus 讀取結果與狀態碼 |
| `pm5350_power_factor_invalid_count` | 非有限 power-factor 讀值累積次數 |

Modbus 或感測器資料無效時，相關數值以 JSON `null` 表示，不會用 `0` 代替。

## AM2320 狀態碼

| 值 | 名稱 |
| --- | --- |
| `0` | Success |
| `1` | NotInitialized |
| `2` | CommandFailed |
| `3` | ResponseTooShort |
| `4` | InvalidFrame |
| `5` | CrcMismatch |

## 執行行為

- 電表與 AM2320 讀取在獨立 FreeRTOS task 執行，HTTP 回傳最近一次 snapshot。
- Wi-Fi 中斷後會依設定週期重新連線。
- NTP 完成後才會提供 UTC 與 boot epoch。
- 未知路由回傳 `404 {"error":"not_found"}`。
- API 本身沒有驗證，應只部署於受信任網路，或由反向代理控制存取。
