import struct

# Caminho da imagem
image_path = '../Build/disk.img'

# Valor inteiro e caractere que você deseja escrever
sectors_inodes_value = 1
shell = 'Shell'

# Calcule o deslocamento com base no setor e no tamanho do setor (por exemplo, 512 bytes)
sector = 201 # Setor em que você deseja escrever
sector_size = 512
offset = sector * sector_size

# Abra o arquivo de imagem
with open(image_path, 'r+b') as img_file:
    # Vá para o deslocamento desejado
    img_file.seek(offset)

    # Escreva o valor inteiro (formato little-endian, 4 bytes)
    img_file.write(struct.pack('<I', 1))

    # Escreva o valor do caractere (1 byte)
    img_file.write(shell.encode('ascii'))

    img_file.seek(200*sector_size)
    img_file.write(struct.pack('<I', sectors_inodes_value))
