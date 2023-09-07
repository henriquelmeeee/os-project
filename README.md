O entrypoint do Kernel (após o Prekernel) é src/Kernel/kernel.cpp::kmain()

Ele receberá uma struct do Bootloader, que contém algumas informações do hardware.

A memória do Kernel (kernel-land) está nos primeiros 1GB, e é identity-mapping na visão do Kernel.
Para os processos, a memória do Kernel é limitada, apenas 'executable' e 'readable', sem 'writable'.

O kmmap() trabalha sobre o objeto VMObject, que é uma representação de alto nível de uma página virtual.
'Region' é um objeto que contém uma lista de VMObject's.
