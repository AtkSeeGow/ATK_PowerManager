using Microsoft.AspNetCore.Mvc;
using PowerManager.Api.Middleware;
using PowerManager.Api.Utility;
using PowerManager.Domain;
using PowerManager.Domain.Devices;
using PowerManager.Repositories;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace PowerManager.Api.Controllers
{
    [Route("Api/[controller]")]
    [ApiController]
    public class DeviceController : ControllerBase
    {
        private readonly DeviceHttpClient _deviceHttpClient;
        private readonly IList<PassiveDevice> _passiveDevices;
        private readonly ActiveDeviceRepository _activeDeviceRepository;
        private readonly TimeProvider _timeProvider;

        public DeviceController(
            DeviceHttpClient deviceHttpClient,
            IList<PassiveDevice> passiveDevices,
            ActiveDeviceRepository activeDeviceRepository,
            TimeProvider timeProvider)
        {
            _deviceHttpClient = deviceHttpClient;
            _passiveDevices = passiveDevices;
            _activeDeviceRepository = activeDeviceRepository;
            _timeProvider = timeProvider;
        }

        [HttpGet("GetHeartbeat")]
        public async Task<ActionResult> GetHeartbeatAsync(string name)
        {
            var result = new DeviceOperationResult();
            var passiveDevice = PassiveDevice.FindByName(_passiveDevices, name);
            if (passiveDevice == null)
                return CreateDeviceNotFoundResult(name, result);

            await _deviceHttpClient.GetAsync(result, passiveDevice.HeartbeatUri);
            return Ok(new { message = result });
        }

        [HttpPost("SetReport")]
        public ActionResult ReceiveReport(IDictionary<string, string> properties)
        {
            var result = new DeviceOperationResult();

            try
            {
                var name = properties["name"];
                _activeDeviceRepository.Upsert(
                    name,
                    properties,
                    _timeProvider.GetUtcNow());
            }
            catch (Exception exception)
            {
                result.ErrorMessages.Add(
                    Guid.NewGuid().ToString(),
                    exception.Message);
            }

            return Ok(new { message = result });
        }

        [HttpGet("GetDigitalValue")]
        [GpioBearerToken]
        public async Task<ActionResult> GetDigitalValueAsync(
            [FromQuery] GetDigitalValueRequest request)
        {
            var result = new DeviceOperationResult();
            var passiveDevice = PassiveDevice.FindByName(
                _passiveDevices,
                request.Name);
            if (passiveDevice == null)
                return CreateDeviceNotFoundResult(request.Name, result);

            var uri = passiveDevice.GetDigitalValueUri(request.Pin.Value);
            await _deviceHttpClient.GetAsync(result, uri);
            return Ok(new { message = result });
        }

        [HttpPost("SetDigitalValue")]
        [GpioBearerToken]
        public async Task<ActionResult> SetDigitalValueAsync(
            [FromBody] SetDigitalValueRequest request)
        {
            var result = new DeviceOperationResult();
            var passiveDevice = PassiveDevice.FindByName(
                _passiveDevices,
                request.Name);
            if (passiveDevice == null)
                return CreateDeviceNotFoundResult(request.Name, result);

            var uri = passiveDevice.SetDigitalValueUri(
                request.Pin.Value,
                request.Value.Value);
            await _deviceHttpClient.PostAsync(result, uri);
            return Ok(new { message = result });
        }

        [HttpPost("PulseDigitalValue")]
        [GpioBearerToken]
        public async Task<ActionResult> PulseDigitalValueAsync(
            [FromBody] PulseDigitalValueRequest request)
        {
            var result = new DeviceOperationResult();
            var passiveDevice = PassiveDevice.FindByName(
                _passiveDevices,
                request.Name);
            if (passiveDevice == null)
                return CreateDeviceNotFoundResult(request.Name, result);

            var uri = passiveDevice.PulseDigitalValueUri(
                request.Pin.Value,
                request.Value.Value,
                request.DurationMilliseconds.Value);
            await _deviceHttpClient.PostAsync(result, uri);
            return Ok(new { message = result });
        }

        private ActionResult CreateDeviceNotFoundResult(
            string name,
            DeviceOperationResult result)
        {
            result.ErrorMessages.Add(
                Guid.NewGuid().ToString(),
                $"Device '{name}' was not found.");
            return NotFound(new { message = result });
        }
    }
}
