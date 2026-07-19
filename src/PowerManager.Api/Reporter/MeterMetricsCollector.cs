using Microsoft.Extensions.Logging;
using PowerManager.Domain.Options;
using Prometheus;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Json;
using System.Threading.Tasks;

namespace PowerManager.Api.Reporter
{
    public class MeterMetricsCollector
    {
        private static readonly string[] Am2320FailureReasons =
        {
            "not_initialized",
            "command_failed",
            "response_too_short",
            "invalid_frame",
            "crc_mismatch"
        };

        private readonly Gauge _voltageLineToNeutral;
        private readonly Gauge _current;
        private readonly Gauge _currentAverage;
        private readonly Gauge _activePowerTotal;
        private readonly Gauge _powerFactorTotal;
        private readonly Gauge _powerFactorLeading;
        private readonly Gauge _frequency;
        private readonly Gauge _activeEnergyDelivered;
        private readonly Gauge _temperature;
        private readonly Gauge _humidity;
        private readonly Gauge _am2320Valid;
        private readonly Gauge _am2320Status;
        private readonly Gauge _am2320WireStatus;
        private readonly Gauge _ntpSynchronized;
        private readonly Gauge _deviceTime;
        private readonly Gauge _bootStartedAt;
        private readonly Gauge _uptime;
        private readonly Gauge _valid;
        private readonly Gauge _modbusValid;
        private readonly Gauge _lastUpdate;
        private readonly Gauge _modbusStatus;
        private readonly Gauge _scrapeSuccess;
        private readonly Gauge _scrapeTimestamp;
        private readonly Counter _pm5350PowerFactorInvalid;
        private readonly Counter _am2320ReadFailures;
        private readonly Counter _am2320RetryRecoveries;
        private readonly IHttpClientFactory _httpClientFactory;
        private readonly ILogger<MeterMetricsCollector> _logger;
        private readonly IReadOnlyCollection<MeterOptions> _meters;
        private readonly object _deviceCounterLock = new object();
        private readonly Dictionary<(string MeterName, string CounterName), ulong> _lastDeviceCounterValues =
            new Dictionary<(string MeterName, string CounterName), ulong>();

