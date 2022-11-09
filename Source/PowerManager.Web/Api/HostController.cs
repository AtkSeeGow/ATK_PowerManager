using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using PowerManager.Domain;
using PowerManager.Domain.Options;
using PowerManager.Web.Utility;
using System;
using System.Threading.Tasks;

namespace PowerManager.Web.Api
{
    [Route("Api/[controller]/[action]")]
    [ApiController]
    [Authorize(AuthenticationSchemes = "PermissionHandler")]
    public class HostController : Controller
    {
        private readonly HttpUtility httpUtility;
        private readonly HostOptions hostOptions;

        public HostController(
            HttpUtility httpUtility,
            HostOptions hostOptions)
        {
            this.httpUtility = httpUtility;
            this.hostOptions = hostOptions;
        }

        [HttpGet]
        public async Task<ActionResult> Status()
        {
            var validResult = new ValidResult();
            var uri = new Uri(hostOptions.Url);
            await this.httpUtility.Get(validResult, uri);
            return Ok(new { validResult });
        }
    }
}
