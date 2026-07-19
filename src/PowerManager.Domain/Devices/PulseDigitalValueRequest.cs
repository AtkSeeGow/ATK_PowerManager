using System.ComponentModel.DataAnnotations;

namespace PowerManager.Domain.Devices
{
    public class PulseDigitalValueRequest
    {
        [Required]
        public string Name { get; set; }

        [Range(0, 39)]
        [Required]
        public int? Pin { get; set; }

        [Required]
        public bool? Value { get; set; }

        [Range(1, 3600000)]
        [Required]
        public int? DurationMilliseconds { get; set; }
    }
}