        public MeterMetricsCollector(
            IHttpClientFactory httpClientFactory,
            ILogger<MeterMetricsCollector> logger,
            IList<MeterOptions> options)
        {
            this._httpClientFactory = httpClientFactory;
            this._logger = logger;
            this._meters = ValidateOptions(options);

            this._voltageLineToNeutral = Metrics.CreateGauge("meter_voltage_ln_volts", "Line-to-neutral voltage in volts.", "name");
            this._current = Metrics.CreateGauge("meter_current_amperes", "Electrical current in amperes.", "name", "phase");
            this._currentAverage = Metrics.CreateGauge("meter_current_average_amperes", "Average electrical current in amperes.", "name");
            this._activePowerTotal = Metrics.CreateGauge("meter_active_power_total_kilowatts", "Total active power in kilowatts.", "name");
            this._powerFactorTotal = Metrics.CreateGauge("meter_power_factor_total", "Total power factor as a ratio.", "name");
            this._powerFactorLeading = Metrics.CreateGauge("meter_power_factor_leading", "Whether the total power factor is leading (1) or not (0).", "name");
            this._frequency = Metrics.CreateGauge("meter_frequency_hertz", "Electrical frequency in hertz.", "name");
            this._activeEnergyDelivered = Metrics.CreateGauge("meter_active_energy_delivered_watt_hours", "Cumulative active energy delivered in watt-hours.", "name");
            this._temperature = Metrics.CreateGauge("meter_temperature_celsius", "AM2320 temperature in degrees Celsius.", "name");
            this._humidity = Metrics.CreateGauge("meter_humidity_percent", "AM2320 relative humidity percentage.", "name");
            this._am2320Valid = Metrics.CreateGauge("meter_am2320_valid", "Whether the AM2320 reading is valid (1) or not (0).", "name");
            this._am2320Status = Metrics.CreateGauge("meter_am2320_status", "AM2320 status code.", "name");
            this._am2320WireStatus = Metrics.CreateGauge("meter_am2320_wire_status", "AM2320 wire status code.", "name");
            this._ntpSynchronized = Metrics.CreateGauge("meter_ntp_synchronized", "Whether device time is synchronized by NTP (1) or not (0).", "name");
            this._deviceTime = Metrics.CreateGauge("meter_device_time_seconds", "Device time as Unix epoch seconds.", "name");
            this._bootStartedAt = Metrics.CreateGauge("meter_boot_started_at_seconds", "Device boot time as Unix epoch seconds.", "name");
            this._uptime = Metrics.CreateGauge("meter_uptime_seconds", "Device uptime in seconds.", "name");
            this._valid = Metrics.CreateGauge("meter_valid", "Whether the complete meter reading is valid (1) or not (0).", "name");
            this._modbusValid = Metrics.CreateGauge("meter_modbus_valid", "Whether the Modbus reading is valid (1) or not (0).", "name");
            this._lastUpdate = Metrics.CreateGauge("meter_last_update_seconds", "Device reading update time relative to boot, in seconds.", "name");
            this._modbusStatus = Metrics.CreateGauge("meter_modbus_status", "Modbus status code.", "name");
            this._scrapeSuccess = Metrics.CreateGauge("meter_scrape_success", "Whether the last meter API request succeeded (1) or failed (0).", "name");
            this._scrapeTimestamp = Metrics.CreateGauge("meter_scrape_timestamp_seconds", "Unix timestamp of the last successful meter API request.", "name");
            this._pm5350PowerFactorInvalid = Metrics.CreateCounter("meter_pm5350_power_factor_invalid_total", "Number of non-finite PM5350 power-factor readings reported by the device.", "name");
            this._am2320ReadFailures = Metrics.CreateCounter("meter_am2320_read_failures_total", "Number of failed AM2320 read attempts reported by the device.", "name", "reason");
            this._am2320RetryRecoveries = Metrics.CreateCounter("meter_am2320_retry_recoveries_total", "Number of AM2320 read cycles recovered by retry.", "name");
        }

        public Task CollectAsync()
        {
            return Task.WhenAll(_meters.Select(CollectMeterAsync));
        }

        private async Task CollectMeterAsync(MeterOptions meter)
        {
            try
            {
                var httpClient = this._httpClientFactory.CreateClient("Meter");
                var response = await httpClient.GetFromJsonAsync<MeterResponse>(meter.Uri);

                if (response == null)
                    throw new InvalidOperationException("The meter API returned an empty response.");

                UpdateMetrics(meter.Name, response);
                this._scrapeSuccess.WithLabels(meter.Name).Set(1);
                this._scrapeTimestamp.WithLabels(meter.Name).SetToCurrentTimeUtc();
            }
            catch (Exception exception)
            {
                this._scrapeSuccess.WithLabels(meter.Name).Set(0);
                this._logger.LogError(
                    exception,
                    "Failed to read meter {MeterName} API at {MeterUri}.",
                    meter.Name,
                    meter.Uri);
            }
        }

