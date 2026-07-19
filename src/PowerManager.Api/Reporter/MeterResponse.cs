using System;
using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace PowerManager.Api.Reporter
{
    public class MeterResponse
    {
        [JsonPropertyName("voltage_ln")]
        public double? VoltageLineToNeutral { get; set; }

        [JsonPropertyName("current_a")]
        public double? CurrentA { get; set; }

        [JsonPropertyName("current_b")]
        public double? CurrentB { get; set; }

        [JsonPropertyName("current_c")]
        public double? CurrentC { get; set; }

        [JsonPropertyName("current_average")]
        public double? CurrentAverage { get; set; }

        [JsonPropertyName("active_power_total_kw")]
        public double? ActivePowerTotalKilowatts { get; set; }

        [JsonPropertyName("power_factor_total")]
        public double? PowerFactorTotal { get; set; }

        [JsonPropertyName("power_factor_leading")]
        public bool? PowerFactorLeading { get; set; }

        [JsonPropertyName("frequency_hz")]
        public double? FrequencyHertz { get; set; }

        [JsonPropertyName("active_energy_delivered_wh")]
        public double? ActiveEnergyDeliveredWattHours { get; set; }

        [JsonPropertyName("active_energy_delivered_kwh")]
        public double? ActiveEnergyDeliveredKilowattHours { get; set; }

        [JsonPropertyName("temperature_c")]
        public double? TemperatureCelsius { get; set; }

        [JsonPropertyName("humidity_percent")]
        public double? HumidityPercent { get; set; }

        [JsonPropertyName("am2320_valid")]
        public bool Am2320Valid { get; set; }

        [JsonPropertyName("am2320_status")]
        public int Am2320Status { get; set; }

        [JsonPropertyName("am2320_wire_status")]
        public int Am2320WireStatus { get; set; }

        [JsonPropertyName("am2320_failure_counts")]
        public Dictionary<string, ulong> Am2320FailureCounts { get; set; }

        [JsonPropertyName("am2320_retry_recovery_count")]
        public ulong Am2320RetryRecoveryCount { get; set; }

        [JsonPropertyName("ntp_synchronized")]
        public bool NtpSynchronized { get; set; }

        [JsonPropertyName("current_time_utc")]
        public DateTimeOffset? CurrentTimeUtc { get; set; }

        [JsonPropertyName("boot_started_at_utc")]
        public DateTimeOffset? BootStartedAtUtc { get; set; }

        [JsonPropertyName("boot_started_at_epoch")]
        public long? BootStartedAtEpoch { get; set; }

        [JsonPropertyName("uptime_ms")]
        public long UptimeMilliseconds { get; set; }

        [JsonPropertyName("units")]
        public MeterUnits Units { get; set; }

        [JsonPropertyName("valid")]
        public bool Valid { get; set; }

        [JsonPropertyName("modbus_valid")]
        public bool ModbusValid { get; set; }

        [JsonPropertyName("updated_at_ms")]
        public long UpdatedAtMilliseconds { get; set; }

        [JsonPropertyName("modbus_status")]
        public int ModbusStatus { get; set; }

        [JsonPropertyName("pm5350_power_factor_invalid_count")]
        public ulong Pm5350PowerFactorInvalidCount { get; set; }
    }

    public class MeterUnits
    {
        [JsonPropertyName("voltage")]
        public string Voltage { get; set; }

        [JsonPropertyName("current")]
        public string Current { get; set; }

        [JsonPropertyName("active_power")]
        public string ActivePower { get; set; }

        [JsonPropertyName("frequency")]
        public string Frequency { get; set; }

        [JsonPropertyName("active_energy")]
        public string ActiveEnergy { get; set; }
    }
}
