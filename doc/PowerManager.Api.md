# PowerManager.Api

`PowerManager.Api` 是系統的 Web API 與 Prometheus exporter。它會依設定向
`PowerManager.Meter` 與 `PowerManager.Switch` 取得資料，並代理 Switch 的 GPIO
控制請求。

## 服務端點

預設監聽位置定義於 `src/PowerManager.Api/appsettings.json`：

- HTTP：`http://0.0.0.0:8920`
- HTTPS：`https://0.0.0.0:8921`
- Prometheus：`GET /metrics`
- Device API：`/Api/Device/*`

正式環境可使用 ASP.NET Core 環境變數覆寫設定，例如
`MetricsBasicAuth__Username` 與 `MetricsBasicAuth__Password`。

## 裝置設定

```json
"HeartbeatOptions": {
  "ActiveDeviceTimeoutSeconds": 15
},
"MeterOptions": [
  {
    "Name": "ESP32-PM5350",
    "Uri": "http://192.168.0.116/api/meter"
  }
],
"PassiveDevices": [
  {
    "Name": "ESP32-SWITCH",
    "Url": "http://192.168.0.115",
    "MonitoredGpioPins": [35]
  }
]
```

`MeterOptions[].Name` 不可為空且不分大小寫不得重複。`PassiveDevices` 的
`Name` 用於 API 查找裝置，`MonitoredGpioPins` 則決定每次抓取 `/metrics` 時要
讀取哪些 Switch GPIO。

## Device API

### Heartbeat

```http
GET /Api/Device/GetHeartbeat?name=ESP32-SWITCH
```

API 會轉送 `GET /api/heartbeat` 到指定裝置。

### 接收主動回報

```http
POST /Api/Device/SetReport
Content-Type: application/json

{
  "name": "device-name",
  "property": "value"
}
```

`name` 是必要欄位。主動回報會更新裝置最後上線時間；超過
`HeartbeatOptions.ActiveDeviceTimeoutSeconds` 未回報時，heartbeat metric 會變成
`0`。

### 讀取 GPIO

```http
GET /Api/Device/GetDigitalValue?name=ESP32-SWITCH&pin=35
Authorization: Bearer <token>
```

### 設定 GPIO

```http
POST /Api/Device/SetDigitalValue
Content-Type: application/json
Authorization: Bearer <token>

{
  "name": "ESP32-SWITCH",
  "pin": 25,
  "value": true
}
```

### GPIO Pulse

```http
POST /Api/Device/PulseDigitalValue
Content-Type: application/json
Authorization: Bearer <token>

{
  "name": "ESP32-SWITCH",
  "pin": 25,
  "value": true,
  "durationMilliseconds": 500
}
```

`pin` 必須介於 `0` 到 `39`；pulse 時間必須介於 `1` 到 `3600000` 毫秒。
裝置名稱不存在時回傳 `404`。其他裝置通訊結果包在 `message` 物件中，並由
`isValid`、`successMessages` 與 `errorMessages` 表示成功或失敗。

## GPIO Bearer Token

上述三個 GPIO endpoint 都需要 Bearer Token：

```json
"GpioBearerToken": {
  "Token": "<至少 32 字元的 token>"
}
```

- Token 未設定或短於 32 字元：`503 Service Unavailable`
- Token 缺少或不正確：`401 Unauthorized`
- 驗證結果使用固定時間比較，回應包含 `Cache-Control: no-store`

跨來源瀏覽器呼叫可透過 `GpioCors.AllowedOrigins` 指定允許來源。

## Prometheus

`GET /metrics` 使用 HTTP Basic Auth：

```text
MetricsBasicAuth__Username=<username>
MetricsBasicAuth__Password=<password>
```

未設定帳密時回傳 `503`，驗證失敗時回傳 `401`。每次抓取 metrics 都會同步
更新 heartbeat、GPIO 與 meter 資料。

主要 metrics：

| 類別 | Metrics |
| --- | --- |
| Heartbeat | `heartbeat{name}` |
| GPIO | `device_gpio_value`、`device_gpio_scrape_success`、`device_gpio_scrape_timestamp_seconds` |
| 電力 | `meter_voltage_ln_volts`、`meter_current_amperes`、`meter_current_average_amperes`、`meter_active_power_total_kilowatts`、`meter_power_factor_total`、`meter_frequency_hertz`、`meter_active_energy_delivered_watt_hours` |
| 環境 | `meter_temperature_celsius`、`meter_humidity_percent`、`meter_am2320_valid` |
| 狀態 | `meter_valid`、`meter_modbus_valid`、`meter_scrape_success`、`meter_scrape_timestamp_seconds`、`meter_uptime_seconds` |
| 診斷 | `meter_pm5350_power_factor_invalid_total`、`meter_am2320_read_failures_total`、`meter_am2320_retry_recoveries_total` |

抓取失敗時 success metric 會設為 `0`，最後一次成功取得的數值會保留；使用端應
搭配 success 與 timestamp 判斷資料是否仍有效。