        private void UpdateMetrics(string name, MeterResponse response)
        {
            SetNullable(this._voltageLineToNeutral.WithLabels(name), response.VoltageLineToNeutral);
            SetNullable(this._current.WithLabels(name, "a"), response.CurrentA);
            SetNullable(this._current.WithLabels(name, "b"), response.CurrentB);
            SetNullable(this._current.WithLabels(name, "c"), response.CurrentC);
            SetNullable(this._currentAverage.WithLabels(name), response.CurrentAverage);
            SetNullable(this._activePowerTotal.WithLabels(name), response.ActivePowerTotalKilowatts);
            SetNullable(this._powerFactorTotal.WithLabels(name), response.PowerFactorTotal);
            SetNullable(this._powerFactorLeading.WithLabels(name), response.PowerFactorLeading);
            SetNullable(this._frequency.WithLabels(name), response.FrequencyHertz);
            SetNullable(this._activeEnergyDelivered.WithLabels(name), response.ActiveEnergyDeliveredWattHours);
            SetNullable(this._temperature.WithLabels(name), response.TemperatureCelsius);
            SetNullable(this._humidity.WithLabels(name), response.HumidityPercent);
            this._am2320Valid.WithLabels(name).Set(ToDouble(response.Am2320Valid));
            this._am2320Status.WithLabels(name).Set(response.Am2320Status);
            this._am2320WireStatus.WithLabels(name).Set(response.Am2320WireStatus);
            this._ntpSynchronized.WithLabels(name).Set(ToDouble(response.NtpSynchronized));
            SetNullable(this._deviceTime.WithLabels(name), response.CurrentTimeUtc?.ToUnixTimeSeconds());
            SetNullable(this._bootStartedAt.WithLabels(name), response.BootStartedAtEpoch);
            this._uptime.WithLabels(name).Set(response.UptimeMilliseconds / 1000d);
            this._valid.WithLabels(name).Set(ToDouble(response.Valid));
            this._modbusValid.WithLabels(name).Set(ToDouble(response.ModbusValid));
            this._lastUpdate.WithLabels(name).Set(response.UpdatedAtMilliseconds / 1000d);
            this._modbusStatus.WithLabels(name).Set(response.ModbusStatus);
            UpdateDeviceCounter(
                this._pm5350PowerFactorInvalid.WithLabels(name),
                name,
                "pm5350_power_factor_invalid",
                response.Pm5350PowerFactorInvalidCount);
            foreach (var reason in Am2320FailureReasons)
            {
                var deviceValue = 0UL;
                response.Am2320FailureCounts?.TryGetValue(reason, out deviceValue);
                UpdateDeviceCounter(
                    this._am2320ReadFailures.WithLabels(name, reason),
                    name,
                    $"am2320_{reason}",
                    deviceValue);
            }
            UpdateDeviceCounter(
                this._am2320RetryRecoveries.WithLabels(name),
                name,
                "am2320_retry_recovery",
                response.Am2320RetryRecoveryCount);
        }

        private void UpdateDeviceCounter(
            Counter.Child counter,
            string meterName,
            string counterName,
            ulong deviceValue)
        {
            lock (this._deviceCounterLock)
            {
                var key = (meterName, counterName);
                ulong delta;
                if (!this._lastDeviceCounterValues.TryGetValue(key, out var previousValue))
                    delta = deviceValue;
                else
                    delta = deviceValue >= previousValue ? deviceValue - previousValue : deviceValue;

                if (delta > 0)
                    counter.Inc(delta);

                this._lastDeviceCounterValues[key] = deviceValue;
            }
        }

        private static IReadOnlyCollection<MeterOptions> ValidateOptions(IList<MeterOptions> options)
        {
            if (options == null)
                throw new ArgumentNullException(nameof(options));

            var meters = options.ToArray();
            foreach (var meter in meters)
            {
                if (meter == null)
                    throw new ArgumentException("Meter options cannot contain a null item.", nameof(options));
                if (string.IsNullOrWhiteSpace(meter.Name))
                    throw new ArgumentException("Every meter must have a name.", nameof(options));
                if (meter.Uri == null)
                    throw new ArgumentException($"Meter '{meter.Name}' must have a URI.", nameof(options));
            }

            var duplicateName = meters
                .GroupBy(meter => meter.Name, StringComparer.OrdinalIgnoreCase)
                .FirstOrDefault(group => group.Count() > 1)
                ?.Key;
            if (duplicateName != null)
                throw new ArgumentException($"Meter name '{duplicateName}' is duplicated.", nameof(options));

            return meters;
        }

        private static double ToDouble(bool value) => value ? 1d : 0d;

        private static void SetNullable(Gauge.Child gauge, double? value) =>
            gauge.Set(value ?? double.NaN);

        private static void SetNullable(Gauge.Child gauge, bool? value) =>
            gauge.Set(value.HasValue ? ToDouble(value.Value) : double.NaN);
    }
}
