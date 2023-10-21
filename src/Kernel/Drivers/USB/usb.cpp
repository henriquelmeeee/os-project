#define CONTROLLER_REGISTER_PORT 0x64
#if 0
struct USBDeviceDescriptor {
  unsigned int bLenght; // tamanho em bytes do descritor
};

namespace Drivers::USB {
  char read_controller_id() {
    unsigned char controller_id;
    __asm__ volatile(
        "inb %1, %0" // lê um byte de uma porta I/O
        : "=a" (controller_id)
        : "Nd" (CONTROLLER_REGISTER_PORT)
        )
  }

  struct USBDeviceDescriptor get_descriptors() {
    USBDeviceDescriptor new_descriptor{0};
    __asm__ volatile (
        "mov dx, %[port];"
        "in eax, dx"
        )
  }

  void handle_new_usb() {
    __asm__ volatile(
        "cli;"
        "pusha;"

        )
    unsigned int endpoint;

  }
}
#endif
