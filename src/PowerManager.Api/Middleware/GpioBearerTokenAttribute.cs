using System;

namespace PowerManager.Api.Middleware
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = true)]
    public sealed class GpioBearerTokenAttribute : Attribute
    {
    }
}
