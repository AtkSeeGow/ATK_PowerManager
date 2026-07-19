using Microsoft.Extensions.Logging;
using PowerManager.Domain.Devices;
using Prometheus;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Json;
using System.Threading.Tasks;

namespace PowerManager.Api.Reporter
{
    public class GpioMetricsCollector
    {
        private readonly Gauge _value;
        private readonly Gauge _scrapeSuccess;
        private readonly Gauge _scrapeTimestamp;
        private readonly IHttpClientFactory _httpClientFactory;
        private readonly ILogger<GpioMetricsCollector> _logger;
        private readonly IReadOnlyCollection<GpioTarget> _targets;

        public GpioMetricsCollector(
            IHttpClientFactory httpClientFactory,
            ILogger<GpioMetricsCollector> logger,
            IList<PassiveDevice> passiveDevices)
        {
            ArgumentNullException.ThrowIfNull(httpClientFactory);
            ArgumentNullException.ThrowIfNull(logger);

            this._httpClientFactory = httpClientFactory;
            this._logger = logger;
            this._targets = ValidateTargets(passiveDevices);
            this._value = Metrics.CreateGauge(
                "device_gpio_value",
                "Last successfully read digital GPIO value (0=LOW, 1=HIGH).",
                "name",
                "pin");
            this._scrapeSuccess = Metrics.CreateGauge(
                "device_gpio_scrape_success",
                "Whether the last GPIO API request succeeded (1) or failed (0).",
                "name",
                "pin");
            this._scrapeTimestamp = Metrics.CreateGauge(
                "device_gpio_scrape_timestamp_seconds",
                "Unix timestamp of the last successful GPIO API request.",
                "name",
                "pin");
        }

        public Task CollectAsync()
        {
            return Task.WhenAll(this._targets.Select(CollectGpioAsync));
        }

        private async Task CollectGpioAsync(GpioTarget target)
        {
            var pinLabel = target.Pin.ToString(CultureInfo.InvariantCulture);

            try
            {
                var httpClient = this._httpClientFactory.CreateClient("Gpio");
                var response = await httpClient.GetFromJsonAsync<GpioResponse>(target.Uri);

                if (response == null)
                    throw new InvalidOperationException("The GPIO API returned an empty response.");
                if (response.Pin != target.Pin)
                    throw new InvalidOperationException(
                        $"The GPIO API returned pin {response.Pin} instead of {target.Pin}.");
                if (response.Value != 0 && response.Value != 1)
                    throw new InvalidOperationException(
                        $"The GPIO API returned invalid digital value {response.Value}.");

                this._value.WithLabels(target.DeviceName, pinLabel).Set(response.Value);
                this._scrapeSuccess.WithLabels(target.DeviceName, pinLabel).Set(1);
                this._scrapeTimestamp
                    .WithLabels(target.DeviceName, pinLabel)
                    .SetToCurrentTimeUtc();
            }
            catch (Exception exception)
            {
                this._scrapeSuccess.WithLabels(target.DeviceName, pinLabel).Set(0);
                this._logger.LogError(
                    exception,
                    "Failed to read GPIO {Pin} from device {DeviceName} at {GpioUri}.",
                    target.Pin,
                    target.DeviceName,
                    target.Uri);
            }
        }

        private static IReadOnlyCollection<GpioTarget> ValidateTargets(
            IList<PassiveDevice> passiveDevices)
        {
            ArgumentNullException.ThrowIfNull(passiveDevices);

            var targets = new List<GpioTarget>();
            foreach (var device in passiveDevices)
            {
                if (device == null)
                    throw new ArgumentException(
                        "Passive device options cannot contain a null item.",
                        nameof(passiveDevices));
                if (string.IsNullOrWhiteSpace(device.Name))
                    throw new ArgumentException(
                        "Every passive device must have a name.",
                        nameof(passiveDevices));
                if (!Uri.TryCreate(device.Url, UriKind.Absolute, out var baseUri) ||
                    (baseUri.Scheme != Uri.UriSchemeHttp &&
                     baseUri.Scheme != Uri.UriSchemeHttps))
                {
                    throw new ArgumentException(
                        $"Passive device '{device.Name}' must have an absolute HTTP or HTTPS URL.",
                        nameof(passiveDevices));
                }

                var pins = device.MonitoredGpioPins ?? new List<int>();
                foreach (var pin in pins)
                {
                    if (pin < 0 || pin > 39)
                        throw new ArgumentException(
                            $"Passive device '{device.Name}' has invalid GPIO pin {pin}.",
                            nameof(passiveDevices));

                    targets.Add(new GpioTarget(
                        device.Name,
                        pin,
                        device.GetDigitalValueUri(pin)));
                }

                var duplicatePin = pins
                    .GroupBy(pin => pin)
                    .FirstOrDefault(group => group.Count() > 1)
                    ?.Key;
                if (duplicatePin.HasValue)
                    throw new ArgumentException(
                        $"Passive device '{device.Name}' has duplicate GPIO pin {duplicatePin.Value}.",
                        nameof(passiveDevices));
            }

            var duplicateTarget = targets
                .GroupBy(
                    target => (target.DeviceName.ToUpperInvariant(), target.Pin))
                .FirstOrDefault(group => group.Count() > 1)
                ?.Key;
            if (duplicateTarget.HasValue)
                throw new ArgumentException(
                    $"GPIO target '{duplicateTarget.Value.Item1}' pin {duplicateTarget.Value.Pin} is duplicated.",
                    nameof(passiveDevices));

            return targets;
        }

        private sealed class GpioTarget
        {
            public GpioTarget(string deviceName, int pin, Uri uri)
            {
                DeviceName = deviceName;
                Pin = pin;
                Uri = uri;
            }

            public string DeviceName { get; }
            public int Pin { get; }
            public Uri Uri { get; }
        }

        private sealed class GpioResponse
        {
            public int Pin { get; set; }
            public int Value { get; set; }
        }
    }
}
