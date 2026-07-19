using PowerManager.Domain;
using PowerManager.Domain.Devices;
using PowerManager.Domain.Options;
using PowerManager.Repositories;
using PowerManager.Api.Utility;
using Prometheus;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace PowerManager.Api.Reporter
{
    public class HeartbeatMetricsCollector
    {
        private readonly Gauge _heartbeat;
        private readonly DeviceHttpClient _deviceHttpClient;
        private readonly IList<PassiveDevice> _passiveDevices;
        private readonly ActiveDeviceRepository _activeDeviceRepository;
        private readonly TimeProvider _timeProvider;
        private readonly TimeSpan _activeDeviceTimeout;

        public HeartbeatMetricsCollector(
            DeviceHttpClient deviceHttpClient,
            IList<PassiveDevice> passiveDevices,
            ActiveDeviceRepository activeDeviceRepository,
            TimeProvider timeProvider,
            HeartbeatOptions heartbeatOptions)
        {
            ArgumentNullException.ThrowIfNull(deviceHttpClient);
            ArgumentNullException.ThrowIfNull(passiveDevices);
            ArgumentNullException.ThrowIfNull(activeDeviceRepository);
            ArgumentNullException.ThrowIfNull(timeProvider);
            ArgumentNullException.ThrowIfNull(heartbeatOptions);

            if (heartbeatOptions.ActiveDeviceTimeoutSeconds <= 0)
                throw new ArgumentOutOfRangeException(nameof(heartbeatOptions.ActiveDeviceTimeoutSeconds));

            this._deviceHttpClient = deviceHttpClient;
            this._passiveDevices = passiveDevices;
            this._activeDeviceRepository = activeDeviceRepository;
            this._timeProvider = timeProvider;
            this._activeDeviceTimeout = TimeSpan.FromSeconds(heartbeatOptions.ActiveDeviceTimeoutSeconds);
            this._heartbeat = Metrics.CreateGauge("heartbeat", "", "name");
        }

        public async Task CollectAsync()
        {
            var tasks = new List<Task>();
            var results = new List<DeviceOperationResult>();

            foreach (var passiveDevice in _passiveDevices)
            {
                var result = new DeviceOperationResult { Name = passiveDevice.Name };
                results.Add(result);
                tasks.Add(_deviceHttpClient.CheckHeartbeatAsync(result, passiveDevice.HeartbeatUri));
            }

            await Task.WhenAll(tasks);

            var activeDevices = this._activeDeviceRepository.GetSnapshot();
            var now = this._timeProvider.GetUtcNow();
            foreach (var activeDevice in activeDevices)
            {
                var result = new DeviceOperationResult { Name = activeDevice.Name };
                results.Add(result);

                if (!activeDevice.LastReportTime.HasValue ||
                    now - activeDevice.LastReportTime.Value > this._activeDeviceTimeout)
                {
                    result.ErrorMessages.Add(Guid.NewGuid().ToString(), string.Empty);
                }
            }

            foreach (var result in results)
                _heartbeat.WithLabels(result.Name).Set(result.IsValid ? 1 : 0);
        }
    }
}
