using Microsoft.AspNetCore.Authorization;

namespace PowerManager.Web
{
    public class PermissionRequirement : IAuthorizationRequirement
    {
        public PermissionRequirement()
        {
        }
    }
}
