import struct, os


# Setor 200:
    # usado para definir a quantidade de inodes
# Setor 201 - ?:
    # usado pelos inodes, que inclui o setor do superbloco daquele inode
# Setor 500 - ?:
    # aqui é onde cada superbloco de cada inode fica
    # ele armazenará uma array de blocos (cada bloco tem 4KB)
# Setor 1000 - até o fim:
    # terá os blocos de cada arquivo.

# Cada inode tem os primeiros 4 bytes como um unsigned int
# que aponta pro superbloco
# seguido de uma array de 32 caracteres para o nome do arquivo

# Caminho da imagem
image_path = '../Build/disk.img'

# Valor inteiro e caractere que você deseja escrever
sectors_inodes_value = 1
shell = 'Shell'

# Calcule o deslocamento com base no setor e no tamanho do setor (por exemplo, 512 bytes)
sector = 201  # Setor do primeiro inode
sector_size = 512
offset = sector * sector_size

blocos_arquivos_setor = 500
ultimo_bloco_usado = 1000

def add_userland_files():
    global total_inodes, blocos_arquivos_setor, ultimo_bloco_usado
    base_path = './Base'
    for entry_name in os.listdir(base_path):
        entry_path = os.path.join(base_path, entry_name)
        if os.path.isfile(entry_path):
            print(f"Adicionando arquivo {entry_path} ao disco final")
            img_file.seek(sector * sector_size)
            blocos_arquivos_setor += 1
            img_file.write(struct.pack('<I', blocos_arquivos_setor))  # bloco relacionado a este arquivo
            img_file.write(entry_path.encode('ascii'))
            total_inodes += 1
            img_file.seek(blocos_arquivos_setor * sector_size)
            # adiciona no setor do inode q tem a array de blocos usados pelo arquivo, na ordem
            with open(entry_path, "r+b") as arquivo:
                while bloco := arquivo.read(4096):
                    img_file.write(struct.pack('<I', ultimo_bloco_usado))
                    img_file.seek(ultimo_bloco_usado * 4096)
                    img_file.write(bloco)
                    ultimo_bloco_usado += 1
                    
    img_file.seek(200 * sector_size)
    img_file.write(struct.pack('<I', total_inodes))


total_inodes = 0
# Abra o arquivo de imagem
with open(image_path, 'r+b') as img_file:
    print("Arquivo de imagem aberto")
    add_userland_files()
