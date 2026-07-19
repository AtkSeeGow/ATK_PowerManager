using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Newtonsoft.Json.Serialization;
using PowerManager.Api.Middleware;
using PowerManager.Api.Reporter;
using PowerManager.Api.Utility;
using PowerManager.Domain.Devices;
using PowerManager.Domain.Options;
using PowerManager.Repositories;
using Prometheus;
using System;
using System.Collections.Generic;
using System.Linq;

namespace PowerManager.Api
{
    public class Startup
    {
        public Startup(IConfiguration configuration)
        {
            Configuration = configuration;
        }

        public IConfiguration Configuration { get; }

        public void ConfigureServices(IServiceCollection services)
        {
            var heartbeatOptions = Configuration.GetSection("HeartbeatOptions").Get<HeartbeatOptions>() ?? new HeartbeatOptions();
            var meterOptions = Configuration.GetSection("MeterOptions").Get<IList<MeterOptions>>() ?? new List<MeterOptions>();
            var metricsBasicAuthOptions = Configuration.GetSection("MetricsBasicAuth").Get<MetricsBasicAuthOptions>() ?? new MetricsBasicAuthOptions();
            var gpioBearerTokenOptions = Configuration.GetSection("GpioBearerToken").Get<GpioBearerTokenOptions>() ?? new GpioBearerTokenOptions();
            var gpioCorsAllowedOrigins = Configuration.GetSection("GpioCors:AllowedOrigins").Get<string[]>() ?? Array.Empty<string>();
            var passiveDevices = Configuration.GetSection("PassiveDevices").Get<IList<PassiveDevice>>() ?? new List<PassiveDevice>();

            services.AddSingleton<HeartbeatOptions>(provider => heartbeatOptions);
            services.AddSingleton<IList<MeterOptions>>(provider => meterOptions);
            services.AddSingleton<MetricsBasicAuthOptions>(provider => metricsBasicAuthOptions);
            services.AddSingleton<GpioBearerTokenOptions>(provider => gpioBearerTokenOptions);
            services.AddSingleton<IList<PassiveDevice>>(provider => passiveDevices);
            services.AddSingleton(TimeProvider.System);
            services.AddSingleton<ActiveDeviceRepository>();
            services.AddSingleton<HeartbeatMetricsCollector>();
            services.AddSingleton<GpioMetricsCollector>();
            services.AddSingleton<MeterMetricsCollector>();

            services.AddTransient<DeviceHttpClient>();

            services.AddCors(options =>
            {
                options.AddPolicy("GpioControl", policy =>
                {
                    if (gpioCorsAllowedOrigins.Contains("*"))
                    {
                        policy.AllowAnyOrigin();
                    }
                    else if (gpioCorsAllowedOrigins.Length > 0)
                    {
                        policy.WithOrigins(gpioCorsAllowedOrigins);
                    }

                    policy
                        .WithMethods("GET", "POST", "OPTIONS")
                        .WithHeaders(
                            "Authorization",
                            "Content-Type");
                });
            });
            
            services
                .AddControllers()
                .AddNewtonsoftJson(options => options.SerializerSettings.ContractResolver = new CamelCasePropertyNamesContractResolver())
                .AddJsonOptions(options => options.JsonSerializerOptions.PropertyNameCaseInsensitive = true);

            services.AddHttpClient();
            services.AddHttpClient("Heartbeat", client =>
            {
                client.Timeout = TimeSpan.FromSeconds(3);
            });
            services.AddHttpClient("Meter", client =>
            {
                client.Timeout = TimeSpan.FromSeconds(3);
            });
            services.AddHttpClient("Gpio", client =>
            {
                client.Timeout = TimeSpan.FromSeconds(3);
            });
        }

        public void Configure(IApplicationBuilder app)
        {
            app.Map("/metrics", builder =>
            {
                builder.UseMiddleware<MetricsBasicAuthMiddleware>();
                builder.UseMiddleware<MetricsRefreshMiddleware>();
                builder.UseMetricServer("");
            });

            app.UseRouting();
            app.UseCors("GpioControl");
            app.UseMiddleware<GpioBearerTokenMiddleware>();

            app.UseEndpoints(endpoints =>
            {
                endpoints.MapControllers();
            });
        }
    }
}
