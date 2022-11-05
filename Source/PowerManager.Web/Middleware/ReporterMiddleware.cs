using Microsoft.AspNetCore.Http;
using PowerManager.Web.Reporter;
using System;
using System.Threading.Tasks;

namespace PowerManager.Web.Middleware
{
    public class ReporterMiddleware
    {
        private readonly RequestDelegate next;

        public ReporterMiddleware(RequestDelegate next)
        {
            this.next = next ?? throw new ArgumentNullException(nameof(next));
        }

        public async Task Invoke(HttpContext httpContext, HeartbeatMetric hartbeatMetric)
        {
            hartbeatMetric.RegisterRequest();
            await next.Invoke(httpContext);
        }
    }
}