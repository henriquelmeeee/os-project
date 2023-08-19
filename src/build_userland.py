import struct, os

# Caminho da imagem
image_path = '../Build/disk.img'

# Valor inteiro e caractere que você deseja escrever
sectors_inodes_value = 1
shell = 'Shell'

# Calcule o deslocamento com base no setor e no tamanho do setor (por exemplo, 512 bytes)
sector = 201  # Setor do primeiro inode
sector_size = 512
offset = sector * sector_size


def add_userland_files():
    global total_inodes
    base_path = './Base'
    for entry_name in os.listdir(base_path):
        entry_path = os.path.join(base_path, entry_name)
        if os.path.isfile(entry_path):
            print(f"Adicionando arquivo {entry_path} ao disco final")
            img_file.seek(sector * sector_size)
            img_file.write(struct.pack('<I', 1))  # bloco relacionado a este arquivo
            img_file.write(entry_path.encode('ascii'))
            total_inodes += 1
    img_file.seek(200 * sector_size)
    img_file.write(struct.pack('<I', total_inodes))


total_inodes = 0
# Abra o arquivo de imagem
with open(image_path, 'r+b') as img_file:
    print("Arquivo de imagem aberto")
    add_userland_files()
