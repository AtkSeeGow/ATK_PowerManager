using Microsoft.AspNetCore.Http;
using PowerManager.Api.Reporter;
using System;
using System.Threading.Tasks;

namespace PowerManager.Api.Middleware
{
    public class MetricsRefreshMiddleware
    {
        private readonly RequestDelegate _next;

        public MetricsRefreshMiddleware(RequestDelegate next)
        {
            this._next = next ?? throw new ArgumentNullException(nameof(next));
        }

        public async Task Invoke(
            HttpContext httpContext,
            HeartbeatMetricsCollector heartbeatMetricsCollector,
            GpioMetricsCollector gpioMetricsCollector,
            MeterMetricsCollector meterMetricsCollector)
        {
            await Task.WhenAll(
                heartbeatMetricsCollector.CollectAsync(),
                gpioMetricsCollector.CollectAsync(),
                meterMetricsCollector.CollectAsync());
            await _next.Invoke(httpContext);
        }
    }
}
