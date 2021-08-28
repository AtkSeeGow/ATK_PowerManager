using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Newtonsoft.Json.Serialization;
using PowerManager.Domain.Options;
using PowerManager.Service;
using System;
using System.Threading.Tasks;
using AuthorizationOptions = PowerManager.Domain.Options.AuthorizationOptions;

namespace PowerManager.Web
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
            var authorizationOptions = Configuration.GetSection("AuthorizationOptions").Get<AuthorizationOptions>();
            var tokenOptions = Configuration.GetSection("TokenOptions").Get<TokenOptions>();
            var serialOptions = Configuration.GetSection("SerialOptions").Get<SerialOptions>();
            var hostOptions = Configuration.GetSection("HostOptions").Get<HostOptions>();

            services.AddSingleton<AuthorizationOptions>(provider => authorizationOptions);
            services.AddSingleton<TokenOptions>(provider => tokenOptions);
            services.AddSingleton<SerialOptions>(provider => serialOptions);
            services.AddSingleton<HostOptions>(provider => hostOptions);

            services.AddTransient<HttpUtility>();
            
            #region Service

            services.AddSingleton<AuthorizationService>();

            #endregion

            services.AddControllersWithViews();

            services
                .AddMvc()
                .AddNewtonsoftJson(options => options.SerializerSettings.ContractResolver = new CamelCasePropertyNamesContractResolver())
                .AddJsonOptions(options => options.JsonSerializerOptions.PropertyNameCaseInsensitive = true)
                .SetCompatibilityVersion(CompatibilityVersion.Version_3_0);

            services.AddAuthorization(options =>
            {
                options.AddPolicy("permissionRequirement", policy => policy.Requirements.Add(new PermissionRequirement()));
            }).AddAuthorizationPolicyEvaluator();
            services.AddScoped<IAuthorizationHandler, PermissionRequirementHandler>();

            services.AddAuthentication(options => options.AddScheme("PermissionHandler", o => o.HandlerType = typeof(PermissionHandler)));

            services.AddHttpClient();
        }

        public void Configure(IApplicationBuilder app, IWebHostEnvironment env)
        {
            app.Use(async (HttpContext context, Func<Task> next) =>
            {
                await next.Invoke();

                if (context.Response.StatusCode == 404)
                {
                    context.Request.Path = new PathString("/");
                    await next.Invoke();
                }
            });

            app.UseStaticFiles();

            app.UseRouting();

            app.UseAuthentication();
            app.UseAuthorization();

            app.UseEndpoints(endpoints =>
            {
                endpoints.MapControllerRoute(
                   name: "default",
                   pattern: "{controller=Home}/{action=Index}/{id?}");
            });
        }
    }
}
