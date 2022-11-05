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
    public class SerialController : Controller
    {
        private readonly HttpUtility httpUtility;
        private readonly SerialOptions serialOptions;

        public SerialController(
            HttpUtility httpUtility,
            SerialOptions serialOptions)
        {
            this.httpUtility = httpUtility;
            this.serialOptions = serialOptions;
        }

        [HttpGet]
        public async Task<ActionResult> ReadDigitalValue()
        {
            var validResult = new ValidResult();
            var uri = new Uri(new Uri(serialOptions.Url), "/ReadDigitalValue/");
            await this.httpUtility.Get(validResult, uri);
            return Ok(new { validResult });
        }

        [HttpGet]
        public async Task<ActionResult> SwitchOpticalCoupler(int interval)
        {
            var validResult = new ValidResult();
            var uri = new Uri(new Uri(serialOptions.Url), $"/SwitchOpticalCoupler/{interval}");
            await this.httpUtility.Get(validResult, uri);
            return Ok(new { validResult });
        }
    }
}