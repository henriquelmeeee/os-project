#define CLI __asm__("cli")
#define STI __asm__("sti")

extern "C" void *memcpy(void* dest, const void *src, int size) {
  return (void*)0;
  char* cdest;
  char* csrc;
  unsigned int i;

  cdest = (char *) dest;
  csrc = (char *) src;
  for(i = 0; i<size; i++)
    cdest[i] = csrc[i];
  return dest;
}

void change_page_table(void* pml4_address) {
  __asm__ volatile(
      "movq %0, %%cr3;"
      :
      : "r"(pml4_address)
      : "memory"
    );
}

namespace Utils {
  // TODO usar template pode ser um pecado para o desempenho porque o CPP cria uma cópia da função pra cada tipo
  // que usa ela, o melhor é copiar byte-por-byte.
  /*template <typename T>
  bool kmemcpy(T* dest, T* orig, unsigned long size) {
    return true;
  }*/
  template <typename T>
  void append_to_array(T* array, T value, unsigned long size) {
    // size == numero de elementos na array
    if(size == 0) {
      for(int i = 0; array[i] != 0; i++) {
        size++;
      }
    }

    array[size+=1] = value;
  }
}

  bool itos(long value, char* to_ret) {
    bool is_negative = false;
    if(value < 0) {
      is_negative = true;
      value = -value;
    }

    int i = (!is_negative) ? 0 : 1;
    while(value != 0) {
      int digit = value % 10;
      to_ret[i] = '0' + digit;
      value /= 10;
      ++i;
    }
    if(is_negative)
      to_ret[0] = '-';

    to_ret[i] = '\0';

    int start = 0;
    int end = i - 1;
    while(start < end) {
      char temp = to_ret[start];
      to_ret[start] = to_ret[end];
      to_ret[end] = temp;
      ++start;
      --end;
    }

    if(to_ret[0] == 0)
      to_ret[0] = '0';
    return true;
  }

  bool itoh(long value, char* to_ret) {
    char* ptr = to_ret;
    char* start = to_ret;
    int x = 0;
    do {
      int digit = value % 16;
      *ptr++ = digit < 10 ? '0' + digit : 'A' + digit - 10;
      value /= 16;
      x++;
    } while (value != 0);
  *(ptr--) = '\0';

  // Inverte a string
  while (start < ptr) {
    char tmp = *start;
    *start++ = *ptr;
    *ptr-- = tmp;
  }

  return true; 
  }



