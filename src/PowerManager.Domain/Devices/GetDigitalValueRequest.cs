using System.ComponentModel.DataAnnotations;

namespace PowerManager.Domain.Devices
{
    public class GetDigitalValueRequest
    {
        [Required]
        public string Name { get; set; }

        [Range(0, 39)]
        [Required]
        public int? Pin { get; set; }
    }
}
