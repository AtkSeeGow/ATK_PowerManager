# PowerManager.Switch

`PowerManager.Switch` 是 ESP32 GPIO 開關韌體，提供 heartbeat、GPIO 讀取、持續
輸出與限時 pulse HTTP API。

## 本機設定

複製範例檔後填入 Wi-Fi 資訊：

```powershell
Copy-Item src/PowerManager.Switch/SwitchSecrets.example.h src/PowerManager.Switch/SwitchSecrets.h
```

`SwitchSecrets.h` 已由 Git 忽略，可定義：

```cpp
#define POWER_MANAGER_WIFI_SSID "<Wi-Fi SSID>"
#define POWER_MANAGER_WIFI_PASSWORD "<Wi-Fi password>"
#define POWER_MANAGER_WIFI_HOSTNAME "ESP32-SWITCH"
```

一般運作設定位於 `SwitchConfiguration.h`。目前 GPIO allowlist：

- Input：GPIO `35`
- Output：GPIO `25`

只有 allowlist 中的腳位可以讀取，只有 Output allowlist 中的腳位可以寫入或
pulse。Output 開機時初始化為 `LOW`。

## HTTP API

服務使用 HTTP port `80`。所有回應皆為
`application/json; charset=utf-8`，並包含 `Cache-Control: no-store`。

### Heartbeat

```http
GET /api/heartbeat
```

```json
{"status":"ok"}
```

### 讀取 GPIO

```http
GET /api/gpio?pin=35
```

```json
{"pin":35,"value":1}
```

Input 與 Output 腳位皆可讀取。

### 設定持續輸出

```http
POST /api/gpio?pin=25&value=1
```

```json
{"pin":25,"value":1}
```

`value=1` 表示 `HIGH`，`value=0` 表示 `LOW`。設定會維持到下一次 set 或 pulse
改變狀態。

### 限時 Pulse

```http
POST /api/gpio/pulse?pin=25&value=1&durationMs=500
```

```json
{"pin":25,"value":1,"durationMs":500}
```

`durationMs` 必須介於 `1` 到 `3600000`。Pulse 會立即套用指定值，時間到後
恢復 pulse 前的穩定值。Pulse 尚未結束時收到另一個 pulse，會更新值與計時器，
但仍恢復第一次 pulse 前的穩定值；持續 set 則會取消 pulse 並成為新的穩定值。

## 錯誤回應

參數只接受無符號十進位數字，不接受正負號、空白或其他字元。

| HTTP | 回應 | 原因 |
| --- | --- | --- |
| `400` | `{"error":"invalid_pin"}` | 讀取請求缺少 pin 或格式錯誤 |
| `400` | `{"error":"invalid_command"}` | set/pulse 缺少參數、格式錯誤或超出範圍 |
| `400` | `{"error":"unsupported_pin"}` | 腳位不在對應 allowlist |
| `404` | `{"error":"not_found"}` | 路由不存在 |

## 執行行為與安全性

- Wi-Fi 連線採非阻塞更新，斷線後每 10 秒重試。
- HTTP server 只在 Wi-Fi 已連線時啟動，斷線時停止。
- NTP 使用 `CST-8` 時區及 `pool.ntp.org`、`time.nist.gov`。
- uptime 超過一天後韌體會主動重新啟動。
- Switch API 本身沒有驗證；正常架構應由 `PowerManager.Api` 代理，並由後者的
  Bearer Token 保護 GPIO 控制 endpoint。
