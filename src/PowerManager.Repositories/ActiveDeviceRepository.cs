using PowerManager.Domain;
using PowerManager.Domain.Devices;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;

namespace PowerManager.Repositories
{
    public class ActiveDeviceRepository
    {
        private readonly ConcurrentDictionary<string, ActiveDevice> _devices = new();

        public void Upsert(
            string name,
            IDictionary<string, string> properties,
            DateTimeOffset lastReportTime)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(name);
            ArgumentNullException.ThrowIfNull(properties);

            var device = new ActiveDevice
            {
                Name = name,
                DeviceType = DeviceType.None,
                Properties = new Dictionary<string, string>(properties),
                LastReportTime = lastReportTime
            };

            _devices.AddOrUpdate(name, device, (_, _) => device);
        }

        public IReadOnlyCollection<ActiveDevice> GetSnapshot()
        {
            return _devices.Values.ToArray();
        }
    }
}
